// Copyright (c) 2026 The codem37 Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CODEM37_SRC_MINE_VAULT_VAULT_SERVICE_FACTORY_H_
#define CODEM37_SRC_MINE_VAULT_VAULT_SERVICE_FACTORY_H_

#include "base/no_destructor.h"
#include "components/keyed_service/content/browser_context_keyed_service_factory.h"

namespace content {
class BrowserContext;
}

namespace codem37 {

class VaultService;

// Factory managing per-profile lifecycle of VaultService.
class VaultServiceFactory : public BrowserContextKeyedServiceFactory {
 public:
  static VaultService* GetForBrowserContext(content::BrowserContext* context);
  static VaultServiceFactory* GetInstance();

 private:
  friend class base::NoDestructor<VaultServiceFactory>;

  VaultServiceFactory();
  ~VaultServiceFactory() override;

  // BrowserContextKeyedServiceFactory implementation:
  std::unique_ptr<KeyedService> BuildServiceInstanceForBrowserContext(
      content::BrowserContext* context) const override;
  content::BrowserContext* GetBrowserContextToUse(
      content::BrowserContext* context) const override;
};

}  // namespace codem37

#endif  // CODEM37_SRC_MINE_VAULT_VAULT_SERVICE_FACTORY_H_
