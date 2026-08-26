// Copyright (c) 2026 The codem37 Authors. All rights reserved.
// Root LitElement application component for chrome://shield.

import {html, css, LitElement} from 'chrome://resources/mwc/lit/index.js';
import './components/telemetry_panel.js';
import './components/ruleset_status_card.js';
import './components/subscription_list.js';
import './components/site_settings_panel.js';

export class MineShieldApp extends LitElement {
  static override get styles() {
    return css`
      :host {
        display: block;
        max-width: 900px;
        margin: 0 auto;
        padding: 40px 24px;
      }
      .header {
        display: flex;
        justify-content: space-between;
        align-items: center;
        margin-bottom: 32px;
        border-bottom: 1px solid var(--c37-border-subtle, rgba(255,255,255,0.08));
        padding-bottom: 16px;
      }
      .brand-title {
        font-size: 1.5rem;
        font-weight: 700;
      }
      .brand-title span {
        color: var(--c37-accent-cyan, #00f0ff);
      }
      .badge {
        font-size: 0.75rem;
        padding: 4px 10px;
        border-radius: var(--c37-radius-pill, 9999px);
        background: rgba(0, 240, 255, 0.12);
        color: var(--c37-accent-cyan, #00f0ff);
        border: 1px solid rgba(0, 240, 255, 0.3);
      }
    `;
  }

  override render() {
    return html`
      <header class="header">
        <div class="brand-title">codem<span>37</span> // Shield Protection</div>
        <div class="badge">adblock-rust Engine</div>
      </header>

      <main>
        <telemetry-panel></telemetry-panel>
        <ruleset-status-card></ruleset-status-card>
        <subscription-list></subscription-list>
        <site-settings-panel></site-settings-panel>
      </main>
    `;
  }
}

customElements.define('mine-shield-app', MineShieldApp);
