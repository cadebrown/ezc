//! `ezc` — a stack-based, reverse-polish-notation programming language.
//!
//! This crate provides the core language implementation: lexing, parsing, and
//! evaluation. The pipeline is:
//!
//! ```text
//! source code ──► lexer ──► tokens ──► parser ──► AST ──► evaluator ──► stack
//! ```
//!
//! # Quick start
//!
//! ```
//! let result = ezc::run("3 4 + 2 *").unwrap();
//! assert_eq!(result[0].to_string(), "14");
//! ```

pub mod ast;
pub mod debug_source_path;
pub mod error;
pub mod eval;
pub mod intern;
pub mod lexer;
pub mod line_index;
pub mod number;
pub mod parser;
pub mod stepper;
pub mod token;
pub mod types;

pub use debug_source_path::debug_source_paths_equivalent;

use std::panic::{catch_unwind, AssertUnwindSafe};

use error::{EzError, ParseError};
use eval::Engine;
use types::Value;

/// The standard library prelude, embedded in the binary.
pub const PRELUDE: &str = include_str!("../../../std/prelude.ezc");

/// Create a new engine with the standard library prelude loaded.
pub fn engine() -> Engine {
    let mut e = Engine::new();
    // Prelude errors are bugs, not user errors.
    let ast = lex_and_parse(PRELUDE).expect("prelude failed to parse");
    e.eval(&ast).expect("prelude failed to evaluate");
    e
}

/// Lex and parse source into an AST, returning structured errors via `EzError`.
pub fn lex_and_parse(src: &str) -> Result<Vec<ast::Spanned<ast::Expr>>, EzError> {
    let tokens = lexer::lex(src).map_err(|errs| {
        EzError::Parse(
            errs.into_iter()
                .map(|e| {
                    let span = e.span().into_range();
                    ParseError {
                        span,
                        message: format!("{e}"),
                        expected: vec![],
                        found: e.found().map(|c| c.to_string()),
                    }
                })
                .collect(),
        )
    })?;

    // chumsky 1.0.0-alpha.8 can panic on some malformed inputs (e.g. unclosed
    // delimiters). Catch the panic and convert to a parse error.
    let parse_result = catch_unwind(AssertUnwindSafe(|| parser::parse(&tokens, src.len())));

    match parse_result {
        Ok(Ok(ast)) => Ok(ast),
        Ok(Err(errs)) => Err(EzError::Parse(
            errs.into_iter()
                .map(|e| {
                    let span = e.span().into_range();
                    ParseError {
                        span,
                        message: format!("{e}"),
                        expected: vec![],
                        found: e.found().map(|t| t.to_string()),
                    }
                })
                .collect(),
        )),
        Err(_) => Err(EzError::Parse(vec![ParseError {
            span: 0..src.len(),
            message: "parse error: unclosed delimiter or malformed input".into(),
            expected: vec![],
            found: None,
        }])),
    }
}

/// Run an ezc program from source, returning the final stack.
/// The standard library prelude is loaded automatically.
pub fn run(src: &str) -> Result<Vec<Value>, EzError> {
    let ast = lex_and_parse(src)?;
    let mut e = engine();
    e.eval(&ast).map_err(EzError::Eval)?;
    Ok(e.into_stack())
}

/// Evaluate a line of source against an existing engine.
pub fn eval_line(engine: &mut Engine, src: &str) -> Result<(), EzError> {
    let ast = lex_and_parse(src)?;
    engine.eval(&ast).map_err(EzError::Eval)
}
