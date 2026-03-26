use chumsky::input::ValueInput;
use chumsky::prelude::*;

use crate::ast::{Expr, Spanned};
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
/// Since ezc is RPN, the parser is almost trivially flat -- the only recursion
/// comes from `(...)` blocks and `[...]` lists. No precedence, no associativity.
fn parser<'src, I>() -> impl Parser<'src, I, Vec<Spanned<Expr>>, extra::Err<Rich<'src, Token>>>
where
    I: ValueInput<'src, Token = Token, Span = SimpleSpan>,
{
    recursive(|expr| {
        // Atomic tokens -> Expr variants.
        let atom = select! {
            Token::Int(s) => Expr::Literal(s),
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
            Token::Eq => Expr::Equal,
            Token::NotEq => Expr::NotEqual,
            Token::Lt => Expr::Lt,
            Token::Gt => Expr::Gt,
            Token::LtEq => Expr::LtEq,
            Token::GtEq => Expr::GtEq,
            Token::Tilde => Expr::Swap,
            Token::Colon => Expr::Dup,
            Token::Underscore => Expr::Over,
            Token::Dollar => Expr::Dollar,
            Token::At => Expr::At,
        };

        // Block: (...) -- deferred code.
        let block = expr
            .clone()
            .repeated()
            .collect::<Vec<_>>()
            .delimited_by(just(Token::OpenParen), just(Token::CloseParen))
            .map(Expr::Block);

        // List: [...] -- eagerly evaluated.
        let list = expr
            .clone()
            .repeated()
            .collect::<Vec<_>>()
            .delimited_by(just(Token::OpenBracket), just(Token::CloseBracket))
            .map(Expr::List);

        choice((block, list, atom)).map_with(|e, ex| {
            let s: SimpleSpan = ex.span();
            (e, s.into_range())
        })
    })
    .repeated()
    .collect()
}

#[cfg(test)]
mod tests {
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
                Expr::Literal("3".into()),
                Expr::Literal("4".into()),
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
                assert_eq!(inner, vec![Expr::Literal("2".into()), Expr::Op(Op::Mul)]);
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
                        Expr::Literal("1".into()),
                        Expr::Literal("2".into()),
                        Expr::Literal("3".into()),
                    ]
                );
            }
            other => panic!("expected List, got {other:?}"),
        }
    }

    #[test]
    fn nested_blocks() {
        let result = exprs("((1 +) !)");
        assert_eq!(result.len(), 1);
        match &result[0] {
            Expr::Block(body) => {
                assert_eq!(body.len(), 2); // inner block + !
            }
            other => panic!("expected Block, got {other:?}"),
        }
    }

    #[test]
    fn all_operators() {
        assert_eq!(
            exprs("! ? ?? | & &! &? == != < > <= >= ~ : _ $ @"),
            vec![
                Expr::Execute,
                Expr::Cond,
                Expr::Ternary,
                Expr::Compose,
                Expr::Loop,
                Expr::Map,
                Expr::Filter,
                Expr::Equal,
                Expr::NotEqual,
                Expr::Lt,
                Expr::Gt,
                Expr::LtEq,
                Expr::GtEq,
                Expr::Swap,
                Expr::Dup,
                Expr::Over,
                Expr::Dollar,
                Expr::At,
            ]
        );
    }

    #[test]
    fn empty_input() {
        assert_eq!(exprs(""), vec![]);
    }

    #[test]
    fn mixed_program() {
        // `3 4 + (2 *) !` -- push 3, push 4, add, push block, execute block.
        let result = exprs("3 4 + (2 *) !");
        assert_eq!(result.len(), 5); // 3, 4, +, (...), !
    }
}
