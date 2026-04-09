//! Symbol indexing for LSP features.
//!
//! Builds from the lexer output — no AST or evaluation needed.
//! Tracks definition sites (@name), reference sites ($name and bare idents),
//! and classifies identifiers as builtins vs user-defined.

use std::collections::{HashMap, HashSet};
use std::ops::Range;

use ezc::token::Token;

/// Index of all symbols in a document.
pub struct SymbolIndex {
    /// All tokens with byte-offset spans, in source order.
    pub tokens: Vec<(Token, Range<usize>)>,
    /// Variable name → definition sites (@name spans).
    pub definitions: HashMap<String, Vec<Range<usize>>>,
    /// Variable name → reference sites ($name and bare-ident spans).
    pub references: HashMap<String, Vec<Range<usize>>>,
}

impl SymbolIndex {
    /// Build a symbol index from source code.
    pub fn build(src: &str, builtins: &BuiltinSets) -> Self {
        let tokens: Vec<(Token, Range<usize>)> = match ezc::lexer::lex(src) {
            Ok(toks) => toks.into_iter().map(|(t, s)| (t, s.into_range())).collect(),
            Err(_) => vec![], // partial lex failures: empty index
        };

        let mut definitions: HashMap<String, Vec<Range<usize>>> = HashMap::new();
        let mut references: HashMap<String, Vec<Range<usize>>> = HashMap::new();

        for (tok, span) in &tokens {
            match tok {
                Token::Bind(name) => {
                    definitions
                        .entry(name.clone())
                        .or_default()
                        .push(span.clone());
                }
                Token::Recall(name) => {
                    references
                        .entry(name.clone())
                        .or_default()
                        .push(span.clone());
                }
                Token::Ident(name) if !builtins.is_known(name) => {
                    references
                        .entry(name.clone())
                        .or_default()
                        .push(span.clone());
                }
                _ => {}
            }
        }

        SymbolIndex {
            tokens,
            definitions,
            references,
        }
    }

    /// Find the token at a byte offset.
    pub fn token_at(&self, offset: usize) -> Option<&(Token, Range<usize>)> {
        self.tokens
            .iter()
            .find(|(_, span)| span.start <= offset && offset < span.end)
    }

    /// Get the variable name at a byte offset (if the token is a Bind, Recall, or user Ident).
    pub fn name_at(&self, offset: usize) -> Option<&str> {
        self.token_at(offset).and_then(|(tok, _)| match tok {
            Token::Bind(n) | Token::Recall(n) | Token::Ident(n) => Some(n.as_str()),
            _ => None,
        })
    }

    /// All definition sites for a name.
    pub fn definition_sites(&self, name: &str) -> &[Range<usize>] {
        self.definitions
            .get(name)
            .map(|v| v.as_slice())
            .unwrap_or(&[])
    }

    /// All occurrences (definitions + references) for a name, sorted by position.
    pub fn all_occurrences(&self, name: &str) -> Vec<Range<usize>> {
        let mut result: Vec<Range<usize>> = Vec::new();
        if let Some(defs) = self.definitions.get(name) {
            result.extend(defs.iter().cloned());
        }
        if let Some(refs) = self.references.get(name) {
            result.extend(refs.iter().cloned());
        }
        result.sort_by_key(|r| r.start);
        result
    }

    /// All defined names, in source order of first definition.
    pub fn defined_names(&self) -> Vec<(&str, &[Range<usize>])> {
        let mut names: Vec<(&str, &[Range<usize>])> = self
            .definitions
            .iter()
            .map(|(n, spans)| (n.as_str(), spans.as_slice()))
            .collect();
        names.sort_by_key(|(_, spans)| spans.first().map(|s| s.start).unwrap_or(0));
        names
    }

    /// Check if a definition at this position is preceded by a CloseParen
    /// (indicating it binds a block → function definition).
    pub fn is_function_def(&self, def_span: &Range<usize>) -> bool {
        // Find the token index of this definition
        let idx = self
            .tokens
            .iter()
            .position(|(_, s)| s.start == def_span.start);
        match idx {
            Some(i) if i > 0 => matches!(self.tokens[i - 1].0, Token::CloseParen),
            _ => false,
        }
    }
}

/// Sets of known builtin/prelude names for classifying identifiers.
pub struct BuiltinSets {
    pub type_constructors: HashSet<&'static str>,
    pub control_flow: HashSet<&'static str>,
    pub builtins: HashSet<&'static str>,
    pub prelude_names: HashSet<String>,
}

impl Default for BuiltinSets {
    fn default() -> Self {
        Self::new()
    }
}

impl BuiltinSets {
    /// Build from the embedded prelude source.
    pub fn new() -> Self {
        let type_constructors: HashSet<&str> = [
            "int", "str", "bin", "u8", "u16", "u32", "u64", "u128", "u256", "i8", "i16", "i32",
            "i64", "i128", "i256", "f16", "f32", "f64",
        ]
        .into_iter()
        .collect();

        let control_flow: HashSet<&str> = ["if", "ifel", "loop", "each", "import"]
            .into_iter()
            .collect();

        let builtins: HashSet<&str> = [
            "len", "nth", "tl", "rev", "srt", "take", "skip", "zip", "range", "typeof", "cut",
            "cat", "map", "fil", "red", "rl", "wl", "rb", "wb", "words", "depth", "clear", "and",
            "or",
        ]
        .into_iter()
        .collect();

        // Parse the prelude to find @name bindings.
        let mut prelude_names = HashSet::new();
        if let Ok(tokens) = ezc::lexer::lex(ezc::PRELUDE) {
            for (tok, _) in tokens {
                if let Token::Bind(name) = tok {
                    prelude_names.insert(name);
                }
            }
        }

        BuiltinSets {
            type_constructors,
            control_flow,
            builtins,
            prelude_names,
        }
    }

    /// Is this identifier a known builtin, type constructor, or prelude name?
    pub fn is_known(&self, name: &str) -> bool {
        self.type_constructors.contains(name)
            || self.control_flow.contains(name)
            || self.builtins.contains(name)
            || self.prelude_names.contains(name)
    }

    /// Return all known names (type constructors, control flow, builtins, prelude).
    pub fn all_names(&self) -> Vec<&str> {
        let mut names: Vec<&str> = Vec::new();
        names.extend(self.type_constructors.iter().copied());
        names.extend(self.control_flow.iter().copied());
        names.extend(self.builtins.iter().copied());
        names.extend(self.prelude_names.iter().map(|s| s.as_str()));
        names
    }

    /// Classify an Ident token for semantic highlighting.
    pub fn classify(&self, name: &str) -> SemanticClass {
        if self.type_constructors.contains(name) {
            SemanticClass::Type
        } else if self.control_flow.contains(name) {
            SemanticClass::Keyword
        } else if self.builtins.contains(name) || self.prelude_names.contains(name) {
            SemanticClass::Function
        } else {
            SemanticClass::Variable
        }
    }
}

/// Semantic classification for an identifier.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum SemanticClass {
    Type,
    Keyword,
    Function,
    Variable,
}
