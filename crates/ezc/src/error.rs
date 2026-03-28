use std::fmt;

use ariadne::{Color, Config, Label, Report, ReportKind, Source};

/// Top-level error type for the ezc language pipeline.
#[derive(Debug)]
pub enum EzError {
    /// One or more parse errors from the lexer or parser stage.
    Parse(Vec<ParseError>),
    /// A runtime evaluation error.
    Eval(EvalError),
}

/// A single parse error with source span and message.
#[derive(Debug, Clone)]
pub struct ParseError {
    pub span: std::ops::Range<usize>,
    pub message: String,
    pub expected: Vec<String>,
    pub found: Option<String>,
}

/// A runtime error during evaluation.
#[derive(Debug, Clone)]
pub struct EvalError {
    pub kind: EvalErrorKind,
    /// Span of the operator/expression that failed.
    pub span: Option<std::ops::Range<usize>>,
    /// Additional labeled spans for context (e.g., operand source locations).
    pub labels: Vec<ErrorLabel>,
}

/// A labeled source span for error context.
#[derive(Debug, Clone)]
pub struct ErrorLabel {
    pub span: std::ops::Range<usize>,
    pub message: String,
}

/// Specific kinds of evaluation errors.
#[derive(Debug, Clone, thiserror::Error)]
pub enum EvalErrorKind {
    #[error("`{op}` needs {expected} value(s) on the stack, got {found}")]
    StackUnderflow {
        op: String,
        expected: usize,
        found: usize,
    },

    #[error("`{op}` got {found} — needs {expected}")]
    TypeMismatch {
        op: String,
        expected: String,
        found: String,
    },

    #[error("division by zero")]
    DivisionByZero,

    #[error("undefined: ${name}")]
    UndefinedVariable { name: String },

    #[error("step limit exceeded ({limit} steps)")]
    StepLimitExceeded { limit: u64 },

    #[error("I/O error: {0}")]
    IoError(String),
}

impl fmt::Display for EzError {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            EzError::Parse(errors) => {
                for (i, e) in errors.iter().enumerate() {
                    if i > 0 {
                        writeln!(f)?;
                    }
                    write!(f, "{e}")?;
                }
                Ok(())
            }
            EzError::Eval(e) => write!(f, "{}", e.kind),
        }
    }
}

impl std::error::Error for EzError {}

impl fmt::Display for ParseError {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "{}", self.message)
    }
}

impl EzError {
    /// Build ariadne reports for this error.
    fn build_reports<'a>(
        &'a self,
        filename: &'a str,
        config: Config,
    ) -> Vec<Report<'a, (&'a str, std::ops::Range<usize>)>> {
        match self {
            EzError::Parse(errors) => errors
                .iter()
                .map(|err| {
                    Report::build(ReportKind::Error, (filename, err.span.clone()))
                        .with_config(config)
                        .with_message(&err.message)
                        .with_label(
                            Label::new((filename, err.span.clone()))
                                .with_message(&err.message)
                                .with_color(Color::Red),
                        )
                        .finish()
                })
                .collect(),
            EzError::Eval(err) => {
                let span = err.span.clone().unwrap_or(0..0);
                let mut builder = Report::build(ReportKind::Error, (filename, span.clone()))
                    .with_config(config)
                    .with_message(err.kind.to_string())
                    .with_label(
                        Label::new((filename, span))
                            .with_message(err.kind.to_string())
                            .with_color(Color::Red),
                    );

                // Add operand context labels with distinct colors.
                let context_colors = [Color::Yellow, Color::Cyan, Color::Magenta];
                for (i, label) in err.labels.iter().enumerate() {
                    let color = context_colors[i % context_colors.len()];
                    builder = builder.with_label(
                        Label::new((filename, label.span.clone()))
                            .with_message(&label.message)
                            .with_color(color)
                            .with_order(-(i as i32) - 1), // render below the primary
                    );
                }

                vec![builder.finish()]
            }
        }
    }

    /// Render this error as a pretty-printed ariadne report to stderr.
    pub fn report(&self, filename: &str, src: &str) {
        for report in self.build_reports(filename, Config::default()) {
            let _ = report.eprint((filename, Source::from(src)));
        }
    }

    /// Render this error as a colored string (for REPLs with ANSI support).
    pub fn report_string(&self, filename: &str, src: &str) -> String {
        let mut buf = Vec::new();
        for report in self.build_reports(filename, Config::default()) {
            let _ = report.write((filename, Source::from(src)), &mut buf);
        }
        String::from_utf8_lossy(&buf).into_owned()
    }

    /// Render this error as a plain-text string (no ANSI colors, for tests).
    pub fn report_plain(&self, filename: &str, src: &str) -> String {
        let config = Config::default().with_color(false);
        let mut buf = Vec::new();
        for report in self.build_reports(filename, config) {
            let _ = report.write((filename, Source::from(src)), &mut buf);
        }
        String::from_utf8_lossy(&buf).into_owned()
    }
}
