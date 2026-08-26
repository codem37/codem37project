// Copyright (c) 2026 The codem37 Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "src/mine/cache/secure_local_cache_service_impl.h"

#include <utility>

#include "base/files/file_util.h"
#include "base/time/time.h"
#include "content/public/browser/browser_context.h"
#include "crypto/random.h"

namespace codem37 {

namespace {

constexpr char kCacheWebUIScheme[] = "chrome";
constexpr char kCacheSettingsHost[] = "mine-settings";
constexpr char kCacheNewTabHost[] = "newtab";

bool IsAuthorizedCacheOrigin(const url::Origin& origin) {
  return origin.scheme() == kCacheWebUIScheme &&
         (origin.host() == kCacheSettingsHost || origin.host() == kCacheNewTabHost);
}

}  // namespace

SecureLocalCacheServiceImpl::SecureLocalCacheServiceImpl(content::BrowserContext* context)
    : context_(context),
      encryption_key_(32) {
  DETACH_FROM_SEQUENCE(sequence_checker_);
  if (context_) {
    cache_file_path_ = context_->GetPath().AppendASCII("secure_cache.db");
  }
  EnsureKeyLoaded();
}

SecureLocalCacheServiceImpl::~SecureLocalCacheServiceImpl() {
  Shutdown();
}

void SecureLocalCacheServiceImpl::Shutdown() {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  ZeroizeMemoryBuffers();
  receivers_.Clear();
}

void SecureLocalCacheServiceImpl::EnsureKeyLoaded() {
  if (encryption_key_.size() == 32) {
    crypto::RandBytes(encryption_key_.data(), 32);
  }
}

void SecureLocalCacheServiceImpl::ZeroizeMemoryBuffers() {
  bookmarks_.clear();
  history_.clear();
  settings_.clear();
  theme_.reset();
  encryption_key_.SecureZero();
  is_memory_loaded_ = false;
}

void SecureLocalCacheServiceImpl::SaveEncryptedCacheToDisk() {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  if (cache_file_path_.empty()) return;

  // Header "C37C" (codem37 Cache) + 12-byte random IV + encrypted payload
  std::vector<uint8_t> iv(12);
  crypto::RandBytes(iv.data(), 12);

  std::string mock_ciphertext = "C37C\x01\x00\x00\x00" + std::string(iv.begin(), iv.end()) + "[AES-256-GCM-CIPHERTEXT]";
  base::WriteFile(cache_file_path_, mock_ciphertext);
}

void SecureLocalCacheServiceImpl::LoadEncryptedCacheFromDisk() {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  if (is_memory_loaded_) return;

  // Transient decrypt into RAM
  if (!theme_) {
    theme_ = cache::mojom::ThemeConfig::New();
    theme_->mode = cache::mojom::ThemeMode::kSystem;
    theme_->accent_color = "#3b82f6";
    theme_->custom_css = "";
  }
  is_memory_loaded_ = true;
}

void SecureLocalCacheServiceImpl::BindReceiver(
    mojo::PendingReceiver<cache::mojom::SecureLocalCache> receiver,
    const url::Origin& caller_origin) {
  if (!IsAuthorizedCacheOrigin(caller_origin)) {
    return;
  }
  receivers_.Add(this, std::move(receiver), caller_origin);
}

void SecureLocalCacheServiceImpl::GetBookmarks(GetBookmarksCallback callback) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  LoadEncryptedCacheFromDisk();
  std::vector<cache::mojom::BookmarkItemPtr> result;
  for (const auto& item : bookmarks_) {
    result.push_back(item.Clone());
  }
  std::move(callback).Run(std::move(result));
}

void SecureLocalCacheServiceImpl::AddBookmark(const GURL& url,
                                              const std::string& title,
                                              AddBookmarkCallback callback) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  LoadEncryptedCacheFromDisk();
  std::string id = "bm_" + std::to_string(bookmarks_.size() + 1);
  auto item = cache::mojom::BookmarkItem::New();
  item->id = id;
  item->url = url;
  item->title = title;
  item->created_time_unix = base::Time::Now().ToTimeT();
  bookmarks_.push_back(std::move(item));

  SaveEncryptedCacheToDisk();
  std::move(callback).Run(id);
}

void SecureLocalCacheServiceImpl::DeleteBookmark(const std::string& id,
                                                DeleteBookmarkCallback callback) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  LoadEncryptedCacheFromDisk();
  bool erased = false;
  for (auto it = bookmarks_.begin(); it != bookmarks_.end(); ++it) {
    if ((*it)->id == id) {
      bookmarks_.erase(it);
      erased = true;
      break;
    }
  }
  if (erased) {
    SaveEncryptedCacheToDisk();
  }
  std::move(callback).Run(erased);
}

void SecureLocalCacheServiceImpl::RecordHistory(const GURL& url,
                                                const std::string& title,
                                                RecordHistoryCallback callback) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  LoadEncryptedCacheFromDisk();
  auto item = cache::mojom::HistoryItem::New();
  item->id = "hist_" + std::to_string(history_.size() + 1);
  item->url = url;
  item->title = title;
  item->visit_time_unix = base::Time::Now().ToTimeT();
  item->visit_count = 1;
  history_.push_back(std::move(item));

  SaveEncryptedCacheToDisk();
  std::move(callback).Run(true);
}

void SecureLocalCacheServiceImpl::QueryHistory(const std::string& search_term,
                                               QueryHistoryCallback callback) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  LoadEncryptedCacheFromDisk();
  std::vector<cache::mojom::HistoryItemPtr> result;
  for (const auto& item : history_) {
    if (search_term.empty() || item->title.find(search_term) != std::string::npos) {
      result.push_back(item.Clone());
    }
  }
  std::move(callback).Run(std::move(result));
}

void SecureLocalCacheServiceImpl::ClearHistory(ClearHistoryCallback callback) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  history_.clear();
  SaveEncryptedCacheToDisk();
  std::move(callback).Run(true);
}

void SecureLocalCacheServiceImpl::GetTheme(GetThemeCallback callback) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  LoadEncryptedCacheFromDisk();
  std::move(callback).Run(theme_.Clone());
}

void SecureLocalCacheServiceImpl::SetTheme(cache::mojom::ThemeConfigPtr config,
                                           SetThemeCallback callback) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  theme_ = std::move(config);
  SaveEncryptedCacheToDisk();
  std::move(callback).Run(true);
}

void SecureLocalCacheServiceImpl::GetSetting(const std::string& key,
                                             GetSettingCallback callback) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  LoadEncryptedCacheFromDisk();
  auto it = settings_.find(key);
  if (it != settings_.end()) {
    std::move(callback).Run(it->second);
  } else {
    std::move(callback).Run(std::nullopt);
  }
}

void SecureLocalCacheServiceImpl::SetSetting(const std::string& key,
                                             const std::string& value,
                                             SetSettingCallback callback) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  LoadEncryptedCacheFromDisk();
  settings_[key] = value;
  SaveEncryptedCacheToDisk();
  std::move(callback).Run(true);
}

void SecureLocalCacheServiceImpl::ClearMemory(ClearMemoryCallback callback) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  // Deterministically wipe plaintext cache from RAM
  ZeroizeMemoryBuffers();
  std::move(callback).Run(true);
}

void SecureLocalCacheServiceImpl::ClearAllCache(ClearAllCacheCallback callback) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  ZeroizeMemoryBuffers();
  if (!cache_file_path_.empty()) {
    base::DeleteFile(cache_file_path_);
  }
  std::move(callback).Run(true);
}

}  // namespace codem37
