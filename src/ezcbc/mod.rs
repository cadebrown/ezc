use std::fmt;

use crate::{
    error::{ErrorCode, EzcError},
    ezclang::parser::{AstNode, AstProgram, Spanned},
    Span,
};

#[derive(Debug, Clone, PartialEq, Eq)]
pub enum Value {
    Int(i64),
    Text(String),
    Symbol(String),
    Block(BlockValue),
    Stack(Vec<Value>),
}

impl Value {
    pub fn is_truthy(&self) -> bool {
        match self {
            Self::Int(value) => *value != 0,
            Self::Text(value) => !value.is_empty(),
            Self::Symbol(value) => !value.is_empty(),
            Self::Block(_) => true,
            Self::Stack(values) => !values.is_empty(),
        }
    }

    pub fn to_source(&self) -> String {
        match self {
            Self::Int(value) => value.to_string(),
            Self::Text(text) => format!("\"{}\"", escape_string(text)),
            Self::Symbol(symbol) => symbol.clone(),
            Self::Block(block) => format!("({})", block.bytecode.to_inline_source()),
            Self::Stack(values) => {
                let inner = values
                    .iter()
                    .map(Self::to_source)
                    .collect::<Vec<_>>()
                    .join(" ");
                format!("[{inner}]")
            }
        }
    }
}

fn escape_string(text: &str) -> String {
    let mut out = String::new();
    for ch in text.chars() {
        match ch {
            '\\' => out.push_str("\\\\"),
            '"' => out.push_str("\\\""),
            '\n' => out.push_str("\\n"),
            '\r' => out.push_str("\\r"),
            '\t' => out.push_str("\\t"),
            _ => out.push(ch),
        }
    }
    out
}

impl fmt::Display for Value {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "{}", self.to_source())
    }
}

#[derive(Debug, Clone, PartialEq, Eq)]
pub struct BlockValue {
    pub bytecode: Bytecode,
}

#[derive(Debug, Clone, PartialEq, Eq)]
pub enum OpCode {
    Push(Value),
    Add,
    Sub,
    Mul,
    Div,
    Mod,
    Dup,
    Drop,
    Swap,
    Over,
    Eq,
    Lt,
    Gt,
    And,
    Or,
    Not,
    Print,
    Exec,
    Select,
    Loop,
}

impl OpCode {
    pub fn mnemonic(&self) -> &'static str {
        match self {
            Self::Push(_) => "push",
            Self::Add => "+",
            Self::Sub => "-",
            Self::Mul => "*",
            Self::Div => "/",
            Self::Mod => "%",
            Self::Dup => "dup",
            Self::Drop => "del",
            Self::Swap => "swp",
            Self::Over => "ovr",
            Self::Eq => "=",
            Self::Lt => "<",
            Self::Gt => ">",
            Self::And => "&",
            Self::Or => "|",
            Self::Not => "not",
            Self::Print => "prt",
            Self::Exec => "!",
            Self::Select => "?",
            Self::Loop => "^",
        }
    }
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct StackEffectDoc {
    pub before: &'static str,
    pub after: &'static str,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct BuiltinDoc {
    pub canonical: &'static str,
    pub aliases: &'static [&'static str],
    pub stack_effect: StackEffectDoc,
    pub summary: &'static str,
    pub details: &'static str,
    pub examples: &'static [&'static str],
}

