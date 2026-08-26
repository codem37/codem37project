// Copyright (c) 2026 The codem37 Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CODEM37_SRC_MINE_SHIELD_SHIELD_TAB_OBSERVER_H_
#define CODEM37_SRC_MINE_SHIELD_SHIELD_TAB_OBSERVER_H_

#include <string>
#include <vector>

namespace codem37 {

class ShieldService;

// Tab observer that monitors page navigations and injects cosmetic CSS
// and isolated-world scriptlets at document-start before untrusted DOM scripts execute.
class ShieldTabObserver {
 public:
  explicit ShieldTabObserver(ShieldService* shield_service);
  ~ShieldTabObserver();

  ShieldTabObserver(const ShieldTabObserver&) = delete;
  ShieldTabObserver& operator=(const ShieldTabObserver&) = delete;

  // Invoked upon navigation start for a given URL origin.
  void OnDidStartNavigation(const std::string& url_origin);

  // Invoked when DOM is ready to inject cosmetic stylesheet.
  void OnReadyToCommitNavigation(const std::string& url_origin);

  // Evaluates and returns cosmetic CSS for the active origin.
  std::string GetCosmeticStylesheetForOrigin(const std::string& origin) const;

  // Returns scriptlet payloads for isolated-world injection.
  std::vector<std::string> GetIsolatedWorldScriptlets(const std::string& origin) const;

 private:
  ShieldService* shield_service_;
};

}  // namespace codem37

#endif  // CODEM37_SRC_MINE_SHIELD_SHIELD_TAB_OBSERVER_H_
