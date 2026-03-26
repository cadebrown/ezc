use crate::number::Number;
use crate::token::Op;

/// A span-annotated value.
pub type Spanned<T> = (T, Span);

/// Source location span (byte offset range).
pub type Span = std::ops::Range<usize>;

/// AST node — the parser output and evaluator input.
///
/// ezc programs are flat sequences of expressions. The only nesting comes from
/// `Block` (parenthesized code), `List` (bracketed code), and `Scope` (braced
/// code). Since the language is RPN, there is no operator precedence or
/// associativity to encode in the AST.
#[derive(Debug, Clone, PartialEq)]
pub enum Expr {
    /// Push a numeric literal onto the stack (BigInt, typed int, or float).
    Literal(Number),
    /// Push a string literal onto the stack.
    StrLiteral(String),
    /// Bare identifier — type constructor or built-in function.
    Ident(String),
    /// Binary arithmetic operator: `+ - * / % ^`
    Op(Op),
    /// `!` — pop a block, execute it.
    Execute,
    /// `?` — pop condition; if falsy, also pop the next value.
    Cond,
    /// `??` — pop condition; keep one of two preceding values.
    Ternary,
    /// `|` — compose / concatenate containers.
    Compose,
    /// `&` — loop (base).
    Loop,
    /// `&!` — map.
    Map,
    /// `&?` — filter.
    Filter,
    /// `&/` — fold / reduce.
    Fold,
    /// `==` — equality comparison, pushes Bool.
    Equal,
    /// `!=` — not-equal comparison, pushes Bool.
    NotEqual,
    /// `<` — less-than comparison, pushes Bool.
    Lt,
    /// `>` — greater-than comparison, pushes Bool.
    Gt,
    /// `<=` — less-than-or-equal comparison, pushes Bool.
    LtEq,
    /// `>=` — greater-than-or-equal comparison, pushes Bool.
    GtEq,
    /// `~` — swap top two stack elements.
    Swap,
    /// `,` — dup: duplicate top of stack.
    Dup,
    /// `;` — drop: discard top of stack.
    Drop,
    /// `:` — write: print top of stack with newline, consume it.
    Write,
    /// `.` — read: read a line from stdin, push as string.
    Read,
    /// `_` — over: copy second element to top.
    Over,
    /// `@name` — bind: pop top of stack and store in variable.
    Bind(String),
    /// `$name` — recall: push variable value onto stack.
    Recall(String),
    /// `(...)` — a block of deferred code.
    Block(Vec<Spanned<Expr>>),
    /// `[...]` — an eagerly-evaluated list.
    List(Vec<Spanned<Expr>>),
    /// `{...}` — a scoped block: evaluates immediately with local bindings.
    Scope(Vec<Spanned<Expr>>),
}
