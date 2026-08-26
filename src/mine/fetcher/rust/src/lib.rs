//! Copyright (c) 2026 The codem37 Authors. All rights reserved.
//! Untrusted HTTP response header and byte-range calculation engine in safe Rust.

use thiserror::Error;

#[derive(Error, Debug)]
pub enum FetcherParseError {
    #[error("Incomplete or malformed HTTP headers")]
    MalformedHeaders,
    #[error("Invalid Content-Range header: {0}")]
    InvalidContentRange(String),
    #[error("Invalid Content-Length header: {0}")]
    InvalidContentLength(String),
}

#[derive(Debug, PartialEq, Eq)]
pub struct ParsedByteRange {
    pub start: u64,
    pub end: u64,
    pub total_size: Option<u64>,
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
        // CXX bridge exports for C++ FetcherService integration
    }
}
