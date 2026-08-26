// Copyright (c) 2026 The codem37 Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CODEM37_SRC_MINE_SHIELD_SHIELD_SERVICE_H_
#define CODEM37_SRC_MINE_SHIELD_SHIELD_SERVICE_H_

#include "components/keyed_service/core/keyed_service.h"
#include "mojo/public/cpp/bindings/pending_receiver.h"
#include "src/mine/shield/mojom/shield.mojom.h"

namespace content {
class BrowserContext;
}

namespace codem37 {

// Abstract interface for the Browser-Process Shield Service.
class ShieldService : public KeyedService, public shield::mojom::ShieldService {
 public:
  ~ShieldService() override = default;

  // Binds a receiver from a verified WebUI renderer.
  virtual void BindReceiver(
      mojo::PendingReceiver<shield::mojom::ShieldService> receiver,
      const url::Origin& caller_origin) = 0;

  // Internal matching query for network request inspection.
  virtual bool ShouldBlockRequest(const GURL& request_url,
                                  const GURL& first_party_url) = 0;
};

}  // namespace codem37

#endif  // CODEM37_SRC_MINE_SHIELD_SHIELD_SERVICE_H_
