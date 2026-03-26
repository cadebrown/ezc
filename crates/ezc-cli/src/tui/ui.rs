use ratatui::layout::{Constraint, Layout, Position, Rect};
use ratatui::style::{Color, Modifier, Style};
use ratatui::text::{Line, Span};
use ratatui::widgets::{Block, BorderType, Borders, Clear, List, ListItem, Paragraph};
use ratatui::Frame;

use ezc::types::Value;

use super::app::{App, EntryKind};

const PROMPT: &str = "Σ  ";
const PROMPT_DISPLAY_WIDTH: u16 = 3;

// ── Value rendering ───────────────────────────────────────────────────────────

fn value_color(v: &Value) -> Color {
    match v {
        Value::Int(_) => Color::Cyan,
        Value::Bool(_) => Color::Yellow,
        Value::Block(_) => Color::Magenta,
        Value::List(_) => Color::Blue,
    }
}

fn value_span(v: &Value) -> Span<'static> {
    Span::styled(v.to_string(), Style::default().fg(value_color(v)))
}

/// Compact stack snapshot: `[v1 v2 v3]` with colored values.
fn stack_spans(stack: &[Value], max_items: usize) -> Vec<Span<'static>> {
    let mut spans = vec![Span::styled("[", Style::default().fg(Color::DarkGray))];
    let len = stack.len();
    let skip = len.saturating_sub(max_items);

    if skip > 0 {
        spans.push(Span::styled(
            format!("...{skip} "),
            Style::default()
                .fg(Color::DarkGray)
                .add_modifier(Modifier::DIM),
        ));
    }

    for (i, v) in stack.iter().skip(skip).enumerate() {
        if i > 0 || skip > 0 {
            spans.push(Span::raw(" "));
        }
        spans.push(value_span(v));
    }

    spans.push(Span::styled("]", Style::default().fg(Color::DarkGray)));
    spans
}

// ── Syntax highlighting for input ─────────────────────────────────────────────

use ezc::token::Token;

/// Color for a lexer token.
fn token_style(tok: &Token) -> Style {
    match tok {
        Token::Int(_) => Style::default().fg(Color::Cyan),
        Token::Op(_) => Style::default()
            .fg(Color::White)
            .add_modifier(Modifier::BOLD),
        Token::OpenParen | Token::CloseParen => Style::default().fg(Color::Magenta),
        Token::OpenBracket | Token::CloseBracket => Style::default().fg(Color::Blue),
        // All operators: bold white.
        Token::Bang
        | Token::Question
        | Token::DoubleQuestion
        | Token::Pipe
        | Token::Amp
        | Token::AmpBang
        | Token::AmpQuestion
        | Token::Eq
        | Token::NotEq
        | Token::Lt
        | Token::Gt
        | Token::LtEq
        | Token::GtEq
        | Token::Tilde
        | Token::Colon
        | Token::Underscore => Style::default()
            .fg(Color::White)
            .add_modifier(Modifier::BOLD),
        Token::Dollar | Token::At => Style::default().fg(Color::DarkGray),
    }
}

/// Syntax-highlight input using the real lexer. Falls back to plain text if
/// the lexer rejects partial input (e.g. unclosed parens mid-typing).
fn highlight_input(input: &str) -> Vec<Span<'static>> {
    if input.is_empty() {
        return vec![];
    }

    // Try the real lexer — it gives us (Token, Span) with byte-accurate ranges.
    if let Ok(tokens) = ezc::lexer::lex(input) {
        let mut spans = Vec::new();
        let mut pos = 0usize;

        for (tok, span) in &tokens {
            let start = span.start;
            let end = span.end;

            // Gap before this token (whitespace).
            if start > pos {
                spans.push(Span::raw(input[pos..start].to_string()));
            }

            spans.push(Span::styled(
                input[start..end].to_string(),
                token_style(tok),
            ));
            pos = end;
        }

        // Trailing whitespace / text after last token.
        if pos < input.len() {
            spans.push(Span::raw(input[pos..].to_string()));
        }

        return spans;
    }

    // Fallback: render unhighlighted (lexer failed on partial input).
    vec![Span::raw(input.to_string())]
}

// ── Top-level draw ────────────────────────────────────────────────────────────

pub fn draw(frame: &mut Frame, app: &mut App) {
    let stack_depth = app.machine.stack().len();
    let term_height = frame.area().height;
    let max_stack = (term_height / 3).max(4);
    let stack_h = ((stack_depth as u16) + 2).clamp(3, max_stack);

    let [stack_area, trace_area, input_area, status_area] = Layout::vertical([
        Constraint::Length(stack_h),
        Constraint::Fill(1),
        Constraint::Length(1),
        Constraint::Length(1),
    ])
    .areas(frame.area());

    render_stack(frame, app, stack_area);
    render_trace(frame, app, trace_area);
    render_input(frame, app, input_area);
    render_status(frame, status_area);

    if app.completion.is_some() {
        render_completion(frame, app, input_area);
    }
}

