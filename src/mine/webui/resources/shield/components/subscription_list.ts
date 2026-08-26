// Copyright (c) 2026 The codem37 Authors. All rights reserved.
// Lit element component managing filter list subscriptions.

import {html, css, LitElement} from 'chrome://resources/mwc/lit/index.js';
import {shieldMojoClient} from '../mojo_client.js';
import {FilterSubscription} from '../shield.mojom-webui.js';

export class SubscriptionList extends LitElement {
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
        padding: 24px;
      }
      .sub-item {
        display: flex;
        justify-content: space-between;
        align-items: center;
        padding: 14px 0;
        border-bottom: 1px solid rgba(255,255,255,0.04);
      }
      .sub-item:last-child {
        border-bottom: none;
      }
      .sub-meta {
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
      subscriptions: {type: Array},
    };
  }

  subscriptions: FilterSubscription[] = [];

  override async connectedCallback() {
    super.connectedCallback();
    const res = await shieldMojoClient.getSubscriptions();
    this.subscriptions = res.subscriptions || [];
  }

  async onToggle(sub: FilterSubscription) {
    const newState = !sub.isEnabled;
    await shieldMojoClient.setSubscriptionEnabled(sub.id, newState);
    sub.isEnabled = newState;
    this.requestUpdate();
  }

  override render() {
    return html`
      <div class="card">
        <h3>Filter List Subscriptions</h3>
        <div>
          ${this.subscriptions.map(s => html`
            <div class="sub-item">
              <div>
                <strong>${s.title}</strong>
                <div class="sub-meta">${Number(s.rulesCount).toLocaleString()} rules</div>
              </div>
              <input type="checkbox" .checked=${s.isEnabled} @change=${() => this.onToggle(s)}>
            </div>
          `)}
        </div>
      </div>
    `;
  }
}

customElements.define('subscription-list', SubscriptionList);