const BUILTIN_DOCS: &[BuiltinDoc] = &[
    BuiltinDoc {
        canonical: "+",
        aliases: &[],
        stack_effect: StackEffectDoc {
            before: "... a b",
            after: "... (a+b)",
        },
        summary: "Adds the top two integers.",
        details: "Pops `b`, then `a`, then pushes `a + b`.",
        examples: &["2 3 +", "10 -4 +"],
    },
    BuiltinDoc {
        canonical: "-",
        aliases: &[],
        stack_effect: StackEffectDoc {
            before: "... a b",
            after: "... (a-b)",
        },
        summary: "Subtracts top integer from next integer.",
        details: "Pops `b`, then `a`, then pushes `a - b`.",
        examples: &["9 2 -", "0 5 -"],
    },
    BuiltinDoc {
        canonical: "*",
        aliases: &[],
        stack_effect: StackEffectDoc {
            before: "... a b",
            after: "... (a*b)",
        },
        summary: "Multiplies the top two integers.",
        details: "Pops `b`, then `a`, then pushes `a * b`.",
        examples: &["6 7 *", "5 -2 *"],
    },
    BuiltinDoc {
        canonical: "/",
        aliases: &[],
        stack_effect: StackEffectDoc {
            before: "... a b",
            after: "... (a/b)",
        },
        summary: "Integer division on top two integers.",
        details: "Pops `b`, then `a`, then pushes integer quotient `a / b`.",
        examples: &["20 4 /", "7 2 /"],
    },
    BuiltinDoc {
        canonical: "%",
        aliases: &[],
        stack_effect: StackEffectDoc {
            before: "... a b",
            after: "... (a%b)",
        },
        summary: "Integer modulo on top two integers.",
        details: "Pops `b`, then `a`, then pushes remainder `a % b`.",
        examples: &["10 3 %", "9 2 %"],
    },
    BuiltinDoc {
        canonical: "dup",
        aliases: &[","],
        stack_effect: StackEffectDoc {
            before: "... x",
            after: "... x x",
        },
        summary: "Duplicates the top stack value.",
        details: "Leaves the original value and pushes a copy. Alias: `,`.",
        examples: &["5 dup *", "5 , *"],
    },
    BuiltinDoc {
        canonical: "del",
        aliases: &["."],
        stack_effect: StackEffectDoc {
            before: "... x",
            after: "...",
        },
        summary: "Removes the top stack value.",
        details: "Pops and discards one value. Alias: `.`.",
        examples: &["1 2 del", "1 2 ."],
    },
    BuiltinDoc {
        canonical: "swp",
        aliases: &["~"],
        stack_effect: StackEffectDoc {
            before: "... a b",
            after: "... b a",
        },
        summary: "Swaps the top two stack values.",
        details: "Exchange the top pair in-place. Alias: `~`.",
        examples: &["1 2 swp", "1 2 ~"],
    },
    BuiltinDoc {
        canonical: "ovr",
        aliases: &["_"],
        stack_effect: StackEffectDoc {
            before: "... a b",
            after: "... a b a",
        },
        summary: "Copies second value to top of stack.",
        details: "Duplicates the value one below the top. Alias: `_`.",
        examples: &["3 4 ovr", "3 4 _"],
    },
    BuiltinDoc {
        canonical: "=",
        aliases: &[],
        stack_effect: StackEffectDoc {
            before: "... a b",
            after: "... bool",
        },
        summary: "Compares top two values for equality.",
        details: "Pushes `1` when equal, else `0`.",
        examples: &["4 4 =", "\"a\" \"b\" ="],
    },
    BuiltinDoc {
        canonical: "<",
        aliases: &[],
        stack_effect: StackEffectDoc {
            before: "... a b",
            after: "... bool",
        },
        summary: "Checks integer less-than.",
        details: "Pushes `1` when `a < b`, else `0`.",
        examples: &["2 5 <", "7 1 <"],
    },
    BuiltinDoc {
        canonical: ">",
        aliases: &[],
        stack_effect: StackEffectDoc {
            before: "... a b",
            after: "... bool",
        },
        summary: "Checks integer greater-than.",
        details: "Pushes `1` when `a > b`, else `0`.",
        examples: &["9 3 >", "2 8 >"],
    },
    BuiltinDoc {
        canonical: "&",
        aliases: &["and"],
        stack_effect: StackEffectDoc {
            before: "... a b",
            after: "... bool",
        },
        summary: "Truthy AND over top two values.",
        details: "Truthy means non-zero int, non-empty text/symbol/stack, or any block.",
        examples: &["1 1 &", "1 1 and"],
    },
    BuiltinDoc {
        canonical: "|",
        aliases: &["or"],
        stack_effect: StackEffectDoc {
            before: "... a b",
            after: "... bool",
        },
        summary: "Truthy OR over top two values.",
        details: "Truthy means non-zero int, non-empty text/symbol/stack, or any block.",
        examples: &["0 3 |", "0 3 or"],
    },
    BuiltinDoc {
        canonical: "not",
        aliases: &[],
        stack_effect: StackEffectDoc {
            before: "... x",
            after: "... bool",
        },
        summary: "Logical NOT for one value.",
        details: "Pushes `1` for falsy input, else `0`.",
        examples: &["0 not", "\"\" not"],
    },
    BuiltinDoc {
        canonical: "prt",
        aliases: &[],
        stack_effect: StackEffectDoc {
            before: "... x",
            after: "...",
        },
        summary: "Prints and removes the top value.",
        details: "Text values print raw, other values print EZC source form.",
        examples: &["\"hello\" prt", "5 dup * prt"],
    },
    BuiltinDoc {
        canonical: "!",
        aliases: &[],
        stack_effect: StackEffectDoc {
            before: "... (block)",
            after: "...",
        },
        summary: "Executes a delayed code block.",
        details: "Pops one block value and runs it immediately.",
        examples: &["(2 3 + prt) !", "(dup *) !"],
    },
    BuiltinDoc {
        canonical: "?",
        aliases: &[],
        stack_effect: StackEffectDoc {
            before: "... if_true if_false cond",
            after: "... result",
        },
        summary: "Selects between two values by condition.",
        details: "If `cond` is truthy, pushes `if_true`; otherwise pushes `if_false`.",
        examples: &["10 20 1 ?", "10 20 0 ?"],
    },
    BuiltinDoc {
        canonical: "^",
        aliases: &[],
        stack_effect: StackEffectDoc {
            before: "... (block)",
            after: "...",
        },
        summary: "Runs loop block until condition becomes falsy.",
        details: "Each iteration executes `block`; block must leave one condition value on top.",
        examples: &["0 (dup prt 1 + dup 5 <) ^", "10 (dup prt 1 - dup) ^"],
    },
];

