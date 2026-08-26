// Copyright (c) 2026 The codem37 Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "src/mine/protocols/mine_protocol_handler.h"

#include "testing/gtest/include/gtest/gtest.h"
#include "url/gurl.h"

namespace codem37 {
namespace {

TEST(MineProtocolHandlerTest, IdentifiesMineSchemeCorrectly) {
  EXPECT_TRUE(MineProtocolHandler::IsMineScheme(GURL("codem37://vault")));
  EXPECT_TRUE(MineProtocolHandler::IsMineScheme(GURL("codem37://shield")));
  EXPECT_TRUE(MineProtocolHandler::IsMineScheme(GURL("codem37://settings")));

  EXPECT_FALSE(MineProtocolHandler::IsMineScheme(GURL("chrome://vault")));
  EXPECT_FALSE(MineProtocolHandler::IsMineScheme(GURL("https://codem37.org")));
}

TEST(MineProtocolHandlerTest, TransformsToInternalWebUI) {
  EXPECT_EQ(MineProtocolHandler::TransformToInternalWebUI(GURL("codem37://vault")),
            GURL("chrome://vault"));
  EXPECT_EQ(MineProtocolHandler::TransformToInternalWebUI(GURL("codem37://shield")),
            GURL("chrome://shield"));
  EXPECT_EQ(MineProtocolHandler::TransformToInternalWebUI(GURL("codem37://settings")),
            GURL("chrome://mine-settings"));
}

}  // namespace
}  // namespace codem37
