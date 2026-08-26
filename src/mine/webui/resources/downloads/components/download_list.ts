// Copyright (c) 2026 The codem37 Authors. All rights reserved.
// Lit element component managing the push-reconciled download item list.

import {html, css, LitElement} from 'chrome://resources/mwc/lit/index.js';
import {DownloadSnapshot} from '../fetcher.mojom-webui.js';
import {downloadsMojoClient} from '../mojo_client.js';
import './download_item.js';

export class DownloadList extends LitElement {
  static override get styles() {
    return css`
      :host {
        display: block;
      }
      .empty-state {
        text-align: center;
        padding: 48px;
        color: var(--c37-text-secondary, #8b949e);
      }
    `;
  }

  static override get properties() {
    return {
      downloads: {type: Array},
    };
  }

  downloads: DownloadSnapshot[] = [];

  override async connectedCallback() {
    super.connectedCallback();
    // 1. Initial snapshot
    const res = await downloadsMojoClient.getSnapshot();
    this.downloads = res.downloads || [];

    // 2. Register push observer (no polling loop)
    downloadsMojoClient.observeDownloads(
      (created) => {
        this.downloads = [created, ...this.downloads.filter(d => d.id !== created.id)];
      },
      (updated) => {
        this.downloads = this.downloads.map(d => d.id === updated.id ? updated : d);
      },
      (destroyedId) => {
        this.downloads = this.downloads.filter(d => d.id !== destroyedId);
      }
    );
  }

  override render() {
    if (this.downloads.length === 0) {
      return html`<div class="empty-state">No downloads yet.</div>`;
    }

    return html`
      <div>
        ${this.downloads.map(d => html`<download-item .download=${d}></download-item>`)}
      </div>
    `;
  }
}

customElements.define('download-list', DownloadList);
