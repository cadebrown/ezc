pub fn execute() -> Result<(), Box<dyn std::error::Error>> {
    ezc_lsp::run_server();
    Ok(())
}
