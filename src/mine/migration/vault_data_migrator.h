// Copyright (c) 2026 The codem37 Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CODEM37_SRC_MINE_MIGRATION_VAULT_DATA_MIGRATOR_H_
#define CODEM37_SRC_MINE_MIGRATION_VAULT_DATA_MIGRATOR_H_

#include <string>
#include "base/files/file_path.h"

namespace codem37 {

enum class MigrationResult {
  kSuccess,
  kNoLegacyDataFound,
  kBackupFailed,
  kParsingFailed,
  kWriteFailed,
  kIntegrityCheckFailed,
};

// Transactional migrator from legacy Electron JSON vault to native encrypted database.
class VaultDataMigrator {
 public:
  static MigrationResult MigrateLegacyVault(const base::FilePath& profile_dir);
  static bool CreateBackup(const base::FilePath& legacy_path,
                           const base::FilePath& backup_path);
};

}  // namespace codem37

#endif  // CODEM37_SRC_MINE_MIGRATION_VAULT_DATA_MIGRATOR_H_
