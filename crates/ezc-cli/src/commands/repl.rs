use std::borrow::Cow;
use std::io::IsTerminal;
use std::path::PathBuf;

use nu_ansi_term::{Color, Style};
use reedline::{
    Completer, DefaultHinter, FileBackedHistory, Highlighter, Prompt, PromptEditMode,
    PromptHistorySearch, PromptHistorySearchStatus, Reedline, Signal, Span, StyledText, Suggestion,
};

use ezc::eval::Machine;

use crate::tui::{is_operator, COMPLETIONS};

// ── Prompt ────────────────────────────────────────────────────────────────────

struct EzcPrompt;

impl Prompt for EzcPrompt {
    fn render_prompt_left(&self) -> Cow<'_, str> {
        Cow::Borrowed("Σ")
    }

    fn render_prompt_right(&self) -> Cow<'_, str> {
        Cow::Borrowed("")
    }

    fn render_prompt_indicator(&self, _mode: PromptEditMode) -> Cow<'_, str> {
        Cow::Borrowed("  ")
    }

    fn render_prompt_multiline_indicator(&self) -> Cow<'_, str> {
        Cow::Borrowed("…  ")
    }

    fn render_prompt_history_search_indicator(&self, search: PromptHistorySearch) -> Cow<'_, str> {
        let indicator = match search.status {
            PromptHistorySearchStatus::Passing => "?  ",
            PromptHistorySearchStatus::Failing => "✗  ",
        };
        Cow::Owned(format!("({}) {}", search.term, indicator))
    }
}

// ── Completer ─────────────────────────────────────────────────────────────────

struct EzcCompleter;

impl Completer for EzcCompleter {
    fn complete(&mut self, line: &str, pos: usize) -> Vec<Suggestion> {
        let before = &line[..pos];
        let prefix = before.split_whitespace().last().unwrap_or("");
        if prefix.is_empty() {
            return vec![];
        }
        let prefix_start = pos - prefix.len();

        COMPLETIONS
            .iter()
            .filter(|&&(op, _)| op.starts_with(prefix) && op != prefix)
            .map(|&(op, desc)| Suggestion {
                value: op.to_string(),
                description: Some(desc.to_string()),
                span: Span::new(prefix_start, pos),
                append_whitespace: true,
                ..Default::default()
            })
            .collect()
    }
}

// ── Syntax highlighter ────────────────────────────────────────────────────────

struct EzcHighlighter;

impl Highlighter for EzcHighlighter {
    fn highlight(&self, line: &str, _cursor: usize) -> StyledText {
        let mut styled = StyledText::new();
        let mut rest = line;

        while !rest.is_empty() {
            // Consume leading whitespace unstyled.
            let ws = rest.len() - rest.trim_start_matches(' ').len();
            if ws > 0 {
                styled.push((Style::new(), rest[..ws].to_string()));
                rest = &rest[ws..];
            }
            if rest.is_empty() {
                break;
            }

            // Grab next whitespace-delimited token.
            let end = rest.find(' ').unwrap_or(rest.len());
            let token = &rest[..end];
            styled.push((token_style(token), token.to_string()));
            rest = &rest[end..];
        }

        styled
    }
}

fn token_style(token: &str) -> Style {
    if is_integer(token) {
        Style::new().fg(Color::Cyan)
    } else if token == "true" || token == "false" {
        Style::new().fg(Color::Yellow)
    } else if token.starts_with('(') || token.ends_with(')') {
        Style::new().fg(Color::Magenta)
    } else if token.starts_with('[') || token.ends_with(']') {
        Style::new().fg(Color::Blue)
    } else if is_operator(token) {
        Style::new().fg(Color::White).bold()
    } else if token.starts_with('#') {
        Style::new().fg(Color::DarkGray)
    } else {
        // Unknown token — highlight red so typos are obvious.
        Style::new().fg(Color::Red)
    }
}

fn is_integer(s: &str) -> bool {
    let s = s.strip_prefix('-').unwrap_or(s);
    !s.is_empty() && s.chars().all(|c| c.is_ascii_digit())
}

// ── History file path ─────────────────────────────────────────────────────────

fn history_path() -> Option<PathBuf> {
    let home = std::env::var("HOME").ok()?;
    let dir = PathBuf::from(home).join(".local/share/ezc");
    std::fs::create_dir_all(&dir).ok()?;
    Some(dir.join("history"))
}

// ── Public entry point ────────────────────────────────────────────────────────

pub fn execute(no_tui: bool) -> Result<(), Box<dyn std::error::Error>> {
    if no_tui || !std::io::stdout().is_terminal() {
        plain()
    } else {
        crate::tui::run()
    }
}

// ── Plain REPL (reedline) ─────────────────────────────────────────────────────

fn plain() -> Result<(), Box<dyn std::error::Error>> {
    let hinter = DefaultHinter::default().with_style(Style::new().italic().fg(Color::DarkGray));

    let mut line_editor = Reedline::create()
        .with_completer(Box::new(EzcCompleter))
        .with_highlighter(Box::new(EzcHighlighter))
        .with_hinter(Box::new(hinter))
        .with_partial_completions(true)
        .with_quick_completions(true)
        .use_bracketed_paste(true);

    // Attach file-backed history if we can create the path.
    if let Some(path) = history_path() {
        if let Ok(history) = FileBackedHistory::with_file(2000, path) {
            line_editor = line_editor.with_history(Box::new(history));
        }
    }

    let prompt = EzcPrompt;
    let mut machine = Machine::new();

    println!("ezc repl — Tab to complete, Ctrl-R to search history, Ctrl-D to exit");

    loop {
        match line_editor.read_line(&prompt)? {
            Signal::Success(line) => {
                let line = line.trim();
                if line.is_empty() {
                    continue;
                }

                match ezc::eval_line(&mut machine, line) {
                    Ok(()) => {
                        let stack = machine.stack();
                        if !stack.is_empty() {
                            let parts: Vec<_> = stack.iter().map(|v| v.to_string()).collect();
                            println!("  {}", parts.join("  "));
                        }
                    }
                    Err(e) => {
                        // Full ariadne error report with source annotations.
                        eprint!("{}", e.report_string("<repl>", line));
                    }
                }
            }
            // Ctrl-C clears the current line but keeps the session running.
            Signal::CtrlC => continue,
            // Ctrl-D exits.
            Signal::CtrlD => {
                println!();
                break;
            }
        }
    }

    Ok(())
}
