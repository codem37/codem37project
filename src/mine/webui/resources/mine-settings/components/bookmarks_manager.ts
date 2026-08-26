// Copyright (c) 2026 The codem37 Authors. All rights reserved.
// Lit element component for managing encrypted local bookmarks.

import {html, css, LitElement} from 'chrome://resources/mwc/lit/index.js';
import {settingsMojoClient} from '../mojo_client.js';
import {BookmarkRecord} from '../cache.mojom-webui.js';

export class BookmarksManager extends LitElement {
  static override get styles() {
    return css`
      :host {
        display: block;
        margin-top: 24px;
      }
      .card {
        background: var(--c37-bg-surface, #0c121d);
        border: 1px solid var(--c37-border-subtle, rgba(255,255,255,0.08));
        border-radius: var(--c37-radius-lg, 12px);
        padding: 24px;
      }
      .form-row {
        display: flex;
        gap: 12px;
        margin-top: 16px;
        margin-bottom: 20px;
      }
      input {
        background: rgba(0,0,0,0.3);
        border: 1px solid var(--c37-border-subtle, rgba(255,255,255,0.08));
        color: var(--c37-text-primary, #fff);
        padding: 8px 12px;
        border-radius: var(--c37-radius-sm, 4px);
        font-size: 0.9rem;
      }
      input:focus {
        border-color: var(--c37-accent-cyan, #00f0ff);
        outline: none;
      }
      .btn-add {
        background: var(--c37-accent-cyan, #00f0ff);
        color: #000;
        font-weight: 600;
        border: none;
        border-radius: var(--c37-radius-sm, 4px);
        padding: 8px 16px;
        cursor: pointer;
      }
      .bookmark-list {
        display: flex;
        flex-direction: column;
        gap: 8px;
      }
      .bookmark-item {
        display: flex;
        justify-content: space-between;
        align-items: center;
        padding: 10px 14px;
        background: rgba(255,255,255,0.02);
        border-radius: var(--c37-radius-sm, 4px);
      }
      .url {
        font-size: 0.8rem;
        color: var(--c37-text-secondary, #8b949e);
      }
      .btn-del {
        background: transparent;
        color: var(--c37-accent-danger, #f85149);
        border: none;
        cursor: pointer;
      }
    `;
  }

  static override get properties() {
    return {
      bookmarks: {type: Array},
      newUrl: {type: String},
      newTitle: {type: String},
    };
  }

  bookmarks: BookmarkRecord[] = [];
  newUrl: string = '';
  newTitle: string = '';

  override async connectedCallback() {
    super.connectedCallback();
    await this.loadBookmarks();
  }

  async loadBookmarks() {
    const res = await settingsMojoClient.getBookmarks();
    this.bookmarks = res.bookmarks || [];
  }

  async onAdd() {
    if (!this.newUrl) return;
    await settingsMojoClient.addBookmark(this.newUrl, this.newTitle || this.newUrl);
    this.newUrl = '';
    this.newTitle = '';
    await this.loadBookmarks();
  }

  async onDelete(id: string) {
    await settingsMojoClient.deleteBookmark(id);
    await this.loadBookmarks();
  }

  override render() {
    return html`
      <div class="card">
        <h3>Encrypted Bookmarks</h3>
        
        <div class="form-row">
          <input type="text" placeholder="Title" .value=${this.newTitle} @input=${(e: any) => this.newTitle = e.target.value}>
          <input type="text" placeholder="https://..." .value=${this.newUrl} @input=${(e: any) => this.newUrl = e.target.value}>
          <button class="btn-add" @click=${this.onAdd}>+ Add Bookmark</button>
        </div>

        <div class="bookmark-list">
          ${this.bookmarks.map(b => html`
            <div class="bookmark-item">
              <div>
                <strong>${b.title}</strong>
                <div class="url">${b.url.url}</div>
              </div>
              <button class="btn-del" @click=${() => this.onDelete(b.id)}>Delete</button>
            </div>
          `)}
        </div>
      </div>
    `;
  }
}

customElements.define('bookmarks-manager', BookmarksManager);
