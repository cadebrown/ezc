use crate::token::Op;

/// A span-annotated value.
pub type Spanned<T> = (T, Span);

/// Source location span (byte offset range).
pub type Span = std::ops::Range<usize>;

/// AST node — the parser output and evaluator input.
///
/// ezc programs are flat sequences of expressions. The only nesting comes from
/// `Block` (parenthesized code) and `List` (bracketed code). Since the language
/// is RPN, there is no operator precedence or associativity to encode in the AST.
#[derive(Debug, Clone, PartialEq, Eq)]
pub enum Expr {
    /// Push an integer literal onto the stack.
    Literal(String),
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
    /// `:` — dup: duplicate top of stack.
    Dup,
    /// `_` — over: copy second element to top.
    Over,
    /// `$` — reserved.
    Dollar,
    /// `@` — reserved.
    At,
    /// `(...)` — a block of deferred code.
    Block(Vec<Spanned<Expr>>),
    /// `[...]` — an eagerly-evaluated list.
    List(Vec<Spanned<Expr>>),
}
