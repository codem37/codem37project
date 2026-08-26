// Copyright (c) 2026 The codem37 Authors. All rights reserved.
// Lit element component representing a single active or completed download item.

import {html, css, LitElement} from 'chrome://resources/mwc/lit/index.js';
import {DownloadSnapshot, DownloadState} from '../fetcher.mojom-webui.js';
import {downloadsMojoClient} from '../mojo_client.js';

export class DownloadItem extends LitElement {
  static override get styles() {
    return css`
      :host {
        display: block;
        margin-bottom: 12px;
      }
      .card {
        background: var(--c37-bg-surface, #0c121d);
        border: 1px solid var(--c37-border-subtle, rgba(255,255,255,0.08));
        border-radius: var(--c37-radius-md, 8px);
        padding: 16px;
        display: flex;
        flex-direction: column;
        gap: 10px;
        transition: border-color 0.2s ease;
      }
      .card:hover {
        border-color: var(--c37-border-glass, rgba(0,240,255,0.18));
      }
      .header-row {
        display: flex;
        justify-content: space-between;
        align-items: center;
      }
      .filename {
        font-weight: 600;
        font-size: 0.95rem;
      }
      .badges {
        display: flex;
        gap: 8px;
      }
      .badge-segmented {
        background: rgba(168, 85, 247, 0.15);
        color: var(--c37-accent-purple, #a855f7);
        border: 1px solid rgba(168, 85, 247, 0.3);
        font-size: 0.75rem;
        padding: 2px 8px;
        border-radius: var(--c37-radius-sm, 4px);
        font-weight: 600;
      }
      .badge-state {
        font-size: 0.75rem;
        padding: 2px 8px;
        border-radius: var(--c37-radius-sm, 4px);
      }
      .progress-bar {
        width: 100%;
        height: 6px;
        background: rgba(255,255,255,0.08);
        border-radius: 3px;
        overflow: hidden;
      }
      .progress-fill {
        height: 100%;
        background: linear-gradient(90deg, var(--c37-accent-cyan, #00f0ff), var(--c37-accent-purple, #a855f7));
        transition: width 0.3s ease;
      }
      .meta-row {
        display: flex;
        justify-content: space-between;
        font-size: 0.8rem;
        color: var(--c37-text-secondary, #8b949e);
      }
      .actions {
        display: flex;
        gap: 8px;
        margin-top: 4px;
      }
      button {
        padding: 6px 12px;
        border-radius: var(--c37-radius-sm, 4px);
        font-size: 0.8rem;
        font-weight: 500;
        cursor: pointer;
        border: 1px solid var(--c37-border-subtle, rgba(255,255,255,0.08));
        background: transparent;
        color: var(--c37-text-primary, #fff);
      }
      button:hover {
        background: rgba(255,255,255,0.05);
      }
    `;
  }

  static override get properties() {
    return {
      download: {type: Object},
    };
  }

  download!: DownloadSnapshot;

  get progressPercent(): number {
    if (!this.download || this.download.totalBytes === 0) return 0;
    return Math.min(100, Math.round((Number(this.download.receivedBytes) / Number(this.download.totalBytes)) * 100));
  }

  async onPause() {
    await downloadsMojoClient.pause(this.download.id);
  }

  async onResume() {
    await downloadsMojoClient.resume(this.download.id);
  }

  async onCancel() {
    await downloadsMojoClient.cancel(this.download.id);
  }

  async onOpenFolder() {
    await downloadsMojoClient.openContainingFolder(this.download.id);
  }

  override render() {
    if (!this.download) return html``;

    return html`
      <div class="card">
        <div class="header-row">
          <span class="filename">${this.download.targetPath}</span>
          <div class="badges">
            ${this.download.isSegmented ? html`<span class="badge-segmented">Segmented (${this.download.segmentCount}x)</span>` : ''}
            <span class="badge-state">${this.download.state === 1 ? 'Downloading' : this.download.state === 2 ? 'Paused' : this.download.state === 3 ? 'Completed' : 'Cancelled'}</span>
          </div>
        </div>

        <div class="progress-bar">
          <div class="progress-fill" style="width: ${this.progressPercent}%"></div>
        </div>

        <div class="meta-row">
          <span>${(Number(this.download.receivedBytes) / (1024 * 1024)).toFixed(1)} MB / ${(Number(this.download.totalBytes) / (1024 * 1024)).toFixed(1)} MB</span>
          <span>${this.download.bytesPerSec ? `${(Number(this.download.bytesPerSec) / (1024 * 1024)).toFixed(2)} MB/s` : ''}</span>
        </div>

        <div class="actions">
          ${this.download.state === 1 ? html`<button @click=${this.onPause}>Pause</button>` : ''}
          ${this.download.state === 2 ? html`<button @click=${this.onResume}>Resume</button>` : ''}
          ${this.download.state === 1 || this.download.state === 2 ? html`<button @click=${this.onCancel}>Cancel</button>` : ''}
          <button @click=${this.onOpenFolder}>Show in folder</button>
        </div>
      </div>
    `;
  }
}

customElements.define('download-item', DownloadItem);
