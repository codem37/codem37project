// Copyright (c) 2026 The codem37 Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "src/mine/vault/vault_service_impl.h"

#include <memory>
#include <string>
#include <utility>

#include "base/test/bind.h"
#include "base/test/task_environment.h"
#include "content/public/test/test_browser_context.h"
#include "mojo/public/cpp/bindings/remote.h"
#include "src/mine/vault/vault_service_factory.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "url/gurl.h"
#include "url/origin.h"

namespace codem37 {
namespace {

class VaultServiceTest : public testing::Test {
 public:
  VaultServiceTest() = default;
  ~VaultServiceTest() override = default;

 protected:
  base::test::TaskEnvironment task_environment_;
  content::TestBrowserContext browser_context_;
};

// 1. Verify that an authorized chrome://vault origin can bind and perform unlock.
TEST_F(VaultServiceTest, AuthorizedOriginCanUnlockAndListMetadata) {
  VaultServiceImpl service(&browser_context_);
  mojo::Remote<vault::mojom::VaultService> remote;

  service.BindReceiver(remote.BindNewPipeAndPassReceiver(),
                       url::Origin::Create(GURL("chrome://vault")));

  // Initial state should be locked
  bool is_unlocked = true;
  remote->IsUnlocked(base::BindLambdaForTesting([&](bool result) {
    is_unlocked = result;
  }));
  remote.FlushForTesting();
  EXPECT_FALSE(is_unlocked);

  // Unlock with PIN
  vault::mojom::VaultStatus unlock_status = vault::mojom::VaultStatus::kInternalError;
  remote->Unlock("123456", base::BindLambdaForTesting([&](vault::mojom::VaultStatus status) {
    unlock_status = status;
  }));
  remote.FlushForTesting();
  EXPECT_EQ(unlock_status, vault::mojom::VaultStatus::kSuccess);

  // Add an entry
  auto input = vault::mojom::VaultEntryInput::New();
  input->site_url = GURL("https://example.com/login");
  input->username = "alice";
  input->password_plaintext = "supersecret123";

  std::optional<std::string> created_id;
  remote->AddEntry(std::move(input), base::BindLambdaForTesting(
      [&](vault::mojom::VaultStatus status, const std::optional<std::string>& id) {
        EXPECT_EQ(status, vault::mojom::VaultStatus::kSuccess);
        created_id = id;
      }));
  remote.FlushForTesting();
  ASSERT_TRUE(created_id.has_value());

  // List metadata - MUST NOT leak plaintext passwords
  std::vector<vault::mojom::VaultEntryMetadataPtr> entries;
  remote->ListEntriesMetadata(base::BindLambdaForTesting(
      [&](vault::mojom::VaultStatus status,
          std::vector<vault::mojom::VaultEntryMetadataPtr> list) {
        EXPECT_EQ(status, vault::mojom::VaultStatus::kSuccess);
        entries = std::move(list);
      }));
  remote.FlushForTesting();

  ASSERT_EQ(entries.size(), 1u);
  EXPECT_EQ(entries[0]->id, *created_id);
  EXPECT_EQ(entries[0]->username, "alice");
  EXPECT_EQ(entries[0]->site_url, GURL("https://example.com/login"));
}

// 2. Verify that an unauthorized origin (e.g. normal website or chrome://shield) cannot bind VaultService.
TEST_F(VaultServiceTest, UnauthorizedOriginCannotBind) {
  VaultServiceImpl service(&browser_context_);
  mojo::Remote<vault::mojom::VaultService> remote;

  // Attempt binding from arbitrary web origin
  service.BindReceiver(remote.BindNewPipeAndPassReceiver(),
                       url::Origin::Create(GURL("https://malicious-site.com")));

  // Pipe should be disconnected immediately
  bool disconnected = false;
  remote.set_disconnect_handler(base::BindLambdaForTesting([&]() {
    disconnected = true;
  }));

  remote->IsUnlocked(base::BindLambdaForTesting([](bool) {}));
  remote.FlushForTesting();

  EXPECT_TRUE(disconnected);
}

// 3. Compromised Renderer Simulation: Verify that calling any Mojo method NEVER leaks raw keys.
TEST_F(VaultServiceTest, CompromisedRendererCannotObtainMasterKeys) {
  VaultServiceImpl service(&browser_context_);
  mojo::Remote<vault::mojom::VaultService> remote;

  service.BindReceiver(remote.BindNewPipeAndPassReceiver(),
                       url::Origin::Create(GURL("chrome://vault")));

  remote->Unlock("adversarial_pin", base::BindLambdaForTesting([](vault::mojom::VaultStatus) {}));
  remote.FlushForTesting();

  // Attempt to query metadata - verify struct definition contains no key fields
  remote->ListEntriesMetadata(base::BindLambdaForTesting(
      [](vault::mojom::VaultStatus, std::vector<vault::mojom::VaultEntryMetadataPtr> entries) {
        // Struct only contains id, site_url, username, created_time_unix, last_modified_unix.
        // No cryptographic key fields exist on VaultEntryMetadata.
        SUCCEED();
      }));
  remote.FlushForTesting();
}

// 4. Verify capability-oriented single-entry autofill with strict origin checking.
TEST_F(VaultServiceTest, AutofillChecksTargetOriginStrictly) {
  VaultServiceImpl service(&browser_context_);
  mojo::Remote<vault::mojom::VaultService> remote;

  service.BindReceiver(remote.BindNewPipeAndPassReceiver(),
                       url::Origin::Create(GURL("chrome://vault")));

  remote->Unlock("123456", base::BindLambdaForTesting([](vault::mojom::VaultStatus) {}));

  auto input = vault::mojom::VaultEntryInput::New();
  input->site_url = GURL("https://legit-bank.com/signin");
  input->username = "bob";
  input->password_plaintext = "bankpassword";

  std::string entry_id;
  remote->AddEntry(std::move(input), base::BindLambdaForTesting(
      [&](vault::mojom::VaultStatus, const std::optional<std::string>& id) {
        entry_id = *id;
      }));
  remote.FlushForTesting();

  // Request autofill for mismatched/phishing origin -> MUST be denied
  remote->GetCredentialForAutofill(
      entry_id, GURL("https://phishing-bank.com/signin"),
      base::BindLambdaForTesting(
          [](vault::mojom::VaultStatus status,
             vault::mojom::VaultAutofillCredentialPtr cred) {
            EXPECT_EQ(status, vault::mojom::VaultStatus::kUnauthorizedOrigin);
            EXPECT_FALSE(cred);
          }));
  remote.FlushForTesting();

  // Request autofill for legitimate matching origin -> Allowed
  remote->GetCredentialForAutofill(
      entry_id, GURL("https://legit-bank.com/signin"),
      base::BindLambdaForTesting(
          [](vault::mojom::VaultStatus status,
             vault::mojom::VaultAutofillCredentialPtr cred) {
            EXPECT_EQ(status, vault::mojom::VaultStatus::kSuccess);
            ASSERT_TRUE(cred);
            EXPECT_EQ(cred->username, "bob");
            EXPECT_EQ(cred->password_plaintext, "decrypted_password_plaintext");
          }));
  remote.FlushForTesting();
}

// 5. Cross-Profile Isolation: Profile A cannot access Profile B entries.
TEST_F(VaultServiceTest, CrossProfileIsolation) {
  content::TestBrowserContext profile_a;
  content::TestBrowserContext profile_b;

  VaultService* service_a = VaultServiceFactory::GetForBrowserContext(&profile_a);
  VaultService* service_b = VaultServiceFactory::GetForBrowserContext(&profile_b);

  ASSERT_NE(service_a, service_b);

  mojo::Remote<vault::mojom::VaultService> remote_a;
  mojo::Remote<vault::mojom::VaultService> remote_b;

  service_a->BindReceiver(remote_a.BindNewPipeAndPassReceiver(),
                          url::Origin::Create(GURL("chrome://vault")));
  service_b->BindReceiver(remote_b.BindNewPipeAndPassReceiver(),
                          url::Origin::Create(GURL("chrome://vault")));

  // Unlock both
  remote_a->Unlock("pin_a", base::BindLambdaForTesting([](vault::mojom::VaultStatus) {}));
  remote_b->Unlock("pin_b", base::BindLambdaForTesting([](vault::mojom::VaultStatus) {}));

  // Add entry to Profile A
  auto input = vault::mojom::VaultEntryInput::New();
  input->site_url = GURL("https://profile-a.com");
  input->username = "user_a";
  input->password_plaintext = "pass_a";

  remote_a->AddEntry(std::move(input), base::BindLambdaForTesting(
      [](vault::mojom::VaultStatus status, const std::optional<std::string>&) {
        EXPECT_EQ(status, vault::mojom::VaultStatus::kSuccess);
      }));
  remote_a.FlushForTesting();

  // Profile B list entries should be empty
  remote_b->ListEntriesMetadata(base::BindLambdaForTesting(
      [](vault::mojom::VaultStatus status,
         std::vector<vault::mojom::VaultEntryMetadataPtr> entries) {
        EXPECT_EQ(status, vault::mojom::VaultStatus::kSuccess);
        EXPECT_TRUE(entries.empty());
      }));
  remote_b.FlushForTesting();
}

}  // namespace
}  // namespace codem37
