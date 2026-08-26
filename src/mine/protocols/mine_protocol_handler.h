// Copyright (c) 2026 The codem37 Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CODEM37_SRC_MINE_PROTOCOLS_MINE_PROTOCOL_HANDLER_H_
#define CODEM37_SRC_MINE_PROTOCOLS_MINE_PROTOCOL_HANDLER_H_

#include <string>
#include "url/gurl.h"

namespace codem37 {

// Dispatches codem37:// URLs to internal destinations.
class MineProtocolHandler {
 public:
  static bool IsMineScheme(const GURL& url);
  static GURL TransformToInternalWebUI(const GURL& url);
};

}  // namespace codem37

#endif  // CODEM37_SRC_MINE_PROTOCOLS_MINE_PROTOCOL_HANDLER_H_
