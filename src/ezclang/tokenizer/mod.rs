use crate::{
    error::{ErrorCode, EzcError},
    Span,
};
use logos::Logos;

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub enum TokenKind {
    Number(i64),
    Text(String),
    Word(String),
    LParen,
    RParen,
    LBracket,
    RBracket,
}

pub type SpannedToken = (TokenKind, Span);

#[derive(Debug, Logos, PartialEq)]
#[logos(skip r"[ \t\r\n\f]+")]
#[logos(skip r"//[^\n]*")]
#[logos(skip r"#[^\n]*")]
enum RawToken<'src> {
    // Numeric literals are signed integers only.
    #[regex(r"-?[0-9]+", parse_i64)]
    Number(i64),

    // EZC text objects support both quote styles with shared escaping rules.
    #[regex(r#""([^"\\]|\\.)*""#, parse_quoted)]
    #[regex(r#"'([^'\\]|\\.)*'"#, parse_quoted)]
    Text(String),

    #[token("+")]
    Plus,
    #[token("-")]
    Minus,
    #[token("*")]
    Mul,
    #[token("/")]
    Div,
    #[token("%")]
    Mod,
    #[token(".")]
    Dot,
    #[token(",")]
    Comma,
    #[token("~")]
    Tilde,

    #[token("=")]
    Eq,
    #[token("<")]
    Lt,
    #[token(">")]
    Gt,
    #[token("|")]
    Pipe,
    #[token("^")]
    Caret,

    #[token("!")]
    Bang,
    #[token("?")]
    Question,
    #[token("&")]
    Amp,

    #[token("(")]
    LParen,
    #[token(")")]
    RParen,
    #[token("[")]
    LBracket,
    #[token("]")]
    RBracket,

    #[regex(r"[A-Za-z_][A-Za-z0-9_-]*", |lex| lex.slice())]
    Word(&'src str),
}

fn parse_i64<'src>(lex: &mut logos::Lexer<'src, RawToken<'src>>) -> Option<i64> {
    lex.slice().parse::<i64>().ok()
}

fn parse_quoted<'src>(lex: &mut logos::Lexer<'src, RawToken<'src>>) -> Option<String> {
    let slice = lex.slice();
    let mut chars = slice.chars();
    let quote = chars.next()?;
    if !(quote == '"' || quote == '\'') {
        return None;
    }
    if !slice.ends_with(quote) || slice.len() < 2 {
        return None;
    }
    unescape_quoted(&slice[1..slice.len() - 1])
}

fn unescape_quoted(input: &str) -> Option<String> {
    let mut out = String::new();
    let mut chars = input.chars();

    while let Some(ch) = chars.next() {
        if ch != '\\' {
            out.push(ch);
            continue;
        }

        let escaped = chars.next()?;
        match escaped {
            'n' => out.push('\n'),
            'r' => out.push('\r'),
            't' => out.push('\t'),
            '\\' => out.push('\\'),
            '"' => out.push('"'),
            '\'' => out.push('\''),
            _ => return None,
        }
    }

    Some(out)
}

pub fn tokenize(source: &str) -> Result<Vec<SpannedToken>, EzcError> {
    // Logos already gives token spans; keep them verbatim for diagnostics and parser spans.
    RawToken::lexer(source)
        .spanned()
        .map(|(result, span)| match result {
            Ok(raw) => Ok((to_token_kind(raw), span)),
            Err(_) => Err(EzcError::new(
                ErrorCode::LexInvalidToken,
                "found an invalid token",
                span,
            )
            .with_note("Use whitespace-separated numbers/operators and balanced () / [] blocks.")
            .with_help("Example: `1 2 + (dup prt)! [a b c]`")),
        })
        .collect()
}

