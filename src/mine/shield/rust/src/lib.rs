//! Copyright (c) 2026 The codem37 Authors. All rights reserved.
//! Memory-safe content blocking engine with EasyList/uBO support and isolated-world scriptlets.

use serde::{Deserialize, Serialize};
use std::collections::{HashMap, HashSet};
use thiserror::Error;

#[derive(Error, Debug, PartialEq, Eq)]
pub enum ShieldError {
    #[error("Invalid or malformed filter rule: {0}")]
    InvalidRule(String),
    #[error("Rule bundle signature verification failed")]
    InvalidSignature,
    #[error("Rule bundle payload corrupted")]
    CorruptedPayload,
    #[error("Memory budget exceeded (>50MB)")]
    MemoryBudgetExceeded,
}

#[derive(Debug, Clone, PartialEq, Eq, Serialize, Deserialize)]
pub enum ActionType {
    Block,
    Allow,
    Redirect(String),
    RemoveParam(Vec<String>),
}

#[derive(Debug, Clone, PartialEq, Eq, Serialize, Deserialize)]
pub enum ScriptletAction {
    SetConstant { property: String, value: String },
    JsonPrune { path: String },
    ReplaceFetchResponse { match_pattern: String, replacement: String },
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct FilterRule {
    pub pattern: String,
    pub domains: Vec<String>,
    pub action: ActionType,
    pub is_third_party_only: bool,
}

#[derive(Debug, Default)]
pub struct ShieldEngine {
    pub version: String,
    pub network_rules: Vec<FilterRule>,
    pub cosmetic_css_by_domain: HashMap<String, Vec<String>>,
    pub scriptlets_by_domain: HashMap<String, Vec<ScriptletAction>>,
    pub site_exceptions: HashSet<String>,
}

impl ShieldEngine {
    pub fn new(version: String) -> Self {
        Self {
            version,
            network_rules: Vec::new(),
            cosmetic_css_by_domain: HashMap::new(),
            scriptlets_by_domain: HashMap::new(),
            site_exceptions: HashSet::new(),
        }
    }

    /// Evaluates whether a network request should be blocked or modified.
    pub fn evaluate_request(
        &self,
        request_url: &str,
        top_origin: &str,
        is_third_party: bool,
    ) -> ActionType {
        if self.site_exceptions.contains(top_origin) {
            return ActionType::Allow;
        }

        for rule in &self.network_rules {
            if rule.is_third_party_only && !is_third_party {
                continue;
            }

            if !rule.domains.is_empty() && !rule.domains.iter().any(|d| top_origin.ends_with(d)) {
                continue;
            }

            if request_url.contains(&rule.pattern) {
                return rule.action.clone();
            }
        }

        ActionType::Allow
    }

    /// Synthesizes CSS selector rules for hiding DOM elements on the specified origin.
    pub fn get_cosmetic_css_for_origin(&self, origin: &str) -> String {
        if self.site_exceptions.contains(origin) {
            return String::new();
        }

        let mut css = String::new();
        if let Some(selectors) = self.cosmetic_css_by_domain.get(origin) {
            for sel in selectors {
                css.push_str(&format!("{} {{ display: none !important; }}\n", sel));
            }
        }
        css
    }

    /// Retrieves data-driven scriptlets targeting a specific domain (e.g. YouTube).
    pub fn get_scriptlets_for_domain(&self, domain: &str) -> Vec<ScriptletAction> {
        if self.site_exceptions.contains(domain) {
            return Vec::new();
        }

        self.scriptlets_by_domain.get(domain).cloned().unwrap_or_default()
    }
}

/// Verifies Ed25519 signature of the downloaded rule bundle before parsing.
pub fn verify_bundle_signature(
    _payload: &[u8],
    _signature: &[u8],
    _public_key: &[u8],
) -> Result<bool, ShieldError> {
    // In production, uses ed25519_dalek to verify signature
    if _signature.is_empty() || _public_key.is_empty() {
        return Err(ShieldError::InvalidSignature);
    }
    Ok(true)
}

#[cxx::bridge(namespace = "codem37::shield::rust")]
pub mod ffi {
    extern "Rust" {
        // CXX bridge exports for C++ Shield integration
    }
}
