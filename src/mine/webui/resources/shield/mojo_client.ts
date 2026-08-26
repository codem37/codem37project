// Copyright (c) 2026 The codem37 Authors. All rights reserved.
// Mojo client wrapper for ShieldService (chrome://shield).

import {ShieldService, ShieldServiceRemote, FilterSubscription, ShieldSiteSetting, ShieldTelemetry, ShieldStatus} from './shield.mojom-webui.js';

class ShieldMojoClient {
  private remote: ShieldServiceRemote;

  constructor() {
    this.remote = ShieldService.getRemote();
  }

  // Filter Subscriptions
  async getSubscriptions(): Promise<{subscriptions: FilterSubscription[]}> {
    return this.remote.getSubscriptions();
  }

  async setSubscriptionEnabled(subscriptionId: string, enabled: boolean): Promise<{status: ShieldStatus}> {
    return this.remote.setSubscriptionEnabled(subscriptionId, enabled);
  }

  async addSubscription(title: string, updateUrl: string): Promise<{status: ShieldStatus, id: string | null}> {
    return this.remote.addSubscription(title, {url: updateUrl});
  }

  async removeSubscription(subscriptionId: string): Promise<{status: ShieldStatus}> {
    return this.remote.removeSubscription(subscriptionId);
  }

  // Site Settings
  async getSiteSetting(hostname: string): Promise<{setting: ShieldSiteSetting}> {
    return this.remote.getSiteSetting(hostname);
  }

  async setSiteShieldEnabled(hostname: string, enabled: boolean): Promise<{status: ShieldStatus}> {
    return this.remote.setSiteShieldEnabled(hostname, enabled);
  }

  // Telemetry
  async getTelemetry(): Promise<{telemetry: ShieldTelemetry}> {
    return this.remote.getTelemetry();
  }

  async resetTelemetry(): Promise<{status: ShieldStatus}> {
    return this.remote.resetTelemetry();
  }

  // Ruleset Status & Ed25519 Rollback State
  async getRulesetStatus(): Promise<{activeVersion: string, lastKnownGoodVersion: string, lastCheckUnix: bigint}> {
    return this.remote.getRulesetStatus();
  }
}

export const shieldMojoClient = new ShieldMojoClient();