pub fn builtin_docs() -> &'static [BuiltinDoc] {
    BUILTIN_DOCS
}

pub fn find_builtin_doc(word: &str) -> Option<&'static BuiltinDoc> {
    BUILTIN_DOCS
        .iter()
        .find(|doc| doc.canonical == word || doc.aliases.iter().any(|alias| alias == &word))
}

fn opcode_from_builtin(doc: &BuiltinDoc) -> OpCode {
    match doc.canonical {
        "+" => OpCode::Add,
        "-" => OpCode::Sub,
        "*" => OpCode::Mul,
        "/" => OpCode::Div,
        "%" => OpCode::Mod,
        "dup" => OpCode::Dup,
        "del" => OpCode::Drop,
        "swp" => OpCode::Swap,
        "ovr" => OpCode::Over,
        "=" => OpCode::Eq,
        "<" => OpCode::Lt,
        ">" => OpCode::Gt,
        "&" => OpCode::And,
        "|" => OpCode::Or,
        "not" => OpCode::Not,
        "prt" => OpCode::Print,
        "!" => OpCode::Exec,
        "?" => OpCode::Select,
        "^" => OpCode::Loop,
        _ => unreachable!("builtin doc table and opcode mapping diverged"),
    }
}

fn builtin_word_list() -> String {
    let mut words = Vec::new();
    for doc in BUILTIN_DOCS {
        words.push(doc.canonical.to_string());
        words.extend(doc.aliases.iter().map(|alias| alias.to_string()));
    }
    words.join(" ")
}

#[derive(Debug, Clone, PartialEq, Eq)]
pub struct Instruction {
    pub op: OpCode,
    pub span: Span,
}

#[derive(Debug, Clone, PartialEq, Eq, Default)]
pub struct Bytecode {
    pub instructions: Vec<Instruction>,
}

