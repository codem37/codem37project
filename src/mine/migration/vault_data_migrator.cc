// Copyright (c) 2026 The codem37 Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "src/mine/migration/vault_data_migrator.h"

#include "base/files/file_util.h"
#include "base/json/json_reader.h"
#include "base/values.h"

namespace codem37 {

bool VaultDataMigrator::CreateBackup(const base::FilePath& legacy_path,
                                    const base::FilePath& backup_path) {
  return base::CopyFile(legacy_path, backup_path);
}

MigrationResult VaultDataMigrator::MigrateLegacyVault(const base::FilePath& profile_dir) {
  base::FilePath legacy_file = profile_dir.AppendASCII("vault.json");
  base::FilePath backup_file = profile_dir.AppendASCII("vault.json.bak");
  base::FilePath target_db = profile_dir.AppendASCII("vault.db");

  if (!base::PathExists(legacy_file)) {
    return MigrationResult::kNoLegacyDataFound;
  }

  // 1. Mandatory Pre-Migration Backup
  if (!CreateBackup(legacy_file, backup_file)) {
    return MigrationResult::kBackupFailed;
  }

  // 2. Read and parse legacy file
  std::string file_content;
  if (!base::ReadFileToString(legacy_file, &file_content)) {
    return MigrationResult::kParsingFailed;
  }

  auto parsed_json = base::JSONReader::Read(file_content);
  if (!parsed_json || !parsed_json->is_dict()) {
    return MigrationResult::kParsingFailed;
  }

  // 3. Atomically write to temporary native database file
  base::FilePath temp_target = profile_dir.AppendASCII("vault.db.tmp");
  std::string native_header = "C37V\x00\x01\x00\x00" + file_content;

  if (!base::WriteFile(temp_target, native_header)) {
    base::DeleteFile(temp_target);
    return MigrationResult::kWriteFailed;
  }

  // 4. Atomic swap
  if (!base::ReplaceFile(temp_target, target_db, nullptr)) {
    base::DeleteFile(temp_target);
    return MigrationResult::kWriteFailed;
  }

  // 5. Deactivate legacy file safely
  base::DeleteFile(legacy_file);

  return MigrationResult::kSuccess;
}

}  // namespace codem37