fn to_token_kind(raw: RawToken<'_>) -> TokenKind {
    match raw {
        RawToken::Number(value) => TokenKind::Number(value),
        RawToken::Text(text) => TokenKind::Text(text),
        RawToken::Plus => TokenKind::Word("+".to_string()),
        RawToken::Minus => TokenKind::Word("-".to_string()),
        RawToken::Mul => TokenKind::Word("*".to_string()),
        RawToken::Div => TokenKind::Word("/".to_string()),
        RawToken::Mod => TokenKind::Word("%".to_string()),
        RawToken::Dot => TokenKind::Word(".".to_string()),
        RawToken::Comma => TokenKind::Word(",".to_string()),
        RawToken::Tilde => TokenKind::Word("~".to_string()),
        RawToken::Eq => TokenKind::Word("=".to_string()),
        RawToken::Lt => TokenKind::Word("<".to_string()),
        RawToken::Gt => TokenKind::Word(">".to_string()),
        RawToken::Pipe => TokenKind::Word("|".to_string()),
        RawToken::Caret => TokenKind::Word("^".to_string()),
        RawToken::Bang => TokenKind::Word("!".to_string()),
        RawToken::Question => TokenKind::Word("?".to_string()),
        RawToken::Amp => TokenKind::Word("&".to_string()),
        RawToken::LParen => TokenKind::LParen,
        RawToken::RParen => TokenKind::RParen,
        RawToken::LBracket => TokenKind::LBracket,
        RawToken::RBracket => TokenKind::RBracket,
        RawToken::Word(word) => TokenKind::Word(word.to_string()),
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn lexes_numbers_words_and_structural_tokens() {
        let tokens = tokenize("1 2 + (dup !) [a b c]").expect("tokenization should succeed");
        assert_eq!(
            tokens
                .iter()
                .map(|(kind, _)| kind.clone())
                .collect::<Vec<_>>(),
            vec![
                TokenKind::Number(1),
                TokenKind::Number(2),
                TokenKind::Word("+".to_string()),
                TokenKind::LParen,
                TokenKind::Word("dup".to_string()),
                TokenKind::Word("!".to_string()),
                TokenKind::RParen,
                TokenKind::LBracket,
                TokenKind::Word("a".to_string()),
                TokenKind::Word("b".to_string()),
                TokenKind::Word("c".to_string()),
                TokenKind::RBracket,
            ]
        );
    }

    #[test]
    fn lexes_text_literals() {
        let tokens = tokenize("\"hello\" 'world\\nline'").expect("tokenization should succeed");
        assert_eq!(
            tokens
                .iter()
                .map(|(kind, _)| kind.clone())
                .collect::<Vec<_>>(),
            vec![
                TokenKind::Text("hello".to_string()),
                TokenKind::Text("world\nline".to_string()),
            ]
        );
    }

    #[test]
    fn lexes_comment_lines_with_hash() {
        let tokens = tokenize("1 # ignored\n2 c?").expect("tokenization should succeed");
        assert_eq!(
            tokens
                .iter()
                .map(|(kind, _)| kind.clone())
                .collect::<Vec<_>>(),
            vec![
                TokenKind::Number(1),
                TokenKind::Number(2),
                TokenKind::Word("c".to_string()),
                TokenKind::Word("?".to_string()),
            ]
        );
    }

    #[test]
    fn lexes_symbolic_alias_words() {
        let tokens = tokenize(". , ~ _ ^ | &").expect("tokenization should succeed");
        assert_eq!(
            tokens
                .iter()
                .map(|(kind, _)| kind.clone())
                .collect::<Vec<_>>(),
            vec![
                TokenKind::Word(".".to_string()),
                TokenKind::Word(",".to_string()),
                TokenKind::Word("~".to_string()),
                TokenKind::Word("_".to_string()),
                TokenKind::Word("^".to_string()),
                TokenKind::Word("|".to_string()),
                TokenKind::Word("&".to_string()),
            ]
        );
    }

    #[test]
    fn reports_invalid_characters() {
        let err = tokenize("1 @").expect_err("tokenization should fail");
        assert_eq!(err.code, ErrorCode::LexInvalidToken);
        assert!(err.message.contains("invalid token"));
    }
}
