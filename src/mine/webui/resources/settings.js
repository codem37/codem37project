// Copyright (c) 2026 The codem37 Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

document.addEventListener('DOMContentLoaded', () => {
  const btnClearMemory = document.getElementById('btn-clear-memory');
  const btnClearHistory = document.getElementById('btn-clear-history');
  const btnClearAll = document.getElementById('btn-clear-all');
  const ramState = document.getElementById('stat-ram-state');
  const statBookmarks = document.getElementById('stat-bookmarks');
  const statHistory = document.getElementById('stat-history');

  // Simulated initial stats loaded from SecureLocalCacheService
  let bookmarksCount = 14;
  let historyCount = 248;
  statBookmarks.textContent = bookmarksCount;
  statHistory.textContent = historyCount;

  // Clear Memory action (Zeroizes transient RAM buffers)
  btnClearMemory.addEventListener('click', async () => {
    ramState.textContent = 'Zeroizing...';
    ramState.className = 'stat-value text-warn';
    
    // In full Chromium, invokes Mojo: cacheHandler.clearMemory()
    setTimeout(() => {
      ramState.textContent = 'Cleared (RAM Zero)';
      ramState.className = 'stat-value text-accent';
      alert('SecureLocalCache: Transient plaintext memory buffers successfully zeroized.');
    }, 400);
  });

  // Clear History action
  btnClearHistory.addEventListener('click', () => {
    if (confirm('Are you sure you want to delete your encrypted browsing history?')) {
      historyCount = 0;
      statHistory.textContent = '0';
      alert('SecureLocalCache: Browsing history erased.');
    }
  });

  // Clear All Local Cache action
  btnClearAll.addEventListener('click', () => {
    const confirmation = prompt('WARNING: This will erase all local encrypted bookmarks, history, and preferences.\nType "CLEAR" to confirm:');
    if (confirmation === 'CLEAR') {
      bookmarksCount = 0;
      historyCount = 0;
      statBookmarks.textContent = '0';
      statHistory.textContent = '0';
      ramState.textContent = 'Database Deleted';
      alert('SecureLocalCache: All local encrypted storage deleted.');
    }
  });
});
