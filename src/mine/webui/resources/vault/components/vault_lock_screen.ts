// Copyright (c) 2026 The codem37 Authors. All rights reserved.
// Lit element component for PIN/WebAuthn vault unlock.

import {html, css, LitElement} from 'chrome://resources/mwc/lit/index.js';
import {vaultClient} from '../mojo_client.js';

export class VaultLockScreen extends LitElement {
  static override get styles() {
    return css`
      :host {
        display: block;
        max-width: 480px;
        margin: 60px auto;
      }
      .card {
        background: var(--c37-bg-glass, rgba(12,18,29,0.72));
        border: 1px solid var(--c37-border-glass, rgba(0,240,255,0.18));
        border-radius: var(--c37-radius-lg, 12px);
        padding: 36px;
        text-align: center;
      }
      .icon {
        font-size: 2.8rem;
        margin-bottom: 16px;
      }
      h2 {
        margin-bottom: 8px;
      }
      .subtext {
        color: var(--c37-text-secondary, #8b949e);
        font-size: 0.9rem;
        margin-bottom: 24px;
      }
      .form {
        display: flex;
        gap: 12px;
      }
      input {
        flex: 1;
        background: rgba(0,0,0,0.4);
        border: 1px solid var(--c37-border-subtle, rgba(255,255,255,0.08));
        color: #fff;
        padding: 12px 16px;
        border-radius: var(--c37-radius-md, 8px);
        font-size: 1rem;
        outline: none;
      }
      input:focus {
        border-color: var(--c37-accent-purple, #a855f7);
      }
      button {
        background: var(--c37-accent-purple, #a855f7);
        color: #fff;
        font-weight: 700;
        border: none;
        border-radius: var(--c37-radius-md, 8px);
        padding: 12px 20px;
        cursor: pointer;
      }
      .error {
        margin-top: 12px;
        color: var(--c37-accent-danger, #f85149);
        font-size: 0.85rem;
      }
    `;
  }

  static override get properties() {
    return {
      pin: {type: String},
      errorMessage: {type: String},
    };
  }

  pin: string = '';
  errorMessage: string = '';

  async onUnlock() {
    if (!this.pin) return;
    this.errorMessage = '';
    const res = await vaultClient.unlock(this.pin);
    if (res.status === 0) { // VaultStatus.kSuccess
      this.pin = '';
      this.dispatchEvent(new CustomEvent('vault-unlocked', {bubbles: true, composed: true}));
    } else {
      this.errorMessage = 'Invalid PIN or hardware enclave authentication failed.';
    }
  }

  override render() {
    return html`
      <div class="card">
        <div class="icon">🔒</div>
        <h2>Hardware-Enclave Protected Vault</h2>
        <div class="subtext">Enter your Master PIN or authenticate via WebAuthn to decrypt credentials.</div>

        <div class="form">
          <input type="password" placeholder="Master PIN" .value=${this.pin} @input=${(e: any) => this.pin = e.target.value} @keydown=${(e: any) => e.key === 'Enter' && this.onUnlock()}>
          <button @click=${this.onUnlock}>Unlock</button>
        </div>

        ${this.errorMessage ? html`<div class="error">${this.errorMessage}</div>` : ''}
      </div>
    `;
  }
}

customElements.define('vault-lock-screen', VaultLockScreen);
