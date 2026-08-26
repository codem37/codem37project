// Copyright (c) 2026 The codem37 Authors. All rights reserved.
// WebUI browser test for chrome://shield.

import {assertEquals, assertTrue} from 'chrome://webui-test/chai_assert.js';
import {shieldMojoClient} from './mojo_client.js';

suite('MineShieldWebUITest', () => {
  test('RendersShieldAppWithoutErrors', () => {
    const app = document.createElement('mine-shield-app');
    document.body.appendChild(app);
    assertTrue(!!app);
  });

  test('GetSubscriptionsReturnsDefaultCoreLists', async () => {
    const res = await shieldMojoClient.getSubscriptions();
    assertTrue(res.subscriptions.length >= 2);
  });

  test('GetRulesetStatusSurfacesRollbackState', async () => {
    const res = await shieldMojoClient.getRulesetStatus();
    assertTrue(res.activeVersion.length > 0);
    assertTrue(res.lastKnownGoodVersion.length > 0);
  });
});
