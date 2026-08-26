// Copyright (c) 2026 The codem37 Authors. All rights reserved.
// Lit element component managing origin-level shield toggles.

import {html, css, LitElement} from 'chrome://resources/mwc/lit/index.js';
import {shieldMojoClient} from '../mojo_client.js';

export class SiteSettingsPanel extends LitElement {
  static override get styles() {
    return css`
      :host {
        display: block;
      }
      .card {
        background: var(--c37-bg-surface, #0c121d);
        border: 1px solid var(--c37-border-subtle, rgba(255,255,255,0.08));
        border-radius: var(--c37-radius-lg, 12px);
        padding: 24px;
      }
      .row {
        display: flex;
        justify-content: space-between;
        align-items: center;
        margin-top: 16px;
      }
      .subtext {
        font-size: 0.8rem;
        color: var(--c37-text-secondary, #8b949e);
        margin-top: 2px;
      }
      input[type="checkbox"] {
        width: 18px;
        height: 18px;
        accent-color: var(--c37-accent-cyan, #00f0ff);
        cursor: pointer;
      }
    `;
  }

  static override get properties() {
    return {
      currentHost: {type: String},
      shieldEnabled: {type: Boolean},
    };
  }

  currentHost: string = 'example.com';
  shieldEnabled: boolean = true;

  async onToggle() {
    this.shieldEnabled = !this.shieldEnabled;
    await shieldMojoClient.setSiteShieldEnabled(this.currentHost, this.shieldEnabled);
  }

  override render() {
    return html`
      <div class="card">
        <h3>Site Protection Level</h3>
        <div class="row">
          <div>
            <strong>Active Site: ${this.currentHost}</strong>
            <div class="subtext">Blocks third-party trackers, cosmetic ads, and uncloaks CNAME aliases.</div>
          </div>
          <input type="checkbox" .checked=${this.shieldEnabled} @change=${this.onToggle}>
        </div>
      </div>
    `;
  }
}

customElements.define('site-settings-panel', SiteSettingsPanel);