// ── Stack pane ────────────────────────────────────────────────────────────────

fn render_stack(frame: &mut Frame, app: &App, area: Rect) {
    let stack = app.machine.stack();
    let depth = stack.len();

    let items: Vec<ListItem> = if stack.is_empty() {
        vec![ListItem::new(Line::from(Span::styled(
            "  ∅  empty",
            Style::default()
                .fg(Color::DarkGray)
                .add_modifier(Modifier::DIM),
        )))]
    } else {
        stack
            .iter()
            .enumerate()
            .map(|(i, v)| {
                let idx = depth - i;
                let line = Line::from(vec![
                    Span::styled(
                        format!(" {idx:>3}  "),
                        Style::default()
                            .fg(Color::DarkGray)
                            .add_modifier(Modifier::DIM),
                    ),
                    value_span(v),
                    Span::styled(
                        format!("  {}", v.type_name()),
                        Style::default()
                            .fg(Color::DarkGray)
                            .add_modifier(Modifier::DIM),
                    ),
                ]);
                ListItem::new(line)
            })
            .collect()
    };

    let list = List::new(items).block(
        Block::default()
            .title(" stack ")
            .borders(Borders::ALL)
            .border_type(BorderType::Rounded)
            .border_style(Style::default().fg(Color::DarkGray)),
    );

    frame.render_widget(list, area);
}

// ── Trace pane ────────────────────────────────────────────────────────────────

fn render_trace(frame: &mut Frame, app: &App, area: Rect) {
    let max_snap = 6;

    let items: Vec<ListItem> = app
        .history
        .iter()
        .flat_map(|entry| {
            match &entry.kind {
                EntryKind::Ok => {
                    let after = entry.after.as_deref().unwrap_or(&[]);

                    // Line 1: ▸ highlighted-input   [before] → [after]
                    let mut spans: Vec<Span> =
                        vec![Span::styled(" ▸ ", Style::default().fg(Color::Green))];
                    spans.extend(highlight_input(&entry.input));
                    spans.push(Span::styled("   ", Style::default()));
                    spans.extend(stack_spans(&entry.before, max_snap));
                    spans.push(Span::styled(" → ", Style::default().fg(Color::DarkGray)));
                    spans.extend(stack_spans(after, max_snap));

                    let line1 = ListItem::new(Line::from(spans));

                    // Line 2: ╰ pop/push diff
                    let (consumed, produced) = stack_diff(&entry.before, after);
                    if consumed.is_empty() && produced.is_empty() {
                        vec![line1]
                    } else {
                        let mut diff: Vec<Span> =
                            vec![Span::styled("   ╰ ", Style::default().fg(Color::DarkGray))];

                        if !consumed.is_empty() {
                            diff.push(Span::styled(
                                "pop ",
                                Style::default().fg(Color::Red).add_modifier(Modifier::DIM),
                            ));
                            for (i, v) in consumed.iter().enumerate() {
                                if i > 0 {
                                    diff.push(Span::raw(" "));
                                }
                                diff.push(value_span(v));
                            }
                        }
                        if !consumed.is_empty() && !produced.is_empty() {
                            diff.push(Span::styled("  ", Style::default()));
                        }
                        if !produced.is_empty() {
                            diff.push(Span::styled(
                                "push ",
                                Style::default()
                                    .fg(Color::Green)
                                    .add_modifier(Modifier::DIM),
                            ));
                            for (i, v) in produced.iter().enumerate() {
                                if i > 0 {
                                    diff.push(Span::raw(" "));
                                }
                                diff.push(value_span(v));
                            }
                        }

                        vec![line1, ListItem::new(Line::from(diff))]
                    }
                }
                EntryKind::Err(msg) => {
                    vec![ListItem::new(Line::from(vec![
                        Span::styled(" ✗ ", Style::default().fg(Color::Red)),
                        Span::styled(
                            entry.input.clone(),
                            Style::default().add_modifier(Modifier::BOLD),
                        ),
                        Span::styled(
                            format!("   {msg}"),
                            Style::default().fg(Color::Red).add_modifier(Modifier::DIM),
                        ),
                    ]))]
                }
            }
        })
        .collect();

    let total = items.len();
    let visible = (area.height as usize).saturating_sub(2);
    let scroll = app.history_scroll.min(total.saturating_sub(visible));
    let start = total.saturating_sub(visible + scroll);
    let end = total.saturating_sub(scroll);
    let visible_items: Vec<ListItem> = items.into_iter().skip(start).take(end - start).collect();

    let list = List::new(visible_items).block(
        Block::default()
            .title(" trace ")
            .borders(Borders::ALL)
            .border_type(BorderType::Rounded)
            .border_style(Style::default().fg(Color::DarkGray)),
    );

    frame.render_widget(list, area);
}

