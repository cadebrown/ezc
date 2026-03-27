# 006: DAP Debugger

Status: done
Created: 2026-03-27
Updated: 2026-03-27

## Context

Full Debug Adapter Protocol (DAP) implementation for EZC. Spans two new
crates (`ezc::stepper`, `ezc-dap`) and a wired VS Code extension.

## Architecture

- `crates/ezc/src/line_index.rs` — byte offset ↔ (line, col) mapping
- `crates/ezc/src/stepper.rs` — `Stepper` type: explicit call stack,
  `step_over`, `step_in`, `step_out`, `continue_execution`, conditional
  breakpoints, logpoints, scope/stack variable inspection
- `crates/ezc/src/eval.rs` — added `eval_one`, `peek_top`, `env_snapshot`
- `crates/ezc-dap/src/codec.rs` — Content-Length I/O framing
- `crates/ezc-dap/src/protocol.rs` — all DAP request/response/event types
- `crates/ezc-dap/src/server.rs` — synchronous DAP server loop, session
  management, variable reference store (bindings + value stack + nested lists)
- `ezc debug` CLI subcommand — spawns the DAP server on stdio
- VS Code extension — `DebugAdapterDescriptorFactory`, `DebugConfigurationProvider`,
  `ezc.debugFile` command, run button in editor title bar, `debuggers` and
  `breakpoints` contributions in `package.json`

## Features

- `stopOnEntry` — pause before executing the first instruction
- Line breakpoints with optional EZC condition expressions
- Logpoints (emit to debug console, no halt)
- Step over / step in / step out
- `step_in` intercepts `Execute (!)` when the stack top is a `Block`, pushing
  a new call frame for the block body
- Call stack panel shows all active frames
- "Variables" scope: all named bindings visible from the current scope chain
- "Value Stack" scope: the full value stack as indexed variables
- Nested `List` values expand recursively in the Variables panel
- Debug console `evaluate` — runs EZC expressions in the current engine state
- Exception breakpoint filter "All Exceptions" — halts on any eval error
- `source` request — returns source text for inline display

## Verification

- `cargo test --all` — 98 tests pass (94 existing + 4 new stepper/codec)
- `npm run compile` — TypeScript compiles cleanly
- `ezc debug` starts without error
- VS Code: F5 on a .ezc file launches a debug session
- Breakpoints halt at the correct line
- Variables panel shows bindings and value stack

## Lessons Learned

Borrow checker: `breakpoint_at_current()` borrows `self` immutably; subsequent
`eval_condition()` needs a mutable borrow. Fixed by cloning the breakpoint
fields (line/condition/log_message) out before calling `eval_condition`.

Auto-formatter hook kept commenting out `pub mod line_index` and `pub mod stepper`
in `lib.rs` after failed cargo checks. Fixed by ensuring all modules compiled
cleanly before the hook ran.

DAP flow: `launch` and `configurationDone` are separate requests. The session
must not start until `configurationDone` so that breakpoints set between
`initialized` and `configurationDone` are applied before the first stop.
