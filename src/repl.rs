use std::{io, time::Duration};

use crossterm::{
    event::{
        self, DisableMouseCapture, EnableMouseCapture, Event, KeyCode, KeyEvent, KeyEventKind,
        KeyModifiers,
    },
    execute,
    terminal::{disable_raw_mode, enable_raw_mode, EnterAlternateScreen, LeaveAlternateScreen},
};
use ezc::{
    build_pipeline,
    error::EzcError,
    ezcbc::{builtin_docs, Value},
    ezcvm::Vm,
};
use ratatui::{
    backend::CrosstermBackend,
    layout::{Constraint, Direction, Layout, Position, Rect},
    style::{Color, Modifier, Style},
    text::{Line, Span},
    widgets::{Block, Borders, Paragraph, Wrap},
    Terminal,
};
use unicode_width::UnicodeWidthStr;

pub fn run() -> io::Result<()> {
    enable_raw_mode()?;
    let mut stdout = io::stdout();
    execute!(stdout, EnterAlternateScreen, EnableMouseCapture)?;
    let backend = CrosstermBackend::new(stdout);
    let mut terminal = Terminal::new(backend)?;
    terminal.clear()?;

    let mut app = ReplApp::new();
    let loop_result = run_loop(&mut terminal, &mut app);

    disable_raw_mode()?;
    execute!(
        terminal.backend_mut(),
        LeaveAlternateScreen,
        DisableMouseCapture
    )?;
    terminal.show_cursor()?;

    loop_result
}

fn run_loop(
    terminal: &mut Terminal<CrosstermBackend<io::Stdout>>,
    app: &mut ReplApp,
) -> io::Result<()> {
    while !app.should_quit {
        terminal.draw(|frame| draw(frame.area(), frame, app))?;

        if event::poll(Duration::from_millis(120))? {
            match event::read()? {
                Event::Key(key) if key.kind == KeyEventKind::Press => handle_key(app, key),
                _ => {}
            }
        }
    }

    Ok(())
}

fn draw(area: Rect, frame: &mut ratatui::Frame<'_>, app: &ReplApp) {
    let chunks = Layout::default()
        .direction(Direction::Vertical)
        .constraints([
            Constraint::Length(4),
            Constraint::Min(1),
            Constraint::Length(1),
        ])
        .split(area);

    let mut header_lines = vec![Line::from(vec![
        Span::styled(
            "EZC REPL",
            Style::default()
                .fg(Color::Cyan)
                .add_modifier(Modifier::BOLD),
        ),
        Span::raw("  |  "),
        Span::styled("persistent stack", Style::default().fg(Color::LightBlue)),
        Span::raw("  |  Enter eval  Tab complete  Up/Down history"),
    ])];
    let hint = app
        .completion_hint
        .as_deref()
        .unwrap_or(":help :clear :quit  |  Tab shows operator docs and stack effects.");
    header_lines.push(Line::from(vec![Span::styled(
        hint,
        Style::default().fg(Color::Yellow),
    )]));

    let header = Paragraph::new(header_lines)
        .wrap(Wrap { trim: false })
        .block(Block::default().borders(Borders::ALL).title("Help"));

    frame.render_widget(header, chunks[0]);

    let rendered_lines = app.render_lines();
    let visible_lines = chunks[1].height as usize;
    let scroll = rendered_lines.len().saturating_sub(visible_lines) as u16;

    let output = Paragraph::new(rendered_lines)
        .wrap(Wrap { trim: false })
        .scroll((scroll, 0));

    frame.render_widget(output, chunks[1]);

    let prompt = "∑ ";
    let input = Paragraph::new(Line::from(vec![
        Span::styled(
            prompt,
            Style::default()
                .fg(Color::Green)
                .add_modifier(Modifier::BOLD),
        ),
        Span::raw(&app.input),
    ]));

    frame.render_widget(input, chunks[2]);

    let prompt_width = UnicodeWidthStr::width(prompt) as u16;
    let input_width = UnicodeWidthStr::width(app.input.as_str()) as u16;
    let mut cursor_x = chunks[2]
        .x
        .saturating_add(prompt_width.saturating_add(input_width));
    let max_x = chunks[2]
        .x
        .saturating_add(chunks[2].width.saturating_sub(1));
    if cursor_x > max_x {
        cursor_x = max_x;
    }
    let cursor_y = chunks[2].y;
    frame.set_cursor_position(Position::new(cursor_x, cursor_y));
}

