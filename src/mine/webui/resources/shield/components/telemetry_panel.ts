// Copyright (c) 2026 The codem37 Authors. All rights reserved.
// Lit element component displaying aggregate privacy-safe blocking metrics.

import {html, css, LitElement} from 'chrome://resources/mwc/lit/index.js';
import {shieldMojoClient} from '../mojo_client.js';
import {ShieldTelemetry} from '../shield.mojom-webui.js';

export class TelemetryPanel extends LitElement {
  static override get styles() {
    return css`
      :host {
        display: block;
        margin-bottom: 24px;
      }
      .grid {
        display: grid;
        grid-template-columns: repeat(3, 1fr);
        gap: 16px;
      }
      .metric-card {
        background: var(--c37-bg-glass, rgba(12,18,29,0.72));
        border: 1px solid var(--c37-border-glass, rgba(0,240,255,0.18));
        border-radius: var(--c37-radius-lg, 12px);
        padding: 20px;
        display: flex;
        flex-direction: column;
        gap: 6px;
      }
      .title {
        font-size: 0.8rem;
        color: var(--c37-text-secondary, #8b949e);
      }
      .number {
        font-size: 1.8rem;
        font-weight: 700;
        font-family: var(--c37-font-mono);
      }
      .text-cyan { color: var(--c37-accent-cyan, #00f0ff); }
      .text-green { color: var(--c37-accent-green, #3fb950); }
      .text-purple { color: var(--c37-accent-purple, #a855f7); }
    `;
  }

  static override get properties() {
    return {
      telemetry: {type: Object},
    };
  }

  telemetry: ShieldTelemetry = {
    totalRequestsBlocked: 0n,
    trackersBlocked: 0n,
    adsBlocked: 0n,
  };

  override async connectedCallback() {
    super.connectedCallback();
    const res = await shieldMojoClient.getTelemetry();
    this.telemetry = res.telemetry || this.telemetry;
  }

  override render() {
    return html`
      <div class="grid">
        <div class="metric-card">
          <span class="title">Total Intercepted</span>
          <span class="number text-cyan">${Number(this.telemetry.totalRequestsBlocked).toLocaleString()}</span>
        </div>
        <div class="metric-card">
          <span class="title">Surveillance Trackers</span>
          <span class="number text-purple">${Number(this.telemetry.trackersBlocked).toLocaleString()}</span>
        </div>
        <div class="metric-card">
          <span class="title">Advertising Requests</span>
          <span class="number text-green">${Number(this.telemetry.adsBlocked).toLocaleString()}</span>
        </div>
      </div>
    `;
  }
}

customElements.define('telemetry-panel', TelemetryPanel);
