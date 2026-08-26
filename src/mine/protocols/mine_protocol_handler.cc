// Copyright (c) 2026 The codem37 Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "src/mine/protocols/mine_protocol_handler.h"

namespace codem37 {

namespace {

constexpr char kMineScheme[] = "codem37";

}  // namespace

bool MineProtocolHandler::IsMineScheme(const GURL& url) {
  return url.is_valid() && url.scheme() == kMineScheme;
}

GURL MineProtocolHandler::TransformToInternalWebUI(const GURL& url) {
  if (!IsMineScheme(url)) {
    return GURL();
  }

  std::string host = url.host();
  if (host == "vault") {
    return GURL("chrome://vault");
  } else if (host == "shield") {
    return GURL("chrome://shield");
  } else if (host == "settings") {
    return GURL("chrome://mine-settings");
  }

  return GURL("chrome://mine-settings");
}

}  // namespace codem37
