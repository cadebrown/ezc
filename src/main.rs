use std::{
    fs,
    path::{Path, PathBuf},
    process::ExitCode,
};

mod repl;

use clap::{Parser, Subcommand};
use ezc::{
    build_pipeline,
    error::EzcError,
    ezclang::{
        parser::{AstNode, Spanned},
        tokenizer::{SpannedToken, TokenKind},
    },
    run_source, run_source_verbose,
};
use tracing_subscriber::EnvFilter;

#[derive(Debug, Parser)]
#[command(author, version, about = "EZC compiler + VM", long_about = None)]
struct Cli {
    #[arg(long, help = "Enable tracing subscriber logs")]
    trace: bool,

    #[arg(long, help = "Print tokenizer/parser/bytecode/vm intermediate output")]
    verbose: bool,

    #[command(subcommand)]
    command: Command,
}

#[derive(Debug, Subcommand)]
enum Command {
    Run {
        file: PathBuf,
        #[arg(long, help = "Print bytecode before execution")]
        dump_bytecode: bool,
    },
    Eval {
        code: String,
        #[arg(long, help = "Print bytecode before execution")]
        dump_bytecode: bool,
    },
    Disasm {
        file: PathBuf,
    },
    Check {
        file: PathBuf,
    },
    Repl,
}

fn main() -> ExitCode {
    let cli = Cli::parse();

    init_tracing(cli.trace);

    let result = match cli.command {
        Command::Run {
            file,
            dump_bytecode,
        } => run_file(&file, dump_bytecode, cli.verbose),
        Command::Eval {
            code,
            dump_bytecode,
        } => run_text("<eval>", &code, dump_bytecode, cli.verbose),
        Command::Disasm { file } => disassemble_file(&file, cli.verbose),
        Command::Check { file } => check_file(&file, cli.verbose),
        Command::Repl => run_repl(),
    };

    if let Err(code) = result {
        return code;
    }

    ExitCode::SUCCESS
}

fn init_tracing(trace: bool) {
    if !trace {
        return;
    }

    let _ = tracing_subscriber::fmt()
        .with_env_filter(
            EnvFilter::from_default_env()
                .add_directive("ezc=trace".parse().expect("valid directive")),
        )
        .with_target(false)
        .compact()
        .try_init();
}

fn run_file(path: &Path, dump_bytecode: bool, verbose: bool) -> Result<(), ExitCode> {
    let source = read_file(path)?;
    run_text(&path.display().to_string(), &source, dump_bytecode, verbose)
}

fn run_text(name: &str, source: &str, dump_bytecode: bool, verbose: bool) -> Result<(), ExitCode> {
    if verbose {
        let run = run_source_verbose(name, source).map_err(report_error)?;
        print_pipeline(
            &run.pipeline.tokens,
            &run.pipeline.ast.nodes,
            &run.pipeline.bytecode.disassemble(),
        );
        print_execution_steps(&run.trace.steps);

        if !run.trace.result.stdout.is_empty() {
            println!("\n== Program Stdout ==\n{}", run.trace.result.stdout);
        }
        println!(
            "\n== Final Stack ==\n{}",
            format_stack(&run.trace.result.stack)
        );
        return Ok(());
    }

    if dump_bytecode {
        let pipeline = build_pipeline(name, source).map_err(report_error)?;
        println!("{}", pipeline.bytecode.disassemble());
        let mut vm = ezc::ezcvm::Vm::default();
        let result = vm.execute(&pipeline.bytecode).map_err(|err| {
            report_error(
                err.with_source_if_missing(name, source)
                    .with_debug("pipeline stage: vm"),
            )
        })?;
        if !result.stdout.is_empty() {
            println!("{}", result.stdout);
        }
        return Ok(());
    }

    let result = run_source(name, source).map_err(report_error)?;
    if !result.stdout.is_empty() {
        println!("{}", result.stdout);
    }

    Ok(())
}

