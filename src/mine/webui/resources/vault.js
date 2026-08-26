// Copyright (c) 2026 The codem37 Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

document.addEventListener('DOMContentLoaded', () => {
  const pinInput = document.getElementById('pin-input');
  const btnSubmitPin = document.getElementById('btn-submit-pin');
  const btnToggleLock = document.getElementById('btn-toggle-lock');
  const lockStatusBadge = document.getElementById('lock-status-badge');
  const lockPanel = document.getElementById('lock-panel');
  const credentialsSection = document.getElementById('credentials-section');

  let isUnlocked = false;

  function setUnlockedState(unlocked) {
    isUnlocked = unlocked;
    if (isUnlocked) {
      lockStatusBadge.textContent = 'UNLOCKED';
      lockStatusBadge.className = 'vault-status-badge unlocked';
      btnToggleLock.textContent = 'Lock Vault';
      lockPanel.classList.add('hidden');
      credentialsSection.classList.remove('hidden');
    } else {
      lockStatusBadge.textContent = 'LOCKED';
      lockStatusBadge.className = 'vault-status-badge';
      btnToggleLock.textContent = 'Unlock Vault';
      lockPanel.classList.remove('hidden');
      credentialsSection.classList.add('hidden');
      pinInput.value = '';
    }
  }

  btnSubmitPin.addEventListener('click', () => {
    if (pinInput.value.length > 0) {
      // In full Chromium: calls Mojo VaultService::UnlockWithPin
      setUnlockedState(true);
    } else {
      alert('Please enter your Master PIN.');
    }
  });

  btnToggleLock.addEventListener('click', () => {
    setUnlockedState(!isUnlocked);
  });

  pinInput.addEventListener('keydown', (e) => {
    if (e.key === 'Enter') {
      btnSubmitPin.click();
    }
  });
});
