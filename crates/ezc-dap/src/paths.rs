//! Normalize DAP `Source.path` values for comparison with `launch.program`.
//!
//! Implementation lives in `ezc` so the CLI stepper and DAP share identical rules.

/// Returns true if `a` and `b` refer to the same source file for breakpoint matching.
pub(crate) fn dap_source_paths_equivalent(a: &str, b: &str) -> bool {
    ezc::debug_source_paths_equivalent(a, b)
}
