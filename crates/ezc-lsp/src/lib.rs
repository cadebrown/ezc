//! `ezc-lsp` — Language Server Protocol implementation for the ezc language.
//!
//! Provides diagnostics, hover documentation, and completions.
//! Start with `ezc lsp` or run the `ezc-lsp` binary — communicates over stdio.

mod docs;

use std::collections::HashMap;
use std::sync::Arc;

use docs::{completion_items, token_docs};
use ezc::lexer;
use ezc::token::Token;
use tokio::sync::RwLock;
use tower_lsp::jsonrpc::Result;
use tower_lsp::lsp_types::*;
use tower_lsp::{Client, LanguageServer, LspService, Server};

// ── Position utilities ────────────────────────────────────────────────────────

/// Convert a byte offset in `src` to an LSP `Position` (line, UTF-16 character).
fn offset_to_position(src: &str, offset: usize) -> Position {
    let offset = offset.min(src.len());
    let prefix = &src[..offset];
    let line = prefix.bytes().filter(|&b| b == b'\n').count() as u32;
    let last_nl = prefix.rfind('\n').map(|i| i + 1).unwrap_or(0);
    let character = prefix[last_nl..]
        .chars()
        .map(|c| c.len_utf16() as u32)
        .sum();
    Position { line, character }
}

/// Convert an LSP `Position` to a byte offset in `src`.
fn position_to_offset(src: &str, pos: Position) -> usize {
    let mut current_line = 0u32;
    let mut line_start = 0usize;
    for (i, ch) in src.char_indices() {
        if current_line == pos.line {
            break;
        }
        if ch == '\n' {
            current_line += 1;
            line_start = i + 1;
        }
    }
    let mut utf16 = 0u32;
    let mut byte_pos = line_start;
    for ch in src[line_start..].chars() {
        if utf16 >= pos.character {
            break;
        }
        utf16 += ch.len_utf16() as u32;
        byte_pos += ch.len_utf8();
    }
    byte_pos
}

// ── Backend ───────────────────────────────────────────────────────────────────

struct Backend {
    client: Client,
    documents: Arc<RwLock<HashMap<Url, String>>>,
}

impl Backend {
    fn new(client: Client) -> Self {
        Self {
            client,
            documents: Arc::new(RwLock::new(HashMap::new())),
        }
    }

    /// Re-parse a document and publish diagnostics.
    async fn on_change(&self, uri: Url, text: String) {
        let diagnostics = parse_diagnostics(&text);
        self.documents.write().await.insert(uri.clone(), text);
        self.client
            .publish_diagnostics(uri, diagnostics, None)
            .await;
    }
}

/// Lex and parse source, convert errors to LSP `Diagnostic` objects.
fn parse_diagnostics(src: &str) -> Vec<Diagnostic> {
    let tokens = match lexer::lex(src) {
        Ok(t) => t,
        Err(errs) => {
            return errs
                .into_iter()
                .map(|e| {
                    let span = e.span().into_range();
                    let range = Range {
                        start: offset_to_position(src, span.start),
                        end: offset_to_position(src, span.end),
                    };
                    Diagnostic {
                        range,
                        severity: Some(DiagnosticSeverity::ERROR),
                        source: Some("ezc".into()),
                        message: format!("{e}"),
                        ..Default::default()
                    }
                })
                .collect();
        }
    };

    match ezc::parser::parse(&tokens, src.len()) {
        Ok(_) => vec![],
        Err(errs) => errs
            .into_iter()
            .map(|e| {
                let span = e.span().into_range();
                let range = Range {
                    start: offset_to_position(src, span.start),
                    end: offset_to_position(src, span.end),
                };
                let message = match e.found() {
                    Some(tok) => format!("unexpected `{tok}`"),
                    None => "unexpected end of input".into(),
                };
                Diagnostic {
                    range,
                    severity: Some(DiagnosticSeverity::ERROR),
                    source: Some("ezc".into()),
                    message,
                    ..Default::default()
                }
            })
            .collect(),
    }
}

/// Lex source and find the token whose span contains `offset`.
fn token_at(src: &str, offset: usize) -> Option<Token> {
    let tokens = lexer::lex(src).ok()?;
    tokens.into_iter().find_map(|(tok, span)| {
        let r = span.into_range();
        if r.start <= offset && offset < r.end {
            Some(tok)
        } else {
            None
        }
    })
}

