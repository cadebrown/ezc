use chumsky::{prelude::*, Stream};

use crate::{
    error::{ErrorCode, EzcError},
    ezclang::tokenizer::{SpannedToken, TokenKind},
    Span,
};

pub type Spanned<T> = (T, Span);

#[derive(Debug, Clone, PartialEq, Eq)]
pub enum AstNode {
    Number(i64),
    Text(String),
    Word(String),
    Block(Vec<Spanned<AstNode>>),
    Stack(Vec<Spanned<AstNode>>),
}

#[derive(Debug, Clone, PartialEq, Eq, Default)]
pub struct AstProgram {
    pub nodes: Vec<Spanned<AstNode>>,
}

pub fn parse(tokens: &[SpannedToken], source_len: usize) -> Result<AstProgram, EzcError> {
    // Recursive grammar: literals/words plus nested block/stack containers.
    let parser = recursive(|expr| {
        let number = select! {
            TokenKind::Number(value) => AstNode::Number(value)
        }
        .map_with_span(|node, span: Span| (node, span));

        let text = select! {
            TokenKind::Text(text) => AstNode::Text(text)
        }
        .map_with_span(|node, span: Span| (node, span));

        let word = select! {
            TokenKind::Word(word) => AstNode::Word(word)
        }
        .map_with_span(|node, span: Span| (node, span));

        let lparen = just(TokenKind::LParen).map_with_span(|_, span: Span| span);
        let rparen = just(TokenKind::RParen).map_with_span(|_, span: Span| span);
        let lbracket = just(TokenKind::LBracket).map_with_span(|_, span: Span| span);
        let rbracket = just(TokenKind::RBracket).map_with_span(|_, span: Span| span);

        let block = lparen
            .then(expr.clone().repeated())
            .then(rparen)
            .map(|((open, nodes), close)| (AstNode::Block(nodes), open.start..close.end));

        let stack = lbracket
            .then(expr.clone().repeated())
            .then(rbracket)
            .map(|((open, nodes), close)| (AstNode::Stack(nodes), open.start..close.end));

        number.or(text).or(word).or(block).or(stack)
    })
    .repeated()
    .then_ignore(end());

    let eoi = source_len..source_len;

    // Feed chumsky a token stream that preserves lexer spans end-to-end.
    parser
        .parse(Stream::from_iter(eoi, tokens.iter().cloned()))
        .map(|nodes| AstProgram { nodes })
        .map_err(convert_parse_error)
}

fn convert_parse_error(errors: Vec<Simple<TokenKind>>) -> EzcError {
    let Some(err) = errors.into_iter().next() else {
        return EzcError::new(
            ErrorCode::ParseUnexpectedToken,
            "unknown parser failure",
            0..0,
        );
    };

    let span = err.span();
    let found = err
        .found()
        .map(|token| format!("{token:?}"))
        .unwrap_or_else(|| "end of input".to_string());

    EzcError::new(
        ErrorCode::ParseUnexpectedToken,
        "could not parse token stream",
        span.start..span.end,
    )
    .with_primary_label(format!("unexpected `{found}`"))
    .with_note("Check that parentheses and brackets are balanced and tokens are well-formed.")
    .with_help("For delayed code use `( ... )`, for stack literals use `[ ... ]`.")
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::ezclang::tokenizer;

    #[test]
    fn parses_numbers_words_blocks_and_stacks() {
        let tokens = tokenizer::tokenize("1 (2 3 +) [4 5]").expect("tokenization should succeed");
        let ast = parse(&tokens, 15).expect("parser should succeed");

        assert_eq!(ast.nodes.len(), 3);
        assert_eq!(ast.nodes[0], (AstNode::Number(1), 0..1));

        let (AstNode::Block(inner), _) = &ast.nodes[1] else {
            panic!("second node should be a block");
        };
        assert_eq!(inner.len(), 3);

        let (AstNode::Stack(inner), _) = &ast.nodes[2] else {
            panic!("third node should be a stack");
        };
        assert_eq!(inner.len(), 2);
    }
}