impl Bytecode {
    pub fn disassemble(&self) -> String {
        self.instructions
            .iter()
            .enumerate()
            .map(|(idx, instruction)| match &instruction.op {
                OpCode::Push(value) => {
                    format!(
                        "{idx:04} push {:<24} ; span {}..{}",
                        value, instruction.span.start, instruction.span.end
                    )
                }
                op => format!(
                    "{idx:04} {:<28} ; span {}..{}",
                    op.mnemonic(),
                    instruction.span.start,
                    instruction.span.end
                ),
            })
            .collect::<Vec<_>>()
            .join("\n")
    }

    pub fn to_inline_source(&self) -> String {
        self.instructions
            .iter()
            .map(|instruction| match &instruction.op {
                OpCode::Push(value) => value.to_source(),
                op => op.mnemonic().to_string(),
            })
            .collect::<Vec<_>>()
            .join(" ")
    }
}

pub fn compile(program: &AstProgram) -> Result<Bytecode, EzcError> {
    compile_nodes(&program.nodes)
}

fn compile_nodes(nodes: &[Spanned<AstNode>]) -> Result<Bytecode, EzcError> {
    let mut instructions = Vec::with_capacity(nodes.len());

    for (node, span) in nodes {
        compile_node(node, span, &mut instructions)?;
    }

    Ok(Bytecode { instructions })
}

fn compile_node(
    node: &AstNode,
    span: &Span,
    instructions: &mut Vec<Instruction>,
) -> Result<(), EzcError> {
    let op = match node {
        AstNode::Number(value) => OpCode::Push(Value::Int(*value)),
        AstNode::Text(text) => OpCode::Push(Value::Text(text.clone())),
        AstNode::Word(word) => compile_word(word, span)?,
        AstNode::Block(nodes) => {
            let bytecode = compile_nodes(nodes)?;
            OpCode::Push(Value::Block(BlockValue { bytecode }))
        }
        AstNode::Stack(nodes) => {
            let values = nodes
                .iter()
                .map(|(node, span)| compile_literal(node, span))
                .collect::<Result<Vec<_>, _>>()?;
            OpCode::Push(Value::Stack(values))
        }
    };

    instructions.push(Instruction {
        op,
        span: span.clone(),
    });

    Ok(())
}

fn compile_literal(node: &AstNode, _span: &Span) -> Result<Value, EzcError> {
    match node {
        AstNode::Number(value) => Ok(Value::Int(*value)),
        AstNode::Text(text) => Ok(Value::Text(text.clone())),
        AstNode::Word(word) => Ok(Value::Symbol(word.clone())),
        AstNode::Block(nodes) => {
            let bytecode = compile_nodes(nodes)?;
            Ok(Value::Block(BlockValue { bytecode }))
        }
        AstNode::Stack(nodes) => {
            let values = nodes
                .iter()
                .map(|(node, span)| compile_literal(node, span))
                .collect::<Result<Vec<_>, _>>()?;
            Ok(Value::Stack(values))
        }
    }
}

