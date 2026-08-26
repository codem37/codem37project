// Copyright (c) 2026 The codem37 Authors. All rights reserved.
// WebUI browser test for chrome://vault.

import {assertEquals, assertTrue} from 'chrome://webui-test/chai_assert.js';
import {vaultClient} from './mojo_client.js';

suite('MineVaultWebUITest', () => {
  test('RendersVaultAppWithoutErrors', () => {
    const app = document.createElement('mine-vault-app');
    document.body.appendChild(app);
    assertTrue(!!app);
  });

  test('IsUnlockedReturnsBoolean', async () => {
    const res = await vaultClient.isUnlocked();
    assertTrue(typeof res.isUnlocked === 'boolean');
  });

  test('GeneratePasswordReturnsBrowserEntropy', async () => {
    const res = await vaultClient.generatePassword(16, true);
    assertEquals(res.password.length, 16);
  });
});
