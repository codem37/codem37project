// Copyright (c) 2026 The codem37 Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CODEM37_SRC_MINE_VAULT_VAULT_SERVICE_IMPL_H_
#define CODEM37_SRC_MINE_VAULT_VAULT_SERVICE_IMPL_H_

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "base/memory/raw_ptr.h"
#include "base/memory/weak_ptr.h"
#include "mojo/public/cpp/bindings/receiver_set.h"
#include "src/mine/vault/vault_service.h"
#include "url/gurl.h"
#include "url/origin.h"

namespace content {
class BrowserContext;
}

namespace codem37 {

// In-memory representation of an encrypted vault entry
struct EncryptedVaultEntry {
  std::string id;
  GURL site_url;
  std::string username;
  std::string encrypted_password_blob;
  int64_t created_time_unix = 0;
  int64_t last_modified_unix = 0;
};

class VaultServiceImpl : public VaultService {
 public:
  explicit VaultServiceImpl(content::BrowserContext* context);
  ~VaultServiceImpl() override;

  VaultServiceImpl(const VaultServiceImpl&) = delete;
  VaultServiceImpl& operator=(const VaultServiceImpl&) = delete;

  // VaultService implementation:
  void BindReceiver(
      mojo::PendingReceiver<vault::mojom::VaultService> receiver,
      const url::Origin& caller_origin) override;

  // mojom::VaultService implementation:
  void IsUnlocked(IsUnlockedCallback callback) override;
  void Unlock(const std::string& pin_or_passkey_assertion,
              UnlockCallback callback) override;
  void Lock(LockCallback callback) override;
  void ListEntriesMetadata(ListEntriesMetadataCallback callback) override;
  void GetCredentialForAutofill(
      const std::string& entry_id,
      const GURL& target_origin,
      GetCredentialForAutofillCallback callback) override;
  void AddEntry(vault::mojom::VaultEntryInputPtr input,
                AddEntryCallback callback) override;
  void EditEntry(const std::string& entry_id,
                 vault::mojom::VaultEntryInputPtr input,
                 EditEntryCallback callback) override;
  void DeleteEntry(const std::string& entry_id,
                   DeleteEntryCallback callback) override;
  void GeneratePassword(uint32_t length,
                        bool include_symbols,
                        GeneratePasswordCallback callback) override;

  // KeyedService lifecycle:
  void Shutdown() override;

 private:
  bool CheckAuthorization(const url::Origin& expected_origin);

  raw_ptr<content::BrowserContext> context_;
  bool is_unlocked_ = false;

  // In-memory key material (purged immediately on Lock())
  std::string active_session_key_;

  // Storage cache
  std::map<std::string, EncryptedVaultEntry> entries_cache_;

  mojo::ReceiverSet<vault::mojom::VaultService, url::Origin> receivers_;

  base::WeakPtrFactory<VaultServiceImpl> weak_factory_{this};
};

}  // namespace codem37

#endif  // CODEM37_SRC_MINE_VAULT_VAULT_SERVICE_IMPL_H_
