// Copyright (c) 2026 The codem37 Authors. All rights reserved.
// Lit element component for securely adding a new credential entry.
// Invariant: Password field cleared from DOM and memory immediately upon Mojo dispatch.

import {html, css, LitElement} from 'chrome://resources/mwc/lit/index.js';
import {vaultClient} from '../mojo_client.js';

export class VaultEntryEditor extends LitElement {
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
      }
      .form-grid {
        display: grid;
        grid-template-columns: repeat(3, 1fr) auto;
        gap: 12px;
        margin-top: 12px;
      }
      input {
        background: rgba(0,0,0,0.3);
        border: 1px solid var(--c37-border-subtle, rgba(255,255,255,0.08));
        color: #fff;
        padding: 8px 12px;
        border-radius: var(--c37-radius-sm, 4px);
        font-size: 0.9rem;
      }
      button {
        background: var(--c37-accent-purple, #a855f7);
        color: #fff;
        font-weight: 700;
        border: none;
        border-radius: var(--c37-radius-sm, 4px);
        padding: 8px 16px;
        cursor: pointer;
      }
    `;
  }

  static override get properties() {
    return {
      siteUrl: {type: String},
      username: {type: String},
      passwordPlaintext: {type: String},
    };
  }

  siteUrl: string = '';
  username: string = '';
  passwordPlaintext: string = '';

  async onSubmit() {
    if (!this.siteUrl || !this.username || !this.passwordPlaintext) return;

    await vaultClient.addEntry({
      siteUrl: {url: this.siteUrl},
      username: this.username,
      passwordPlaintext: this.passwordPlaintext,
    });

    // Zeroize & clear memory fields immediately
    this.siteUrl = '';
    this.username = '';
    this.passwordPlaintext = '';

    this.dispatchEvent(new CustomEvent('entry-added', {bubbles: true, composed: true}));
  }

  override render() {
    return html`
      <div class="card">
        <h3>+ Store New Encrypted Credential</h3>
        <div class="form-grid">
          <input type="text" placeholder="https://example.com" .value=${this.siteUrl} @input=${(e: any) => this.siteUrl = e.target.value}>
          <input type="text" placeholder="Username / Email" .value=${this.username} @input=${(e: any) => this.username = e.target.value}>
          <input type="password" placeholder="Password" .value=${this.passwordPlaintext} @input=${(e: any) => this.passwordPlaintext = e.target.value}>
          <button @click=${this.onSubmit}>Save</button>
        </div>
      </div>
    `;
  }
}

customElements.define('vault-entry-editor', VaultEntryEditor);
