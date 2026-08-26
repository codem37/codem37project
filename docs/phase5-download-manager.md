# codem37 — Phase 5: Download Manager & Native Segmented Fetch Specification

> **Governing Principle:**
> **`DownloadManager` is the single authoritative record of "what is being downloaded." The segmented fetcher is a specialized producer that writes into that record, never a parallel, competing source of truth.** If at any point the two disagree about a download's state, `DownloadManager`'s view wins, and the segmented fetcher's job is to keep itself synchronized to it — not the other way around.

---

## 1. Responsibilities & Architecture

```text
chrome-ui (WebUI Control Plane)
   │
   │ Mojo: commands (Pause/Resume/Cancel) + 250ms coalesced progress pushes
   ▼
MineFetcherService (Browser Process KeyedService)
   │
   ├── [Sole Authoritative External State: DownloadManager / single DownloadItem]
   │
   └── [Internal Write Strategy: SegmentedFetchProducer]
          ├── Range Planner (Rust: 8MB min chunk, max 8 concurrent ranges)
          ├── Direct-to-Offset Sparse File Writer (preallocated destination file)
          ├── Sidecar State Persistence (.c37state completed-ranges bitmap)
          └── Fallback Engine (switches to single-stream on capability mismatch)
```

---

## 2. Segmented Fetch Engine Parameters

| Parameter | Configuration Value | Architectural Purpose |
| :--- | :---: | :--- |
| **Large-File Threshold** | **100 MB** | Minimum file size with declared `Content-Length` eligible for segmentation. |
| **Maximum Concurrency** | **8 Ranges** | Upper bound on simultaneous parallel HTTP range connections. |
| **Minimum Chunk Size** | **8 MB** | Prevents excessive HTTP connection overhead on smaller files. |
| **Coalesced Push Interval**| **250 ms** (4Hz) | Throttles high-frequency progress updates to protect UI paint performance. |
| **Segment Retry Budget** | **3 Retries** | Exponential backoff (500ms, 1s, 2s) with jitter before fallback. |

---

## 3. Correctness & Resource Consistency Guarantees

1. **`Accept-Ranges: bytes` Mandatory Pre-Check**:
   - Segmented fetching is strictly attempted after confirming range capability.
   - If `Accept-Ranges` is absent or returns `200 OK` instead of `206 Partial Content`, the engine falls back to standard single-stream downloading unconditionally.
2. **Resource-Consistency Guarantee (§60)**:
   - Assembled output is produced **exclusively** from responses sharing the same `ETag` (or `Last-Modified` + `Content-Length`) captured at download initiation.
   - If `ETag` changes mid-download or upon resume, stale partial bytes are discarded, and the download restarts from scratch.
3. **Direct-to-Offset Reassembly**:
   - Chunks are written directly into their target byte offsets in a pre-allocated sparse destination file.
   - No separate per-segment temporary files or disk concatenation passes.
4. **Sidecar State Persistence (`.c37state`)**:
   - In-progress segmented downloads store a bitmap of completed byte ranges and validators in `<destination>.c37state`, enabling safe resume across process crashes.

---

## 4. Mojo Interface & Push Model

- **No Polling**: Steady-state progress is delivered exclusively via push events (`MineFetcherObserver`).
- **One-Shot Reconciliation**: Upon WebUI reconnect or tab reload, clients call `GetSnapshot()` to retrieve active download states in a single request-response.
- **Opaque Handles**: Renderers manipulate downloads using numeric IDs (`uint64 download_id`), never raw C++ `DownloadItem*` pointers or arbitrary filesystem paths.
