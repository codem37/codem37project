// Copyright (c) 2026 The codem37 Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CODEM37_SRC_MINE_SHIELD_SHIELD_URL_LOADER_INTERCEPTOR_H_
#define CODEM37_SRC_MINE_SHIELD_SHIELD_URL_LOADER_INTERCEPTOR_H_

#include <memory>
#include "base/memory/raw_ptr.h"
#include "url/gurl.h"
#include "url/origin.h"

namespace codem37 {

class ShieldService;

enum class InterceptionResult {
  kAllow,
  kBlock,
  kRedirect,
};

// Intercepts outgoing requests in the browser process before they hit the network.
class ShieldURLLoaderInterceptor {
 public:
  explicit ShieldURLLoaderInterceptor(ShieldService* shield_service);
  ~ShieldURLLoaderInterceptor();

  ShieldURLLoaderInterceptor(const ShieldURLLoaderInterceptor&) = delete;
  ShieldURLLoaderInterceptor& operator=(const ShieldURLLoaderInterceptor&) = delete;

  InterceptionResult MaybeInterceptRequest(const GURL& request_url,
                                          const url::Origin& top_origin,
                                          bool is_third_party) const;

 private:
  raw_ptr<ShieldService> shield_service_;
};

}  // namespace codem37

#endif  // CODEM37_SRC_MINE_SHIELD_SHIELD_URL_LOADER_INTERCEPTOR_H_
