// Copyright (c) 2026 The codem37 Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "src/mine/webui/mine_settings_webui_controller.h"

#include "content/public/browser/browser_context.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_ui.h"
#include "content/public/browser/web_ui_data_source.h"

namespace codem37 {

WEB_UI_CONTROLLER_TYPE_IMPL(MineSettingsWebUIController)

MineSettingsWebUIController::MineSettingsWebUIController(content::WebUI* web_ui)
    : content::WebUIController(web_ui) {
  content::BrowserContext* context =
      web_ui->GetWebContents()->GetBrowserContext();

  content::WebUIDataSource* source =
      content::WebUIDataSource::CreateAndAdd(context, "mine-settings");

  source->OverrideContentSecurityPolicy(
      network::mojom::CSPDirectiveName::ScriptSrc,
      "script-src 'self';");
  source->OverrideContentSecurityPolicy(
      network::mojom::CSPDirectiveName::ObjectSrc,
      "object-src 'none';");
  source->OverrideContentSecurityPolicy(
      network::mojom::CSPDirectiveName::FrameAncestors,
      "frame-ancestors 'none';");
}

MineSettingsWebUIController::~MineSettingsWebUIController() = default;

}  // namespace codem37