fn handle_key(app: &mut ReplApp, key: KeyEvent) {
    if key.modifiers.contains(KeyModifiers::CONTROL) && key.code == KeyCode::Char('c') {
        app.should_quit = true;
        return;
    }

    match key.code {
        KeyCode::Esc => app.should_quit = true,
        KeyCode::Enter => app.submit_current_input(),
        KeyCode::Tab => app.complete_input(),
        KeyCode::Backspace => {
            app.input.pop();
            app.history_cursor = None;
            app.clear_completion_state();
        }
        KeyCode::Up => {
            app.history_up();
            app.clear_completion_state();
        }
        KeyCode::Down => {
            app.history_down();
            app.clear_completion_state();
        }
        KeyCode::Char(ch) => {
            if key.modifiers.is_empty() || key.modifiers == KeyModifiers::SHIFT {
                app.input.push(ch);
                app.history_cursor = None;
                app.clear_completion_state();
            }
        }
        _ => {}
    }
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
enum EntryKind {
    Input,
    Output,
    Stack,
    Error,
    System,
}

#[derive(Debug, Clone)]
struct ReplEntry {
    kind: EntryKind,
    text: String,
}

#[derive(Debug, Clone)]
struct CompletionCandidate {
    replacement: String,
    hint: String,
}

#[derive(Debug, Clone)]
struct CompletionState {
    start: usize,
    prefix: String,
    candidates: Vec<CompletionCandidate>,
    index: usize,
}

#[derive(Debug, Default)]
struct ReplApp {
    vm: Vm,
    input: String,
    entries: Vec<ReplEntry>,
    history: Vec<String>,
    history_cursor: Option<usize>,
    completion_state: Option<CompletionState>,
    completion_hint: Option<String>,
    should_quit: bool,
}

impl ReplApp {
    fn new() -> Self {
        let mut app = Self::default();
        app.push_system("Welcome to EZC. Type :help for commands.");
        app
    }

    fn submit_current_input(&mut self) {
        let line = self.input.trim().to_string();
        self.input.clear();
        self.history_cursor = None;
        self.clear_completion_state();

        if line.is_empty() {
            return;
        }

        self.push(EntryKind::Input, format!("∑ {line}"));
        self.history.push(line.clone());

        match line.as_str() {
            ":q" | ":quit" | ":exit" => {
                self.push_system("Exiting REPL.");
                self.should_quit = true;
                return;
            }
            ":clear" => {
                self.entries.clear();
                self.push_system("Transcript cleared.");
                return;
            }
            ":help" => {
                self.push_system(":help  show commands");
                self.push_system(":clear clear transcript");
                self.push_system(":quit  exit REPL");
                self.push_system("Stack persists across each evaluated line.");
                self.push_system("Tab complete shows builtin docs and stack effects.");
                return;
            }
            _ => {}
        }

        let pipeline = match build_pipeline("<repl>", &line) {
            Ok(pipeline) => pipeline,
            Err(err) => {
                for line in summarize_error(&err) {
                    self.push(EntryKind::Error, line);
                }
                return;
            }
        };

        let vm_snapshot = self.vm.clone();
        match self.vm.execute(&pipeline.bytecode).map_err(|err| {
            err.with_source_if_missing("<repl>", &line)
                .with_debug("pipeline stage: vm")
        }) {
            Ok(result) => {
                if !result.stdout.is_empty() {
                    for out_line in result.stdout.lines() {
                        self.push(EntryKind::Output, out_line.to_string());
                    }
                }

                self.push(EntryKind::Stack, format_stack(&result.stack));
            }
            Err(err) => {
                self.vm = vm_snapshot;
                for line in summarize_error(&err) {
                    self.push(EntryKind::Error, line);
                }
            }
        }
    }

    fn history_up(&mut self) {
        if self.history.is_empty() {
            return;
        }

        let next = match self.history_cursor {
            Some(idx) if idx > 0 => idx - 1,
            Some(_) => 0,
            None => self.history.len() - 1,
        };

        self.history_cursor = Some(next);
        self.input = self.history[next].clone();
    }

    fn history_down(&mut self) {
        let Some(idx) = self.history_cursor else {
            return;
        };

        if idx + 1 >= self.history.len() {
            self.history_cursor = None;
            self.input.clear();
            return;
        }

        let next = idx + 1;
        self.history_cursor = Some(next);
        self.input = self.history[next].clone();
    }

    fn push_system(&mut self, text: impl Into<String>) {
        self.push(EntryKind::System, text.into());
    }

    fn clear_completion_state(&mut self) {
        self.completion_state = None;
        self.completion_hint = None;
    }

    fn complete_input(&mut self) {
        if self.input.is_empty() {
            self.completion_hint =
                Some("Type a builtin/operator prefix or :command, then press Tab.".to_string());
            self.completion_state = None;
            return;
        }

        let start = current_token_start(&self.input);
        let suffix = self.input[start..].to_string();
        if suffix.is_empty() {
            self.completion_hint =
                Some("Type a builtin/operator prefix or :command, then press Tab.".to_string());
            self.completion_state = None;
            return;
        }

        if let Some(state) = &mut self.completion_state {
            let current_suffix = &self.input[start..];
            let active = &state.candidates[state.index].replacement;
            // Repeated Tab cycles candidates as long as the cursor is still on the same token.
            if state.start == start && (current_suffix == active || current_suffix == state.prefix)
            {
                state.index = (state.index + 1) % state.candidates.len();
                let candidate = &state.candidates[state.index];
                self.input.replace_range(start.., &candidate.replacement);
                self.completion_hint = Some(candidate.hint.clone());
                return;
            }
        }

        let candidates = completion_candidates(&suffix);
        if candidates.is_empty() {
            self.completion_state = None;
            self.completion_hint = Some(format!("No completion for `{suffix}`"));
            return;
        }

        let candidate = candidates[0].clone();
        self.input.replace_range(start.., &candidate.replacement);
        self.completion_hint = Some(candidate.hint.clone());
        self.completion_state = Some(CompletionState {
            start,
            prefix: suffix,
            candidates,
            index: 0,
        });
    }

    fn push(&mut self, kind: EntryKind, text: impl Into<String>) {
        let text = text.into();
        for line in text.lines() {
            self.entries.push(ReplEntry {
                kind,
                text: line.to_string(),
            });
        }
    }

    fn render_lines(&self) -> Vec<Line<'static>> {
        self.entries
            .iter()
            .map(|entry| {
                let style = style_for_entry(entry.kind);
                Line::from(vec![Span::styled(entry.text.clone(), style)])
            })
            .collect()
    }
}

