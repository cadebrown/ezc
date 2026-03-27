use std::path::Path;

use tracing::info;

pub fn execute(file: &Path, trace: bool) -> Result<(), Box<dyn std::error::Error>> {
    let src = std::fs::read_to_string(file)?;
    let filename = file.display().to_string();
    info!(file = %filename, "running");

    if trace {
        execute_trace_src(&src, &filename)
    } else {
        match ezc::run(&src) {
            Ok(stack) => {
                for value in &stack {
                    println!("{value}");
                }
                Ok(())
            }
            Err(e) => {
                e.report(&filename, &src);
                Err(e.into())
            }
        }
    }
}

/// Run source with trace output: after each expression, print the full stack.
///
/// Output format (written to stderr so it doesn't pollute stdout):
///
/// ```text
///   1 │ 3              → [3]
///   1 │ 4              → [3 4]
///   1 │ +              → [7]
/// ```
pub fn execute_trace_src(src: &str, filename: &str) -> Result<(), Box<dyn std::error::Error>> {
    use ezc::eval::Engine;
    use ezc::line_index::LineIndex;

    let ast = ezc::lex_and_parse(src).map_err(|e| {
        e.report(filename, src);
        e
    })?;

    let line_index = LineIndex::new(src);
    let mut engine = Engine::new();

    for spanned in &ast {
        let (_expr, span) = spanned;
        let line = line_index.line_of(span.start) + 1;

        // Format the expression as source text (extract the literal span)
        let expr_src = src.get(span.clone()).unwrap_or("?").trim();

        match engine.eval_one(spanned) {
            Ok(()) => {
                // Build stack display
                let stack_str = engine
                    .stack()
                    .iter()
                    .map(|v| v.to_string())
                    .collect::<Vec<_>>()
                    .join(" ");

                eprintln!("{line:>4} │ {expr_src:<20} → [{stack_str}]");
            }
            Err(e) => {
                eprintln!("{line:>4} │ {expr_src:<20} → ERROR");
                let eval_err = ezc::error::EzError::Eval(e);
                eval_err.report(filename, src);
                return Err(eval_err.into());
            }
        }
    }

    // Print final stack to stdout (same as normal run)
    for value in engine.stack() {
        println!("{value}");
    }
    Ok(())
}
