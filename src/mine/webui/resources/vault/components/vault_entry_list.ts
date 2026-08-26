// Copyright (c) 2026 The codem37 Authors. All rights reserved.
// Lit element component displaying credential metadata only.
// Invariant: NEVER renders plaintext passwords or adds client-side password caching.

import {html, css, LitElement} from 'chrome://resources/mwc/lit/index.js';
import {VaultEntryMetadata} from '../vault.mojom-webui.js';
import {vaultClient} from '../mojo_client.js';

export class VaultEntryList extends LitElement {
  static override get styles() {
    return css`
      :host {
        display: block;
      }
      .grid {
        display: grid;
        grid-template-columns: repeat(auto-fill, minmax(280px, 1fr));
        gap: 16px;
      }
      .card {
        background: var(--c37-bg-surface, #0c121d);
        border: 1px solid var(--c37-border-subtle, rgba(255,255,255,0.08));
        border-radius: var(--c37-radius-md, 8px);
        padding: 16px;
        display: flex;
        flex-direction: column;
        gap: 8px;
      }
      .site {
        font-weight: 700;
        color: var(--c37-accent-cyan, #00f0ff);
        font-size: 1rem;
      }
      .user {
        font-size: 0.85rem;
        color: var(--c37-text-secondary, #8b949e);
      }
      .mask {
        font-family: var(--c37-font-mono);
        letter-spacing: 2px;
        color: var(--c37-text-muted, #586069);
      }
      .actions {
        display: flex;
        justify-content: space-between;
        margin-top: 8px;
      }
      button {
        background: transparent;
        border: 1px solid var(--c37-border-subtle, rgba(255,255,255,0.08));
        color: var(--c37-text-primary, #fff);
        padding: 4px 10px;
        border-radius: var(--c37-radius-sm, 4px);
        font-size: 0.8rem;
        cursor: pointer;
      }
      .btn-del {
        color: var(--c37-accent-danger, #f85149);
      }
    `;
  }

  static override get properties() {
    return {
      entries: {type: Array},
    };
  }

  entries: VaultEntryMetadata[] = [];

  override async connectedCallback() {
    super.connectedCallback();
    await this.loadEntries();
  }

  async loadEntries() {
    const res = await vaultClient.listEntries();
    this.entries = res.entries || [];
  }

  async onDelete(id: string) {
    if (confirm('Delete this credential from the secure vault?')) {
      await vaultClient.deleteEntry(id);
      await this.loadEntries();
    }
  }

  override render() {
    if (this.entries.length === 0) {
      return html`<div>No vault credentials stored yet.</div>`;
    }

    return html`
      <div class="grid">
        ${this.entries.map(e => html`
          <div class="card">
            <span class="site">${e.siteUrl.url}</span>
            <span class="user">${e.username}</span>
            <span class="mask">••••••••••••••••</span>
            <div class="actions">
              <button @click=${() => navigator.clipboard.writeText(e.username)}>Copy User</button>
              <button class="btn-del" @click=${() => this.onDelete(e.id)}>Delete</button>
            </div>
          </div>
        `)}
      </div>
    `;
  }
}

customElements.define('vault-entry-list', VaultEntryList);
