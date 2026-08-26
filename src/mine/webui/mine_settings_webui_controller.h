// Copyright (c) 2026 The codem37 Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CODEM37_SRC_MINE_WEBUI_MINE_SETTINGS_WEBUI_CONTROLLER_H_
#define CODEM37_SRC_MINE_WEBUI_MINE_SETTINGS_WEBUI_CONTROLLER_H_

#include "content/public/browser/web_ui_controller.h"

namespace content {
class WebUI;
}

namespace codem37 {

// Controller for chrome://mine-settings.
// Low-privilege: modifies standard preferences via PrefService, no raw crypto access.
class MineSettingsWebUIController : public content::WebUIController {
 public:
  explicit MineSettingsWebUIController(content::WebUI* web_ui);
  ~MineSettingsWebUIController() override;

  MineSettingsWebUIController(const MineSettingsWebUIController&) = delete;
  MineSettingsWebUIController& operator=(const MineSettingsWebUIController&) = delete;

 private:
  WEB_UI_CONTROLLER_TYPE_DECL();
};

}  // namespace codem37

#endif  // CODEM37_SRC_MINE_WEBUI_MINE_SETTINGS_WEBUI_CONTROLLER_H_
