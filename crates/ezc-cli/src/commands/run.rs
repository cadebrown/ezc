use std::path::Path;

use tracing::info;

pub fn execute(file: &Path) -> Result<(), Box<dyn std::error::Error>> {
    let src = std::fs::read_to_string(file)?;
    let filename = file.display().to_string();
    info!(file = %filename, "running");

    match ezc::run(&src) {
        Ok(stack) => {
            for value in &stack {
                println!("{value}");
            }
            Ok(())
        }
        Err(e) => {
            e.report(&filename, &src);
            Err(e.into())
        }
    }
}
