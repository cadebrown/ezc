pub mod error;
pub mod ezcbc;
pub mod ezclang;
pub mod ezcvm;

use error::EzcError;
use ezcbc::Bytecode;
use ezclang::{parser::AstProgram, tokenizer::SpannedToken};
use ezcvm::{ExecutionResult, ExecutionTrace};

/// Byte range in the original source string.
pub type Span = std::ops::Range<usize>;

#[derive(Debug, Clone)]
pub struct PipelineArtifacts {
    pub tokens: Vec<SpannedToken>,
    pub ast: AstProgram,
    pub bytecode: Bytecode,
}

#[derive(Debug, Clone)]
pub struct VerboseRun {
    pub pipeline: PipelineArtifacts,
    pub trace: ExecutionTrace,
}

pub fn build_pipeline(source_name: &str, source: &str) -> Result<PipelineArtifacts, EzcError> {
    // Keep stage boundaries explicit so structured errors can identify where failure occurred.
    tracing::debug!(%source_name, "tokenizing source");
    let tokens = ezclang::tokenizer::tokenize(source).map_err(|err| {
        err.with_source_if_missing(source_name, source)
            .with_debug("pipeline stage: tokenizer")
    })?;

    tracing::debug!(%source_name, token_count = tokens.len(), "parsing token stream");
    let ast = ezclang::parser::parse(&tokens, source.len()).map_err(|err| {
        err.with_source_if_missing(source_name, source)
            .with_debug("pipeline stage: parser")
    })?;

    tracing::debug!(%source_name, node_count = ast.nodes.len(), "lowering AST into bytecode");
    let bytecode = ezcbc::compile(&ast).map_err(|err| {
        err.with_source_if_missing(source_name, source)
            .with_debug("pipeline stage: compiler")
    })?;

    Ok(PipelineArtifacts {
        tokens,
        ast,
        bytecode,
    })
}

pub fn compile_source(source_name: &str, source: &str) -> Result<Bytecode, EzcError> {
    build_pipeline(source_name, source).map(|pipeline| pipeline.bytecode)
}

pub fn run_source(source_name: &str, source: &str) -> Result<ExecutionResult, EzcError> {
    let bytecode = compile_source(source_name, source)?;
    let mut vm = ezcvm::Vm::default();
    vm.execute(&bytecode).map_err(|err| {
        err.with_source_if_missing(source_name, source)
            .with_debug("pipeline stage: vm")
    })
}

pub fn run_source_verbose(source_name: &str, source: &str) -> Result<VerboseRun, EzcError> {
    let pipeline = build_pipeline(source_name, source)?;
    let mut vm = ezcvm::Vm::default();
    let trace = vm.execute_verbose(&pipeline.bytecode).map_err(|err| {
        err.with_source_if_missing(source_name, source)
            .with_debug("pipeline stage: vm (verbose)")
    })?;
    Ok(VerboseRun { pipeline, trace })
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::ezcbc::Value;

    #[test]
    fn evaluates_arithmetic_expression() {
        let result = run_source("test.ezc", "2 3 + 4 *").expect("program should run");
        assert_eq!(result.stack, vec![Value::Int(20)]);
        assert!(result.stdout.is_empty());
    }

    #[test]
    fn supports_printing_with_prt() {
        let result = run_source("test.ezc", "5 dup * prt").expect("program should run");
        assert_eq!(result.stack, vec![]);
        assert_eq!(result.stdout, "25");
    }

    #[test]
    fn supports_hash_comments() {
        let result = run_source("test.ezc", "5 # comment\n6 +").expect("program should run");
        assert_eq!(result.stack, vec![Value::Int(11)]);
    }
}
