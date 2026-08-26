// Copyright (c) 2026 The codem37 Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CODEM37_SRC_MINE_CACHE_SECURE_LOCAL_CACHE_SERVICE_H_
#define CODEM37_SRC_MINE_CACHE_SECURE_LOCAL_CACHE_SERVICE_H_

#include "components/keyed_service/core/keyed_service.h"
#include "mojo/public/cpp/bindings/pending_receiver.h"
#include "src/mine/cache/mojom/cache.mojom.h"
#include "url/origin.h"

namespace codem37 {

// Abstract KeyedService for managing encrypted local browser state.
class SecureLocalCacheService : public KeyedService,
                                public cache::mojom::SecureLocalCache {
 public:
  ~SecureLocalCacheService() override = default;

  virtual void BindReceiver(
      mojo::PendingReceiver<cache::mojom::SecureLocalCache> receiver,
      const url::Origin& caller_origin) = 0;
};

}  // namespace codem37

#endif  // CODEM37_SRC_MINE_CACHE_SECURE_LOCAL_CACHE_SERVICE_H_
