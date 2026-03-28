use crossterm::event::{KeyCode, KeyEvent, KeyModifiers};
use ezc::eval::{Engine, Tagged};
use ezc::types::Value;

/// Operators with short descriptions, for tab completion.
pub const COMPLETIONS: &[(&str, &str)] = &[
    // Math
    ("+", "add"),
    ("-", "subtract"),
    ("*", "multiply"),
    ("/", "divide"),
    ("%", "modulo"),
    ("^", "power"),
    // Comparison
    ("==", "equal"),
    ("!=", "not equal"),
    ("<", "less than"),
    (">", "greater than"),
    ("<=", "less or equal"),
    (">=", "greater or equal"),
    // Control
    ("!", "execute/splat/eval"),
    ("?", "conditional drop"),
    ("??", "ternary branch"),
    ("|", "compose/concat"),
    ("&", "loop"),
    ("&!", "map"),
    ("&?", "filter"),
    ("&/", "fold/reduce"),
    // Stack
    ("~", "swap top two"),
    (",", "dup (copy top)"),
    (";", "drop (discard top)"),
    ("_", "over (dup under)"),
    // I/O
    (":", "write (print + newline)"),
    (".", "read line"),
    ("rl", "read line"),
    ("wl", "write line"),
    ("rb", "read byte"),
    ("wb", "write byte"),
    // Type constructors
    ("int", "cast to bigint"),
    ("u8", "cast to u8"),
    ("u16", "cast to u16"),
    ("u32", "cast to u32"),
    ("u64", "cast to u64"),
    ("u128", "cast to u128"),
    ("u256", "cast to u256"),
    ("i8", "cast to i8"),
    ("i16", "cast to i16"),
    ("i32", "cast to i32"),
    ("i64", "cast to i64"),
    ("i128", "cast to i128"),
    ("i256", "cast to i256"),
    ("f16", "cast to f16"),
    ("f32", "cast to f32"),
    ("f64", "cast to f64"),
    ("str", "cast to string"),
    ("bin", "cast to binary"),
    // Builtins
    ("neg", "negate"),
    ("abs", "absolute value"),
    ("len", "length"),
    ("nth", "index into list"),
    ("iota", "range [0..n)"),
    ("typeof", "type name as str"),
    ("rev", "reverse list/str"),
    ("srt", "sort list"),
    ("hd", "head (first elem)"),
    ("tl", "tail (rest of list)"),
    ("not", "logical not"),
    ("and", "logical and"),
    ("or", "logical or"),
    ("min", "minimum of two"),
    ("max", "maximum of two"),
    ("cut", "split string"),
    ("cat", "join list"),
    ("if", "cond (block) if"),
    ("ifel", "cond (then) (else) ifel"),
    ("loop", "(cond) (body) loop"),
    ("each", "list/n (block) each"),
    ("map", "list (block) map"),
    ("fil", "list (block) filter"),
    ("red", "list init (block) reduce"),
];

/// Check whether a token string is a known operator (for syntax highlighting).
pub fn is_operator(s: &str) -> bool {
    COMPLETIONS.iter().any(|(op, _)| *op == s)
}

#[derive(Debug, Clone)]
pub struct CompletionState {
    pub items: Vec<(&'static str, &'static str)>,
    pub selected: usize,
}

impl CompletionState {
    fn new(prefix: &str) -> Option<Self> {
        let items: Vec<(&str, &str)> = COMPLETIONS
            .iter()
            .copied()
            .filter(|(op, _)| op.starts_with(prefix) && *op != prefix)
            .collect();
        if items.is_empty() {
            None
        } else {
            Some(CompletionState { items, selected: 0 })
        }
    }
}

pub struct App {
    pub engine: Engine,

    pub input: String,
    pub cursor: usize,

    pub history: Vec<HistoryEntry>,
    pub history_scroll: usize,

    pub cmd_history: Vec<String>,
    pub cmd_history_idx: Option<usize>,
    pub cmd_draft: String,

    pub snapshots: Vec<Vec<Tagged>>,
    pub completion: Option<CompletionState>,
    pub should_quit: bool,
}

pub struct HistoryEntry {
    pub input: String,
    pub kind: EntryKind,
    pub before: Vec<Value>,
    pub after: Option<Vec<Value>>,
}

pub enum EntryKind {
    Ok,
    Err(String),
}