fn compile_word(word: &str, span: &Span) -> Result<OpCode, EzcError> {
    let Some(doc) = find_builtin_doc(word) else {
        return Err(EzcError::new(
            ErrorCode::CompileUnknownWord,
            format!("unknown EZC word `{word}`"),
            span.clone(),
        )
        .with_note(format!(
            "Builtins: {}. Use [] for symbolic data.",
            builtin_word_list()
        ))
        .with_help(
            "Unknown words inside `[]` become symbols. Outside `[]` they must be builtins.",
        ));
    };

    Ok(opcode_from_builtin(doc))
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::ezclang::{parser, tokenizer};

    #[test]
    fn compiles_delayed_block_and_exec_operator() {
        let source = "(2 3 + prt) !";
        let tokens = tokenizer::tokenize(source).expect("tokenization should succeed");
        let ast = parser::parse(&tokens, source.len()).expect("parsing should succeed");
        let bytecode = compile(&ast).expect("compilation should succeed");

        assert_eq!(bytecode.instructions.len(), 2);
        assert!(matches!(
            bytecode.instructions[0].op,
            OpCode::Push(Value::Block(_))
        ));
        assert!(matches!(bytecode.instructions[1].op, OpCode::Exec));
    }

    #[test]
    fn compiles_stack_literals_with_symbols() {
        let tokens = tokenizer::tokenize("[a 1 (2 3 +)]").expect("tokenization should succeed");
        let ast = parser::parse(&tokens, 13).expect("parsing should succeed");
        let bytecode = compile(&ast).expect("compilation should succeed");

        let OpCode::Push(Value::Stack(values)) = &bytecode.instructions[0].op else {
            panic!("expected stack literal push");
        };

        assert_eq!(values[0], Value::Symbol("a".to_string()));
        assert_eq!(values[1], Value::Int(1));
        assert!(matches!(values[2], Value::Block(_)));
    }

    #[test]
    fn compiles_text_and_swp_word() {
        let source = "\"hello\" 1 2 swp";
        let tokens = tokenizer::tokenize(source).expect("tokenization should succeed");
        let ast = parser::parse(&tokens, source.len()).expect("parsing should succeed");
        let bytecode = compile(&ast).expect("compilation should succeed");

        assert!(matches!(
            bytecode.instructions[0].op,
            OpCode::Push(Value::Text(_))
        ));
        assert!(matches!(bytecode.instructions[3].op, OpCode::Swap));
    }

    #[test]
    fn compiles_symbolic_alias_words() {
        let source = "1 , 2 _ 3 4 ~ .";
        let tokens = tokenizer::tokenize(source).expect("tokenization should succeed");
        let ast = parser::parse(&tokens, source.len()).expect("parsing should succeed");
        let bytecode = compile(&ast).expect("compilation should succeed");

        assert!(matches!(bytecode.instructions[1].op, OpCode::Dup));
        assert!(matches!(bytecode.instructions[3].op, OpCode::Over));
        assert!(matches!(bytecode.instructions[6].op, OpCode::Swap));
        assert!(matches!(bytecode.instructions[7].op, OpCode::Drop));
    }

    #[test]
    fn rejects_unknown_words_at_executable_level() {
        let tokens = tokenizer::tokenize("1 mystery").expect("tokenization should succeed");
        let ast = parser::parse(&tokens, 9).expect("parsing should succeed");
        let err = compile(&ast).expect_err("compilation should fail");

        assert_eq!(err.code, ErrorCode::CompileUnknownWord);
        assert!(err.message.contains("unknown EZC word"));
    }

    #[test]
    fn rejects_legacy_swap_word() {
        let source = "1 2 swap";
        let tokens = tokenizer::tokenize(source).expect("tokenization should succeed");
        let ast = parser::parse(&tokens, source.len()).expect("parsing should succeed");
        let err = compile(&ast).expect_err("compilation should fail");

        assert_eq!(err.code, ErrorCode::CompileUnknownWord);
        assert!(err.message.contains("swap"));
    }

    #[test]
    fn rejects_legacy_drp_word() {
        let source = "1 drp";
        let tokens = tokenizer::tokenize(source).expect("tokenization should succeed");
        let ast = parser::parse(&tokens, source.len()).expect("parsing should succeed");
        let err = compile(&ast).expect_err("compilation should fail");

        assert_eq!(err.code, ErrorCode::CompileUnknownWord);
        assert!(err.message.contains("drp"));
    }

    #[test]
    fn builtin_docs_use_short_canonical_stack_words() {
        let swp = find_builtin_doc("swp").expect("swp doc should exist");
        let comma = find_builtin_doc(",").expect("comma alias should exist");
        let dot = find_builtin_doc(".").expect("dot alias should exist");
        let underscore = find_builtin_doc("_").expect("underscore alias should exist");
        let tilde = find_builtin_doc("~").expect("tilde alias should exist");

        assert_eq!(swp.canonical, "swp");
        assert_eq!(swp.stack_effect.before, "... a b");
        assert_eq!(swp.stack_effect.after, "... b a");
        assert_eq!(comma.canonical, "dup");
        assert_eq!(dot.canonical, "del");
        assert_eq!(underscore.canonical, "ovr");
        assert_eq!(tilde.canonical, "swp");
        assert!(find_builtin_doc("swap").is_none());
        assert!(find_builtin_doc("drp").is_none());
    }
}
