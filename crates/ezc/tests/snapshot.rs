use std::path::Path;

#[test]
fn test_ezc_programs() {
    let test_dir = Path::new(env!("CARGO_MANIFEST_DIR")).join("../../tests/ezc");

    // Walk all .ezc files and run each as a snapshot test.
    for entry in walkdir(&test_dir) {
        let path = entry;
        let src = std::fs::read_to_string(&path).unwrap();

        // Snapshot goes next to the .ezc file, named <file>.ezc.snap
        let snap_dir = path.parent().unwrap();
        let snap_name = path.file_name().unwrap().to_str().unwrap();

        let mut settings = insta::Settings::clone_current();
        settings.set_snapshot_path(snap_dir);
        settings.set_snapshot_suffix("");
        settings.set_prepend_module_to_snapshot(false);
        settings.set_input_file(&path);

        let result = ezc::run(&src);
        settings.bind(|| match result {
            Ok(stack) => {
                let output = stack
                    .iter()
                    .map(|v| v.to_string())
                    .collect::<Vec<_>>()
                    .join("\n");
                insta::assert_snapshot!(snap_name.to_string(), output);
            }
            Err(e) => {
                // Use ariadne report (plain text, no ANSI) for readable snapshots.
                let report = e.report_plain(snap_name, &src);
                insta::assert_snapshot!(snap_name.to_string(), report);
            }
        });
    }
}

/// Recursively find all .ezc files under a directory.
fn walkdir(dir: &Path) -> Vec<std::path::PathBuf> {
    let mut files = Vec::new();
    if let Ok(entries) = std::fs::read_dir(dir) {
        for entry in entries.flatten() {
            let path = entry.path();
            if path.is_dir() {
                files.extend(walkdir(&path));
            } else if path.extension().is_some_and(|e| e == "ezc") {
                files.push(path);
            }
        }
    }
    files.sort();
    files
}
