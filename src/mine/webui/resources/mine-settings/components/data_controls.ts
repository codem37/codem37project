// Copyright (c) 2026 The codem37 Authors. All rights reserved.
// Lit element component enforcing the two distinct cache-clearing invariants.

import {html, css, LitElement} from 'chrome://resources/mwc/lit/index.js';
import {settingsMojoClient} from '../mojo_client.js';

export class DataControls extends LitElement {
  static override get styles() {
    return css`
      :host {
        display: block;
        margin-top: 24px;
      }
      .card {
        background: var(--c37-bg-glass, rgba(12,18,29,0.72));
        border: 1px solid var(--c37-border-glass, rgba(0,240,255,0.18));
        border-radius: var(--c37-radius-lg, 12px);
        padding: 24px;
      }
      .description {
        color: var(--c37-text-secondary, #8b949e);
        font-size: 0.9rem;
        line-height: 1.5;
        margin-bottom: 20px;
      }
      .actions {
        display: flex;
        gap: 12px;
        flex-wrap: wrap;
      }
      button {
        padding: 10px 18px;
        border-radius: var(--c37-radius-md, 8px);
        font-weight: 600;
        cursor: pointer;
        border: none;
        transition: all 0.2s ease;
      }
      .btn-warn {
        background: rgba(210, 153, 34, 0.15);
        color: var(--c37-accent-warn, #d29922);
        border: 1px solid rgba(210, 153, 34, 0.3);
      }
      .btn-warn:hover {
        background: rgba(210, 153, 34, 0.25);
      }
      .btn-danger {
        background: rgba(248, 81, 73, 0.15);
        color: var(--c37-accent-danger, #f85149);
        border: 1px solid rgba(248, 81, 73, 0.3);
      }
      .btn-danger:hover {
        background: rgba(248, 81, 73, 0.25);
      }
      .btn-danger-solid {
        background: var(--c37-accent-danger, #f85149);
        color: #fff;
      }
      .btn-danger-solid:hover {
        opacity: 0.9;
      }
      .status-text {
        margin-top: 12px;
        font-size: 0.85rem;
        color: var(--c37-accent-cyan, #00f0ff);
      }
    `;
  }

  static override get properties() {
    return {
      statusMessage: {type: String},
    };
  }

  statusMessage: string = '';

  async onClearMemory() {
    this.statusMessage = 'Zeroizing transient RAM plaintext buffers...';
    await settingsMojoClient.clearMemory();
    this.statusMessage = 'RAM plaintext zeroized. On-disk database remains intact.';
  }

  async onClearHistory() {
    if (confirm('Delete local encrypted browsing history?')) {
      await settingsMojoClient.clearHistory();
      this.statusMessage = 'Browsing history deleted from encrypted cache.';
    }
  }

  async onClearAll() {
    const confirmText = prompt('WARNING: This permanently deletes your local encrypted bookmarks, history, and preferences.\nType "CLEAR" to confirm:');
    if (confirmText === 'CLEAR') {
      await settingsMojoClient.clearAllCache();
      this.statusMessage = 'All local encrypted cache and on-disk files wiped.';
    }
  }

  override render() {
    return html`
      <div class="card">
        <h3>Local Data & Memory Zeroization</h3>
        <p class="description">
          codem37 strictly distinguishes between clearing decrypted memory in RAM vs wiping on-disk encrypted database storage.
        </p>

        <div class="actions">
          <button class="btn-warn" @click=${this.onClearMemory}>
            🧹 Clear Memory (Zeroize RAM)
          </button>
          <button class="btn-danger" @click=${this.onClearHistory}>
            🗑️ Clear History
          </button>
          <button class="btn-danger-solid" @click=${this.onClearAll}>
            ⚠️ Clear All Local Cache
          </button>
        </div>

        ${this.statusMessage ? html`<div class="status-text">${this.statusMessage}</div>` : ''}
      </div>
    `;
  }
}

customElements.define('data-controls', DataControls);
