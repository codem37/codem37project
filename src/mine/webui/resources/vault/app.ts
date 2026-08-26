// Copyright (c) 2026 The codem37 Authors. All rights reserved.
// Root LitElement application component for chrome://vault.

import {html, css, LitElement} from 'chrome://resources/mwc/lit/index.js';
import {vaultClient} from './mojo_client.js';
import './components/vault_lock_screen.js';
import './components/vault_entry_list.js';
import './components/vault_entry_editor.js';
import './components/vault_password_generator.js';

export class MineVaultApp extends LitElement {
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
        color: var(--c37-accent-purple, #a855f7);
      }
      .lock-btn {
        background: transparent;
        border: 1px solid var(--c37-accent-purple, #a855f7);
        color: var(--c37-accent-purple, #a855f7);
        padding: 6px 14px;
        border-radius: var(--c37-radius-sm, 4px);
        font-weight: 600;
        cursor: pointer;
      }
    `;
  }

  static override get properties() {
    return {
      isUnlocked: {type: Boolean},
    };
  }

  isUnlocked: boolean = false;

  override async connectedCallback() {
    super.connectedCallback();
    const res = await vaultClient.isUnlocked();
    this.isUnlocked = res.isUnlocked;
  }

  onUnlocked() {
    this.isUnlocked = true;
  }

  async onLock() {
    await vaultClient.lock();
    this.isUnlocked = false;
  }

  override render() {
    return html`
      <header class="header">
        <div class="brand-title">codem<span>37</span> // Vault Custody</div>
        ${this.isUnlocked ? html`<button class="lock-btn" @click=${this.onLock}>Lock Vault</button>` : ''}
      </header>

      <main>
        ${!this.isUnlocked ? html`
          <vault-lock-screen @vault-unlocked=${this.onUnlocked}></vault-lock-screen>
        ` : html`
          <vault-password-generator></vault-password-generator>
          <vault-entry-editor @entry-added=${() => this.shadowRoot?.querySelector('vault-entry-list')?.loadEntries()}></vault-entry-editor>
          <vault-entry-list></vault-entry-list>
        `}
      </main>
    `;
  }
}

customElements.define('mine-vault-app', MineVaultApp);