fn disassemble_file(path: &Path, verbose: bool) -> Result<(), ExitCode> {
    let source = read_file(path)?;
    let name = path.display().to_string();
    let pipeline = build_pipeline(&name, &source).map_err(report_error)?;

    if verbose {
        print_pipeline(
            &pipeline.tokens,
            &pipeline.ast.nodes,
            &pipeline.bytecode.disassemble(),
        );
        return Ok(());
    }

    println!("{}", pipeline.bytecode.disassemble());
    Ok(())
}

fn check_file(path: &Path, verbose: bool) -> Result<(), ExitCode> {
    let source = read_file(path)?;
    let name = path.display().to_string();
    let pipeline = build_pipeline(&name, &source).map_err(report_error)?;

    if verbose {
        print_pipeline(
            &pipeline.tokens,
            &pipeline.ast.nodes,
            &pipeline.bytecode.disassemble(),
        );
    }

    println!(
        "OK: {name} ({} instructions)",
        pipeline.bytecode.instructions.len()
    );
    Ok(())
}

fn read_file(path: &Path) -> Result<String, ExitCode> {
    fs::read_to_string(path).map_err(|err| {
        eprintln!("failed to read {}: {err}", path.display());
        ExitCode::FAILURE
    })
}

fn report_error(err: EzcError) -> ExitCode {
    eprintln!("{}", err.render());
    ExitCode::FAILURE
}

fn run_repl() -> Result<(), ExitCode> {
    repl::run().map_err(|err| {
        eprintln!("repl failed: {err}");
        ExitCode::FAILURE
    })
}

fn print_pipeline(tokens: &[SpannedToken], nodes: &[Spanned<AstNode>], disassembly: &str) {
    println!("== Tokens ==");
    for (idx, (kind, span)) in tokens.iter().enumerate() {
        let kind_label = format_token_kind(kind);
        println!("{idx:04} {kind_label} @ {}..{}", span.start, span.end);
    }

    println!("\n== AST ==");
    print_ast(nodes, 0);

    println!("\n== Bytecode ==\n{disassembly}");
}

fn format_token_kind(kind: &TokenKind) -> String {
    match kind {
        TokenKind::Number(value) => format!("Number({value})"),
        TokenKind::Text(text) => format!("Text({text:?})"),
        TokenKind::Word(word) => format!("Word({word})"),
        TokenKind::LParen => "LParen".to_string(),
        TokenKind::RParen => "RParen".to_string(),
        TokenKind::LBracket => "LBracket".to_string(),
        TokenKind::RBracket => "RBracket".to_string(),
    }
}

fn print_ast(nodes: &[Spanned<AstNode>], depth: usize) {
    for (node, span) in nodes {
        let indent = "  ".repeat(depth);
        match node {
            AstNode::Number(value) => {
                println!("{indent}Number({value}) @ {}..{}", span.start, span.end)
            }
            AstNode::Text(text) => {
                println!("{indent}Text({text:?}) @ {}..{}", span.start, span.end)
            }
            AstNode::Word(word) => println!("{indent}Word({word}) @ {}..{}", span.start, span.end),
            AstNode::Block(inner) => {
                println!("{indent}Block @ {}..{}", span.start, span.end);
                print_ast(inner, depth + 1);
            }
            AstNode::Stack(inner) => {
                println!("{indent}Stack @ {}..{}", span.start, span.end);
                print_ast(inner, depth + 1);
            }
        }
    }
}

fn print_execution_steps(steps: &[ezc::ezcvm::ExecutionStep]) {
    println!("\n== VM Steps ==");
    for step in steps {
        let indent = "  ".repeat(step.depth);
        println!(
            "{indent}ip={} op={} span={}..{}\n{indent}  before: {}\n{indent}  after:  {}",
            step.ip,
            step.op,
            step.span.start,
            step.span.end,
            format_stack(&step.stack_before),
            format_stack(&step.stack_after)
        );
    }
}

fn format_stack(values: &[ezc::ezcbc::Value]) -> String {
    let inner = values
        .iter()
        .map(ezc::ezcbc::Value::to_source)
        .collect::<Vec<_>>()
        .join(" ");
    format!("[{inner}]")
}
