// Copyright (c) 2026 The codem37 Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "src/mine/shield/shield_service_factory.h"

#include "components/keyed_service/content/browser_context_dependency_manager.h"
#include "content/public/browser/browser_context.h"
#include "src/mine/shield/shield_service_impl.h"

namespace codem37 {

// static
ShieldService* ShieldServiceFactory::GetForBrowserContext(
    content::BrowserContext* context) {
  return static_cast<ShieldService*>(
      GetInstance()->GetServiceForBrowserContext(context, true));
}

// static
ShieldServiceFactory* ShieldServiceFactory::GetInstance() {
  static base::NoDestructor<ShieldServiceFactory> instance;
  return instance.get();
}

ShieldServiceFactory::ShieldServiceFactory()
    : BrowserContextKeyedServiceFactory(
          "ShieldService",
          BrowserContextDependencyManager::GetInstance()) {}

ShieldServiceFactory::~ShieldServiceFactory() = default;

std::unique_ptr<KeyedService>
ShieldServiceFactory::BuildServiceInstanceForBrowserContext(
    content::BrowserContext* context) const {
  return std::make_unique<ShieldServiceImpl>(context);
}

content::BrowserContext* ShieldServiceFactory::GetBrowserContextToUse(
    content::BrowserContext* context) const {
  return context;
}

}  // namespace codem37
