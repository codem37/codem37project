// Copyright (c) 2026 The codem37 Authors. All rights reserved.
// Lit element component for requesting strong passwords generated in the browser process.

import {html, css, LitElement} from 'chrome://resources/mwc/lit/index.js';
import {vaultClient} from '../mojo_client.js';

export class VaultPasswordGenerator extends LitElement {
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
      .row {
        display: flex;
        gap: 12px;
        align-items: center;
        margin-top: 12px;
      }
      .generated-box {
        flex: 1;
        background: rgba(0,0,0,0.4);
        border: 1px solid var(--c37-border-glass, rgba(0,240,255,0.18));
        padding: 10px 14px;
        border-radius: var(--c37-radius-md, 8px);
        font-family: var(--c37-font-mono);
        color: var(--c37-accent-cyan, #00f0ff);
        letter-spacing: 1px;
      }
      button {
        background: transparent;
        border: 1px solid var(--c37-border-glass, rgba(0,240,255,0.18));
        color: #fff;
        padding: 10px 16px;
        border-radius: var(--c37-radius-md, 8px);
        font-weight: 600;
        cursor: pointer;
      }
      button:hover {
        background: rgba(0,240,255,0.1);
      }
    `;
  }

  static override get properties() {
    return {
      generatedPassword: {type: String},
    };
  }

  generatedPassword: string = '••••••••••••••••••••';

  async onGenerate() {
    // Generated via browser-process CSPRNG, UI never generates entropy client-side
    const res = await vaultClient.generatePassword(20, true);
    this.generatedPassword = res.password;
  }

  async onCopy() {
    if (this.generatedPassword.includes('•')) return;
    await navigator.clipboard.writeText(this.generatedPassword);
    alert('Copied secure password to clipboard.');
  }

  override render() {
    return html`
      <div class="card">
        <h3>Cryptographic Password Generator</h3>
        <div class="row">
          <div class="generated-box">${this.generatedPassword}</div>
          <button @click=${this.onGenerate}>Generate (20 chars)</button>
          <button @click=${this.onCopy}>Copy</button>
        </div>
      </div>
    `;
  }
}

customElements.define('vault-password-generator', VaultPasswordGenerator);
