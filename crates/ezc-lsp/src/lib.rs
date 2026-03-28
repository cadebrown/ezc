//! `ezc-lsp` — Language Server Protocol implementation for the ezc language.
//!
//! Provides diagnostics, hover documentation, completions, go-to-definition,
//! references, document symbols, rename, and semantic tokens.
//! Start with `ezc lsp` or run the `ezc-lsp` binary — communicates over stdio.

mod docs;
mod symbols;

use std::collections::HashMap;
use std::sync::Arc;

use docs::{completion_items, token_docs};
use ezc::lexer;
use ezc::line_index::LineIndex;
use ezc::token::Token;
use symbols::{BuiltinSets, SemanticClass, SymbolIndex};
use tokio::sync::RwLock;
use tower_lsp::jsonrpc::Result;
use tower_lsp::lsp_types::*;
use tower_lsp::{Client, LanguageServer, LspService, Server};

// ── Position utilities ────────────────────────────────────────────────────────

/// Convert a byte offset in `src` to an LSP `Position` (0-based line, UTF-16 character).
fn offset_to_position(src: &str, offset: usize) -> Position {
    let li = LineIndex::new(src);
    let (line, col) = li.line_col(offset.min(src.len()));
    Position {
        line: line as u32,
        character: col as u32,
    }
}

/// Convert an LSP `Position` to a byte offset in `src`.
fn position_to_offset(src: &str, pos: Position) -> usize {
    let li = LineIndex::new(src);
    let line_start = li.line_start(pos.line as usize);
    // Walk UTF-16 code units to find the byte position.
    let mut utf16 = 0u32;
    let mut byte_pos = line_start;
    for ch in src[line_start..].chars() {
        if ch == '\n' || utf16 >= pos.character {
            break;
        }
        utf16 += ch.len_utf16() as u32;
        byte_pos += ch.len_utf8();
    }
    byte_pos
}

/// Convert a byte-offset range to an LSP `Range`, using a pre-built `LineIndex`.
fn span_to_range(li: &LineIndex, span: &std::ops::Range<usize>) -> Range {
    let (sl, sc) = li.line_col(span.start);
    let (el, ec) = li.line_col(span.end);
    Range {
        start: Position {
            line: sl as u32,
            character: sc as u32,
        },
        end: Position {
            line: el as u32,
            character: ec as u32,
        },
    }
}

// ── Semantic token legend ────────────────────────────────────────────────────

const TOKEN_TYPES: &[SemanticTokenType] = &[
    SemanticTokenType::NUMBER,    // 0: number
    SemanticTokenType::STRING,    // 1: string
    SemanticTokenType::OPERATOR,  // 2: operator
    SemanticTokenType::VARIABLE,  // 3: variable ($name)
    SemanticTokenType::PARAMETER, // 4: parameter (@name)
    SemanticTokenType::FUNCTION,  // 5: function (builtin/prelude)
    SemanticTokenType::TYPE,      // 6: type constructor
    SemanticTokenType::KEYWORD,   // 7: keyword (control flow)
];

fn semantic_token_legend() -> SemanticTokensLegend {
    SemanticTokensLegend {
        token_types: TOKEN_TYPES.to_vec(),
        token_modifiers: vec![],
    }
}

// ── Backend ───────────────────────────────────────────────────────────────────

struct Backend {
    client: Client,
    /// Document URI → (source text, symbol index).
    documents: Arc<RwLock<HashMap<Url, (String, SymbolIndex)>>>,
    builtins: BuiltinSets,
}

impl Backend {
    fn new(client: Client) -> Self {
        Self {
            client,
            documents: Arc::new(RwLock::new(HashMap::new())),
            builtins: BuiltinSets::new(),
        }
    }

