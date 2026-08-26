//! Copyright (c) 2026 The codem37 Authors. All rights reserved.
//! Safe URL and action parsing for custom codem37:// internal schemes.

use thiserror::Error;

#[derive(Error, Debug, PartialEq, Eq)]
pub enum ProtocolParseError {
    #[error("Invalid scheme (expected codem37://)")]
    InvalidScheme,
    #[error("Empty host or path")]
    EmptyTarget,
    #[error("Unknown or unauthorized internal action: {0}")]
    UnknownAction(String),
}

#[derive(Debug, PartialEq, Eq)]
pub enum MineProtocolAction {
    OpenVault,
    OpenShield,
    OpenSettings,
    InternalHelp,
}

#[derive(Debug, PartialEq, Eq)]
pub struct ParsedProtocolTarget {
    pub action: MineProtocolAction,
    pub query_param: Option<String>,
}

/// Parses an attacker-influenced codem37:// URL cleanly in safe Rust.
pub fn parse_protocol_url(url_str: &str) -> Result<ParsedProtocolTarget, ProtocolParseError> {
    let trimmed = url_str.trim();
    if !trimmed.starts_with("codem37://") {
        return Err(ProtocolParseError::InvalidScheme);
    }

    let path_and_query = &trimmed[10..];
    if path_and_query.is_empty() {
        return Err(ProtocolParseError::EmptyTarget);
    }

    let parts: Vec<&str> = path_and_query.splitn(2, '?').collect();
    let action_str = parts[0].trim_matches('/');
    let query_param = parts.get(1).map(|q| q.to_string());

    let action = match action_str {
        "vault" => MineProtocolAction::OpenVault,
        "shield" => MineProtocolAction::OpenShield,
        "settings" => MineProtocolAction::OpenSettings,
        "help" => MineProtocolAction::InternalHelp,
        other => return Err(ProtocolParseError::UnknownAction(other.to_string())),
    };

    Ok(ParsedProtocolTarget {
        action,
        query_param,
    })
}

#[cxx::bridge(namespace = "codem37::protocols::rust")]
pub mod ffi {
    extern "Rust" {
        // CXX bridge exports for C++ ProtocolHandler integration
    }
}