/// Lex source and collect unique variable names from `@name` bindings.
fn bound_variables(src: &str) -> Vec<String> {
    let Ok(tokens) = lexer::lex(src) else {
        return vec![];
    };
    let mut names: Vec<String> = tokens
        .into_iter()
        .filter_map(|(tok, _)| {
            if let Token::Bind(name) = tok {
                Some(name)
            } else {
                None
            }
        })
        .collect();
    names.sort();
    names.dedup();
    names
}

// ── LanguageServer implementation ─────────────────────────────────────────────

#[tower_lsp::async_trait]
impl LanguageServer for Backend {
    async fn initialize(&self, _params: InitializeParams) -> Result<InitializeResult> {
        Ok(InitializeResult {
            capabilities: ServerCapabilities {
                text_document_sync: Some(TextDocumentSyncCapability::Kind(
                    TextDocumentSyncKind::FULL,
                )),
                hover_provider: Some(HoverProviderCapability::Simple(true)),
                completion_provider: Some(CompletionOptions {
                    trigger_characters: Some(vec!["$".into(), "@".into()]),
                    resolve_provider: Some(false),
                    ..Default::default()
                }),
                ..Default::default()
            },
            server_info: Some(ServerInfo {
                name: "ezc-lsp".into(),
                version: Some(env!("CARGO_PKG_VERSION").into()),
            }),
            ..Default::default()
        })
    }

    async fn initialized(&self, _: InitializedParams) {
        self.client
            .log_message(MessageType::INFO, "ezc language server ready")
            .await;
    }

    async fn shutdown(&self) -> Result<()> {
        Ok(())
    }

    async fn did_open(&self, params: DidOpenTextDocumentParams) {
        self.on_change(params.text_document.uri, params.text_document.text)
            .await;
    }

    async fn did_change(&self, params: DidChangeTextDocumentParams) {
        // FULL sync — last entry is the complete document
        if let Some(change) = params.content_changes.into_iter().last() {
            self.on_change(params.text_document.uri, change.text).await;
        }
    }

    async fn did_close(&self, params: DidCloseTextDocumentParams) {
        let uri = params.text_document.uri;
        self.documents.write().await.remove(&uri);
        self.client.publish_diagnostics(uri, vec![], None).await;
    }

    async fn hover(&self, params: HoverParams) -> Result<Option<Hover>> {
        let uri = &params.text_document_position_params.text_document.uri;
        let pos = params.text_document_position_params.position;

        let src = {
            let docs = self.documents.read().await;
            match docs.get(uri) {
                Some(s) => s.clone(),
                None => return Ok(None),
            }
        };

        let offset = position_to_offset(&src, pos);
        let token = match token_at(&src, offset) {
            Some(t) => t,
            None => return Ok(None),
        };

        Ok(token_docs(&token).map(|md| Hover {
            contents: HoverContents::Markup(MarkupContent {
                kind: MarkupKind::Markdown,
                value: md.into(),
            }),
            range: None,
        }))
    }

    async fn completion(&self, params: CompletionParams) -> Result<Option<CompletionResponse>> {
        let uri = &params.text_document_position.text_document.uri;
        let pos = params.text_document_position.position;

        let src = {
            let docs = self.documents.read().await;
            match docs.get(uri) {
                Some(s) => s.clone(),
                None => return Ok(Some(CompletionResponse::Array(completion_items(&[])))),
            }
        };

        let offset = position_to_offset(&src, pos);
        let trigger = src[..offset].chars().last();
        let vars = bound_variables(&src);

        // When triggered by $ or @, return only variable-name completions
        if let Some(ch) = trigger {
            if ch == '$' || ch == '@' {
                let prefix = ch.to_string();
                let items = vars
                    .iter()
                    .map(|name| CompletionItem {
                        label: format!("{prefix}{name}"),
                        kind: Some(CompletionItemKind::VARIABLE),
                        detail: Some("variable".into()),
                        insert_text: Some(name.clone()),
                        ..Default::default()
                    })
                    .collect();
                return Ok(Some(CompletionResponse::Array(items)));
            }
        }

        Ok(Some(CompletionResponse::Array(completion_items(&vars))))
    }
}

// ── Entry point ───────────────────────────────────────────────────────────────

/// Start the LSP server on stdio. Blocks until the client disconnects.
pub fn run_server() {
    let rt = tokio::runtime::Builder::new_multi_thread()
        .enable_all()
        .build()
        .expect("tokio runtime");

    rt.block_on(async {
        let stdin = tokio::io::stdin();
        let stdout = tokio::io::stdout();
        let (service, socket) = LspService::new(Backend::new);
        Server::new(stdin, stdout, socket).serve(service).await;
    });
}