    /// Re-parse a document, rebuild symbol index, and publish diagnostics.
    async fn on_change(&self, uri: Url, text: String) {
        let diagnostics = parse_diagnostics(&text);
        let index = SymbolIndex::build(&text, &self.builtins);
        self.documents
            .write()
            .await
            .insert(uri.clone(), (text, index));
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

// ── Classify token for semantic highlighting ─────────────────────────────────

/// Return the semantic token type index for a token, or `None` to skip.
fn classify_token(tok: &Token, builtins: &BuiltinSets) -> Option<u32> {
    match tok {
        Token::Int(_) | Token::TypedInt(_, _) | Token::Float(_) | Token::TypedFloat(_, _) => {
            Some(0)
        }
        Token::Str(_) => Some(1),
        Token::Op(_)
        | Token::Bang
        | Token::Question
        | Token::DoubleQuestion
        | Token::Pipe
        | Token::AmpBang
        | Token::AmpQuestion
        | Token::AmpSlash
        | Token::Amp
        | Token::Eq
        | Token::NotEq
        | Token::Lt
        | Token::Gt
        | Token::LtEq
        | Token::GtEq
        | Token::Tilde
        | Token::Comma
        | Token::Semicolon
        | Token::Colon
        | Token::Dot
        | Token::Underscore => Some(2),
        Token::Recall(_) => Some(3),
        Token::Bind(_) => Some(4),
        Token::Ident(name) => match builtins.classify(name) {
            SemanticClass::Function => Some(5),
            SemanticClass::Type => Some(6),
            SemanticClass::Keyword => Some(7),
            SemanticClass::Variable => Some(3), // user-defined ident treated as variable
        },
        // Brackets/braces/parens — skip (editor handles matching).
        Token::OpenParen
        | Token::CloseParen
        | Token::OpenBracket
        | Token::CloseBracket
        | Token::OpenBrace
        | Token::CloseBrace => None,
    }
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
                definition_provider: Some(OneOf::Left(true)),
                references_provider: Some(OneOf::Left(true)),
                document_symbol_provider: Some(OneOf::Left(true)),
                rename_provider: Some(OneOf::Right(RenameOptions {
                    prepare_provider: Some(true),
                    work_done_progress_options: Default::default(),
                })),
                semantic_tokens_provider: Some(
                    SemanticTokensServerCapabilities::SemanticTokensOptions(
                        SemanticTokensOptions {
                            legend: semantic_token_legend(),
                            full: Some(SemanticTokensFullOptions::Bool(true)),
                            range: None,
                            work_done_progress_options: Default::default(),
                        },
                    ),
                ),
                folding_range_provider: Some(FoldingRangeProviderCapability::Simple(true)),
                document_link_provider: Some(DocumentLinkOptions {
                    resolve_provider: Some(false),
                    work_done_progress_options: Default::default(),
                }),
                signature_help_provider: Some(SignatureHelpOptions {
                    trigger_characters: Some(vec![" ".into()]),
                    retrigger_characters: None,
                    work_done_progress_options: Default::default(),
                }),
                code_lens_provider: Some(CodeLensOptions {
                    resolve_provider: Some(false),
                }),
                workspace_symbol_provider: Some(OneOf::Left(true)),
                inlay_hint_provider: Some(OneOf::Left(true)),
                document_formatting_provider: Some(OneOf::Left(true)),
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

        let docs = self.documents.read().await;
        let (src, index) = match docs.get(uri) {
            Some(pair) => pair,
            None => return Ok(None),
        };

        let offset = position_to_offset(src, pos);
        let token = match index.token_at(offset) {
            Some((t, _)) => t,
            None => return Ok(None),
        };

        Ok(token_docs(token).map(|md| Hover {
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

        let docs = self.documents.read().await;
        let (src, index) = match docs.get(uri) {
            Some(pair) => pair,
            None => return Ok(Some(CompletionResponse::Array(completion_items(&[])))),
        };

        let offset = position_to_offset(src, pos);
        let trigger = src[..offset].chars().last();

        // Collect unique variable names from cached index.
        let mut vars: Vec<String> = index.definitions.keys().cloned().collect();
        vars.sort();

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

    async fn goto_definition(
        &self,
        params: GotoDefinitionParams,
    ) -> Result<Option<GotoDefinitionResponse>> {
        let uri = &params.text_document_position_params.text_document.uri;
        let pos = params.text_document_position_params.position;

        let docs = self.documents.read().await;
        let (src, index) = match docs.get(uri) {
            Some(pair) => pair,
            None => return Ok(None),
        };

        let offset = position_to_offset(src, pos);
        let name = match index.name_at(offset) {
            Some(n) => n,
            None => return Ok(None),
        };

        let def_sites = index.definition_sites(name);
        if def_sites.is_empty() {
            return Ok(None);
        }

        let li = LineIndex::new(src);
        let locations: Vec<Location> = def_sites
            .iter()
            .map(|span| Location {
                uri: uri.clone(),
                range: span_to_range(&li, span),
            })
            .collect();

        Ok(Some(if locations.len() == 1 {
            GotoDefinitionResponse::Scalar(locations.into_iter().next().unwrap())
        } else {
            GotoDefinitionResponse::Array(locations)
        }))
    }

    async fn references(&self, params: ReferenceParams) -> Result<Option<Vec<Location>>> {
        let uri = &params.text_document_position.text_document.uri;
        let pos = params.text_document_position.position;

        let docs = self.documents.read().await;
        let (src, index) = match docs.get(uri) {
            Some(pair) => pair,
            None => return Ok(None),
        };

        let offset = position_to_offset(src, pos);
        let name = match index.name_at(offset) {
            Some(n) => n.to_owned(),
            None => return Ok(None),
        };

        let occurrences = index.all_occurrences(&name);
        if occurrences.is_empty() {
            return Ok(None);
        }

        let li = LineIndex::new(src);
        let locations: Vec<Location> = occurrences
            .iter()
            .map(|span| Location {
                uri: uri.clone(),
                range: span_to_range(&li, span),
            })
            .collect();

        Ok(Some(locations))
    }

    async fn document_symbol(
        &self,
        params: DocumentSymbolParams,
    ) -> Result<Option<DocumentSymbolResponse>> {
        let uri = &params.text_document.uri;

        let docs = self.documents.read().await;
        let (src, index) = match docs.get(uri) {
            Some(pair) => pair,
            None => return Ok(None),
        };

        let li = LineIndex::new(src);
        let defined = index.defined_names();
        if defined.is_empty() {
            return Ok(None);
        }

        #[allow(deprecated)]
        let symbols: Vec<DocumentSymbol> = defined
            .iter()
            .map(|(name, spans)| {
                let first_span = &spans[0];
                let kind = if index.is_function_def(first_span) {
                    SymbolKind::FUNCTION
                } else {
                    SymbolKind::VARIABLE
                };
                let range = span_to_range(&li, first_span);
                DocumentSymbol {
                    name: name.to_string(),
                    detail: None,
                    kind,
                    tags: None,
                    deprecated: None,
                    range,
                    selection_range: range,
                    children: None,
                }
            })
            .collect();

        Ok(Some(DocumentSymbolResponse::Nested(symbols)))
    }

    async fn prepare_rename(
        &self,
        params: TextDocumentPositionParams,
    ) -> Result<Option<PrepareRenameResponse>> {
        let uri = &params.text_document.uri;
        let pos = params.position;

        let docs = self.documents.read().await;
        let (src, index) = match docs.get(uri) {
            Some(pair) => pair,
            None => return Ok(None),
        };

        let offset = position_to_offset(src, pos);
        let (tok, span) = match index.token_at(offset) {
            Some(pair) => pair,
            None => return Ok(None),
        };

        // Only allow rename on Bind, Recall, or user-defined Ident.
        let name = match tok {
            Token::Bind(n) | Token::Recall(n) => n.as_str(),
            Token::Ident(n) if !self.builtins.is_known(n) => n.as_str(),
            _ => return Ok(None),
        };

        let li = LineIndex::new(src);
        let range = span_to_range(&li, span);

        Ok(Some(PrepareRenameResponse::RangeWithPlaceholder {
            range,
            placeholder: name.to_string(),
        }))
    }

    async fn rename(&self, params: RenameParams) -> Result<Option<WorkspaceEdit>> {
        let uri = &params.text_document_position.text_document.uri;
        let pos = params.text_document_position.position;
        let new_name = &params.new_name;

        let docs = self.documents.read().await;
        let (src, index) = match docs.get(uri) {
            Some(pair) => pair,
            None => return Ok(None),
        };

        let offset = position_to_offset(src, pos);
        let name = match index.name_at(offset) {
            Some(n) => n.to_owned(),
            None => return Ok(None),
        };

        // Build edits for every occurrence. Each token's text includes the sigil
        // (@name, $name, or bare name), so we reconstruct the replacement with the
        // appropriate sigil.
        let li = LineIndex::new(src);
        let mut edits: Vec<TextEdit> = Vec::new();

        for (tok, span) in &index.tokens {
            let replacement = match tok {
                Token::Bind(n) if n == &name => format!("@{new_name}"),
                Token::Recall(n) if n == &name => format!("${new_name}"),
                Token::Ident(n) if n == &name && !self.builtins.is_known(n) => new_name.clone(),
                _ => continue,
            };
            edits.push(TextEdit {
                range: span_to_range(&li, span),
                new_text: replacement,
            });
        }

        if edits.is_empty() {
            return Ok(None);
        }

        let mut changes = HashMap::new();
        changes.insert(uri.clone(), edits);

        Ok(Some(WorkspaceEdit {
            changes: Some(changes),
            ..Default::default()
        }))
    }

    async fn semantic_tokens_full(
        &self,
        params: SemanticTokensParams,
    ) -> Result<Option<SemanticTokensResult>> {
        let uri = &params.text_document.uri;

        let docs = self.documents.read().await;
        let (src, index) = match docs.get(uri) {
            Some(pair) => pair,
            None => return Ok(None),
        };

        let li = LineIndex::new(src);
        let mut data: Vec<SemanticToken> = Vec::new();
        let mut prev_line: u32 = 0;
        let mut prev_start: u32 = 0;

        for (tok, span) in &index.tokens {
            let type_idx = match classify_token(tok, &self.builtins) {
                Some(idx) => idx,
                None => continue,
            };

            let (line, col) = li.line_col(span.start);
            let line = line as u32;
            let col = col as u32;
            let length = (span.end - span.start) as u32;

            let delta_line = line - prev_line;
            let delta_start = if delta_line == 0 {
                col - prev_start
            } else {
                col
            };

            data.push(SemanticToken {
                delta_line,
                delta_start,
                length,
                token_type: type_idx,
                token_modifiers_bitset: 0,
            });

            prev_line = line;
            prev_start = col;
        }

        Ok(Some(SemanticTokensResult::Tokens(SemanticTokens {
            result_id: None,
            data,
        })))
    }

    // ── Folding Ranges ──────────────────────────────────────────────────────

    async fn folding_range(&self, params: FoldingRangeParams) -> Result<Option<Vec<FoldingRange>>> {
        let uri = &params.text_document.uri;

        let docs = self.documents.read().await;
        let (src, index) = match docs.get(uri) {
            Some(pair) => pair,
            None => return Ok(None),
        };

        let li = LineIndex::new(src);
        let mut ranges: Vec<FoldingRange> = Vec::new();

        // Delimiter-based folding: match ()/[]/{}
        let mut stack: Vec<(u32, &Token)> = Vec::new();
        for (tok, span) in &index.tokens {
            match tok {
                Token::OpenParen | Token::OpenBracket | Token::OpenBrace => {
                    let (line, _) = li.line_col(span.start);
                    stack.push((line as u32, tok));
                }
                Token::CloseParen | Token::CloseBracket | Token::CloseBrace => {
                    let expected_open = match tok {
                        Token::CloseParen => Token::OpenParen,
                        Token::CloseBracket => Token::OpenBracket,
                        Token::CloseBrace => Token::OpenBrace,
                        _ => unreachable!(),
                    };
                    // Pop matching opener
                    if let Some(pos) = stack.iter().rposition(|(_, t)| **t == expected_open) {
                        let (start_line, _) = stack.remove(pos);
                        let (end_line, _) = li.line_col(span.start);
                        let end_line = end_line as u32;
                        if end_line > start_line {
                            ranges.push(FoldingRange {
                                start_line,
                                start_character: None,
                                end_line,
                                end_character: None,
                                kind: Some(FoldingRangeKind::Region),
                                collapsed_text: None,
                            });
                        }
                    }
                }
                _ => {}
            }
        }

        // Comment-block folding: consecutive lines starting with #
        let mut comment_start: Option<u32> = None;
        for (line_num, line) in src.lines().enumerate() {
            let trimmed = line.trim_start();
            if trimmed.starts_with('#') {
                if comment_start.is_none() {
                    comment_start = Some(line_num as u32);
                }
            } else if let Some(start) = comment_start.take() {
                let end = line_num as u32 - 1;
                if end > start {
                    ranges.push(FoldingRange {
                        start_line: start,
                        start_character: None,
                        end_line: end,
                        end_character: None,
                        kind: Some(FoldingRangeKind::Comment),
                        collapsed_text: None,
                    });
                }
            }
        }
        // Trailing comment block
        if let Some(start) = comment_start {
            let end = src.lines().count() as u32 - 1;
            if end > start {
                ranges.push(FoldingRange {
                    start_line: start,
                    start_character: None,
                    end_line: end,
                    end_character: None,
                    kind: Some(FoldingRangeKind::Comment),
                    collapsed_text: None,
                });
            }
        }

        if ranges.is_empty() {
            Ok(None)
        } else {
            Ok(Some(ranges))
        }
    }

    // ── Document Links ──────────────────────────────────────────────────────

    async fn document_link(&self, params: DocumentLinkParams) -> Result<Option<Vec<DocumentLink>>> {
        let uri = &params.text_document.uri;

        let docs = self.documents.read().await;
        let (src, index) = match docs.get(uri) {
            Some(pair) => pair,
            None => return Ok(None),
        };

        let li = LineIndex::new(src);
        let mut links: Vec<DocumentLink> = Vec::new();

        // Look for Token::Str followed by Token::Ident("import")
        for window in index.tokens.windows(2) {
            let (tok, span) = &window[0];
            let (next_tok, _) = &window[1];

            if let Token::Str(path) = tok {
                if matches!(next_tok, Token::Ident(name) if name == "import") {
                    // Resolve the path
                    let target = if path.starts_with("std/") {
                        // Embedded module — try workspace root
                        uri.join(path).ok()
                    } else {
                        // Relative to the document
                        let base = uri.join(".").unwrap_or_else(|_| uri.clone());
                        base.join(path).ok()
                    };

                    links.push(DocumentLink {
                        range: span_to_range(&li, span),
                        target,
                        tooltip: Some(format!("Import: {path}")),
                        data: None,
                    });
                }
            }
        }

        if links.is_empty() {
            Ok(None)
        } else {
            Ok(Some(links))
        }
    }

    // ── Signature Help ──────────────────────────────────────────────────────

    async fn signature_help(&self, params: SignatureHelpParams) -> Result<Option<SignatureHelp>> {
        let uri = &params.text_document_position_params.text_document.uri;
        let pos = params.text_document_position_params.position;

        let docs = self.documents.read().await;
        let (src, index) = match docs.get(uri) {
            Some(pair) => pair,
            None => return Ok(None),
        };

        let offset = position_to_offset(src, pos);

        // Find the token at or just before cursor
        let token = index.token_at(offset).or_else(|| {
            if offset > 0 {
                index.token_at(offset - 1)
            } else {
                None
            }
        });

        let (tok, _) = match token {
            Some(pair) => pair,
            None => return Ok(None),
        };

        let doc = match token_docs(tok) {
            Some(d) => d,
            None => return Ok(None),
        };

        // Extract first line as signature label (the stack effect)
        let label = doc
            .lines()
            .find(|l| !l.is_empty())
            .unwrap_or(doc)
            .to_string();

        Ok(Some(SignatureHelp {
            signatures: vec![SignatureInformation {
                label,
                documentation: Some(tower_lsp::lsp_types::Documentation::MarkupContent(
                    MarkupContent {
                        kind: MarkupKind::Markdown,
                        value: doc.to_string(),
                    },
                )),
                parameters: None,
                active_parameter: None,
            }],
            active_signature: Some(0),
            active_parameter: None,
        }))
    }

    // ── Code Lens ───────────────────────────────────────────────────────────

    async fn code_lens(&self, params: CodeLensParams) -> Result<Option<Vec<CodeLens>>> {
        let uri = &params.text_document.uri;

        let docs = self.documents.read().await;
        let (src, index) = match docs.get(uri) {
            Some(pair) => pair,
            None => return Ok(None),
        };

        let li = LineIndex::new(src);
        let mut lenses: Vec<CodeLens> = Vec::new();

        for (name, def_spans) in &index.definitions {
            let ref_count = index.references.get(name).map(|v| v.len()).unwrap_or(0);

            for def_span in def_spans {
                let range = span_to_range(&li, def_span);
                let title = if ref_count == 1 {
                    "1 reference".to_string()
                } else {
                    format!("{ref_count} references")
                };

                lenses.push(CodeLens {
                    range,
                    command: Some(Command {
                        title,
                        command: "editor.action.findReferences".into(),
                        arguments: None,
                    }),
                    data: None,
                });
            }
        }

        if lenses.is_empty() {
            Ok(None)
        } else {
            Ok(Some(lenses))
        }
    }

    // ── Workspace Symbols ───────────────────────────────────────────────────

    #[allow(deprecated)]
    async fn symbol(
        &self,
        params: WorkspaceSymbolParams,
    ) -> Result<Option<Vec<SymbolInformation>>> {
        let query = &params.query;
        let docs = self.documents.read().await;
        let mut symbols: Vec<SymbolInformation> = Vec::new();

        for (uri, (src, index)) in docs.iter() {
            let li = LineIndex::new(src);
            for (name, spans) in index.defined_names() {
                // Filter by query: empty query matches all, otherwise substring match
                if !query.is_empty() && !name.contains(query.as_str()) {
                    continue;
                }
                let first_span = &spans[0];
                let kind = if index.is_function_def(first_span) {
                    SymbolKind::FUNCTION
                } else {
                    SymbolKind::VARIABLE
                };

                symbols.push(SymbolInformation {
                    name: name.to_string(),
                    kind,
                    tags: None,
                    deprecated: None,
                    location: Location {
                        uri: uri.clone(),
                        range: span_to_range(&li, first_span),
                    },
                    container_name: None,
                });
            }
        }

        if symbols.is_empty() {
            Ok(None)
        } else {
            Ok(Some(symbols))
        }
    }

    // ── Inlay Hints ─────────────────────────────────────────────────────────

    async fn inlay_hint(&self, params: InlayHintParams) -> Result<Option<Vec<InlayHint>>> {
        let uri = &params.text_document.uri;

        let docs = self.documents.read().await;
        let (src, index) = match docs.get(uri) {
            Some(pair) => pair,
            None => return Ok(None),
        };

        let li = LineIndex::new(src);
        let mut hints: Vec<InlayHint> = Vec::new();

        // Show reference count and type info after @name definitions.
        for (name, def_spans) in &index.definitions {
            let ref_count = index.references.get(name).map(|v| v.len()).unwrap_or(0);

            let kind_label = if def_spans.iter().any(|s| index.is_function_def(s)) {
                "block"
            } else {
                "value"
            };

            for def_span in def_spans {
                let (line, col) = li.line_col(def_span.end);

                // Only show hints within the requested range
                let hint_line = line as u32;
                if hint_line < params.range.start.line || hint_line > params.range.end.line {
                    continue;
                }

                let label = format!(
                    " \u{2190} {ref_count} ref{}, {kind_label}",
                    if ref_count == 1 { "" } else { "s" }
                );

                hints.push(InlayHint {
                    position: Position {
                        line: hint_line,
                        character: col as u32,
                    },
                    label: InlayHintLabel::String(label),
                    kind: Some(InlayHintKind::TYPE),
                    text_edits: None,
                    tooltip: None,
                    padding_left: Some(true),
                    padding_right: None,
                    data: None,
                });
            }
        }

        if hints.is_empty() {
            Ok(None)
        } else {
            Ok(Some(hints))
        }
    }

    // ── Formatting ──────────────────────────────────────────────────────────

    async fn formatting(&self, params: DocumentFormattingParams) -> Result<Option<Vec<TextEdit>>> {
        let uri = &params.text_document.uri;

        let docs = self.documents.read().await;
        let (src, index) = match docs.get(uri) {
            Some(pair) => pair,
            None => return Ok(None),
        };

        // Rebuild source from tokens with normalized whitespace.
        // Strategy: for each line, collect the tokens on that line and
        // re-emit them with single spaces. Preserve comment text as-is.
        let li = LineIndex::new(src);
        let mut formatted_lines: Vec<String> = Vec::new();
        let line_count = li.line_count();

        for line_num in 0..line_count {
            let line_start = li.line_start(line_num);
            let line_end = if line_num + 1 < line_count {
                // line_start of next line minus the newline
                li.line_start(line_num + 1) - 1
            } else {
                src.len()
            };

            let raw_line = &src[line_start..line_end];

            // Check for comment: find # not inside a string token
            let comment_start = find_comment_start(raw_line, line_start, &index.tokens);
            let (code_part, comment_part) = match comment_start {
                Some(offset) => {
                    let rel = offset - line_start;
                    (&raw_line[..rel], Some(raw_line[rel..].trim_end()))
                }
                None => (raw_line, None),
            };

            // Collect tokens on this line
            let line_tokens: Vec<&str> = index
                .tokens
                .iter()
                .filter(|(_, span)| {
                    let (tok_line, _) = li.line_col(span.start);
                    tok_line == line_num && span.start < line_start + code_part.len()
                })
                .map(|(_, span)| &src[span.start..span.end])
                .collect();

            let mut line_out = if line_tokens.is_empty() {
                // Blank or comment-only line — preserve emptiness
                String::new()
            } else {
                line_tokens.join(" ")
            };

            // Append comment
            if let Some(comment) = comment_part {
                if !line_out.is_empty() {
                    line_out.push(' ');
                }
                line_out.push_str(comment);
            }

            // Trim trailing whitespace
            let trimmed = line_out.trim_end().to_string();
            formatted_lines.push(trimmed);
        }

        // Strip trailing empty lines, then ensure exactly one trailing newline
        while formatted_lines
            .last()
            .map(|l| l.is_empty())
            .unwrap_or(false)
        {
            formatted_lines.pop();
        }

        let mut formatted = formatted_lines.join("\n");
        formatted.push('\n');

        // If nothing changed, return no edits
        if formatted == *src {
            return Ok(None);
        }

        // Replace the entire document
        let last_line = li.line_count().saturating_sub(1);
        let last_line_start = li.line_start(last_line);
        let last_col = src.len() - last_line_start;

        Ok(Some(vec![TextEdit {
            range: Range {
                start: Position {
                    line: 0,
                    character: 0,
                },
                end: Position {
                    line: last_line as u32,
                    character: last_col as u32,
                },
            },
            new_text: formatted,
        }]))
    }
}

/// Find the byte offset of a `#` comment start on a raw source line,
/// skipping any `#` that falls inside a string token.
fn find_comment_start(
    line: &str,
    line_offset: usize,
    tokens: &[(Token, std::ops::Range<usize>)],
) -> Option<usize> {
    for (i, ch) in line.char_indices() {
        if ch == '#' {
            let abs = line_offset + i;
            // Check if this offset is inside a string token
            let in_string = tokens.iter().any(|(tok, span)| {
                matches!(tok, Token::Str(_)) && span.start <= abs && abs < span.end
            });
            if !in_string {
                return Some(abs);
            }
        }
    }
    None
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