fn style_for_entry(kind: EntryKind) -> Style {
    match kind {
        EntryKind::Input => Style::default().fg(Color::Cyan),
        EntryKind::Output => Style::default().fg(Color::Green),
        EntryKind::Stack => Style::default().fg(Color::Magenta),
        EntryKind::Error => Style::default().fg(Color::Red),
        EntryKind::System => Style::default().fg(Color::Yellow),
    }
}

fn current_token_start(input: &str) -> usize {
    // Completion only targets the final whitespace-delimited token.
    for (idx, ch) in input.char_indices().rev() {
        if ch.is_whitespace() {
            return idx + ch.len_utf8();
        }
    }
    0
}

fn completion_candidates(prefix: &str) -> Vec<CompletionCandidate> {
    const COMMANDS: &[(&str, &str)] = &[
        (":help", "Show REPL command help."),
        (":clear", "Clear transcript output."),
        (":quit", "Exit the REPL session."),
        (":exit", "Exit the REPL session."),
        (":q", "Exit the REPL session."),
    ];

    let mut candidates = Vec::new();

    if prefix.starts_with(':') {
        for (command, summary) in COMMANDS {
            if command.starts_with(prefix) {
                candidates.push(CompletionCandidate {
                    replacement: (*command).to_string(),
                    hint: format!("{command}: {summary}"),
                });
            }
        }
    } else {
        for doc in builtin_docs() {
            for word in std::iter::once(doc.canonical).chain(doc.aliases.iter().copied()) {
                if word.starts_with(prefix) {
                    let alias_note = if word == doc.canonical {
                        String::new()
                    } else {
                        format!(" (alias of `{}`)", doc.canonical)
                    };
                    candidates.push(CompletionCandidate {
                        replacement: word.to_string(),
                        hint: format!(
                            "{word}{alias_note}: {} | stack {} -> {} | {}",
                            doc.summary,
                            doc.stack_effect.before,
                            doc.stack_effect.after,
                            doc.details
                        ),
                    });
                }
            }
        }
    }

    candidates.sort_by(|a, b| a.replacement.cmp(&b.replacement));
    candidates.dedup_by(|a, b| a.replacement == b.replacement);
    candidates
}

