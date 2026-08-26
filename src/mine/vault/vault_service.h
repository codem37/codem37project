// Copyright (c) 2026 The codem37 Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CODEM37_SRC_MINE_VAULT_VAULT_SERVICE_H_
#define CODEM37_SRC_MINE_VAULT_VAULT_SERVICE_H_

#include "components/keyed_service/core/keyed_service.h"
#include "mojo/public/cpp/bindings/pending_receiver.h"
#include "src/mine/vault/mojom/vault.mojom.h"

namespace content {
class BrowserContext;
}  // namespace content

namespace codem37 {

// Abstract interface for the Browser-Process Vault Service.
class VaultService : public KeyedService, public vault::mojom::VaultService {
 public:
  ~VaultService() override = default;

  // Binds a receiver from a verified WebUI renderer.
  virtual void BindReceiver(
      mojo::PendingReceiver<vault::mojom::VaultService> receiver,
      const url::Origin& caller_origin) = 0;
};

}  // namespace codem37

#endif  // CODEM37_SRC_MINE_VAULT_VAULT_SERVICE_H_
