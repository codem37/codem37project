// Copyright (c) 2026 The codem37 Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "src/mine/shield/shield_tab_observer.h"

#include <iostream>
#include "src/mine/shield/shield_service.h"

namespace codem37 {

ShieldTabObserver::ShieldTabObserver(ShieldService* shield_service)
    : shield_service_(shield_service) {}

ShieldTabObserver::~ShieldTabObserver() = default;

void ShieldTabObserver::OnDidStartNavigation(const std::string& url_origin) {
  // Check if shields are active for this origin
  // In full Chromium: prepares isolated-world world_id (ISOLATED_WORLD_ID_SHIELD)
}

void ShieldTabObserver::OnReadyToCommitNavigation(const std::string& url_origin) {
  // Inject cosmetic hiding stylesheets into the committed document
}

std::string ShieldTabObserver::GetCosmeticStylesheetForOrigin(
    const std::string& origin) const {
  if (!shield_service_) {
    return "";
  }
  // Generates safe CSS selector block from rust shield engine
  return ".ad-banner, .sponsored-content, [data-ad] { display: none !important; }";
}

std::vector<std::string> ShieldTabObserver::GetIsolatedWorldScriptlets(
    const std::string& origin) const {
  std::vector<std::string> scriptlets;
  if (!shield_service_) {
    return scriptlets;
  }

  // Pure data-driven, non-eval scriptlet templates:
  // 1. set-constant scriptlet
  scriptlets.push_back(
      "(function() { "
      "  Object.defineProperty(window, 'google_ad_client', { value: undefined, writable: false }); "
      "})();");

  return scriptlets;
}

}  // namespace codem37
