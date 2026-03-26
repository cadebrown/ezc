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
pub mod lexer;
pub mod parser;
pub mod token;
pub mod types;

use error::{EzError, ParseError};
use eval::Machine;
use types::Value;

/// Lex and parse source into an AST, returning structured errors via `EzError`.
fn lex_and_parse(src: &str) -> Result<Vec<ast::Spanned<ast::Expr>>, EzError> {
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
///
/// This is the primary entry point for the language. It lexes, parses, and
/// evaluates the source in sequence. Errors from any stage are returned as
/// `EzError`.
pub fn run(src: &str) -> Result<Vec<Value>, EzError> {
    let ast = lex_and_parse(src)?;
    let mut machine = Machine::new();
    machine.eval(&ast).map_err(EzError::Eval)?;
    Ok(machine.into_stack())
}

/// Evaluate a line of source against an existing machine.
///
/// Used by the REPL to maintain a persistent stack across inputs.
/// Returns `Ok(())` on success, or `EzError` with full span information.
pub fn eval_line(machine: &mut Machine, src: &str) -> Result<(), EzError> {
    let ast = lex_and_parse(src)?;
    machine.eval(&ast).map_err(EzError::Eval)
}
