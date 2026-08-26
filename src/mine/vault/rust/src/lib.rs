//! Copyright (c) 2026 The codem37 Authors. All rights reserved.
//! Vault on-disk file parser and cryptographic envelope deserializer.

use serde::{Deserialize, Serialize};
use thiserror::Error;
use zeroize::{Zeroize, ZeroizeOnDrop};

#[derive(Error, Debug)]
pub enum VaultParseError {
    #[error("File is truncated or smaller than header minimum")]
    TruncatedHeader,
    #[error("Invalid magic bytes or unsupported format version: {0}")]
    InvalidMagic(u32),
    #[error("Payload corrupted or failed checksum")]
    CorruptedPayload,
    #[error("JSON deserialization error: {0}")]
    DeserializationError(String),
}

#[derive(Serialize, Deserialize, Zeroize, ZeroizeOnDrop, Debug)]
pub struct VaultSecretPayload {
    pub username: String,
    pub password_plaintext: String,
}

#[derive(Serialize, Deserialize, Debug)]
pub struct SerializedVaultRecord {
    pub id: String,
    pub site_url: String,
    pub username: String,
    pub encrypted_blob_hex: String,
    pub created_time_unix: i64,
}

const VAULT_MAGIC_HEADER: &[u8; 4] = b"C37V";
const VAULT_FORMAT_VERSION: u16 = 1;

/// Parses on-disk raw vault file bytes into structured metadata records.
/// Enforces bounded slices and structured error propagation without panicking.
pub fn parse_vault_file(raw_bytes: &[u8]) -> Result<Vec<SerializedVaultRecord>, VaultParseError> {
    if raw_bytes.len() < 8 {
        return Err(VaultParseError::TruncatedHeader);
    }

    if &raw_bytes[0..4] != VAULT_MAGIC_HEADER {
        let magic = u32::from_be_bytes(raw_bytes[0..4].try_into().unwrap_or([0, 0, 0, 0]));
        return Err(VaultParseError::InvalidMagic(magic));
    }

    let version = u16::from_be_bytes(raw_bytes[4..6].try_into().unwrap_or([0, 0]));
    if version != VAULT_FORMAT_VERSION {
        return Err(VaultParseError::InvalidMagic(version as u32));
    }

    let payload_bytes = &raw_bytes[8..];
    if payload_bytes.is_empty() {
        return Ok(Vec::new());
    }

    serde_json::from_slice(payload_bytes)
        .map_err(|e| VaultParseError::DeserializationError(e.to_string()))
}

#[cxx::bridge(namespace = "codem37::vault::rust")]
pub mod ffi {
    extern "Rust" {
        // CXX bridge exports for C++ VaultService integration
    }
}
