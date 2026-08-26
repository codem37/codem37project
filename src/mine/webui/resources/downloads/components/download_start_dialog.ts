// Copyright (c) 2026 The codem37 Authors. All rights reserved.
// Lit element component for manually triggering downloads by URL.

import {html, css, LitElement} from 'chrome://resources/mwc/lit/index.js';
import {downloadsMojoClient} from '../mojo_client.js';

export class DownloadStartDialog extends LitElement {
  static override get styles() {
    return css`
      :host {
        display: block;
        margin-bottom: 24px;
      }
      .dialog-card {
        background: var(--c37-bg-glass, rgba(12,18,29,0.72));
        border: 1px solid var(--c37-border-glass, rgba(0,240,255,0.18));
        border-radius: var(--c37-radius-lg, 12px);
        padding: 20px;
        display: flex;
        gap: 12px;
      }
      input {
        flex: 1;
        background: rgba(0,0,0,0.3);
        border: 1px solid var(--c37-border-subtle, rgba(255,255,255,0.08));
        color: var(--c37-text-primary, #fff);
        padding: 10px 14px;
        border-radius: var(--c37-radius-md, 8px);
        font-size: 0.9rem;
      }
      input:focus {
        border-color: var(--c37-accent-cyan, #00f0ff);
        outline: none;
      }
      button {
        background: var(--c37-accent-cyan, #00f0ff);
        color: #000;
        font-weight: 700;
        border: none;
        border-radius: var(--c37-radius-md, 8px);
        padding: 10px 20px;
        cursor: pointer;
      }
    `;
  }

  static override get properties() {
    return {
      url: {type: String},
    };
  }

  url: string = '';

  async onStart() {
    if (!this.url.trim()) return;
    await downloadsMojoClient.startFromUrl(this.url.trim(), '');
    this.url = '';
  }

  override render() {
    return html`
      <div class="dialog-card">
        <input type="text" placeholder="Paste direct download URL (HTTP/HTTPS)..." .value=${this.url} @input=${(e: any) => this.url = e.target.value}>
        <button @click=${this.onStart}>Start Download</button>
      </div>
    `;
  }
}

customElements.define('download-start-dialog', DownloadStartDialog);
