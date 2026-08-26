// Copyright (c) 2026 The codem37 Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CODEM37_SRC_MINE_MIGRATION_SHIELD_DATA_MIGRATOR_H_
#define CODEM37_SRC_MINE_MIGRATION_SHIELD_DATA_MIGRATOR_H_

#include "base/files/file_path.h"

namespace codem37 {

class ShieldDataMigrator {
 public:
  static bool MigrateLegacyShieldSettings(const base::FilePath& profile_dir);
};

}  // namespace codem37

#endif  // CODEM37_SRC_MINE_MIGRATION_SHIELD_DATA_MIGRATOR_H_
