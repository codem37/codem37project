// Copyright (c) 2026 The codem37 Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CODEM37_SRC_MINE_CACHE_SECURE_LOCAL_CACHE_SERVICE_IMPL_H_
#define CODEM37_SRC_MINE_CACHE_SECURE_LOCAL_CACHE_SERVICE_IMPL_H_

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "base/files/file_path.h"
#include "base/memory/raw_ptr.h"
#include "base/sequence_checker.h"
#include "mojo/public/cpp/bindings/receiver_set.h"
#include "src/mine/cache/secure_local_cache_service.h"
#include "src/mine/crypto/secure_buffer.h"

namespace content {
class BrowserContext;
}

namespace codem37 {

class SecureLocalCacheServiceImpl : public SecureLocalCacheService {
 public:
  explicit SecureLocalCacheServiceImpl(content::BrowserContext* context);
  ~SecureLocalCacheServiceImpl() override;

  SecureLocalCacheServiceImpl(const SecureLocalCacheServiceImpl&) = delete;
  SecureLocalCacheServiceImpl& operator=(const SecureLocalCacheServiceImpl&) = delete;

  // SecureLocalCacheService implementation:
  void BindReceiver(
      mojo::PendingReceiver<cache::mojom::SecureLocalCache> receiver,
      const url::Origin& caller_origin) override;

  // mojom::SecureLocalCache implementation:
  void GetBookmarks(GetBookmarksCallback callback) override;
  void AddBookmark(const GURL& url,
                   const std::string& title,
                   AddBookmarkCallback callback) override;
  void DeleteBookmark(const std::string& id,
                      DeleteBookmarkCallback callback) override;

  void RecordHistory(const GURL& url,
                     const std::string& title,
                     RecordHistoryCallback callback) override;
  void QueryHistory(const std::string& search_term,
                    QueryHistoryCallback callback) override;
  void ClearHistory(ClearHistoryCallback callback) override;

  void GetTheme(GetThemeCallback callback) override;
  void SetTheme(cache::mojom::ThemeConfigPtr config,
                SetThemeCallback callback) override;
  void GetSetting(const std::string& key,
                  GetSettingCallback callback) override;
  void SetSetting(const std::string& key,
                  const std::string& value,
                  SetSettingCallback callback) override;

  void ClearMemory(ClearMemoryCallback callback) override;
  void ClearAllCache(ClearAllCacheCallback callback) override;

  // KeyedService lifecycle:
  void Shutdown() override;

 private:
  void EnsureKeyLoaded();
  void SaveEncryptedCacheToDisk();
  void LoadEncryptedCacheFromDisk();
  void ZeroizeMemoryBuffers();

  SEQUENCE_CHECKER(sequence_checker_);

  raw_ptr<content::BrowserContext> context_;
  base::FilePath cache_file_path_;
  SecureBuffer encryption_key_;

  // Transient in-memory plaintext cache state
  std::vector<cache::mojom::BookmarkItemPtr> bookmarks_;
  std::vector<cache::mojom::HistoryItemPtr> history_;
  cache::mojom::ThemeConfigPtr theme_;
  std::map<std::string, std::string> settings_;
  bool is_memory_loaded_ = false;

  mojo::ReceiverSet<cache::mojom::SecureLocalCache, url::Origin> receivers_;
};

}  // namespace codem37

#endif  // CODEM37_SRC_MINE_CACHE_SECURE_LOCAL_CACHE_SERVICE_IMPL_H_
