use std::path::Path;

use tracing::info;

pub fn execute(file: &Path) -> Result<(), Box<dyn std::error::Error>> {
    let src = std::fs::read_to_string(file)?;
    let filename = file.display().to_string();
    info!(file = %filename, "checking");

    // Lex and parse only — no evaluation.
    let tokens = ezc::lexer::lex(&src).map_err(|errs| {
        let e = ezc::error::EzError::Parse(
            errs.into_iter()
                .map(|e| {
                    let span = e.span().into_range();
                    ezc::error::ParseError {
                        span,
                        message: format!("{e}"),
                        expected: vec![],
                        found: e.found().map(|c| c.to_string()),
                    }
                })
                .collect(),
        );
        e.report(&filename, &src);
        e
    })?;

    ezc::parser::parse(&tokens, src.len()).map_err(|errs| {
        let e = ezc::error::EzError::Parse(
            errs.into_iter()
                .map(|e| {
                    let span = e.span().into_range();
                    ezc::error::ParseError {
                        span,
                        message: format!("{e}"),
                        expected: vec![],
                        found: e.found().map(|t| t.to_string()),
                    }
                })
                .collect(),
        );
        e.report(&filename, &src);
        e
    })?;

    println!("OK");
    Ok(())
}
