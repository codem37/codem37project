// Copyright (c) 2026 The codem37 Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "src/mine/shield/shield_service_impl.h"

#include <utility>

#include "base/rand_util.h"
#include "base/strings/string_number_conversions.h"
#include "base/time/time.h"

namespace codem37 {

namespace {

constexpr char kShieldOriginScheme[] = "chrome";
constexpr char kShieldOriginHost[] = "shield";

bool IsAuthorizedShieldOrigin(const url::Origin& origin) {
  return origin.scheme() == kShieldOriginScheme && origin.host() == kShieldOriginHost;
}

}  // namespace

ShieldServiceImpl::ShieldServiceImpl(content::BrowserContext* context)
    : context_(context) {
  // Initialize default built-in filter list
  auto default_sub = shield::mojom::FilterSubscription::New();
  default_sub->id = "default_easy_privacy";
  default_sub->title = "EasyPrivacy Standard Protection";
  default_sub->update_url = GURL("https://easylist.to/easylist/easyprivacy.txt");
  default_sub->is_enabled = true;
  default_sub->rules_count = 45000;
  default_sub->last_updated_unix = base::Time::Now().ToTimeT();
  subscriptions_[default_sub->id] = std::move(default_sub);
}

ShieldServiceImpl::~ShieldServiceImpl() {
  Shutdown();
}

void ShieldServiceImpl::Shutdown() {
  subscriptions_.clear();
  site_toggles_.clear();
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

bool ShieldServiceImpl::ShouldBlockRequest(const GURL& request_url,
                                          const GURL& first_party_url) {
  // Check if shield is disabled for first party site
  auto it = site_toggles_.find(first_party_url.host());
  if (it != site_toggles_.end() && !it->second) {
    return false;
  }

  // Placeholder matching against tracker hosts
  if (request_url.host().find("tracker") != std::string::npos ||
      request_url.host().find("analytics") != std::string::npos ||
      request_url.host().find("doubleclick") != std::string::npos) {
    total_blocked_++;
    trackers_blocked_++;
    return true;
  }

  return false;
}

void ShieldServiceImpl::GetSubscriptions(GetSubscriptionsCallback callback) {
  std::vector<shield::mojom::FilterSubscriptionPtr> list;
  for (const auto& [id, sub] : subscriptions_) {
    list.push_back(sub.Clone());
  }
  std::move(callback).Run(std::move(list));
}

void ShieldServiceImpl::SetSubscriptionEnabled(const std::string& subscription_id,
                                               bool enabled,
                                               SetSubscriptionEnabledCallback callback) {
  auto it = subscriptions_.find(subscription_id);
  if (it == subscriptions_.end()) {
    std::move(callback).Run(shield::mojom::ShieldStatus::kNotFound);
    return;
  }

  it->second->is_enabled = enabled;
  std::move(callback).Run(shield::mojom::ShieldStatus::kSuccess);
}

void ShieldServiceImpl::AddSubscription(const std::string& title,
                                        const GURL& update_url,
                                        AddSubscriptionCallback callback) {
  if (!update_url.is_valid()) {
    std::move(callback).Run(shield::mojom::ShieldStatus::kInvalidRule, std::nullopt);
    return;
  }

  std::string new_id = base::NumberToString(base::RandUint64());
  auto sub = shield::mojom::FilterSubscription::New();
  sub->id = new_id;
  sub->title = title;
  sub->update_url = update_url;
  sub->is_enabled = true;
  sub->rules_count = 0;
  sub->last_updated_unix = base::Time::Now().ToTimeT();

  subscriptions_[new_id] = std::move(sub);
  std::move(callback).Run(shield::mojom::ShieldStatus::kSuccess, new_id);
}

void ShieldServiceImpl::RemoveSubscription(const std::string& subscription_id,
                                           RemoveSubscriptionCallback callback) {
  auto erased = subscriptions_.erase(subscription_id);
  if (erased == 0) {
    std::move(callback).Run(shield::mojom::ShieldStatus::kNotFound);
    return;
  }
  std::move(callback).Run(shield::mojom::ShieldStatus::kSuccess);
}

void ShieldServiceImpl::GetSiteSetting(const std::string& hostname,
                                       GetSiteSettingCallback callback) {
  auto setting = shield::mojom::ShieldSiteSetting::New();
  setting->hostname = hostname;

  auto it = site_toggles_.find(hostname);
  setting->shield_enabled = (it == site_toggles_.end()) ? true : it->second;
  setting->cosmetic_filtering_enabled = setting->shield_enabled;
  setting->tracker_blocking_enabled = setting->shield_enabled;

  std::move(callback).Run(std::move(setting));
}

void ShieldServiceImpl::SetSiteShieldEnabled(const std::string& hostname,
                                             bool enabled,
                                             SetSiteShieldEnabledCallback callback) {
  site_toggles_[hostname] = enabled;
  std::move(callback).Run(shield::mojom::ShieldStatus::kSuccess);
}

void ShieldServiceImpl::GetTelemetry(GetTelemetryCallback callback) {
  auto telem = shield::mojom::ShieldTelemetry::New();
  telem->total_requests_blocked = total_blocked_;
  telem->trackers_blocked = trackers_blocked_;
  telem->ads_blocked = ads_blocked_;
  std::move(callback).Run(std::move(telem));
}

void ShieldServiceImpl::ResetTelemetry(ResetTelemetryCallback callback) {
  total_blocked_ = 0;
  trackers_blocked_ = 0;
  ads_blocked_ = 0;
  std::move(callback).Run(shield::mojom::ShieldStatus::kSuccess);
}

}  // namespace codem37
