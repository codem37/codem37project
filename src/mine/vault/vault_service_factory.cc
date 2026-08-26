// Copyright (c) 2026 The codem37 Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "src/mine/vault/vault_service_factory.h"

#include "components/keyed_service/content/browser_context_dependency_manager.h"
#include "content/public/browser/browser_context.h"
#include "src/mine/vault/vault_service_impl.h"

namespace codem37 {

// static
VaultService* VaultServiceFactory::GetForBrowserContext(
    content::BrowserContext* context) {
  return static_cast<VaultService*>(
      GetInstance()->GetServiceForBrowserContext(context, true));
}

// static
VaultServiceFactory* VaultServiceFactory::GetInstance() {
  static base::NoDestructor<VaultServiceFactory> instance;
  return instance.get();
}

VaultServiceFactory::VaultServiceFactory()
    : BrowserContextKeyedServiceFactory(
          "VaultService",
          BrowserContextDependencyManager::GetInstance()) {}

VaultServiceFactory::~VaultServiceFactory() = default;

std::unique_ptr<KeyedService>
VaultServiceFactory::BuildServiceInstanceForBrowserContext(
    content::BrowserContext* context) const {
  return std::make_unique<VaultServiceImpl>(context);
}

content::BrowserContext* VaultServiceFactory::GetBrowserContextToUse(
    content::BrowserContext* context) const {
  // Return same context so incognito / guest get their own isolated instances
  return context;
}

}  // namespace codem37
