use chumsky::prelude::*;

use crate::token::{Op, Token};

/// Lex source code into a sequence of spanned tokens.
pub fn lex(src: &str) -> Result<Vec<(Token, SimpleSpan)>, Vec<Rich<'_, char>>> {
    lexer().parse(src).into_result()
}

/// Type suffixes recognized on numeric literals.
const INT_SUFFIXES: &[&str] = &[
    "u8", "u16", "u32", "u64", "u128", "u256", "i8", "i16", "i32", "i64", "i128", "i256",
];
const FLOAT_SUFFIXES: &[&str] = &["f16", "f32", "f64"];

fn lexer<'src>(
) -> impl Parser<'src, &'src str, Vec<(Token, SimpleSpan)>, extra::Err<Rich<'src, char>>> {
    // ── Identifier helper ─────────────────────────────────────────────────
    let ident = any()
        .filter(|c: &char| c.is_ascii_alphabetic() || *c == '_')
        .then(
            any()
                .filter(|c: &char| c.is_ascii_alphanumeric() || *c == '_')
                .repeated()
                .to_slice(),
        )
        .to_slice();

    // ── String literal ────────────────────────────────────────────────────
    let escape = just('\\').ignore_then(choice((
        just('n').to('\n'),
        just('t').to('\t'),
        just('r').to('\r'),
        just('\\').to('\\'),
        just('"').to('"'),
        just('0').to('\0'),
    )));
    let string_char = escape.or(any().filter(|c: &char| *c != '"' && *c != '\\'));
    let string = string_char
        .repeated()
        .collect::<String>()
        .delimited_by(just('"'), just('"'))
        .map(Token::Str);

    // ── Numeric literals ──────────────────────────────────────────────────

    // Hex integer: 0x[0-9a-fA-F]+ with optional type suffix.
    let hex_int = just("0x")
        .ignore_then(
            any()
                .filter(|c: &char| c.is_ascii_hexdigit())
                .repeated()
                .at_least(1)
                .to_slice(),
        )
        .then(ident.or_not())
        .map(|(digits, suffix): (&str, Option<&str>)| {
            let hex_str = format!("0x{digits}");
            match suffix {
                Some(s) if INT_SUFFIXES.contains(&s) => Token::TypedInt(hex_str, s.to_string()),
                _ => Token::Int(hex_str),
            }
        });

    // Float: digits.digits with optional exponent and suffix.
    let float = text::int(10)
        .then(just('.').then(text::digits(10)))
        .then(
            just('e')
                .or(just('E'))
                .then(just('+').or(just('-')).or_not())
                .then(text::digits(10))
                .or_not(),
        )
        .to_slice()
        .then(ident.or_not())
        .map(|(num, suffix): (&str, Option<&str>)| match suffix {
            Some(s) if FLOAT_SUFFIXES.contains(&s) => {
                Token::TypedFloat(num.to_string(), s.to_string())
            }
            _ => Token::Float(num.to_string()),
        });

    // Decimal integer with optional type suffix: 42, 42u8, 42i32.
    let dec_int = text::int(10).then(ident.or_not()).map(
        |(digits, suffix): (&str, Option<&str>)| match suffix {
            Some(s) if INT_SUFFIXES.contains(&s) => {
                Token::TypedInt(digits.to_string(), s.to_string())
            }
            Some(s) if FLOAT_SUFFIXES.contains(&s) => {
                Token::TypedFloat(digits.to_string(), s.to_string())
            }
            _ => Token::Int(digits.to_string()),
        },
    );

    // ── Variable binding/recall ───────────────────────────────────────────
    let bind = just('@')
        .ignore_then(ident)
        .map(|s: &str| Token::Bind(s.to_string()));
    let recall = just('$')
        .ignore_then(ident)
        .map(|s: &str| Token::Recall(s.to_string()));

    // ── Multi-character operators ─────────────────────────────────────────
    let double_question = just("??").to(Token::DoubleQuestion);
    let amp_bang = just("&!").to(Token::AmpBang);
    let amp_question = just("&?").to(Token::AmpQuestion);
    let amp_slash = just("&/").to(Token::AmpSlash);
    let eq = just("==").to(Token::Eq);
    let not_eq = just("!=").to(Token::NotEq);
    let lt_eq = just("<=").to(Token::LtEq);
    let gt_eq = just(">=").to(Token::GtEq);

    // ── Single-character operators ────────────────────────────────────────
    let bang = just('!').to(Token::Bang);
    let lt = just('<').to(Token::Lt);
    let gt = just('>').to(Token::Gt);
    let question = just('?').to(Token::Question);
    let pipe = just('|').to(Token::Pipe);
    let amp = just('&').to(Token::Amp);
    let tilde = just('~').to(Token::Tilde);
    let comma = just(',').to(Token::Comma);
    let semicolon = just(';').to(Token::Semicolon);
    let colon = just(':').to(Token::Colon);
    let dot = just('.').to(Token::Dot);
    let underscore_op = just('_').to(Token::Underscore);

    // ── Arithmetic operators ──────────────────────────────────────────────
    let add = just('+').to(Token::Op(Op::Add));
    let sub = just('-').to(Token::Op(Op::Sub));
    let mul = just('*').to(Token::Op(Op::Mul));
    let div = just('/').to(Token::Op(Op::Div));
    let modulo = just('%').to(Token::Op(Op::Mod));
    let pow = just('^').to(Token::Op(Op::Pow));

    // ── Delimiters ────────────────────────────────────────────────────────
    let open_paren = just('(').to(Token::OpenParen);
    let close_paren = just(')').to(Token::CloseParen);
    let open_bracket = just('[').to(Token::OpenBracket);
    let close_bracket = just(']').to(Token::CloseBracket);
    let open_brace = just('{').to(Token::OpenBrace);
    let close_brace = just('}').to(Token::CloseBrace);

    // ── Bare identifiers (keywords + type constructors) ───────────────────
    // Must start with a letter (not underscore alone — that's the `_` operator).
    // Must come AFTER @name/$name and numeric suffixes in priority.
    let bare_ident = any()
        .filter(|c: &char| c.is_ascii_alphabetic())
        .then(
            any()
                .filter(|c: &char| c.is_ascii_alphanumeric() || *c == '_')
                .repeated()
                .to_slice(),
        )
        .to_slice()
        .map(|s: &str| Token::Ident(s.to_string()));

    // ── Comments ──────────────────────────────────────────────────────────
    let comment = just('#')
        .then(any().and_is(just('\n').not()).repeated())
        .padded();

    // ── Assemble token groups ─────────────────────────────────────────────
    // Split into groups to stay within chumsky's tuple-choice arity limit.
    let multi_char_ops = choice((
        double_question,
        amp_bang,
        amp_question,
        amp_slash,
        eq,
        not_eq,
        lt_eq,
        gt_eq,
        bind,
        recall,
    ));

    let single_char_ops = choice((
        bang,
        lt,
        gt,
        question,
        pipe,
        amp,
        tilde,
        comma,
        semicolon,
        colon,
        dot,
        underscore_op,
        add,
        sub,
        mul,
        div,
        modulo,
        pow,
    ));

    let delimiters = choice((
        open_paren,
        close_paren,
        open_bracket,
        close_bracket,
        open_brace,
        close_brace,
    ));

    let token = choice((
        multi_char_ops,
        string,
        // Hex before float/dec (0x prefix).
        hex_int,
        // Float before dec (3.14 must not split at the dot).
        float,
        // Decimal int (with optional suffix).
        dec_int,
        // Bare identifiers after numbers (so 42u8 suffix is consumed by dec_int).
        bare_ident,
        single_char_ops,
        delimiters,
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
    }

    #[test]
    fn hex_literals() {
        assert_eq!(tokens("0xFF"), vec![Token::Int("0xFF".into())]);
        assert_eq!(tokens("0xDEAD"), vec![Token::Int("0xDEAD".into())]);
        assert_eq!(
            tokens("0xFFu8"),
            vec![Token::TypedInt("0xFF".into(), "u8".into())]
        );
    }

    #[test]
    fn typed_int_suffixes() {
        assert_eq!(
            tokens("42u8"),
            vec![Token::TypedInt("42".into(), "u8".into())]
        );
        assert_eq!(
            tokens("100i32"),
            vec![Token::TypedInt("100".into(), "i32".into())]
        );
        assert_eq!(
            tokens("0u256"),
            vec![Token::TypedInt("0".into(), "u256".into())]
        );
    }

    #[test]
    fn float_literals() {
        assert_eq!(tokens("3.14"), vec![Token::Float("3.14".into())]);
        assert_eq!(tokens("1.0e10"), vec![Token::Float("1.0e10".into())]);
    }

    #[test]
    fn typed_float_suffixes() {
        assert_eq!(
            tokens("3.14f32"),
            vec![Token::TypedFloat("3.14".into(), "f32".into())]
        );
        assert_eq!(
            tokens("42f64"),
            vec![Token::TypedFloat("42".into(), "f64".into())]
        );
    }

    #[test]
    fn single_char_idents() {
        assert_eq!(tokens("y"), vec![Token::Ident("y".into())]);
        assert_eq!(tokens("n"), vec![Token::Ident("n".into())]);
    }

    #[test]
    fn bare_identifiers() {
        assert_eq!(tokens("f32"), vec![Token::Ident("f32".into())]);
        assert_eq!(tokens("int"), vec![Token::Ident("int".into())]);
        assert_eq!(tokens("str"), vec![Token::Ident("str".into())]);
    }

    #[test]
    fn string_literals() {
        assert_eq!(tokens(r#""hello""#), vec![Token::Str("hello".into())]);
        assert_eq!(tokens(r#""a\nb""#), vec![Token::Str("a\nb".into())]);
    }

    #[test]
    fn bind_recall() {
        assert_eq!(tokens("@x"), vec![Token::Bind("x".into())]);
        assert_eq!(tokens("$x"), vec![Token::Recall("x".into())]);
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
        assert_eq!(tokens("=="), vec![Token::Eq]);
        assert_eq!(tokens("!="), vec![Token::NotEq]);
        assert_eq!(tokens("<="), vec![Token::LtEq]);
        assert_eq!(tokens(">="), vec![Token::GtEq]);
    }

    #[test]
    fn delimiters() {
        assert_eq!(
            tokens("( ) [ ] { }"),
            vec![
                Token::OpenParen,
                Token::CloseParen,
                Token::OpenBracket,
                Token::CloseBracket,
                Token::OpenBrace,
                Token::CloseBrace,
            ]
        );
    }

    #[test]
    fn comments_stripped() {
        assert_eq!(
            tokens("3 4 + # comment"),
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
    }

    #[test]
    fn mixed_types() {
        assert_eq!(
            tokens(r#"42u8 3.14f32 "hi""#),
            vec![
                Token::TypedInt("42".into(), "u8".into()),
                Token::TypedFloat("3.14".into(), "f32".into()),
                Token::Str("hi".into()),
            ]
        );
    }
}
