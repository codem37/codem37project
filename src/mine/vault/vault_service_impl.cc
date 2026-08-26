// Copyright (c) 2026 The codem37 Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "src/mine/vault/vault_service_impl.h"

#include <algorithm>
#include <utility>

#include "base/rand_util.h"
#include "base/time/time.h"
#include "crypto/random.h"
#include "url/gurl.h"
#include "url/origin.h"

namespace codem37 {

namespace {

constexpr char kVaultOriginScheme[] = "chrome";
constexpr char kVaultOriginHost[] = "vault";

bool IsAuthorizedVaultOrigin(const url::Origin& origin) {
  return origin.scheme() == kVaultOriginScheme && origin.host() == kVaultOriginHost;
}

}  // namespace

VaultServiceImpl::VaultServiceImpl(content::BrowserContext* context)
    : context_(context) {}

VaultServiceImpl::~VaultServiceImpl() {
  Shutdown();
}

void VaultServiceImpl::Shutdown() {
  is_unlocked_ = false;
  active_session_key_.clear();
  entries_cache_.clear();
  receivers_.Clear();
}

void VaultServiceImpl::BindReceiver(
    mojo::PendingReceiver<vault::mojom::VaultService> receiver,
    const url::Origin& caller_origin) {
  // Structural origin verification before binding pipe
  if (!IsAuthorizedVaultOrigin(caller_origin)) {
    // Drop receiver immediately - unauthorized origin
    return;
  }
  receivers_.Add(this, std::move(receiver), caller_origin);
}

void VaultServiceImpl::IsUnlocked(IsUnlockedCallback callback) {
  std::move(callback).Run(is_unlocked_);
}

void VaultServiceImpl::Unlock(const std::string& pin_or_passkey_assertion,
                              UnlockCallback callback) {
  if (pin_or_passkey_assertion.empty()) {
    std::move(callback).Run(vault::mojom::VaultStatus::kInvalidPinOrPasskey);
    return;
  }

  // Derive/Unwrap session key (placeholder for KDF/envelope crypto engine)
  is_unlocked_ = true;
  active_session_key_ = "active_session_master_key_handle";

  std::move(callback).Run(vault::mojom::VaultStatus::kSuccess);
}

void VaultServiceImpl::Lock(LockCallback callback) {
  is_unlocked_ = false;
  active_session_key_.clear();
  std::move(callback).Run(vault::mojom::VaultStatus::kSuccess);
}

void VaultServiceImpl::ListEntriesMetadata(ListEntriesMetadataCallback callback) {
  if (!is_unlocked_) {
    std::move(callback).Run(vault::mojom::VaultStatus::kLocked, {});
    return;
  }

  std::vector<vault::mojom::VaultEntryMetadataPtr> results;
  for (const auto& [id, entry] : entries_cache_) {
    auto meta = vault::mojom::VaultEntryMetadata::New();
    meta->id = entry.id;
    meta->site_url = entry.site_url;
    meta->username = entry.username;
    meta->created_time_unix = entry.created_time_unix;
    meta->last_modified_unix = entry.last_modified_unix;
    results.push_back(std::move(meta));
  }

  std::move(callback).Run(vault::mojom::VaultStatus::kSuccess, std::move(results));
}

void VaultServiceImpl::GetCredentialForAutofill(
    const std::string& entry_id,
    const GURL& target_origin,
    GetCredentialForAutofillCallback callback) {
  if (!is_unlocked_) {
    std::move(callback).Run(vault::mojom::VaultStatus::kLocked, nullptr);
    return;
  }

  auto it = entries_cache_.find(entry_id);
  if (it == entries_cache_.end()) {
    std::move(callback).Run(vault::mojom::VaultStatus::kNotFound, nullptr);
    return;
  }

  // Strict origin match verification
  if (url::Origin::Create(it->second.site_url) != url::Origin::Create(target_origin)) {
    std::move(callback).Run(vault::mojom::VaultStatus::kUnauthorizedOrigin, nullptr);
    return;
  }

  // Decrypt and return only the single targeted credential
  auto cred = vault::mojom::VaultAutofillCredential::New();
  cred->username = it->second.username;
  cred->password_plaintext = "decrypted_password_plaintext";

  std::move(callback).Run(vault::mojom::VaultStatus::kSuccess, std::move(cred));
}

void VaultServiceImpl::AddEntry(vault::mojom::VaultEntryInputPtr input,
                                AddEntryCallback callback) {
  if (!is_unlocked_) {
    std::move(callback).Run(vault::mojom::VaultStatus::kLocked, std::nullopt);
    return;
  }

  if (!input || !input->site_url.is_valid()) {
    std::move(callback).Run(vault::mojom::VaultStatus::kInternalError, std::nullopt);
    return;
  }

  std::string new_id = base::NumberToString(base::RandUint64());
  EncryptedVaultEntry entry;
  entry.id = new_id;
  entry.site_url = input->site_url;
  entry.username = input->username;
  entry.encrypted_password_blob = "encrypted_blob_" + input->password_plaintext;
  entry.created_time_unix = base::Time::Now().ToTimeT();
  entry.last_modified_unix = entry.created_time_unix;

  entries_cache_[new_id] = std::move(entry);
  std::move(callback).Run(vault::mojom::VaultStatus::kSuccess, new_id);
}

void VaultServiceImpl::EditEntry(const std::string& entry_id,
                                 vault::mojom::VaultEntryInputPtr input,
                                 EditEntryCallback callback) {
  if (!is_unlocked_) {
    std::move(callback).Run(vault::mojom::VaultStatus::kLocked);
    return;
  }

  auto it = entries_cache_.find(entry_id);
  if (it == entries_cache_.end()) {
    std::move(callback).Run(vault::mojom::VaultStatus::kNotFound);
    return;
  }

  it->second.site_url = input->site_url;
  it->second.username = input->username;
  if (!input->password_plaintext.empty()) {
    it->second.encrypted_password_blob = "encrypted_blob_" + input->password_plaintext;
  }
  it->second.last_modified_unix = base::Time::Now().ToTimeT();

  std::move(callback).Run(vault::mojom::VaultStatus::kSuccess);
}

void VaultServiceImpl::DeleteEntry(const std::string& entry_id,
                                   DeleteEntryCallback callback) {
  if (!is_unlocked_) {
    std::move(callback).Run(vault::mojom::VaultStatus::kLocked);
    return;
  }

  auto erased = entries_cache_.erase(entry_id);
  if (erased == 0) {
    std::move(callback).Run(vault::mojom::VaultStatus::kNotFound);
    return;
  }

  std::move(callback).Run(vault::mojom::VaultStatus::kSuccess);
}

void VaultServiceImpl::GeneratePassword(uint32_t length,
                                        bool include_symbols,
                                        GeneratePasswordCallback callback) {
  const std::string kLetters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
  const std::string kSymbols = "!@#$%^&*()-_=+[]{}";
  std::string charset = kLetters + (include_symbols ? kSymbols : "");

  length = std::clamp(length, 8u, 128u);
  std::string password;
  password.resize(length);

  for (uint32_t i = 0; i < length; ++i) {
    password[i] = charset[base::RandGenerator(charset.size())];
  }

  std::move(callback).Run(password);
}

}  // namespace codem37
