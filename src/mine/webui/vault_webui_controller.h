// Copyright (c) 2026 The codem37 Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CODEM37_SRC_MINE_WEBUI_VAULT_WEBUI_CONTROLLER_H_
#define CODEM37_SRC_MINE_WEBUI_VAULT_WEBUI_CONTROLLER_H_

#include "content/public/browser/web_ui_controller.h"
#include "mojo/public/cpp/bindings/pending_receiver.h"
#include "src/mine/vault/mojom/vault.mojom.h"

namespace content {
class WebUI;
}

namespace codem37 {

// Controller for chrome://vault.
class VaultWebUIController : public content::WebUIController {
 public:
  explicit VaultWebUIController(content::WebUI* web_ui);
  ~VaultWebUIController() override;

  VaultWebUIController(const VaultWebUIController&) = delete;
  VaultWebUIController& operator=(const VaultWebUIController&) = delete;

  // Binds the VaultService Mojo interface for chrome://vault renderer.
  void BindInterface(mojo::PendingReceiver<vault::mojom::VaultService> receiver);

 private:
  WEB_UI_CONTROLLER_TYPE_DECL();
};

}  // namespace codem37

#endif  // CODEM37_SRC_MINE_WEBUI_VAULT_WEBUI_CONTROLLER_H_
