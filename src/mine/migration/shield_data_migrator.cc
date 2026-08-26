// Copyright (c) 2026 The codem37 Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "src/mine/migration/shield_data_migrator.h"

#include "base/files/file_util.h"

namespace codem37 {

bool ShieldDataMigrator::MigrateLegacyShieldSettings(const base::FilePath& profile_dir) {
  base::FilePath legacy_file = profile_dir.AppendASCII("shield_config.json");
  base::FilePath target_file = profile_dir.AppendASCII("shield_prefs.json");

  if (!base::PathExists(legacy_file)) {
    return true;  // Clean new profile
  }

  // Backup and copy
  base::FilePath backup = profile_dir.AppendASCII("shield_config.json.bak");
  base::CopyFile(legacy_file, backup);

  if (base::CopyFile(legacy_file, target_file)) {
    base::DeleteFile(legacy_file);
    return true;
  }

  return false;
}

}  // namespace codem37
