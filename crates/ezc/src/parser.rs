use chumsky::input::ValueInput;
use chumsky::prelude::*;

use std::sync::Arc;

use num_bigint::BigInt;

use crate::ast::{Expr, Spanned};
use crate::number::Number;
use crate::token::{Op, Token};

/// Parse a token stream into a sequence of spanned AST expressions.
pub fn parse(
    tokens: &[(Token, SimpleSpan)],
    src_len: usize,
) -> Result<Vec<Spanned<Expr>>, Vec<Rich<'_, Token>>> {
    let eoi: SimpleSpan = (src_len..src_len).into();
    parser()
        .parse(tokens.map(eoi, |(t, s)| (t, s)))
        .into_result()
}

/// Build the chumsky token-level parser.
///
/// Since ezc is RPN, the parser is almost trivially flat — the only recursion
/// comes from `(...)` blocks, `[...]` lists, and `{...}` scopes.
fn parser<'src, I>() -> impl Parser<'src, I, Vec<Spanned<Expr>>, extra::Err<Rich<'src, Token>>>
where
    I: ValueInput<'src, Token = Token, Span = SimpleSpan>,
{
    recursive(|expr| {
        // Atomic tokens -> Expr variants.
        let atom = select! {
            Token::Int(s) => Expr::Literal(parse_int(&s)),
            Token::TypedInt(s, suffix) => Expr::Literal(parse_typed_int(&s, &suffix)),
            Token::Float(s) => Expr::Literal(Number::F64(s.parse::<f64>().unwrap())),
            Token::TypedFloat(s, suffix) => Expr::Literal(parse_typed_float(&s, &suffix)),
            Token::Str(s) => Expr::StrLiteral(s),
            Token::Ident(s) => Expr::Ident(s),
            Token::Op(Op::Add) => Expr::Op(Op::Add),
            Token::Op(Op::Sub) => Expr::Op(Op::Sub),
            Token::Op(Op::Mul) => Expr::Op(Op::Mul),
            Token::Op(Op::Div) => Expr::Op(Op::Div),
            Token::Op(Op::Mod) => Expr::Op(Op::Mod),
            Token::Op(Op::Pow) => Expr::Op(Op::Pow),
            Token::Bang => Expr::Execute,
            Token::Question => Expr::Cond,
            Token::DoubleQuestion => Expr::Ternary,
            Token::Pipe => Expr::Compose,
            Token::Amp => Expr::Loop,
            Token::AmpBang => Expr::Map,
            Token::AmpQuestion => Expr::Filter,
            Token::AmpSlash => Expr::Fold,
            Token::Eq => Expr::Equal,
            Token::NotEq => Expr::NotEqual,
            Token::Lt => Expr::Lt,
            Token::Gt => Expr::Gt,
            Token::LtEq => Expr::LtEq,
            Token::GtEq => Expr::GtEq,
            Token::Tilde => Expr::Swap,
            Token::Comma => Expr::Dup,
            Token::Semicolon => Expr::Drop,
            Token::Colon => Expr::Write,
            Token::Dot => Expr::Read,
            Token::Underscore => Expr::Over,
            Token::Bind(name) => Expr::Bind(name),
            Token::Recall(name) => Expr::Recall(name),
        };

        // Block: (...) — deferred code.
        let block = expr
            .clone()
            .repeated()
            .collect::<Vec<_>>()
            .delimited_by(just(Token::OpenParen), just(Token::CloseParen))
            .map(Expr::Block);

        // List: [...] — eagerly evaluated.
        let list = expr
            .clone()
            .repeated()
            .collect::<Vec<_>>()
            .delimited_by(just(Token::OpenBracket), just(Token::CloseBracket))
            .map(Expr::List);

        // Scope: {...} — immediately evaluated with local bindings.
        let scope = expr
            .clone()
            .repeated()
            .collect::<Vec<_>>()
            .delimited_by(just(Token::OpenBrace), just(Token::CloseBrace))
            .map(Expr::Scope);

        choice((block, list, scope, atom)).map_with(|e, ex| {
            let s: SimpleSpan = ex.span();
            (e, s.into_range())
        })
    })
    .repeated()
    .collect()
}

/// Parse an integer literal (decimal or hex) to `Number::Int`.
fn parse_int(s: &str) -> Number {
    let n = if let Some(hex) = s.strip_prefix("0x") {
        BigInt::parse_bytes(hex.as_bytes(), 16).unwrap()
    } else {
        s.parse::<BigInt>().unwrap()
    };
    Number::Int(Arc::new(n))
}

