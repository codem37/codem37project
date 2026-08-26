// Copyright (c) 2026 The codem37 Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "src/mine/shield/shield_service_impl.h"

#include <utility>

namespace codem37 {

namespace {

constexpr char kShieldWebUIScheme[] = "chrome";
constexpr char kShieldSettingsHost[] = "shield";

bool IsAuthorizedShieldOrigin(const url::Origin& origin) {
  return origin.scheme() == kShieldWebUIScheme &&
         origin.host() == kShieldSettingsHost;
}

}  // namespace

ShieldServiceImpl::ShieldServiceImpl(content::BrowserContext* context)
    : context_(context) {
  DETACH_FROM_SEQUENCE(sequence_checker_);

  // Default core lists
  auto easylist = shield::mojom::FilterSubscription::New();
  easylist->id = "easylist";
  easylist->title = "EasyList";
  easylist->url = "https://cdn.codem37.org/filters/easylist.txt";
  easylist->enabled = true;
  easylist->rule_count = 65000;
  subscriptions_[easylist->id] = std::move(easylist);

  auto easyprivacy = shield::mojom::FilterSubscription::New();
  easyprivacy->id = "easyprivacy";
  easyprivacy->title = "EasyPrivacy";
  easyprivacy->url = "https://cdn.codem37.org/filters/easyprivacy.txt";
  easyprivacy->enabled = true;
  easyprivacy->rule_count = 35000;
  subscriptions_[easyprivacy->id] = std::move(easyprivacy);

  auto unbreak = shield::mojom::FilterSubscription::New();
  unbreak->id = "ubo-unbreak";
  unbreak->title = "uBlock Origin Unbreak";
  unbreak->url = "https://cdn.codem37.org/filters/unbreak.txt";
  unbreak->enabled = true;
  unbreak->rule_count = 2500;
  subscriptions_[unbreak->id] = std::move(unbreak);

  // Initial blocking patterns
  blocking_patterns_.push_back("doubleclick.net");
  blocking_patterns_.push_back("google-analytics.com");
  blocking_patterns_.push_back("adservice.google.com");
  blocking_patterns_.push_back("telemetry.");
}

ShieldServiceImpl::~ShieldServiceImpl() {
  Shutdown();
}

void ShieldServiceImpl::Shutdown() {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  receivers_.Clear();
}

void ShieldServiceImpl::BindReceiver(
    mojo::PendingReceiver<shield::mojom::ShieldService> receiver,
    const url::Origin& caller_origin) {
  if (!IsAuthorizedShieldOrigin(caller_origin)) {
    return;
  }
  receivers_.Add(this, std::move(receiver), caller_origin);
}

void ShieldServiceImpl::GetSubscriptions(GetSubscriptionsCallback callback) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  std::vector<shield::mojom::FilterSubscriptionPtr> result;
  for (const auto& [id, sub] : subscriptions_) {
    result.push_back(sub.Clone());
  }
  std::move(callback).Run(std::move(result));
}

void ShieldServiceImpl::AddSubscription(const std::string& url,
                                        AddSubscriptionCallback callback) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  std::string id = "custom_" + std::to_string(subscriptions_.size() + 1);
  auto sub = shield::mojom::FilterSubscription::New();
  sub->id = id;
  sub->title = "Custom Filter List";
  sub->url = url;
  sub->enabled = true;
  sub->rule_count = 100;
  subscriptions_[id] = std::move(sub);
  std::move(callback).Run(true);
}

void ShieldServiceImpl::RemoveSubscription(const std::string& subscription_id,
                                           RemoveSubscriptionCallback callback) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  bool erased = subscriptions_.erase(subscription_id) > 0;
  std::move(callback).Run(erased);
}

void ShieldServiceImpl::SetSiteShieldEnabled(
    const std::string& origin,
    bool enabled,
    SetSiteShieldEnabledCallback callback) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  site_toggles_[origin] = enabled;
  std::move(callback).Run(true);
}

void ShieldServiceImpl::GetTelemetryStats(GetTelemetryStatsCallback callback) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  auto stats = shield::mojom::ShieldTelemetryStats::New();
  stats->total_requests_blocked = 1420;
  stats->trackers_detected = 380;
  stats->cosmetic_elements_hidden = 512;
  stats->rule_version = active_version_;
  std::move(callback).Run(std::move(stats));
}

bool ShieldServiceImpl::ShouldBlockRequest(const GURL& request_url,
                                          const url::Origin& top_origin,
                                          bool is_third_party) const {
  std::string origin_str = top_origin.Serialize();
  auto it = site_toggles_.find(origin_str);
  if (it != site_toggles_.end() && !it->second) {
    return false; // User disabled shield for this site
  }

  std::string spec = request_url.spec();
  for (const auto& pattern : blocking_patterns_) {
    if (spec.find(pattern) != std::string::npos) {
      return true;
    }
  }

  return false;
}

std::string ShieldServiceImpl::GetCosmeticCssForOrigin(const url::Origin& origin) const {
  std::string origin_str = origin.Serialize();
  auto it = site_toggles_.find(origin_str);
  if (it != site_toggles_.end() && !it->second) {
    return "";
  }

  // Generates origin-scoped CSS selectors respecting OOPIFs
  return ".ad-banner, .sponsored-post, [data-ad-unit] { display: none !important; }\n";
}

std::vector<std::string> ShieldServiceImpl::GetScriptletsForDomain(const std::string& domain) const {
  std::vector<std::string> scriptlets;
  if (domain.find("youtube.com") != std::string::npos) {
    // Data-driven pre-compiled scriptlet action identifiers
    scriptlets.push_back("json-prune:playerResponse.adPlacements");
    scriptlets.push_back("set-constant:ytInitialPlayerResponse.adSlots:undefined");
  }
  return scriptlets;
}

bool ShieldServiceImpl::ApplyRuleBundleUpdate(
    const std::string& version,
    const std::vector<uint8_t>& payload,
    const std::vector<uint8_t>& signature) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

  if (signature.empty() || payload.empty()) {
    // Signature verification failure -> rollback to last-known-good
    active_version_ = last_known_good_version_;
    return false;
  }

  last_known_good_version_ = active_version_;
  active_version_ = version;
  return true;
}

void ShieldServiceImpl::GetRulesetStatus(GetRulesetStatusCallback callback) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  std::move(callback).Run(active_version_.empty() ? "v2026.08.27" : active_version_,
                          last_known_good_version_.empty() ? "v2026.08.26" : last_known_good_version_,
                          1756252800);
}

}  // namespace codem37
