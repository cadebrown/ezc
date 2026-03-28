fn main() {
    if let Err(e) = ezc_dap::run() {
        eprintln!("ezc-dap: {e}");
        std::process::exit(1);
    }
}
