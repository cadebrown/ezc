use std::io::IsTerminal;
use std::path::PathBuf;

use clap::{Parser, Subcommand};

mod commands;
mod logging;
mod tui;

#[derive(Parser)]
#[command(
    name = "ezc",
    about = "The ezc programming language — stack-based, RPN, mathematical",
    version
)]
struct Cli {
    /// Source file to run. If omitted, starts the REPL (or reads stdin if piped).
    file: Option<PathBuf>,

    /// Evaluate a string as ezc code
    #[arg(short, long)]
    eval: Option<String>,

    /// Check for errors without running
    #[arg(short, long)]
    check: bool,

    /// Use plain terminal REPL instead of TUI
    #[arg(long)]
    no_tui: bool,

    /// Verbosity level (-v, -vv, -vvv)
    #[arg(short, long, action = clap::ArgAction::Count)]
    verbose: u8,

    #[command(subcommand)]
    command: Option<Commands>,
}

#[derive(Subcommand)]
enum Commands {
    /// Start the language server (communicates over stdio)
    Lsp,
}

fn main() {
    let cli = Cli::parse();
    logging::init(cli.verbose);

    let result = if let Some(cmd) = cli.command {
        match cmd {
            Commands::Lsp => commands::lsp::execute(),
        }
    } else if let Some(expr) = cli.eval {
        // ezc -e "3 4 +"
        run_string(&expr, cli.check)
    } else if let Some(file) = cli.file {
        // ezc file.ezc  or  ezc -c file.ezc
        if cli.check {
            commands::check::execute(&file)
        } else {
            commands::run::execute(&file)
        }
    } else if std::io::stdin().is_terminal() {
        // ezc  (interactive)
        commands::repl::execute(cli.no_tui)
    } else {
        // echo "3 4 +" | ezc  (piped stdin)
        run_stdin(cli.check)
    };

    if let Err(e) = result {
        eprintln!("error: {e}");
        std::process::exit(1);
    }
}

fn run_string(src: &str, check_only: bool) -> Result<(), Box<dyn std::error::Error>> {
    if check_only {
        ezc::lex_and_parse(src).inspect_err(|e| {
            e.report("<eval>", src);
        })?;
        eprintln!("OK");
    } else {
        match ezc::run(src) {
            Ok(stack) => {
                for value in &stack {
                    println!("{value}");
                }
            }
            Err(e) => {
                e.report("<eval>", src);
                return Err(e.into());
            }
        }
    }
    Ok(())
}

fn run_stdin(check_only: bool) -> Result<(), Box<dyn std::error::Error>> {
    use std::io::Read;
    let mut src = String::new();
    std::io::stdin().read_to_string(&mut src)?;
    run_string(&src, check_only)
}
