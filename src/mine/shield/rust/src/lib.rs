//! Copyright (c) 2026 The codem37 Authors. All rights reserved.
//! High-performance filter-list parser and rule engine in safe Rust.

use std::collections::HashSet;
use thiserror::Error;

#[derive(Error, Debug)]
pub enum ShieldParseError {
    #[error("Empty filter list input")]
    EmptyInput,
    #[error("Malformed rule syntax on line {0}: {1}")]
    MalformedRule(usize, String),
    #[error("Rule set exceeded maximum capacity: {0}")]
    CapacityExceeded(usize),
}

#[derive(Default, Debug)]
pub struct FilterSet {
    pub blocked_domains: HashSet<String>,
    pub cosmetic_selectors: Vec<String>,
    pub total_rules_parsed: usize,
}

impl FilterSet {
    pub fn should_block(&self, domain: &str) -> bool {
        self.blocked_domains.contains(domain)
    }
}

/// Parses raw filter-list bytes (EasyList / ABP syntax) into a structured FilterSet.
/// Guarantees bounded iteration and zero-panic error handling.
pub fn parse_filter_list(raw_bytes: &[u8]) -> Result<FilterSet, ShieldParseError> {
    if raw_bytes.is_empty() {
        return Err(ShieldParseError::EmptyInput);
    }

    let content = std::str::from_utf8(raw_bytes)
        .map_err(|_| ShieldParseError::MalformedRule(0, "Invalid UTF-8 encoding".to_string()))?;

    let mut filter_set = FilterSet::default();

    for (line_idx, line) in content.lines().enumerate() {
        let trimmed = line.trim();
        if trimmed.is_empty() || trimmed.starts_with('!') || trimmed.starts_with('#') {
            continue;
        }

        // Domain blocking rule (e.g. ||tracker.example.com^)
        if let Some(domain_rule) = trimmed.strip_prefix("||") {
            let clean_domain = domain_rule.trim_end_matches('^');
            filter_set.blocked_domains.insert(clean_domain.to_string());
            filter_set.total_rules_parsed += 1;
        } else if trimmed.contains("##") {
            // Cosmetic selector rule
            let parts: Vec<&str> = trimmed.splitn(2, "##").collect();
            if parts.len() == 2 {
                filter_set.cosmetic_selectors.push(parts[1].to_string());
                filter_set.total_rules_parsed += 1;
            }
        }
    }

    Ok(filter_set)
}

#[cxx::bridge(namespace = "codem37::shield::rust")]
pub mod ffi {
    extern "Rust" {
        // CXX bridge exports for C++ ShieldService integration
    }
}
