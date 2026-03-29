//! Debug Adapter Protocol (DAP) server for the ezc language.
//!
//! This crate implements a synchronous DAP server that runs as `ezc debug`
//! (stdio transport). VS Code and other DAP-capable editors spawn this process
//! and communicate with it over stdin/stdout using Content-Length–framed JSON.
//!
//! # Protocol flow
//!
//! 1. VS Code sends `initialize` → server responds with `Capabilities`, emits
//!    `initialized` event.
//! 2. VS Code sends `launch` with `{ "program": "/path/to/file.ezc",
//!    "stopOnEntry": true }`, then `setBreakpoints` (zero or more times), then
//!    `configurationDone`. The debug session starts once both `launch` and
//!    `configurationDone` have been received (either order).
//! 3. Server parses the program, creates a `Stepper`, and if `stopOnEntry`
//!    emits a `stopped(entry)` event.  Otherwise runs until a breakpoint.
//!    Breakpoint `source.path` values are matched to `program` using normalized
//!    paths (e.g. `file:///…` vs absolute paths).
//! 4. VS Code sends stepping commands (`next`, `stepIn`, `stepOut`,
//!    `continue`). Server advances the stepper and emits `stopped` events.
//! 5. VS Code queries `stackTrace`, `scopes`, `variables`, and `evaluate`.
//! 6. When the program finishes, the server emits `terminated` + `exited`.

pub mod codec;
mod paths;
pub mod protocol;
pub mod server;

/// Entry point: run the DAP server on stdin/stdout until the session ends.
pub fn run() -> Result<(), Box<dyn std::error::Error>> {
    let stdin = std::io::stdin().lock();
    let stdout = std::io::stdout().lock();
    let reader = std::io::BufReader::new(stdin);
    let writer = std::io::BufWriter::new(stdout);
    let mut srv = server::DapServer::new(reader, writer);
    srv.run()
}
