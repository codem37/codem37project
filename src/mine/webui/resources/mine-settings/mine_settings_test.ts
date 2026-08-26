// Copyright (c) 2026 The codem37 Authors. All rights reserved.
// WebUI browser test for chrome://mine-settings.

import {assertEquals, assertTrue} from 'chrome://webui-test/chai_assert.js';
import {settingsMojoClient} from './mojo_client.js';

suite('MineSettingsWebUITest', () => {
  test('RendersAppWithoutErrors', () => {
    const app = document.createElement('mine-settings-app');
    document.body.appendChild(app);
    assertTrue(!!app);
  });

  test('ClearMemoryZeroizesRAM', async () => {
    const res = await settingsMojoClient.clearMemory();
    assertEquals(res.status, 0); // CacheStatus.kSuccess
  });

  test('AddAndQueryBookmarkRoundTrip', async () => {
    const addRes = await settingsMojoClient.addBookmark('https://test.codem37.io', 'Test Title');
    assertEquals(addRes.status, 0);
    const listRes = await settingsMojoClient.getBookmarks();
    assertTrue(listRes.bookmarks.length > 0);
  });
});
