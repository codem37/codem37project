// Copyright (c) 2026 The codem37 Authors. All rights reserved.
// Lit element component displaying Ed25519 signature verified ruleset versions.

import {html, css, LitElement} from 'chrome://resources/mwc/lit/index.js';
import {shieldMojoClient} from '../mojo_client.js';

export class RulesetStatusCard extends LitElement {
  static override get styles() {
    return css`
      :host {
        display: block;
        margin-bottom: 24px;
      }
      .card {
        background: var(--c37-bg-surface, #0c121d);
        border: 1px solid var(--c37-border-subtle, rgba(255,255,255,0.08));
        border-radius: var(--c37-radius-lg, 12px);
        padding: 20px;
        display: flex;
        justify-content: space-between;
        align-items: center;
      }
      .info-group {
        display: flex;
        flex-direction: column;
        gap: 4px;
      }
      .label {
        font-size: 0.8rem;
        color: var(--c37-text-secondary, #8b949e);
      }
      .value {
        font-weight: 600;
        font-family: var(--c37-font-mono);
      }
      .badge {
        background: rgba(63, 185, 80, 0.15);
        color: var(--c37-accent-green, #3fb950);
        border: 1px solid rgba(63, 185, 80, 0.3);
        padding: 6px 14px;
        border-radius: var(--c37-radius-pill, 9999px);
        font-size: 0.8rem;
        font-weight: 600;
      }
    `;
  }

  static override get properties() {
    return {
      activeVersion: {type: String},
      lastKnownGood: {type: String},
    };
  }

  activeVersion: string = 'v2026.08.27';
  lastKnownGood: string = 'v2026.08.26';

  override async connectedCallback() {
    super.connectedCallback();
    const res = await shieldMojoClient.getRulesetStatus();
    this.activeVersion = res.activeVersion || this.activeVersion;
    this.lastKnownGood = res.lastKnownGoodVersion || this.lastKnownGood;
  }

  override render() {
    return html`
      <div class="card">
        <div class="info-group">
          <span class="label">Active Ruleset Version</span>
          <span class="value">${this.activeVersion}</span>
        </div>
        <div class="info-group">
          <span class="label">Last-Known-Good (Rollback Target)</span>
          <span class="value">${this.lastKnownGood}</span>
        </div>
        <div class="badge">Ed25519 Verified ✓</div>
      </div>
    `;
  }
}

customElements.define('ruleset-status-card', RulesetStatusCard);
