//! Copyright (c) 2026 The codem37 Authors. All rights reserved.
//! Untrusted HTTP response header and byte-range calculation engine in safe Rust.

use serde::{Deserialize, Serialize};
use thiserror::Error;

#[derive(Error, Debug, PartialEq, Eq)]
pub enum FetcherParseError {
    #[error("Incomplete or malformed HTTP headers")]
    MalformedHeaders,
    #[error("Invalid Content-Range header: {0}")]
    InvalidContentRange(String),
    #[error("Invalid Content-Length header: {0}")]
    InvalidContentLength(String),
    #[error("Resource version mismatch (ETag / Last-Modified changed)")]
    ResourceVersionMismatch,
    #[error("Server does not support byte ranges")]
    RangesNotSupported,
}

#[derive(Debug, Clone, PartialEq, Eq, Serialize, Deserialize)]
pub struct ParsedByteRange {
    pub start: u64,
    pub end: u64,
    pub total_size: Option<u64>,
}

#[derive(Debug, Clone, PartialEq, Eq, Serialize, Deserialize)]
pub struct SegmentPlan {
    pub index: usize,
    pub start_offset: u64,
    pub end_offset: u64,
    pub is_completed: bool,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct SidecarDownloadState {
    pub url: String,
    pub destination_path: String,
    pub total_bytes: u64,
    pub etag: Option<String>,
    pub last_modified: Option<String>,
    pub segments: Vec<SegmentPlan>,
}

pub const DEFAULT_LARGE_FILE_THRESHOLD: u64 = 100 * 1024 * 1024; // 100 MB
pub const DEFAULT_MAX_CONCURRENCY: usize = 8;
pub const DEFAULT_MIN_CHUNK_SIZE: u64 = 8 * 1024 * 1024; // 8 MB

/// Plans non-overlapping byte ranges for parallel segmented download.
/// Absorbs integer division remainders into the final segment.
pub fn plan_segmented_download(
    total_size: u64,
    max_concurrency: usize,
    min_chunk_size: u64,
) -> Vec<SegmentPlan> {
    if total_size == 0 {
        return Vec::new();
    }

    let possible_chunks = total_size / min_chunk_size.max(1);
    let num_segments = (possible_chunks as usize).clamp(1, max_concurrency.max(1));

    if num_segments <= 1 {
        return vec![SegmentPlan {
            index: 0,
            start_offset: 0,
            end_offset: total_size - 1,
            is_completed: false,
        }];
    }

    let chunk_size = total_size / (num_segments as u64);
    let mut plans = Vec::with_capacity(num_segments);

    for i in 0..num_segments {
        let start_offset = (i as u64) * chunk_size;
        let end_offset = if i == num_segments - 1 {
            total_size - 1 // Last segment absorbs remainder
        } else {
            start_offset + chunk_size - 1
        };

        plans.push(SegmentPlan {
            index: i,
            start_offset,
            end_offset,
            is_completed: false,
        });
    }

    plans
}

/// Validates whether a response ETag matches the initially captured validator.
pub fn validate_resource_consistency(
    initial_etag: Option<&str>,
    response_etag: Option<&str>,
) -> Result<(), FetcherParseError> {
    match (initial_etag, response_etag) {
        (Some(init), Some(resp)) => {
            if init.trim() == resp.trim() {
                Ok(())
            } else {
                Err(FetcherParseError::ResourceVersionMismatch)
            }
        }
        (Some(_), None) => Err(FetcherParseError::ResourceVersionMismatch),
        _ => Ok(()),
    }
}

/// Parses standard HTTP Content-Range header (e.g., "bytes 200-1000/67589").
pub fn parse_content_range(header_value: &str) -> Result<ParsedByteRange, FetcherParseError> {
    let trimmed = header_value.trim();
    if !trimmed.starts_with("bytes ") {
        return Err(FetcherParseError::InvalidContentRange(trimmed.to_string()));
    }

    let range_part = &trimmed[6..];
    let parts: Vec<&str> = range_part.split('/').collect();
    if parts.len() != 2 {
        return Err(FetcherParseError::InvalidContentRange(trimmed.to_string()));
    }

    let byte_bounds: Vec<&str> = parts[0].split('-').collect();
    if byte_bounds.len() != 2 {
        return Err(FetcherParseError::InvalidContentRange(trimmed.to_string()));
    }

    let start = byte_bounds[0]
        .parse::<u64>()
        .map_err(|_| FetcherParseError::InvalidContentRange(trimmed.to_string()))?;
    let end = byte_bounds[1]
        .parse::<u64>()
        .map_err(|_| FetcherParseError::InvalidContentRange(trimmed.to_string()))?;

    let total_size = if parts[1] == "*" {
        None
    } else {
        Some(
            parts[1]
                .parse::<u64>()
                .map_err(|_| FetcherParseError::InvalidContentRange(trimmed.to_string()))?,
        )
    };

    Ok(ParsedByteRange {
        start,
        end,
        total_size,
    })
}

#[cxx::bridge(namespace = "codem37::fetcher::rust")]
pub mod ffi {
    extern "Rust" {
        // CXX bridge exports for C++ Fetcher integration
    }
}
