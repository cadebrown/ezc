use std::fmt;

use crate::Span;
use ariadne::{Color, Label, Report, ReportKind, Source};

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum ErrorPhase {
    Lexer,
    Parser,
    Compiler,
    Runtime,
}

impl ErrorPhase {
    pub fn as_str(self) -> &'static str {
        match self {
            Self::Lexer => "lexer",
            Self::Parser => "parser",
            Self::Compiler => "compiler",
            Self::Runtime => "runtime",
        }
    }
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum ErrorCode {
    LexInvalidToken,
    ParseUnexpectedToken,
    CompileUnknownWord,
    RuntimeStackUnderflow,
    RuntimeTypeMismatch,
    RuntimeDivisionByZero,
    RuntimeModuloByZero,
    RuntimeOverflow,
}

impl ErrorCode {
    pub fn id(self) -> &'static str {
        match self {
            Self::LexInvalidToken => "E1001",
            Self::ParseUnexpectedToken => "E2001",
            Self::CompileUnknownWord => "E3001",
            Self::RuntimeStackUnderflow => "E4001",
            Self::RuntimeTypeMismatch => "E4002",
            Self::RuntimeDivisionByZero => "E4003",
            Self::RuntimeModuloByZero => "E4004",
            Self::RuntimeOverflow => "E4005",
        }
    }

    pub fn title(self) -> &'static str {
        match self {
            Self::LexInvalidToken => "Invalid token",
            Self::ParseUnexpectedToken => "Unexpected token",
            Self::CompileUnknownWord => "Unknown word",
            Self::RuntimeStackUnderflow => "Stack underflow",
            Self::RuntimeTypeMismatch => "Type mismatch",
            Self::RuntimeDivisionByZero => "Division by zero",
            Self::RuntimeModuloByZero => "Modulo by zero",
            Self::RuntimeOverflow => "Integer overflow",
        }
    }

    pub fn phase(self) -> ErrorPhase {
        match self {
            Self::LexInvalidToken => ErrorPhase::Lexer,
            Self::ParseUnexpectedToken => ErrorPhase::Parser,
            Self::CompileUnknownWord => ErrorPhase::Compiler,
            Self::RuntimeStackUnderflow
            | Self::RuntimeTypeMismatch
            | Self::RuntimeDivisionByZero
            | Self::RuntimeModuloByZero
            | Self::RuntimeOverflow => ErrorPhase::Runtime,
        }
    }
}

impl fmt::Display for ErrorCode {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "{}", self.id())
    }
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum LabelStyle {
    Primary,
    Secondary,
}

#[derive(Debug, Clone, PartialEq, Eq)]
pub struct DiagnosticLabel {
    pub style: LabelStyle,
    pub span: Option<Span>,
    pub message: String,
}

impl DiagnosticLabel {
    fn primary(message: impl Into<String>) -> Self {
        Self {
            style: LabelStyle::Primary,
            span: None,
            message: message.into(),
        }
    }

    fn secondary(message: impl Into<String>) -> Self {
        Self {
            style: LabelStyle::Secondary,
            span: None,
            message: message.into(),
        }
    }

    fn primary_at(span: Span, message: impl Into<String>) -> Self {
        Self {
            style: LabelStyle::Primary,
            span: Some(span),
            message: message.into(),
        }
    }

    fn secondary_at(span: Span, message: impl Into<String>) -> Self {
        Self {
            style: LabelStyle::Secondary,
            span: Some(span),
            message: message.into(),
        }
    }
}

#[derive(Debug, Clone, PartialEq, Eq)]
pub struct SourceInfo {
    pub name: String,
    pub text: String,
}

#[derive(Debug, Clone, thiserror::Error)]
#[error("{code}: {message}")]
pub struct EzcError {
    pub code: ErrorCode,
    pub message: String,
    pub span: Span,
    pub labels: Vec<DiagnosticLabel>,
    pub notes: Vec<String>,
    pub helps: Vec<String>,
    pub debug: Vec<String>,
    pub source_info: Option<SourceInfo>,
}

