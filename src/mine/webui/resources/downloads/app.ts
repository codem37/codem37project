// Copyright (c) 2026 The codem37 Authors. All rights reserved.
// Root LitElement application component for chrome://downloads.

import {html, css, LitElement} from 'chrome://resources/mwc/lit/index.js';
import './components/download_start_dialog.js';
import './components/download_list.js';

export class MineDownloadsApp extends LitElement {
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
        background: rgba(168, 85, 247, 0.15);
        color: var(--c37-accent-purple, #a855f7);
        border: 1px solid rgba(168, 85, 247, 0.3);
      }
    `;
  }

  override render() {
    return html`
      <header class="header">
        <div class="brand-title">codem<span>37</span> // Downloads</div>
        <div class="badge">Segmented Fetch Engine Active</div>
      </header>

      <main>
        <download-start-dialog></download-start-dialog>
        <download-list></download-list>
      </main>
    `;
  }
}

customElements.define('mine-downloads-app', MineDownloadsApp);
