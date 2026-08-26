// Copyright (c) 2026 The codem37 Authors. All rights reserved.
// Mojo client wrapper for SecureLocalCacheService (chrome://mine-settings).

import {CacheService, CacheServiceRemote, CacheTheme, CacheStatus, BookmarkRecord, HistoryQueryFilter, HistoryQueryResult} from './cache.mojom-webui.js';

class SettingsMojoClient {
  private remote: CacheServiceRemote;

  constructor() {
    this.remote = CacheService.getRemote();
  }

  // Bookmarks
  async getBookmarks(): Promise<{bookmarks: BookmarkRecord[]}> {
    return this.remote.getBookmarks();
  }

  async addBookmark(url: string, title: string): Promise<{status: CacheStatus, recordId: string | null}> {
    return this.remote.addBookmark({url}, title);
  }

  async deleteBookmark(recordId: string): Promise<{status: CacheStatus}> {
    return this.remote.deleteBookmark(recordId);
  }

  // History
  async queryHistory(filter: HistoryQueryFilter): Promise<{result: HistoryQueryResult}> {
    return this.remote.queryHistory(filter);
  }

  async clearHistory(): Promise<{status: CacheStatus}> {
    return this.remote.clearHistory();
  }

  // Theme & Preferences
  async getTheme(): Promise<{theme: CacheTheme}> {
    return this.remote.getTheme();
  }

  async setTheme(theme: CacheTheme): Promise<{status: CacheStatus}> {
    return this.remote.setTheme(theme);
  }

  // Distinct Memory & Cache clearing invariants
  async clearMemory(): Promise<{status: CacheStatus}> {
    return this.remote.clearMemory();
  }

  async clearAllCache(): Promise<{status: CacheStatus}> {
    return this.remote.clearAllCache();
  }
}

export const settingsMojoClient = new SettingsMojoClient();
