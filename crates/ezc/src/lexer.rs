use chumsky::prelude::*;

use crate::token::{Op, Token};

/// Lex source code into a sequence of spanned tokens.
///
/// Returns `Ok(tokens)` on success, or `Err(errors)` with parse errors.
/// Comments (`# ...`) and whitespace are stripped.
pub fn lex(src: &str) -> Result<Vec<(Token, SimpleSpan)>, Vec<Rich<'_, char>>> {
    lexer().parse(src).into_result()
}

/// Build the chumsky lexer parser.
///
/// The lexer operates at the character level, producing `Token` values with
/// source spans. Longest-match ordering is critical for multi-character tokens
/// like `??` vs `?` and `&!` vs `&`.
fn lexer<'src>(
) -> impl Parser<'src, &'src str, Vec<(Token, SimpleSpan)>, extra::Err<Rich<'src, char>>> {
    let int = text::int(10).map(|s: &str| Token::Int(s.to_string()));

    // Multi-character operators first (longest match).
    let double_question = just("??").to(Token::DoubleQuestion);
    let amp_bang = just("&!").to(Token::AmpBang);
    let amp_question = just("&?").to(Token::AmpQuestion);
    let eq = just("==").to(Token::Eq);
    let not_eq = just("!=").to(Token::NotEq);
    let lt_eq = just("<=").to(Token::LtEq);
    let gt_eq = just(">=").to(Token::GtEq);

    // Single-character operators.
    let bang = just('!').to(Token::Bang);
    let lt = just('<').to(Token::Lt);
    let gt = just('>').to(Token::Gt);
    let question = just('?').to(Token::Question);
    let pipe = just('|').to(Token::Pipe);
    let amp = just('&').to(Token::Amp);
    let tilde = just('~').to(Token::Tilde);
    let colon = just(':').to(Token::Colon);
    let underscore = just('_').to(Token::Underscore);
    let dollar = just('$').to(Token::Dollar);
    let at = just('@').to(Token::At);

    // Arithmetic operators.
    let add = just('+').to(Token::Op(Op::Add));
    let sub = just('-').to(Token::Op(Op::Sub));
    let mul = just('*').to(Token::Op(Op::Mul));
    let div = just('/').to(Token::Op(Op::Div));
    let modulo = just('%').to(Token::Op(Op::Mod));
    let pow = just('^').to(Token::Op(Op::Pow));

    // Delimiters.
    let open_paren = just('(').to(Token::OpenParen);
    let close_paren = just(')').to(Token::CloseParen);
    let open_bracket = just('[').to(Token::OpenBracket);
    let close_bracket = just(']').to(Token::CloseBracket);

    // Comments: `#` to end of line, discarded.
    let comment = just('#')
        .then(any().and_is(just('\n').not()).repeated())
        .padded();

    // Split into two groups to stay within chumsky's tuple-choice arity limit.
    let multi_char = choice((
        double_question,
        amp_bang,
        amp_question,
        eq,
        not_eq,
        lt_eq,
        gt_eq,
    ));

    let single_char = choice((
        bang,
        lt,
        gt,
        question,
        pipe,
        amp,
        tilde,
        colon,
        underscore,
        dollar,
        at,
        add,
        sub,
        mul,
        div,
        modulo,
        pow,
        open_paren,
        close_paren,
        open_bracket,
        close_bracket,
    ));

    let token = choice((
        // Multi-char tokens first for longest match.
        multi_char,
        // Integers before single-char to avoid ambiguity.
        int,
        // Everything else.
        single_char,
    ))
    .map_with(|tok, e| (tok, e.span()));

    token
        .padded_by(comment.repeated())
        .padded()
        .repeated()
        .collect()
        .padded_by(comment.repeated())
        .padded()
        .then_ignore(end())
}

#[cfg(test)]
mod tests {
    use super::*;

    fn tokens(src: &str) -> Vec<Token> {
        lex(src)
            .expect("lex failed")
            .into_iter()
            .map(|(t, _)| t)
            .collect()
    }

    #[test]
    fn integer_literals() {
        assert_eq!(tokens("42"), vec![Token::Int("42".into())]);
        assert_eq!(tokens("0"), vec![Token::Int("0".into())]);
        assert_eq!(
            tokens("123 456"),
            vec![Token::Int("123".into()), Token::Int("456".into())]
        );
    }

