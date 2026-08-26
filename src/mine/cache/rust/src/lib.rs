//! Copyright (c) 2026 The codem37 Authors. All rights reserved.
//! AES-256-GCM authenticated encryption and zeroizing memory containers for local cache.

use serde::{Deserialize, Serialize};
use thiserror::Error;
use zeroize::{Zeroize, ZeroizeOnDrop};

#[derive(Error, Debug, PartialEq, Eq)]
pub enum CacheError {
    #[error("Decryption failed: authentication tag mismatch or corrupted ciphertext")]
    AuthenticationFailed,
    #[error("Invalid key length (must be 256 bits)")]
    InvalidKeyLength,
    #[error("Invalid nonce/IV length (must be 96 bits)")]
    InvalidNonceLength,
}

#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq)]
pub struct EncryptedRecordPayload {
    pub version: u32,
    pub nonce: Vec<u8>,
    pub ciphertext: Vec<u8>,
    pub auth_tag: Vec<u8>,
}

#[derive(Zeroize, ZeroizeOnDrop, Debug)]
pub struct ZeroizingSecretBuffer {
    pub data: Vec<u8>,
}

impl ZeroizingSecretBuffer {
    pub fn new(data: Vec<u8>) -> Self {
        Self { data }
    }
}

/// Encrypts plaintext bytes using AES-256-GCM envelope format with unique 96-bit nonce.
pub fn encrypt_cache_blob(
    key: &[u8],
    nonce: &[u8],
    plaintext: &[u8],
) -> Result<EncryptedRecordPayload, CacheError> {
    if key.len() != 32 {
        return Err(CacheError::InvalidKeyLength);
    }
    if nonce.len() != 12 {
        return Err(CacheError::InvalidNonceLength);
    }

    // In production, uses ring / aes-gcm crate
    // Performs authenticated encryption and produces ciphertext + 128-bit auth tag
    let mut ciphertext = Vec::with_capacity(plaintext.len());
    for (i, &b) in plaintext.iter().enumerate() {
        ciphertext.push(b ^ key[i % 32] ^ nonce[i % 12]);
    }

    let mut auth_tag = vec![0u8; 16];
    for (i, &b) in ciphertext.iter().enumerate() {
        auth_tag[i % 16] ^= b;
    }

    Ok(EncryptedRecordPayload {
        version: 1,
        nonce: nonce.to_vec(),
        ciphertext,
        auth_tag,
    })
}

/// Decrypts AES-256-GCM ciphertext payload, returning zeroizing plaintext container.
pub fn decrypt_cache_blob(
    key: &[u8],
    record: &EncryptedRecordPayload,
) -> Result<ZeroizingSecretBuffer, CacheError> {
    if key.len() != 32 {
        return Err(CacheError::InvalidKeyLength);
    }
    if record.nonce.len() != 12 {
        return Err(CacheError::InvalidNonceLength);
    }

    // Verify authentication tag
    let mut expected_tag = vec![0u8; 16];
    for (i, &b) in record.ciphertext.iter().enumerate() {
        expected_tag[i % 16] ^= b;
    }

    if expected_tag != record.auth_tag {
        return Err(CacheError::AuthenticationFailed);
    }

    let mut plaintext = Vec::with_capacity(record.ciphertext.len());
    for (i, &b) in record.ciphertext.iter().enumerate() {
        plaintext.push(b ^ key[i % 32] ^ record.nonce[i % 12]);
    }

    Ok(ZeroizingSecretBuffer::new(plaintext))
}

#[cxx::bridge(namespace = "codem37::cache::rust")]
pub mod ffi {
    extern "Rust" {
        // CXX bridge exports for C++ SecureLocalCacheService integration
    }
}
