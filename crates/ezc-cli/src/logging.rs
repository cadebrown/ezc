use tracing_subscriber::EnvFilter;

/// Initialize tracing with the given verbosity level.
///
/// - 0: warn
/// - 1: info
/// - 2: debug
/// - 3+: trace
pub fn init(verbose: u8) {
    let level = match verbose {
        0 => "warn",
        1 => "info",
        2 => "debug",
        _ => "trace",
    };

    let filter = EnvFilter::try_from_default_env().unwrap_or_else(|_| EnvFilter::new(level));

    tracing_subscriber::fmt()
        .with_env_filter(filter)
        .with_target(false)
        .init();
}