    #[test]
    fn arithmetic_ops() {
        assert_eq!(
            tokens("+ - * / % ^"),
            vec![
                Token::Op(Op::Add),
                Token::Op(Op::Sub),
                Token::Op(Op::Mul),
                Token::Op(Op::Div),
                Token::Op(Op::Mod),
                Token::Op(Op::Pow),
            ]
        );
    }

    #[test]
    fn multi_char_operators() {
        assert_eq!(tokens("??"), vec![Token::DoubleQuestion]);
        assert_eq!(tokens("&!"), vec![Token::AmpBang]);
        assert_eq!(tokens("&?"), vec![Token::AmpQuestion]);
        assert_eq!(tokens("=="), vec![Token::Eq]);
        assert_eq!(tokens("!="), vec![Token::NotEq]);
        assert_eq!(tokens("<="), vec![Token::LtEq]);
        assert_eq!(tokens(">="), vec![Token::GtEq]);
    }

    #[test]
    fn single_char_operators() {
        assert_eq!(tokens("!"), vec![Token::Bang]);
        assert_eq!(tokens("?"), vec![Token::Question]);
        assert_eq!(tokens("|"), vec![Token::Pipe]);
        assert_eq!(tokens("&"), vec![Token::Amp]);
        assert_eq!(tokens("~"), vec![Token::Tilde]);
        assert_eq!(tokens("_"), vec![Token::Underscore]);
        assert_eq!(tokens("$"), vec![Token::Dollar]);
        assert_eq!(tokens("@"), vec![Token::At]);
        assert_eq!(tokens(":"), vec![Token::Colon]);
        assert_eq!(tokens("<"), vec![Token::Lt]);
        assert_eq!(tokens(">"), vec![Token::Gt]);
    }

    #[test]
    fn longest_match_comparison() {
        // `!=` should not be parsed as `!` then `=`... but `=` is not a token.
        // `<=` should not be `<` then `=`.
        assert_eq!(tokens("!="), vec![Token::NotEq]);
        assert_eq!(tokens("<="), vec![Token::LtEq]);
        assert_eq!(tokens(">="), vec![Token::GtEq]);
        // Separated: `< =` won't parse (= is not a valid token), but `< >` should be two tokens.
        assert_eq!(tokens("< >"), vec![Token::Lt, Token::Gt]);
    }

    #[test]
    fn delimiters() {
        assert_eq!(
            tokens("( ) [ ]"),
            vec![
                Token::OpenParen,
                Token::CloseParen,
                Token::OpenBracket,
                Token::CloseBracket,
            ]
        );
    }

    #[test]
    fn comments_stripped() {
        assert_eq!(
            tokens("3 4 + # this is a comment"),
            vec![
                Token::Int("3".into()),
                Token::Int("4".into()),
                Token::Op(Op::Add),
            ]
        );
    }

    #[test]
    fn comments_on_own_line() {
        assert_eq!(
            tokens("# full line comment\n3 4 +"),
            vec![
                Token::Int("3".into()),
                Token::Int("4".into()),
                Token::Op(Op::Add),
            ]
        );
    }

    #[test]
    fn full_program() {
        assert_eq!(
            tokens("3 4 + (2 *) !"),
            vec![
                Token::Int("3".into()),
                Token::Int("4".into()),
                Token::Op(Op::Add),
                Token::OpenParen,
                Token::Int("2".into()),
                Token::Op(Op::Mul),
                Token::CloseParen,
                Token::Bang,
            ]
        );
    }

    #[test]
    fn empty_input() {
        assert_eq!(tokens(""), vec![]);
        assert_eq!(tokens("   "), vec![]);
        assert_eq!(tokens("# just a comment"), vec![]);
    }

    #[test]
    fn longest_match_question() {
        // `??` should not be parsed as two `?` tokens.
        assert_eq!(tokens("??"), vec![Token::DoubleQuestion]);
        // But `? ?` should be two separate `?`.
        assert_eq!(tokens("? ?"), vec![Token::Question, Token::Question]);
    }

    #[test]
    fn longest_match_amp() {
        assert_eq!(tokens("&!"), vec![Token::AmpBang]);
        assert_eq!(tokens("&?"), vec![Token::AmpQuestion]);
        assert_eq!(tokens("& !"), vec![Token::Amp, Token::Bang]);
    }
}
