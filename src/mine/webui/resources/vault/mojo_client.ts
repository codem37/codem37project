// Copyright (c) 2026 The codem37 Authors. All rights reserved.
// Capability-only Mojo client wrapper for VaultService (chrome://vault).

import {VaultService, VaultServiceRemote, VaultEntryMetadata, VaultEntryInput, VaultStatus} from './vault.mojom-webui.js';

class VaultClient {
  private remote: VaultServiceRemote;

  constructor() {
    this.remote = VaultService.getRemote();
  }

  async isUnlocked(): Promise<{isUnlocked: boolean}> {
    return this.remote.isUnlocked();
  }

  async unlock(pinOrAssertion: string): Promise<{status: VaultStatus}> {
    return this.remote.unlock(pinOrAssertion);
  }

  async lock(): Promise<{status: VaultStatus}> {
    return this.remote.lock();
  }

  // Returns METADATA ONLY - never returns plaintext password
  async listEntries(): Promise<{entries: VaultEntryMetadata[]}> {
    return this.remote.listEntriesMetadata();
  }

  async addEntry(input: VaultEntryInput): Promise<{status: VaultStatus, entryId: string | null}> {
    return this.remote.addEntry(input);
  }

  async deleteEntry(id: string): Promise<{status: VaultStatus}> {
    return this.remote.deleteEntry(id);
  }

  // Entropy sourced strictly from browser-process cryptographic RNG
  async generatePassword(length: number, includeSymbols: boolean): Promise<{password: string}> {
    return this.remote.generatePassword(length, includeSymbols);
  }
}

export const vaultClient = new VaultClient();
