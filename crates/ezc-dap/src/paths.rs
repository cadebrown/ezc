//! Normalize DAP `Source.path` values for comparison with `launch.program`.
//!
//! Clients may send `file:///…` URLs while `program` is a platform path (or the reverse).
//! Strict string equality then skips applying breakpoints.

use std::path::PathBuf;

/// Returns true if `a` and `b` refer to the same source file for breakpoint matching.
pub(crate) fn dap_source_paths_equivalent(a: &str, b: &str) -> bool {
    let pa = normalize_dap_path(a);
    let pb = normalize_dap_path(b);
    if pa == pb {
        return true;
    }
    // Symlinks / identical files different spelling (e.g. `/var` vs `/private/var` on macOS)
    match (std::fs::canonicalize(&pa), std::fs::canonicalize(&pb)) {
        (Ok(ca), Ok(cb)) => ca == cb,
        _ => false,
    }
}

fn normalize_dap_path(s: &str) -> PathBuf {
    let t = s.trim();
    if let Some(rest) = t.strip_prefix("file://") {
        return file_uri_to_path(rest);
    }
    PathBuf::from(t)
}

/// Path portion of a `file:` URI after the `file://` prefix (authority + path).
fn file_uri_to_path(rest: &str) -> PathBuf {
    if rest.starts_with('/') {
        // `file:///absolute` → rest is `/absolute`
        return PathBuf::from(rest);
    }
    // `file://localhost/path` or `file://host/path`
    if let Some(idx) = rest.find('/') {
        return PathBuf::from(&rest[idx..]);
    }
    PathBuf::from(rest)
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::io::Write;

    #[test]
    fn file_url_matches_absolute_path_unix() {
        assert!(dap_source_paths_equivalent(
            "file:///tmp/foo.ezc",
            "/tmp/foo.ezc"
        ));
    }

    #[test]
    fn plain_paths_equal() {
        assert!(dap_source_paths_equivalent(
            "/project/demo.ezc",
            "/project/demo.ezc"
        ));
    }

    #[test]
    fn different_files_not_equal() {
        assert!(!dap_source_paths_equivalent("/tmp/a.ezc", "/tmp/b.ezc"));
    }

    #[test]
    fn file_localhost_form() {
        assert!(dap_source_paths_equivalent(
            "file://localhost/tmp/x.ezc",
            "/tmp/x.ezc"
        ));
    }

    #[test]
    fn canonicalize_same_file() {
        let dir = std::env::temp_dir().join("ezc-dap-path-test");
        let _ = std::fs::remove_dir_all(&dir);
        std::fs::create_dir_all(&dir).unwrap();
        let file = dir.join("t.ezc");
        let mut f = std::fs::File::create(&file).unwrap();
        writeln!(f, "1").unwrap();
        drop(f);

        let canon = std::fs::canonicalize(&file).unwrap();
        let as_str = canon.to_string_lossy();
        assert!(
            dap_source_paths_equivalent(&as_str, file.to_str().unwrap()),
            "canonical vs non-canonical should match when file exists"
        );
        let _ = std::fs::remove_dir_all(&dir);
    }

    #[test]
    fn trims_whitespace() {
        assert!(dap_source_paths_equivalent("  /tmp/a.ezc  ", "/tmp/a.ezc"));
    }
}
