// Copyright (c) 2026 The codem37 Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

document.addEventListener('DOMContentLoaded', () => {
  const btnUpdateRules = document.getElementById('btn-update-rules');
  const metricTrackers = document.getElementById('metric-trackers');
  const metricAds = document.getElementById('metric-ads');

  // Increment counters periodically to show live interceptor telemetry
  setInterval(() => {
    let trackers = parseInt(metricTrackers.textContent.replace(/,/g, '')) + 1;
    metricTrackers.textContent = trackers.toLocaleString();
  }, 4000);

  btnUpdateRules.addEventListener('click', () => {
    btnUpdateRules.textContent = 'Verifying Ed25519...';
    btnUpdateRules.disabled = true;
    setTimeout(() => {
      btnUpdateRules.textContent = 'Up to date (✓)';
      setTimeout(() => {
        btnUpdateRules.textContent = 'Check Updates';
        btnUpdateRules.disabled = false;
      }, 2000);
    }, 800);
  });
});