impl App {
    pub fn new() -> Self {
        App {
            engine: ezc::engine(),
            input: String::new(),
            cursor: 0,
            history: Vec::new(),
            history_scroll: 0,
            cmd_history: Vec::new(),
            cmd_history_idx: None,
            cmd_draft: String::new(),
            snapshots: Vec::new(),
            completion: None,
            should_quit: false,
        }
    }

    /// Fish-style inline hint: find the most recent history entry starting with
    /// the current input and return the suffix (the part after what's already typed).
    pub fn history_hint(&self) -> Option<&str> {
        if self.input.is_empty() || self.cursor < self.input.len() {
            return None;
        }
        self.cmd_history
            .iter()
            .rev()
            .find(|cmd| cmd.starts_with(&self.input) && *cmd != &self.input)
            .map(|cmd| &cmd[self.input.len()..])
    }

    pub fn handle_key(&mut self, key: KeyEvent) {
        // Tab completion takes priority when active.
        if self.completion.is_some() {
            match key.code {
                KeyCode::Tab => {
                    self.completion_next();
                    return;
                }
                KeyCode::BackTab => {
                    self.completion_prev();
                    return;
                }
                KeyCode::Enter => {
                    self.completion_accept();
                    return;
                }
                KeyCode::Esc => {
                    self.completion = None;
                    return;
                }
                _ => {
                    self.completion = None;
                }
            }
        }

        match key.code {
            // Quit
            KeyCode::Char('c') if key.modifiers.contains(KeyModifiers::CONTROL) => {
                self.should_quit = true;
            }
            KeyCode::Char('d') if key.modifiers.contains(KeyModifiers::CONTROL) => {
                self.should_quit = true;
            }
            KeyCode::Esc => {
                self.should_quit = true;
            }

            // Clear
            KeyCode::Char('l') if key.modifiers.contains(KeyModifiers::CONTROL) => {
                self.engine.set_stack(vec![]);
                self.history.clear();
                self.snapshots.clear();
                self.history_scroll = 0;
            }

            // Undo
            KeyCode::Char('z') if key.modifiers.contains(KeyModifiers::CONTROL) => {
                self.undo();
            }

            // Submit
            KeyCode::Enter => {
                self.eval_input();
            }

            // Tab completion
            KeyCode::Tab => {
                self.try_complete();
            }

            // Command history recall
            KeyCode::Up => self.recall_prev(),
            KeyCode::Down => self.recall_next(),

            // Trace scroll
            KeyCode::PageUp => {
                self.history_scroll += 1;
            }
            KeyCode::PageDown => {
                self.history_scroll = self.history_scroll.saturating_sub(1);
            }

            // Cursor movement
            KeyCode::Right => {
                if self.cursor >= self.input.len() {
                    // At end of input: accept history hint if available.
                    if let Some(hint) = self.history_hint().map(|s| s.to_string()) {
                        self.input.push_str(&hint);
                        self.cursor = self.input.len();
                    }
                } else if key.modifiers.contains(KeyModifiers::CONTROL) {
                    self.cursor = self.word_right(self.cursor);
                } else {
                    self.cursor = self.next_char_boundary(self.cursor);
                }
            }
            KeyCode::Left => {
                if key.modifiers.contains(KeyModifiers::CONTROL) {
                    self.cursor = self.word_left(self.cursor);
                } else {
                    self.cursor = self.prev_char_boundary(self.cursor);
                }
            }
            KeyCode::Home => {
                self.cursor = 0;
            }
            KeyCode::End => {
                self.cursor = self.input.len();
            }

            // Editing
            KeyCode::Backspace => {
                if key.modifiers.contains(KeyModifiers::CONTROL) {
                    let target = self.word_left(self.cursor);
                    self.input.drain(target..self.cursor);
                    self.cursor = target;
                } else if self.cursor > 0 {
                    let prev = self.prev_char_boundary(self.cursor);
                    self.input.drain(prev..self.cursor);
                    self.cursor = prev;
                }
            }
            KeyCode::Delete => {
                if self.cursor < self.input.len() {
                    let next = self.next_char_boundary(self.cursor);
                    self.input.drain(self.cursor..next);
                }
            }
            KeyCode::Char(c) => {
                self.input.insert(self.cursor, c);
                self.cursor += c.len_utf8();
                self.cmd_history_idx = None;
            }

            _ => {}
        }
    }

