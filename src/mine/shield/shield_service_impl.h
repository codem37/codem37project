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
#include "base/sequence_checker.h"
#include "mojo/public/cpp/bindings/receiver_set.h"
#include "src/mine/shield/shield_service.h"

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

  // mojom::ShieldService implementation:
  void GetSubscriptions(GetSubscriptionsCallback callback) override;
  void AddSubscription(const std::string& url,
                       AddSubscriptionCallback callback) override;
  void RemoveSubscription(const std::string& subscription_id,
                          RemoveSubscriptionCallback callback) override;
  void SetSiteShieldEnabled(const std::string& origin,
                            bool enabled,
                            SetSiteShieldEnabledCallback callback) override;
  void GetTelemetryStats(GetTelemetryStatsCallback callback) override;
  void GetRulesetStatus(GetRulesetStatusCallback callback) override;

  // Engine evaluation APIs (called by Network Interceptor & WebContentsObserver):
  bool ShouldBlockRequest(const GURL& request_url,
                          const url::Origin& top_origin,
                          bool is_third_party) const;
  std::string GetCosmeticCssForOrigin(const url::Origin& origin) const;
  std::vector<std::string> GetScriptletsForDomain(const std::string& domain) const;

  // Cryptographic update verification & rollback:
  bool ApplyRuleBundleUpdate(const std::string& version,
                             const std::vector<uint8_t>& payload,
                             const std::vector<uint8_t>& signature);

  // KeyedService lifecycle:
  void Shutdown() override;

 private:
  SEQUENCE_CHECKER(sequence_checker_);

  raw_ptr<content::BrowserContext> context_;
  std::map<std::string, shield::mojom::FilterSubscriptionPtr> subscriptions_;
  std::map<std::string, bool> site_toggles_;

  std::string active_version_ = "v20260826.1";
  std::string last_known_good_version_ = "v20260826.1";
  std::vector<std::string> blocking_patterns_;

  mojo::ReceiverSet<shield::mojom::ShieldService, url::Origin> receivers_;
};

}  // namespace codem37

#endif  // CODEM37_SRC_MINE_SHIELD_SHIELD_SERVICE_IMPL_H_
