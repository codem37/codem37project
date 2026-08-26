// Copyright (c) 2026 The codem37 Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/public/test/test_browser_context.h"
#include "content/public/test/test_web_ui.h"
#include "src/mine/webui/mine_settings_webui_controller.h"
#include "src/mine/webui/shield_webui_controller.h"
#include "src/mine/webui/vault_webui_controller.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "url/gurl.h"

namespace codem37 {
namespace {

// Unit tests validating WebUI security parameters and distinct controller types.
TEST(WebUISecurityTest, DistinctControllerTypeDeclarations) {
  content::TestBrowserContext context;
  content::TestWebUI web_ui;

  // Verify that distinct WebUIController subclasses exist per origin
  // and maintain separate controller types.
  VaultWebUIController vault_controller(&web_ui);
  EXPECT_EQ(vault_controller.GetType(), VaultWebUIController::kWebUIControllerType);

  ShieldWebUIController shield_controller(&web_ui);
  EXPECT_EQ(shield_controller.GetType(), ShieldWebUIController::kWebUIControllerType);

  EXPECT_NE(vault_controller.GetType(), shield_controller.GetType());
}

}  // namespace
}  // namespace codem37
