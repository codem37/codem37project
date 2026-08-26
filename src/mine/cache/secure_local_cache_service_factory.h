// Copyright (c) 2026 The codem37 Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CODEM37_SRC_MINE_CACHE_SECURE_LOCAL_CACHE_SERVICE_FACTORY_H_
#define CODEM37_SRC_MINE_CACHE_SECURE_LOCAL_CACHE_SERVICE_FACTORY_H_

#include "base/no_destructor.h"
#include "components/keyed_service/content/browser_context_keyed_service_factory.h"

namespace content {
class BrowserContext;
}

namespace codem37 {

class SecureLocalCacheService;

// Factory managing per-profile lifecycle of SecureLocalCacheService.
class SecureLocalCacheServiceFactory : public BrowserContextKeyedServiceFactory {
 public:
  static SecureLocalCacheService* GetForBrowserContext(content::BrowserContext* context);
  static SecureLocalCacheServiceFactory* GetInstance();

 private:
  friend class base::NoDestructor<SecureLocalCacheServiceFactory>;

  SecureLocalCacheServiceFactory();
  ~SecureLocalCacheServiceFactory() override;

  // BrowserContextKeyedServiceFactory implementation:
  std::unique_ptr<KeyedService> BuildServiceInstanceForBrowserContext(
      content::BrowserContext* context) const override;
  content::BrowserContext* GetBrowserContextToUse(
      content::BrowserContext* context) const override;
};

}  // namespace codem37

#endif  // CODEM37_SRC_MINE_CACHE_SECURE_LOCAL_CACHE_SERVICE_FACTORY_H_
