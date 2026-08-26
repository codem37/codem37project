// Copyright (c) 2026 The codem37 Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "src/mine/migration/vault_data_migrator.h"

#include "base/files/file_util.h"
#include "base/files/scoped_temp_dir.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace codem37 {
namespace {

TEST(VaultDataMigratorTest, MigratesLegacyJsonVaultWithBackup) {
  base::ScopedTempDir temp_dir;
  ASSERT_TRUE(temp_dir.CreateUniqueTempDir());

  base::FilePath profile_dir = temp_dir.GetPath();
  base::FilePath legacy_file = profile_dir.AppendASCII("vault.json");
  base::FilePath backup_file = profile_dir.AppendASCII("vault.json.bak");
  base::FilePath target_db = profile_dir.AppendASCII("vault.db");

  // Create mock legacy Electron JSON vault file
  std::string legacy_data = "{\"version\": 1, \"entries\": [{\"id\": \"1\", \"username\": \"alice\"}]}";
  ASSERT_TRUE(base::WriteFile(legacy_file, legacy_data));

  // Run migration
  MigrationResult result = VaultDataMigrator::MigrateLegacyVault(profile_dir);
  EXPECT_EQ(result, MigrationResult::kSuccess);

  // Verify backup was created
  EXPECT_TRUE(base::PathExists(backup_file));

  // Verify native database file was created
  EXPECT_TRUE(base::PathExists(target_db));

  // Verify legacy file was deactivated
  EXPECT_FALSE(base::PathExists(legacy_file));
}

TEST(VaultDataMigratorTest, HandlesMissingLegacyGracefully) {
  base::ScopedTempDir temp_dir;
  ASSERT_TRUE(temp_dir.CreateUniqueTempDir());

  MigrationResult result = VaultDataMigrator::MigrateLegacyVault(temp_dir.GetPath());
  EXPECT_EQ(result, MigrationResult::kNoLegacyDataFound);
}

}  // namespace
}  // namespace codem37
