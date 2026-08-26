// Copyright (c) 2026 The codem37 Authors. All rights reserved.
// WebUI browser test for chrome://downloads.

import {assertEquals, assertTrue} from 'chrome://webui-test/chai_assert.js';
import {downloadsMojoClient} from './mojo_client.js';

suite('MineDownloadsWebUITest', () => {
  test('RendersDownloadsAppWithoutErrors', () => {
    const app = document.createElement('mine-downloads-app');
    document.body.appendChild(app);
    assertTrue(!!app);
  });

  test('GetSnapshotReturnsList', async () => {
    const res = await downloadsMojoClient.getSnapshot();
    assertTrue(Array.isArray(res.downloads));
  });

  test('StartFromUrlReturnsStatus', async () => {
    const res = await downloadsMojoClient.startFromUrl('https://releases.codem37.org/bundle.tar.gz', 'bundle.tar.gz');
    assertEquals(res.status, 0); // FetcherStatus.kSuccess
  });
});
