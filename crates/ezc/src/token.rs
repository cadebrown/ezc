use std::fmt;

/// Arithmetic operator.
#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash)]
pub enum Op {
    Add,
    Sub,
    Mul,
    Div,
    Mod,
    Pow,
}

impl fmt::Display for Op {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            Op::Add => write!(f, "+"),
            Op::Sub => write!(f, "-"),
            Op::Mul => write!(f, "*"),
            Op::Div => write!(f, "/"),
            Op::Mod => write!(f, "%"),
            Op::Pow => write!(f, "^"),
        }
    }
}

/// Lexer output token.
///
/// Each token corresponds to a single syntactic element in the source.
/// Tokens carry no span information themselves — spans are attached externally
/// via `Spanned<Token>`.
#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub enum Token {
    /// Integer literal (raw string, parsed to BigInt in the parser).
    Int(String),
    /// Typed integer literal: value + suffix (e.g. "42" + "u8").
    TypedInt(String, String),
    /// Float literal (raw string, defaults to f64).
    Float(String),
    /// Typed float literal: value + suffix (e.g. "3.14" + "f32").
    TypedFloat(String, String),
    /// String literal (escape-processed).
    Str(String),
    /// Bare identifier (type constructors, future keywords).
    Ident(String),
    /// Arithmetic operator.
    Op(Op),
    /// `!` — execute.
    Bang,
    /// `??` — ternary conditional.
    DoubleQuestion,
    /// `?` — conditional.
    Question,
    /// `|` — compose / concatenate.
    Pipe,
    /// `&!` — map.
    AmpBang,
    /// `&?` — filter.
    AmpQuestion,
    /// `&` — loop.
    Amp,
    /// `==` — equality.
    Eq,
    /// `!=` — not-equal.
    NotEq,
    /// `<` — less-than.
    Lt,
    /// `>` — greater-than.
    Gt,
    /// `<=` — less-than-or-equal.
    LtEq,
    /// `>=` — greater-than-or-equal.
    GtEq,
    /// `~` — swap.
    Tilde,
    /// `:` — dup.
    Colon,
    /// `_` — over.
    Underscore,
    /// `@name` — bind: pop and store in variable.
    Bind(String),
    /// `$name` — recall: push variable value.
    Recall(String),
    /// `(` — open block.
    OpenParen,
    /// `)` — close block.
    CloseParen,
    /// `[` — open list.
    OpenBracket,
    /// `]` — close list.
    CloseBracket,
    /// `{` — open scope.
    OpenBrace,
    /// `}` — close scope.
    CloseBrace,
}

impl fmt::Display for Token {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            Token::Int(s) => write!(f, "{s}"),
            Token::TypedInt(s, suffix) => write!(f, "{s}{suffix}"),
            Token::Float(s) => write!(f, "{s}"),
            Token::TypedFloat(s, suffix) => write!(f, "{s}{suffix}"),
            Token::Str(s) => write!(f, "\"{s}\""),
            Token::Ident(s) => write!(f, "{s}"),
            Token::Op(op) => write!(f, "{op}"),
            Token::Bang => write!(f, "!"),
            Token::DoubleQuestion => write!(f, "??"),
            Token::Question => write!(f, "?"),
            Token::Pipe => write!(f, "|"),
            Token::AmpBang => write!(f, "&!"),
            Token::AmpQuestion => write!(f, "&?"),
            Token::Amp => write!(f, "&"),
            Token::Eq => write!(f, "=="),
            Token::NotEq => write!(f, "!="),
            Token::Lt => write!(f, "<"),
            Token::Gt => write!(f, ">"),
            Token::LtEq => write!(f, "<="),
            Token::GtEq => write!(f, ">="),
            Token::Tilde => write!(f, "~"),
            Token::Colon => write!(f, ":"),
            Token::Underscore => write!(f, "_"),
            Token::Bind(name) => write!(f, "@{name}"),
            Token::Recall(name) => write!(f, "${name}"),
            Token::OpenParen => write!(f, "("),
            Token::CloseParen => write!(f, ")"),
            Token::OpenBracket => write!(f, "["),
            Token::CloseBracket => write!(f, "]"),
            Token::OpenBrace => write!(f, "{{"),
            Token::CloseBrace => write!(f, "}}"),
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn op_display_roundtrip() {
        for (op, s) in [
            (Op::Add, "+"),
            (Op::Sub, "-"),
            (Op::Mul, "*"),
            (Op::Div, "/"),
            (Op::Mod, "%"),
            (Op::Pow, "^"),
        ] {
            assert_eq!(op.to_string(), s);
        }
    }

    #[test]
    fn token_display() {
        assert_eq!(Token::DoubleQuestion.to_string(), "??");
        assert_eq!(Token::AmpBang.to_string(), "&!");
        assert_eq!(Token::Eq.to_string(), "==");
        assert_eq!(Token::Int("42".into()).to_string(), "42");
        assert_eq!(Token::Bind("x".into()).to_string(), "@x");
        assert_eq!(Token::Recall("x".into()).to_string(), "$x");
        assert_eq!(Token::OpenBrace.to_string(), "{");
        assert_eq!(Token::CloseBrace.to_string(), "}");
    }
}