fn stack_diff<'a>(before: &'a [Value], after: &'a [Value]) -> (Vec<&'a Value>, Vec<&'a Value>) {
    let common = before
        .iter()
        .zip(after.iter())
        .take_while(|(a, b)| a == b)
        .count();
    (
        before[common..].iter().collect(),
        after[common..].iter().collect(),
    )
}

// ── Input line ────────────────────────────────────────────────────────────────

fn render_input(frame: &mut Frame, app: &App, area: Rect) {
    let mut spans = vec![Span::styled(
        PROMPT,
        Style::default()
            .fg(Color::Cyan)
            .add_modifier(Modifier::BOLD),
    )];

    // Syntax-highlighted input.
    spans.extend(highlight_input(&app.input));

    // Fish-style inline history hint (dimmed suffix after cursor).
    if let Some(hint) = app.history_hint() {
        spans.push(Span::styled(
            hint.to_string(),
            Style::default()
                .fg(Color::DarkGray)
                .add_modifier(Modifier::DIM),
        ));
    }

    frame.render_widget(Paragraph::new(Line::from(spans)), area);

    let cursor_x = area.x + PROMPT_DISPLAY_WIDTH + app.cursor as u16;
    frame.set_cursor_position(Position::new(cursor_x, area.y));
}

// ── Status bar ────────────────────────────────────────────────────────────────

fn render_status(frame: &mut Frame, area: Rect) {
    let dim = Style::default().fg(Color::DarkGray);
    let key = Style::default().fg(Color::Cyan).add_modifier(Modifier::DIM);

    let line = Line::from(vec![
        Span::styled(" Tab", key),
        Span::styled(" complete  ", dim),
        Span::styled("↑↓", key),
        Span::styled(" history  ", dim),
        Span::styled("→", key),
        Span::styled(" accept hint  ", dim),
        Span::styled("^Z", key),
        Span::styled(" undo  ", dim),
        Span::styled("^L", key),
        Span::styled(" clear  ", dim),
        Span::styled("Esc", key),
        Span::styled(" quit", dim),
    ]);

    frame.render_widget(Paragraph::new(line), area);
}

// ── Completion popup ──────────────────────────────────────────────────────────

fn render_completion(frame: &mut Frame, app: &App, input_area: Rect) {
    let completion = match &app.completion {
        Some(c) => c,
        None => return,
    };

    let max_items = 10usize;
    let show_count = completion.items.len().min(max_items);

    // Width: operator + gap + description + padding.
    let max_op_w = completion
        .items
        .iter()
        .map(|(op, _)| op.len())
        .max()
        .unwrap_or(2);
    let max_desc_w = completion
        .items
        .iter()
        .map(|(_, d)| d.len())
        .max()
        .unwrap_or(6);
    let popup_width = (max_op_w + max_desc_w + 6) as u16;
    let popup_width = popup_width.clamp(16, 50);
    let popup_height = show_count as u16 + 2;

    let x = input_area.x + PROMPT_DISPLAY_WIDTH;
    let y = input_area.y.saturating_sub(popup_height);

    let popup_area = Rect {
        x,
        y,
        width: popup_width,
        height: popup_height,
    };

    frame.render_widget(Clear, popup_area);

    let items: Vec<ListItem> = completion
        .items
        .iter()
        .take(max_items)
        .enumerate()
        .map(|(i, (op, desc))| {
            let selected = i == completion.selected;
            let op_style = if selected {
                Style::default()
                    .fg(Color::Black)
                    .bg(Color::Cyan)
                    .add_modifier(Modifier::BOLD)
            } else {
                Style::default()
                    .fg(Color::White)
                    .add_modifier(Modifier::BOLD)
            };
            let desc_style = if selected {
                Style::default().fg(Color::Black).bg(Color::Cyan)
            } else {
                Style::default().fg(Color::DarkGray)
            };
            let bg_style = if selected {
                Style::default().bg(Color::Cyan)
            } else {
                Style::default()
            };

            // Pad operator to align descriptions.
            let padded_op = format!(" {op:<width$}", width = max_op_w);
            let line = Line::from(vec![
                Span::styled(padded_op, op_style),
                Span::styled("  ", bg_style),
                Span::styled(format!("{desc} "), desc_style),
            ]);
            ListItem::new(line)
        })
        .collect();

    let list = List::new(items).block(
        Block::default()
            .title(" complete ")
            .borders(Borders::ALL)
            .border_type(BorderType::Rounded)
            .border_style(Style::default().fg(Color::Cyan)),
    );

    frame.render_widget(list, popup_area);
}
