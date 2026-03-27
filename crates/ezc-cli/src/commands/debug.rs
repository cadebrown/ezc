/// Launch the DAP debug server on stdin/stdout.
///
/// This command is invoked by VS Code (and other DAP clients) as a
/// `DebugAdapterExecutable`. The debug session configuration is received via
/// the DAP `launch` request — not as command-line arguments.
pub fn execute() -> Result<(), Box<dyn std::error::Error>> {
    ezc_dap::run()
}
