// Copyright (c) 2026 The codem37 Authors. All rights reserved.
// Lit element component for selecting browser theme modes.

import {html, css, LitElement} from 'chrome://resources/mwc/lit/index.js';
import {settingsMojoClient} from '../mojo_client.js';

export class ThemePicker extends LitElement {
  static override get styles() {
    return css`
      :host {
        display: block;
        margin-bottom: 20px;
      }
      .theme-options {
        display: flex;
        gap: 16px;
        margin-top: 12px;
      }
      .theme-card {
        flex: 1;
        padding: 16px;
        border-radius: var(--c37-radius-md, 8px);
        background: var(--c37-bg-surface, #0c121d);
        border: 1px solid var(--c37-border-subtle, rgba(255,255,255,0.08));
        cursor: pointer;
        text-align: center;
        transition: all 0.2s ease;
      }
      .theme-card:hover {
        border-color: var(--c37-accent-cyan, #00f0ff);
      }
      .theme-card.active {
        border-color: var(--c37-accent-cyan, #00f0ff);
        background: var(--c37-accent-cyan-dim, rgba(0,240,255,0.12));
      }
      .title {
        font-weight: 600;
        font-size: 0.95rem;
      }
    `;
  }

  static override get properties() {
    return {
      selectedMode: {type: Number},
    };
  }

  selectedMode: number = 2; // Dark default

  async selectTheme(mode: number) {
    this.selectedMode = mode;
    await settingsMojoClient.setTheme({
      mode: this.selectedMode,
      accentColorHex: '#00f0ff',
      customCss: '', // Left unused in UI per security policy
    });
  }

  override render() {
    return html`
      <div>
        <h3>Appearance & Theme</h3>
        <div class="theme-options">
          <div class="theme-card ${this.selectedMode === 0 ? 'active' : ''}" @click=${() => this.selectTheme(0)}>
            <div class="title">System Default</div>
          </div>
          <div class="theme-card ${this.selectedMode === 1 ? 'active' : ''}" @click=${() => this.selectTheme(1)}>
            <div class="title">Light</div>
          </div>
          <div class="theme-card ${this.selectedMode === 2 ? 'active' : ''}" @click=${() => this.selectTheme(2)}>
            <div class="title">Cybernetic Dark</div>
          </div>
        </div>
      </div>
    `;
  }
}

customElements.define('theme-picker', ThemePicker);