/// Parse a typed integer literal (e.g. "42" + "u8") to the appropriate Number variant.
fn parse_typed_int(s: &str, suffix: &str) -> Number {
    use ethnum::{I256, U256};

    let n = if let Some(hex) = s.strip_prefix("0x") {
        BigInt::parse_bytes(hex.as_bytes(), 16).unwrap()
    } else {
        s.parse::<BigInt>().unwrap()
    };

    // Convert BigInt to the target width. Panics on out-of-range literals
    // (this is a parse-time check, not a runtime check).
    match suffix {
        "u8" => Number::U8(n.try_into().expect("literal out of range for u8")),
        "u16" => Number::U16(n.try_into().expect("literal out of range for u16")),
        "u32" => Number::U32(n.try_into().expect("literal out of range for u32")),
        "u64" => Number::U64(n.try_into().expect("literal out of range for u64")),
        "u128" => Number::U128(n.try_into().expect("literal out of range for u128")),
        "u256" => {
            let (_, bytes) = n.to_bytes_be();
            Number::U256(U256::from_be_bytes({
                let mut buf = [0u8; 32];
                let start = 32usize.saturating_sub(bytes.len());
                buf[start..].copy_from_slice(&bytes[..bytes.len().min(32)]);
                buf
            }))
        }
        "i8" => Number::I8(n.try_into().expect("literal out of range for i8")),
        "i16" => Number::I16(n.try_into().expect("literal out of range for i16")),
        "i32" => Number::I32(n.try_into().expect("literal out of range for i32")),
        "i64" => Number::I64(n.try_into().expect("literal out of range for i64")),
        "i128" => Number::I128(n.try_into().expect("literal out of range for i128")),
        "i256" => {
            let (_, bytes) = n.to_bytes_be();
            Number::I256(I256::from_be_bytes({
                let mut buf = [0u8; 32];
                let start = 32usize.saturating_sub(bytes.len());
                buf[start..].copy_from_slice(&bytes[..bytes.len().min(32)]);
                buf
            }))
        }
        _ => unreachable!("unknown int suffix: {suffix}"),
    }
}

/// Parse a typed float literal.
fn parse_typed_float(s: &str, suffix: &str) -> Number {
    let f: f64 = s.parse().unwrap();
    match suffix {
        "f16" => Number::F16(half::f16::from_f64(f)),
        "f32" => Number::F32(f as f32),
        "f64" => Number::F64(f),
        _ => unreachable!("unknown float suffix: {suffix}"),
    }
}

#[cfg(test)]
mod tests {
    use std::sync::Arc;

    use super::*;
    use crate::lexer;

    /// Helper: lex and parse, returning just the Expr values (no spans).
    fn exprs(src: &str) -> Vec<Expr> {
        let tokens = lexer::lex(src).expect("lex failed");
        parse(&tokens, src.len())
            .expect("parse failed")
            .into_iter()
            .map(|(e, _)| e)
            .collect()
    }

    #[test]
    fn simple_arithmetic() {
        assert_eq!(
            exprs("3 4 +"),
            vec![
                Expr::Literal(Number::Int(Arc::new(3.into()))),
                Expr::Literal(Number::Int(Arc::new(4.into()))),
                Expr::Op(Op::Add),
            ]
        );
    }

    #[test]
    fn block() {
        let result = exprs("(2 *)");
        assert_eq!(result.len(), 1);
        match &result[0] {
            Expr::Block(body) => {
                let inner: Vec<_> = body.iter().map(|(e, _)| e.clone()).collect();
                assert_eq!(
                    inner,
                    vec![
                        Expr::Literal(Number::Int(Arc::new(2.into()))),
                        Expr::Op(Op::Mul)
                    ]
                );
            }
            other => panic!("expected Block, got {other:?}"),
        }
    }

    #[test]
    fn list() {
        let result = exprs("[1 2 3]");
        assert_eq!(result.len(), 1);
        match &result[0] {
            Expr::List(body) => {
                let inner: Vec<_> = body.iter().map(|(e, _)| e.clone()).collect();
                assert_eq!(
                    inner,
                    vec![
                        Expr::Literal(Number::Int(Arc::new(1.into()))),
                        Expr::Literal(Number::Int(Arc::new(2.into()))),
                        Expr::Literal(Number::Int(Arc::new(3.into()))),
                    ]
                );
            }
            other => panic!("expected List, got {other:?}"),
        }
    }

    #[test]
    fn scope() {
        let result = exprs("{ 5 @x $x }");
        assert_eq!(result.len(), 1);
        match &result[0] {
            Expr::Scope(body) => {
                let inner: Vec<_> = body.iter().map(|(e, _)| e.clone()).collect();
                assert_eq!(
                    inner,
                    vec![
                        Expr::Literal(Number::Int(Arc::new(5.into()))),
                        Expr::Bind("x".into()),
                        Expr::Recall("x".into()),
                    ]
                );
            }
            other => panic!("expected Scope, got {other:?}"),
        }
    }

    #[test]
    fn nested_blocks() {
        let result = exprs("((1 +) !)");
        assert_eq!(result.len(), 1);
        match &result[0] {
            Expr::Block(body) => {
                assert_eq!(body.len(), 2);
            }
            other => panic!("expected Block, got {other:?}"),
        }
    }

    #[test]
    fn bind_recall() {
        assert_eq!(
            exprs("5 @x $x"),
            vec![
                Expr::Literal(Number::Int(Arc::new(5.into()))),
                Expr::Bind("x".into()),
                Expr::Recall("x".into()),
            ]
        );
    }

    #[test]
    fn all_operators() {
        assert_eq!(
            exprs("! ? ?? | & &! &? &/ == != < > <= >= ~ , ; : . _"),
            vec![
                Expr::Execute,
                Expr::Cond,
                Expr::Ternary,
                Expr::Compose,
                Expr::Loop,
                Expr::Map,
                Expr::Filter,
                Expr::Fold,
                Expr::Equal,
                Expr::NotEqual,
                Expr::Lt,
                Expr::Gt,
                Expr::LtEq,
                Expr::GtEq,
                Expr::Swap,
                Expr::Dup,
                Expr::Drop,
                Expr::Write,
                Expr::Read,
                Expr::Over,
            ]
        );
    }

    #[test]
    fn empty_input() {
        assert_eq!(exprs(""), vec![]);
    }

    #[test]
    fn mixed_program() {
        let result = exprs("3 4 + (2 *) !");
        assert_eq!(result.len(), 5);
    }
}
