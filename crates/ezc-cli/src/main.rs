use std::path::PathBuf;

use clap::{Parser, Subcommand};

mod commands;
mod logging;
mod tui;

#[derive(Parser)]
#[command(
    name = "ezc",
    about = "The ezc programming language — stack-based, RPN, mathematical",
    version,
    propagate_version = true
)]
struct Cli {
    #[command(subcommand)]
    command: Commands,

    /// Verbosity level (-v, -vv, -vvv)
    #[arg(short, long, action = clap::ArgAction::Count, global = true)]
    verbose: u8,
}

#[derive(Subcommand)]
enum Commands {
    /// Run an ezc program
    Run {
        /// Path to the .ezc source file
        file: PathBuf,
    },
    /// Check an ezc program for errors without running it
    Check {
        /// Path to the .ezc source file
        file: PathBuf,
    },
    /// Start an interactive REPL (TUI by default)
    Repl {
        /// Use plain stdin loop instead of TUI
        #[arg(long)]
        no_tui: bool,
    },
}

fn main() {
    let cli = Cli::parse();
    logging::init(cli.verbose);

    let result = match cli.command {
        Commands::Run { file } => commands::run::execute(&file),
        Commands::Check { file } => commands::check::execute(&file),
        Commands::Repl { no_tui } => commands::repl::execute(no_tui),
    };

    if let Err(e) = result {
        eprintln!("error: {e}");
        std::process::exit(1);
    }
}