fn format_stack(values: &[Value]) -> String {
    let inner = values
        .iter()
        .map(Value::to_source)
        .collect::<Vec<_>>()
        .join(" ");
    format!("[{inner}]")
}

fn summarize_error(err: &EzcError) -> Vec<String> {
    let mut lines = Vec::new();
    lines.push(format!(
        "[{}@{}..{}] {}: {}",
        err.code.id(),
        err.span.start,
        err.span.end,
        err.code.title(),
        err.message
    ));

    for note in &err.notes {
        lines.push(format!("note: {note}"));
    }
    for help in &err.helps {
        lines.push(format!("help: {help}"));
    }
    for debug in &err.debug {
        lines.push(format!("debug: {debug}"));
    }

    lines
}

#[cfg(test)]
mod tests {
    use super::*;

    fn last_entry_text(app: &ReplApp, kind: EntryKind) -> Option<&str> {
        app.entries
            .iter()
            .rev()
            .find(|entry| entry.kind == kind)
            .map(|entry| entry.text.as_str())
    }

    #[test]
    fn repl_stack_persists_across_evaluations() {
        let mut app = ReplApp::new();

        app.input = "2 3 +".to_string();
        app.submit_current_input();
        assert_eq!(last_entry_text(&app, EntryKind::Stack), Some("[5]"));

        app.input = "4 *".to_string();
        app.submit_current_input();
        assert_eq!(last_entry_text(&app, EntryKind::Stack), Some("[20]"));
    }

    #[test]
    fn repl_rolls_back_stack_on_runtime_error() {
        let mut app = ReplApp::new();

        app.input = "10".to_string();
        app.submit_current_input();
        assert_eq!(last_entry_text(&app, EntryKind::Stack), Some("[10]"));

        app.input = "0 /".to_string();
        app.submit_current_input();
        assert!(last_entry_text(&app, EntryKind::Error).is_some());

        app.input = "2 +".to_string();
        app.submit_current_input();
        assert_eq!(last_entry_text(&app, EntryKind::Stack), Some("[12]"));
    }

    #[test]
    fn repl_uses_sigma_for_input_entries() {
        let mut app = ReplApp::new();

        app.input = "1".to_string();
        app.submit_current_input();

        assert_eq!(last_entry_text(&app, EntryKind::Input), Some("∑ 1"));
    }

    #[test]
    fn tab_completion_resolves_canonical_builtin_words() {
        let mut app = ReplApp::new();

        app.input = "sw".to_string();
        app.complete_input();
        assert_eq!(app.input, "swp");
        assert!(app
            .completion_hint
            .as_deref()
            .is_some_and(|hint| hint.contains("stack ... a b -> ... b a")));

        app.complete_input();
        assert_eq!(app.input, "swp");
        assert!(app
            .completion_hint
            .as_deref()
            .is_some_and(|hint| hint.contains("Swaps the top two stack values")));
    }

    #[test]
    fn tab_completion_works_for_commands() {
        let mut app = ReplApp::new();

        app.input = ":he".to_string();
        app.complete_input();
        assert_eq!(app.input, ":help");
        assert!(app
            .completion_hint
            .as_deref()
            .is_some_and(|hint| hint.contains("Show REPL command help")));
    }
}
