// Copyright (c) 2026 The codem37 Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "src/mine/shield/shield_url_loader_interceptor.h"

#include "src/mine/shield/shield_service_impl.h"

namespace codem37 {

ShieldURLLoaderInterceptor::ShieldURLLoaderInterceptor(ShieldService* shield_service)
    : shield_service_(shield_service) {}

ShieldURLLoaderInterceptor::~ShieldURLLoaderInterceptor() = default;

InterceptionResult ShieldURLLoaderInterceptor::MaybeInterceptRequest(
    const GURL& request_url,
    const url::Origin& top_origin,
    bool is_third_party) const {
  if (!shield_service_) {
    return InterceptionResult::kAllow;
  }

  ShieldServiceImpl* impl = static_cast<ShieldServiceImpl*>(shield_service_.get());
  if (impl->ShouldBlockRequest(request_url, top_origin, is_third_party)) {
    return InterceptionResult::kBlock;
  }

  return InterceptionResult::kAllow;
}

}  // namespace codem37
