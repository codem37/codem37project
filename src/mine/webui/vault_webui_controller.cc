// Copyright (c) 2026 The codem37 Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "src/mine/webui/vault_webui_controller.h"

#include <utility>

#include "content/public/browser/browser_context.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_ui.h"
#include "content/public/browser/web_ui_data_source.h"
#include "src/mine/vault/vault_service_factory.h"
#include "url/origin.h"

namespace codem37 {

WEB_UI_CONTROLLER_TYPE_IMPL(VaultWebUIController)

VaultWebUIController::VaultWebUIController(content::WebUI* web_ui)
    : content::WebUIController(web_ui) {
  content::BrowserContext* context =
      web_ui->GetWebContents()->GetBrowserContext();

  // Create WebUIDataSource with strict CSP
  content::WebUIDataSource* source =
      content::WebUIDataSource::CreateAndAdd(context, "vault");

  // Strict CSP configuration: no unsafe-inline, no remote scripts/fonts
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

VaultWebUIController::~VaultWebUIController() = default;

void VaultWebUIController::BindInterface(
    mojo::PendingReceiver<vault::mojom::VaultService> receiver) {
  content::BrowserContext* context =
      web_ui()->GetWebContents()->GetBrowserContext();
  VaultService* service = VaultServiceFactory::GetForBrowserContext(context);
  if (service) {
    service->BindReceiver(std::move(receiver),
                          url::Origin::Create(GURL("chrome://vault")));
  }
}

}  // namespace codem37
