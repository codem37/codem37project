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

    pub fn add_rule(&mut self, rule: FilterRule) {
        self.network_rules.push(rule);
    }

    pub fn add_cosmetic_rule(&mut self, domain: String, selector: String) {
        self.cosmetic_css_by_domain
            .entry(domain)
            .or_default()
            .push(selector);
    }

    pub fn add_scriptlet(&mut self, domain: String, scriptlet: ScriptletAction) {
        self.scriptlets_by_domain
            .entry(domain)
            .or_default()
            .push(scriptlet);
    }

    pub fn set_site_exception(&mut self, domain: String, allow: bool) {
        if allow {
            self.site_exceptions.insert(domain);
        } else {
            self.site_exceptions.remove(&domain);
        }
    }

    /// Evaluates a URL against all network blocking rules with third-party domain awareness.
    pub fn match_url(&self, url: &str, source_domain: &str, is_third_party: bool) -> ActionType {
        if self.site_exceptions.contains(source_domain) {
            return ActionType::Allow;
        }

        for rule in &self.network_rules {
            if rule.is_third_party_only && !is_third_party {
                continue;
            }

            if !rule.domains.is_empty() && !rule.domains.iter().any(|d| d == source_domain) {
                continue;
            }

            if url.contains(&rule.pattern) {
                return rule.action.clone();
            }
        }

        ActionType::Allow
    }

    /// Compiles cosmetic CSS hiding rules into a single stylesheet payload.
    pub fn get_cosmetic_stylesheet(&self, domain: &str) -> Option<String> {
        if self.site_exceptions.contains(domain) {
            return None;
        }

        let mut selectors = Vec::new();
        if let Some(global_selectors) = self.cosmetic_css_by_domain.get("*") {
            selectors.extend(global_selectors.clone());
        }
        if let Some(domain_selectors) = self.cosmetic_css_by_domain.get(domain) {
            selectors.extend(domain_selectors.clone());
        }

        if selectors.is_empty() {
            None
        } else {
            Some(format!("{} {{ display: none !important; }}", selectors.join(", ")))
        }
    }

    /// Returns active isolated-world scriptlets for a given origin.
    pub fn get_scriptlets_for_domain(&self, domain: &str) -> Vec<ScriptletAction> {
        if self.site_exceptions.contains(domain) {
            return Vec::new();
        }
        self.scriptlets_by_domain.get(domain).cloned().unwrap_or_default()
    }
}

#[no_mangle]
pub extern "C" fn mine_shield_engine_create() -> *mut ShieldEngine {
    Box::into_raw(Box::new(ShieldEngine::new("1.0.0".to_string())))
}

#[no_mangle]
pub extern "C" fn mine_shield_engine_destroy(engine: *mut ShieldEngine) {
    if !engine.is_null() {
        // SAFETY: Pointer was created by Box::into_raw in mine_shield_engine_create and checked non-null.
        unsafe {
            drop(Box::from_raw(engine));
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_network_blocking_and_exceptions() {
        let mut engine = ShieldEngine::new("1.0.0".to_string());
        engine.add_rule(FilterRule {
            pattern: "doubleclick.net".to_string(),
            domains: vec![],
            action: ActionType::Block,
            is_third_party_only: false,
        });

        assert_eq!(
            engine.match_url("https://ad.doubleclick.net/ad.js", "example.com", true),
            ActionType::Block
        );

        engine.set_site_exception("example.com".to_string(), true);
        assert_eq!(
            engine.match_url("https://ad.doubleclick.net/ad.js", "example.com", true),
            ActionType::Allow
        );
    }

    #[test]
    fn test_cosmetic_stylesheet_generation() {
        let mut engine = ShieldEngine::new("1.0.0".to_string());
        engine.add_cosmetic_rule("news.com".to_string(), ".ad-banner".to_string());
        engine.add_cosmetic_rule("news.com".to_string(), "#sponsored-post".to_string());

        let css = engine.get_cosmetic_stylesheet("news.com").unwrap();
        assert_eq!(css, ".ad-banner, #sponsored-post { display: none !important; }");
    }
}
