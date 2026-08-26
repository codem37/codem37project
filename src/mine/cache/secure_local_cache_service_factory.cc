// Copyright (c) 2026 The codem37 Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "src/mine/cache/secure_local_cache_service_factory.h"

#include "components/keyed_service/content/browser_context_dependency_manager.h"
#include "content/public/browser/browser_context.h"
#include "src/mine/cache/secure_local_cache_service_impl.h"

namespace codem37 {

// static
SecureLocalCacheService* SecureLocalCacheServiceFactory::GetForBrowserContext(
    content::BrowserContext* context) {
  return static_cast<SecureLocalCacheService*>(
      GetInstance()->GetServiceForBrowserContext(context, true));
}

// static
SecureLocalCacheServiceFactory* SecureLocalCacheServiceFactory::GetInstance() {
  static base::NoDestructor<SecureLocalCacheServiceFactory> instance;
  return instance.get();
}

SecureLocalCacheServiceFactory::SecureLocalCacheServiceFactory()
    : BrowserContextKeyedServiceFactory(
          "SecureLocalCacheService",
          BrowserContextDependencyManager::GetInstance()) {}

SecureLocalCacheServiceFactory::~SecureLocalCacheServiceFactory() = default;

std::unique_ptr<KeyedService>
SecureLocalCacheServiceFactory::BuildServiceInstanceForBrowserContext(
    content::BrowserContext* context) const {
  return std::make_unique<SecureLocalCacheServiceImpl>(context);
}

content::BrowserContext* SecureLocalCacheServiceFactory::GetBrowserContextToUse(
    content::BrowserContext* context) const {
  return context;
}

}  // namespace codem37
