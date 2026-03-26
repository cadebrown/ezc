# 001: TUI REPL

Status: work
Created: 2026-03-25
Updated: 2026-03-25

## Context

The current `ezc repl` command is a plain stdin loop — it works but gives no visibility
into the stack, no input history, and no syntax feedback. The TUI replaces it with a
ratatui-based interface that shows the stack live as you type, making the RPN model
much easier to explore and debug.

Dependencies (already in workspace Cargo.toml): `ratatui = "0.30"`, `crossterm = "0.29"`.

## Layout

```
┌─ stack ──────────────────────────────────────────┐
│ [3] 42                                           │
│ [2] [1 2 3]                                      │
│ [1] (2 *)           ← top of stack               │
├─ input ──────────────────────────────────────────┤
│ ezc> 3 4 +█                                      │
├─ history ────────────────────────────────────────┤
│ 3 4 +    → 7                                     │
│ [1 2 3]  → [1 2 3]                               │
└──────────────────────────────────────────────────┘
```

Stack pane fills most of the screen, grows from the bottom (top-of-stack at bottom
of pane). Input bar is a single line at the bottom. History is a scrollable log above
it showing each expression entered and the resulting top-of-stack.

## Plan

### Step 1 — Wire up ratatui in ezc-cli

Add `ratatui` and `crossterm` to `crates/ezc-cli/Cargo.toml` from workspace deps.

### Step 2 — Create `crates/ezc-cli/src/tui/` module

```
crates/ezc-cli/src/
├── tui/
│   ├── mod.rs      -- public fn run() -> Result<()>
│   ├── app.rs      -- App struct: machine, input buffer, history
│   └── ui.rs       -- draw() fn: renders the three panes
```

**`app.rs` — App state:**
```rust
pub struct App {
    pub machine: Machine,
    pub input: String,
    pub history: Vec<HistoryEntry>,
    pub scroll: usize,        // history scroll offset
    pub should_quit: bool,
}

pub struct HistoryEntry {
    pub input: String,
    pub result: HistoryResult,
}

pub enum HistoryResult {
    Stack(Vec<Value>),   // resulting stack top (or full stack?)
    Error(String),
}
```

**`mod.rs` — Event loop:**
```rust
pub fn run() -> Result<()> {
    // 1. enable_raw_mode(), alternate screen
    // 2. loop: terminal.draw(ui::draw) then poll crossterm events
    // 3. on Enter: eval input, push to history, clear input buffer
    // 4. on Esc/q or Ctrl-C: set should_quit
    // 5. on Ctrl-D (EOF): quit
    // 6. disable_raw_mode(), leave alternate screen on exit
}
```

**`ui.rs` — Rendering:**
- Use `ratatui::layout::Layout` with three vertical constraints:
  - Stack pane: `Min(3)`
  - Input bar: `Length(3)`
  - History pane: `Length(8)` or `Min(3)`
- Stack pane: list widget, items rendered bottom-up (reverse the stack slice for display)
- Input bar: paragraph widget with a cursor, styled with a border
- History pane: scrollable list of `HistoryEntry` items, most recent at bottom

### Step 3 — Hook into `commands/repl.rs`

Replace the current stdin loop body with `tui::run()`. Keep the old plain-stdin
path behind a `--no-tui` flag in case the terminal doesn't support raw mode.

```rust
// commands/repl.rs
pub fn execute(no_tui: bool) -> Result<(), Box<dyn std::error::Error>> {
    if no_tui || !std::io::stdout().is_terminal() {
        plain::run()  // current stdin loop, moved to plain.rs
    } else {
        tui::run()
    }
}
```

Add `--no-tui` flag to the `Repl` variant in `main.rs`:
```rust
Commands::Repl { no_tui: bool }
```

### Step 4 — Keybindings

| Key | Action |
|-----|--------|
| Enter | Eval input line, push to history, clear input |
| Backspace | Delete char |
| ←/→ | Move cursor in input |
| ↑/↓ | Scroll history |
| Ctrl-C / Esc | Quit |
| Ctrl-L | Clear stack |
| Ctrl-Z | Undo last eval (restore previous machine state) |

For undo: keep a `Vec<Vec<Value>>` stack of previous machine stacks. Each eval
snapshots the current stack before executing.

### Step 5 — Error display

Errors show inline in the history pane in red (using ratatui `Style`). The stack
does not change on error (eval is transactional — snapshot before, restore on error).

## Verification

```bash
cargo build -p ezc-cli           # compiles without warnings
cargo run -p ezc-cli -- repl     # TUI opens, shows empty stack
# type: 3 4 +  →  stack shows 7
# type: (2 *)  →  stack shows 7, (2 *)
# type: !      →  stack shows 14
# type: bad    →  error in history, stack unchanged
cargo run -p ezc-cli -- repl --no-tui  # falls back to plain stdin loop
```

## Status Log

- 2026-03-25: Created plan, ready to implement.