    fn eval_input(&mut self) {
        let line = self.input.trim().to_string();
        self.input.clear();
        self.cursor = 0;
        self.cmd_history_idx = None;
        self.completion = None;

        if line.is_empty() {
            return;
        }

        if self.cmd_history.last().map(|s| s.as_str()) != Some(&line) {
            self.cmd_history.push(line.clone());
        }

        let before: Vec<Value> = self.engine.stack().into_iter().cloned().collect();
        let raw_snapshot = self.engine.clone_raw_stack();

        let entry = match ezc::eval_line(&mut self.engine, &line) {
            Ok(()) => {
                let after: Vec<Value> = self.engine.stack().into_iter().cloned().collect();
                self.snapshots.push(raw_snapshot);
                HistoryEntry {
                    input: line,
                    kind: EntryKind::Ok,
                    before,
                    after: Some(after),
                }
            }
            Err(e) => {
                // Full ariadne report with ANSI colors — parsed into ratatui styles.
                let msg = e.report_string("<repl>", &line);
                HistoryEntry {
                    input: line,
                    kind: EntryKind::Err(msg),
                    before,
                    after: None,
                }
            }
        };

        self.history.push(entry);
        self.history_scroll = 0;
    }

    fn undo(&mut self) {
        if let Some(snapshot) = self.snapshots.pop() {
            self.engine.set_stack(snapshot);
            if let Some(pos) = self
                .history
                .iter()
                .rposition(|e| matches!(e.kind, EntryKind::Ok))
            {
                self.history.remove(pos);
            }
        }
    }

    fn recall_prev(&mut self) {
        if self.cmd_history.is_empty() {
            return;
        }
        let idx = match self.cmd_history_idx {
            None => {
                self.cmd_draft = self.input.clone();
                self.cmd_history.len() - 1
            }
            Some(0) => 0,
            Some(i) => i - 1,
        };
        self.cmd_history_idx = Some(idx);
        self.input = self.cmd_history[idx].clone();
        self.cursor = self.input.len();
    }

    fn recall_next(&mut self) {
        match self.cmd_history_idx {
            None => {}
            Some(i) if i + 1 >= self.cmd_history.len() => {
                self.cmd_history_idx = None;
                self.input = self.cmd_draft.clone();
                self.cursor = self.input.len();
            }
            Some(i) => {
                let idx = i + 1;
                self.cmd_history_idx = Some(idx);
                self.input = self.cmd_history[idx].clone();
                self.cursor = self.input.len();
            }
        }
    }

    fn try_complete(&mut self) {
        let prefix = self.current_token_prefix();
        if prefix.is_empty() {
            return;
        }
        self.completion = CompletionState::new(&prefix);
    }

    fn completion_next(&mut self) {
        if let Some(ref mut c) = self.completion {
            if c.selected + 1 < c.items.len() {
                c.selected += 1;
            }
        }
    }

    fn completion_prev(&mut self) {
        if let Some(ref mut c) = self.completion {
            c.selected = c.selected.saturating_sub(1);
        }
    }

    fn completion_accept(&mut self) {
        if let Some(ref c) = self.completion {
            let (chosen, _) = c.items[c.selected];
            let prefix = self.current_token_prefix();
            let prefix_start = self.cursor - prefix.len();
            self.input.drain(prefix_start..self.cursor);
            self.input.insert_str(prefix_start, chosen);
            self.cursor = prefix_start + chosen.len();
        }
        self.completion = None;
    }

    fn current_token_prefix(&self) -> String {
        self.input[..self.cursor]
            .split_whitespace()
            .last()
            .unwrap_or("")
            .to_string()
    }

    fn prev_char_boundary(&self, pos: usize) -> usize {
        if pos == 0 {
            return 0;
        }
        let mut p = pos - 1;
        while !self.input.is_char_boundary(p) {
            p -= 1;
        }
        p
    }

    fn next_char_boundary(&self, pos: usize) -> usize {
        if pos >= self.input.len() {
            return self.input.len();
        }
        let mut p = pos + 1;
        while !self.input.is_char_boundary(p) {
            p += 1;
        }
        p
    }

    fn word_left(&self, pos: usize) -> usize {
        let bytes = self.input.as_bytes();
        let mut p = pos;
        while p > 0 && bytes[p - 1] == b' ' {
            p -= 1;
        }
        while p > 0 && bytes[p - 1] != b' ' {
            p -= 1;
        }
        p
    }

    fn word_right(&self, pos: usize) -> usize {
        let bytes = self.input.as_bytes();
        let len = self.input.len();
        let mut p = pos;
        while p < len && bytes[p] != b' ' {
            p += 1;
        }
        while p < len && bytes[p] == b' ' {
            p += 1;
        }
        p
    }
}