impl EzcError {
    pub fn new(code: ErrorCode, message: impl Into<String>, span: Span) -> Self {
        let message = message.into();
        Self {
            code,
            message: message.clone(),
            span,
            labels: vec![DiagnosticLabel::primary(message)],
            notes: Vec::new(),
            helps: Vec::new(),
            debug: Vec::new(),
            source_info: None,
        }
    }

    pub fn with_primary_label(mut self, message: impl Into<String>) -> Self {
        self.labels.push(DiagnosticLabel::primary(message));
        self
    }

    pub fn with_secondary_label(mut self, message: impl Into<String>) -> Self {
        self.labels.push(DiagnosticLabel::secondary(message));
        self
    }

    pub fn with_primary_label_at(mut self, span: Span, message: impl Into<String>) -> Self {
        self.labels.push(DiagnosticLabel::primary_at(span, message));
        self
    }

    pub fn with_secondary_label_at(mut self, span: Span, message: impl Into<String>) -> Self {
        self.labels
            .push(DiagnosticLabel::secondary_at(span, message));
        self
    }

    pub fn with_note(mut self, note: impl Into<String>) -> Self {
        self.notes.push(note.into());
        self
    }

    pub fn with_help(mut self, help: impl Into<String>) -> Self {
        self.helps.push(help.into());
        self
    }

    pub fn with_debug(mut self, debug: impl Into<String>) -> Self {
        self.debug.push(debug.into());
        self
    }

    pub fn with_source(mut self, name: impl Into<String>, text: impl Into<String>) -> Self {
        self.source_info = Some(SourceInfo {
            name: name.into(),
            text: text.into(),
        });
        self
    }

    pub fn with_source_if_missing(mut self, name: &str, text: &str) -> Self {
        // Runtime and compiler paths can enrich an existing error without clobbering source info.
        if self.source_info.is_none() {
            self.source_info = Some(SourceInfo {
                name: name.to_string(),
                text: text.to_string(),
            });
        }
        self
    }

    fn code_with_span(&self) -> String {
        format!("{}@{}..{}", self.code.id(), self.span.start, self.span.end)
    }

    pub fn render(&self) -> String {
        let Some(source) = &self.source_info else {
            let mut text = format!(
                "[{}] {} ({}) at {}..{}: {}",
                self.code.id(),
                self.code.title(),
                self.code.phase().as_str(),
                self.span.start,
                self.span.end,
                self.message
            );

            for note in &self.notes {
                text.push_str(&format!("\nnote: {note}"));
            }
            for help in &self.helps {
                text.push_str(&format!("\nhelp: {help}"));
            }
            for debug in &self.debug {
                text.push_str(&format!("\ndebug: {debug}"));
            }
            return text;
        };

        let mut rendered = Vec::new();
        let mut report = Report::build(ReportKind::Error, source.name.as_str(), self.span.start)
            .with_code(self.code_with_span())
            .with_message(format!(
                "{} ({})",
                self.code.title(),
                self.code.phase().as_str()
            ));

        for label in &self.labels {
            let resolved_span = label.span.clone().unwrap_or_else(|| self.span.clone());
            let color = match label.style {
                LabelStyle::Primary => Color::Red,
                LabelStyle::Secondary => Color::Yellow,
            };
            report = report.with_label(
                Label::new((source.name.as_str(), resolved_span))
                    .with_message(label.message.clone())
                    .with_color(color),
            );
        }

        let mut note_lines = Vec::new();
        note_lines.extend(self.notes.iter().cloned());
        note_lines.extend(self.helps.iter().map(|help| format!("help: {help}")));
        note_lines.extend(self.debug.iter().map(|dbg| format!("debug: {dbg}")));
        if !note_lines.is_empty() {
            report = report.with_note(note_lines.join("\n"));
        }

        let write_result = report.finish().write(
            (source.name.as_str(), Source::from(source.text.as_str())),
            &mut rendered,
        );

        if write_result.is_err() {
            return format!("{}: {}", self.code, self.message);
        }

        String::from_utf8_lossy(&rendered).into_owned()
    }
}
