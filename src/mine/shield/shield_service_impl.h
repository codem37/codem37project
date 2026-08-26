// Copyright (c) 2026 The codem37 Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CODEM37_SRC_MINE_SHIELD_SHIELD_SERVICE_IMPL_H_
#define CODEM37_SRC_MINE_SHIELD_SHIELD_SERVICE_IMPL_H_

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "base/memory/raw_ptr.h"
#include "base/memory/weak_ptr.h"
#include "mojo/public/cpp/bindings/receiver_set.h"
#include "src/mine/shield/shield_service.h"
#include "url/gurl.h"
#include "url/origin.h"

namespace content {
class BrowserContext;
}

namespace codem37 {

class ShieldServiceImpl : public ShieldService {
 public:
  explicit ShieldServiceImpl(content::BrowserContext* context);
  ~ShieldServiceImpl() override;

  ShieldServiceImpl(const ShieldServiceImpl&) = delete;
  ShieldServiceImpl& operator=(const ShieldServiceImpl&) = delete;

  // ShieldService implementation:
  void BindReceiver(
      mojo::PendingReceiver<shield::mojom::ShieldService> receiver,
      const url::Origin& caller_origin) override;
  bool ShouldBlockRequest(const GURL& request_url,
                          const GURL& first_party_url) override;

  // mojom::ShieldService implementation:
  void GetSubscriptions(GetSubscriptionsCallback callback) override;
  void SetSubscriptionEnabled(const std::string& subscription_id,
                              bool enabled,
                              SetSubscriptionEnabledCallback callback) override;
  void AddSubscription(const std::string& title,
                       const GURL& update_url,
                       AddSubscriptionCallback callback) override;
  void RemoveSubscription(const std::string& subscription_id,
                          RemoveSubscriptionCallback callback) override;
  void GetSiteSetting(const std::string& hostname,
                      GetSiteSettingCallback callback) override;
  void SetSiteShieldEnabled(const std::string& hostname,
                            bool enabled,
                            SetSiteShieldEnabledCallback callback) override;
  void GetTelemetry(GetTelemetryCallback callback) override;
  void ResetTelemetry(ResetTelemetryCallback callback) override;

  // KeyedService lifecycle:
  void Shutdown() override;

 private:
  raw_ptr<content::BrowserContext> context_;

  std::map<std::string, shield::mojom::FilterSubscriptionPtr> subscriptions_;
  std::map<std::string, bool> site_toggles_;

  int64_t total_blocked_ = 0;
  int64_t trackers_blocked_ = 0;
  int64_t ads_blocked_ = 0;

  mojo::ReceiverSet<shield::mojom::ShieldService, url::Origin> receivers_;

  base::WeakPtrFactory<ShieldServiceImpl> weak_factory_{this};
};

}  // namespace codem37

#endif  // CODEM37_SRC_MINE_SHIELD_SHIELD_SERVICE_IMPL_H_
