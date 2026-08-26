// Copyright (c) 2026 The codem37 Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "src/mine/diagnostics/compatibility_diagnostics.h"

namespace codem37 {

SanitizedFailureReport CompatibilityDiagnostics::SanitizeReport(
    const RawFailureReport& raw_report) {
  SanitizedFailureReport sanitized;
  // Extract strictly domain / eTLD+1, stripping full path and query tokens
  if (raw_report.failed_url.is_valid()) {
    sanitized.domain_etld_plus_one = raw_report.failed_url.host();
  } else {
    sanitized.domain_etld_plus_one = "invalid-domain";
  }

  sanitized.http_status = raw_report.http_status;
  sanitized.net_error_code = raw_report.net_error_code;
  sanitized.auth_error_category = raw_report.auth_error_category;
  sanitized.is_cdm_available = raw_report.is_cdm_available;
  sanitized.browser_version = "codem37/134.0.6998.88";

  return sanitized;
}

std::string CompatibilityDiagnostics::FormatDrmUnavailableMessage() {
  return "Protected content requires DRM support that is not currently available.";
}

}  // namespace codem37
