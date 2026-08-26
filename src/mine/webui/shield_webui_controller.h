// Copyright (c) 2026 The codem37 Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CODEM37_SRC_MINE_WEBUI_SHIELD_WEBUI_CONTROLLER_H_
#define CODEM37_SRC_MINE_WEBUI_SHIELD_WEBUI_CONTROLLER_H_

#include "content/public/browser/web_ui_controller.h"
#include "mojo/public/cpp/bindings/pending_receiver.h"
#include "src/mine/shield/mojom/shield.mojom.h"

namespace content {
class WebUI;
}

namespace codem37 {

// Controller for chrome://shield.
class ShieldWebUIController : public content::WebUIController {
 public:
  explicit ShieldWebUIController(content::WebUI* web_ui);
  ~ShieldWebUIController() override;

  ShieldWebUIController(const ShieldWebUIController&) = delete;
  ShieldWebUIController& operator=(const ShieldWebUIController&) = delete;

  // Binds the ShieldService Mojo interface for chrome://shield renderer.
  void BindInterface(mojo::PendingReceiver<shield::mojom::ShieldService> receiver);

 private:
  WEB_UI_CONTROLLER_TYPE_DECL();
};

}  // namespace codem37

#endif  // CODEM37_SRC_MINE_WEBUI_SHIELD_WEBUI_CONTROLLER_H_
