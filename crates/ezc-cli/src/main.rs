use std::io::IsTerminal;
use std::path::PathBuf;

use clap::{CommandFactory, Parser, Subcommand};
use clap_complete::Shell;

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

    /// Print the stack after each expression (trace mode)
    #[arg(long)]
    trace: bool,

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
    /// Start the debug adapter server (DAP, communicates over stdio)
    Debug,
    /// Print shell completions for the given shell
    Completions {
        /// The shell to generate completions for
        #[arg(value_enum)]
        shell: Shell,
    },
}

fn main() {
    let cli = Cli::parse();
    logging::init(cli.verbose);

    let result = if let Some(cmd) = cli.command {
        match cmd {
            Commands::Lsp => commands::lsp::execute(),
            Commands::Debug => commands::debug::execute(),
            Commands::Completions { shell } => {
                let mut cmd = Cli::command();
                clap_complete::generate(shell, &mut cmd, "ezc", &mut std::io::stdout());
                Ok(())
            }
        }
    } else if let Some(expr) = cli.eval {
        // ezc -e "3 4 +"
        run_string(&expr, cli.check, cli.trace)
    } else if let Some(file) = cli.file {
        // ezc file.ezc  or  ezc -c file.ezc
        if cli.check {
            commands::check::execute(&file)
        } else {
            commands::run::execute(&file, cli.trace)
        }
    } else if std::io::stdin().is_terminal() {
        // ezc  (interactive)
        commands::repl::execute(cli.no_tui)
    } else {
        // echo "3 4 +" | ezc  (piped stdin)
        run_stdin(cli.check, cli.trace)
    };

    if let Err(e) = result {
        eprintln!("error: {e}");
        std::process::exit(1);
    }
}

fn run_string(src: &str, check_only: bool, trace: bool) -> Result<(), Box<dyn std::error::Error>> {
    if check_only {
        ezc::lex_and_parse(src).inspect_err(|e| {
            e.report("<eval>", src);
        })?;
        eprintln!("OK");
    } else if trace {
        commands::run::execute_trace_src(src, "<eval>")?;
    } else {
        match ezc::run(src) {
            Ok(stack) => {
                for value in &stack {
                    println!("{}", value.print_string());
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

fn run_stdin(check_only: bool, trace: bool) -> Result<(), Box<dyn std::error::Error>> {
    use std::io::Read;
    let mut src = String::new();
    std::io::stdin().read_to_string(&mut src)?;
    run_string(&src, check_only, trace)
}
