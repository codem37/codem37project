// Copyright (c) 2026 The codem37 Authors. All rights reserved.
// Mojo client wrapper for FetcherService & DownloadManager (chrome://downloads).

import {FetcherService, FetcherServiceRemote, MineFetcherObserverReceiver, DownloadSnapshot, FetcherStatus} from './fetcher.mojom-webui.js';

class DownloadsMojoClient {
  public remote: FetcherServiceRemote;

  constructor() {
    this.remote = FetcherService.getRemote();
  }

  async getSnapshot(): Promise<{downloads: DownloadSnapshot[]}> {
    return this.remote.getSnapshot();
  }

  async startFromUrl(url: string, suggestedFilename: string): Promise<{status: FetcherStatus, downloadId: string | null}> {
    return this.remote.startFromUrl({url}, suggestedFilename);
  }

  async pause(downloadId: string): Promise<{status: FetcherStatus}> {
    return this.remote.pauseDownload(downloadId);
  }

  async resume(downloadId: string): Promise<{status: FetcherStatus}> {
    return this.remote.resumeDownload(downloadId);
  }

  async cancel(downloadId: string): Promise<{status: FetcherStatus}> {
    return this.remote.cancelDownload(downloadId);
  }

  async openContainingFolder(downloadId: string): Promise<{status: FetcherStatus}> {
    return this.remote.openContainingFolder(downloadId);
  }

  observeDownloads(
    onCreated: (snapshot: DownloadSnapshot) => void,
    onUpdated: (snapshot: DownloadSnapshot) => void,
    onDestroyed: (downloadId: string) => void
  ) {
    const receiver = new MineFetcherObserverReceiver({
      onDownloadCreated: onCreated,
      onDownloadUpdated: onUpdated,
      onDownloadDestroyed: onDestroyed,
    });
    this.remote.addObserver(receiver.$.bindNewPipeAndPassRemote());
  }
}

export const downloadsMojoClient = new DownloadsMojoClient();
