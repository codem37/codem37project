// Copyright (c) 2026 The codem37 Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "src/mine/fetcher/fetcher_service_factory.h"

#include "components/keyed_service/content/browser_context_dependency_manager.h"
#include "content/public/browser/browser_context.h"
#include "src/mine/fetcher/fetcher_service_impl.h"

namespace codem37 {

// static
FetcherService* FetcherServiceFactory::GetForBrowserContext(
    content::BrowserContext* context) {
  return static_cast<FetcherService*>(
      GetInstance()->GetServiceForBrowserContext(context, true));
}

// static
FetcherServiceFactory* FetcherServiceFactory::GetInstance() {
  static base::NoDestructor<FetcherServiceFactory> instance;
  return instance.get();
}

FetcherServiceFactory::FetcherServiceFactory()
    : BrowserContextKeyedServiceFactory(
          "FetcherService",
          BrowserContextDependencyManager::GetInstance()) {}

FetcherServiceFactory::~FetcherServiceFactory() = default;

std::unique_ptr<KeyedService>
FetcherServiceFactory::BuildServiceInstanceForBrowserContext(
    content::BrowserContext* context) const {
  return std::make_unique<FetcherServiceImpl>(context);
}

content::BrowserContext* FetcherServiceFactory::GetBrowserContextToUse(
    content::BrowserContext* context) const {
  return context;
}

}  // namespace codem37
