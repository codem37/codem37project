// Copyright (c) 2026 The codem37 Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CODEM37_SRC_MINE_DIAGNOSTICS_COMPATIBILITY_DIAGNOSTICS_H_
#define CODEM37_SRC_MINE_DIAGNOSTICS_COMPATIBILITY_DIAGNOSTICS_H_

#include <string>
#include "url/gurl.h"

namespace codem37 {

struct RawFailureReport {
  GURL failed_url;
  int http_status = 0;
  int net_error_code = 0;
  std::string auth_error_category;
  bool is_cdm_available = false;
};

struct SanitizedFailureReport {
  std::string domain_etld_plus_one;
  int http_status = 0;
  int net_error_code = 0;
  std::string auth_error_category;
  bool is_cdm_available = false;
  std::string browser_version;
};

// Generates privacy-safe sanitized failure diagnostics for opt-in reporting.
class CompatibilityDiagnostics {
 public:
  static SanitizedFailureReport SanitizeReport(const RawFailureReport& raw_report);
  static std::string FormatDrmUnavailableMessage();
};

}  // namespace codem37

#endif  // CODEM37_SRC_MINE_DIAGNOSTICS_COMPATIBILITY_DIAGNOSTICS_H_
