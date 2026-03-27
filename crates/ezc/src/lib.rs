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

use error::{EzError, ParseError};
use eval::Engine;
use types::Value;

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

    parser::parse(&tokens, src.len()).map_err(|errs| {
        EzError::Parse(
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
        )
    })
}

/// Run an ezc program from source, returning the final stack.
pub fn run(src: &str) -> Result<Vec<Value>, EzError> {
    let ast = lex_and_parse(src)?;
    let mut engine = Engine::new();
    engine.eval(&ast).map_err(EzError::Eval)?;
    Ok(engine.into_stack())
}

/// Evaluate a line of source against an existing engine.
pub fn eval_line(engine: &mut Engine, src: &str) -> Result<(), EzError> {
    let ast = lex_and_parse(src)?;
    engine.eval(&ast).map_err(EzError::Eval)
}
