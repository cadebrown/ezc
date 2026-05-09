//! `ezc-lsp` — Language Server Protocol implementation for the ezc language.
//!
//! Provides diagnostics, hover documentation, completions, go-to-definition,
//! references, document symbols, rename, and semantic tokens.
//! Start with `ezc lsp` or run the `ezc-lsp` binary — communicates over stdio.

pub mod docs;
pub mod symbols;

/// CodeLens command registered by the VS Code extension. Arguments: document URI string,
/// then LSP [`Position`] `{ line, character }`. The extension forwards to
/// `editor.action.findReferences` with proper `vscode.Uri` / `vscode.Position` instances
/// (JSON from the server alone does not satisfy VS Code's `URI.isUri` / `Position.isIPosition`).
pub const EZC_SHOW_REFERENCES_COMMAND: &str = "ezc.showReferences";

/// JSON arguments for [`EZC_SHOW_REFERENCES_COMMAND`], for tests and clients.
pub fn references_code_lens_arguments(uri: &Url, position: Position) -> Vec<serde_json::Value> {
    vec![
        serde_json::Value::String(uri.to_string()),
        serde_json::json!({
            "line": position.line,
            "character": position.character,
        }),
    ]
}

use std::collections::HashMap;
use std::ops::Range as ByteRange;
use std::sync::Arc;

use docs::{completion_items, token_docs};
use ezc::eval::EzIo;
use ezc::lexer;
use ezc::line_index::LineIndex;
use ezc::token::Token;
use symbols::{BuiltinSets, SemanticClass, SymbolIndex};
use tokio::sync::RwLock;
use tower_lsp::jsonrpc::Result;
use tower_lsp::lsp_types::*;
use tower_lsp::{Client, LanguageServer, LspService, Server};

// ── No-op I/O for LSP evaluation ─────────────────────────────────────────────

/// No-op I/O backend for LSP evaluation (no stdin/stdout side effects).
struct NoopIo;

impl EzIo for NoopIo {
    fn read_line(&mut self) -> std::io::Result<String> {
        Ok(String::new())
    }
    fn read_byte(&mut self) -> std::io::Result<u8> {
        Ok(0)
    }
    fn write_str(&mut self, _s: &str) -> std::io::Result<()> {
        Ok(())
    }
    fn write_byte(&mut self, _b: u8) -> std::io::Result<()> {
        Ok(())
    }
}

// ── Stack state computation ─────────────────────────────────────────────────

/// Compute stack state after each line by running the evaluator.
/// Returns (line_number, display_string) pairs.
/// Uses NoopIo and a step limit to prevent blocking.
pub fn compute_stack_states(src: &str, max_display: usize) -> Vec<(u32, String)> {
    let ast = match ezc::lex_and_parse(src) {
        Ok(ast) => ast,
        Err(_) => return vec![],
    };

    let li = ezc::line_index::LineIndex::new(src);
    let mut engine = ezc::eval::Engine::with_io(Box::new(NoopIo));
    engine.set_step_limit(100_000);

    // Load prelude
    if let Ok(prelude_ast) = ezc::lex_and_parse(ezc::PRELUDE) {
        let _ = engine.eval(&prelude_ast);
    }

    // Track stack state per line (last expression per line wins)
    let mut line_states: std::collections::HashMap<u32, String> = std::collections::HashMap::new();

    for spanned in &ast {
        let (_expr, span) = spanned;
        let line = li.line_of(span.start) as u32;

        match engine.eval_one(spanned) {
            Ok(()) => {
                let stack = engine.stack();
                let display = format_stack_display(&stack, max_display);
                line_states.insert(line, format!("→ {display}"));
            }
            Err(e) => {
                line_states.insert(line, format!("⚠ {}", e.kind));
                break; // Stop evaluation after error
            }
        }
    }

    let mut result: Vec<(u32, String)> = line_states.into_iter().collect();
    result.sort_by_key(|(line, _)| *line);
    result
}

/// Format a stack for inline display.
pub fn format_stack_display(stack: &[&ezc::types::Value], max_items: usize) -> String {
    if stack.is_empty() {
        return "[]".to_string();
    }
    let len = stack.len();
    let skip = len.saturating_sub(max_items);
    let mut parts = Vec::new();
    if skip > 0 {
        parts.push(format!("...{skip}"));
    }
    for v in stack.iter().skip(skip) {
        parts.push(v.to_string());
    }
    format!("[{}]", parts.join(" "))
}

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
        let index = SymbolIndex::build(&text, &self.builtins);
        let diagnostics = all_diagnostics(&text, &index, &self.builtins);
        self.documents
            .write()
            .await
            .insert(uri.clone(), (text, index));
        self.client
            .publish_diagnostics(uri, diagnostics, None)
            .await;
    }
}

/// All diagnostics: syntax errors + semantic warnings (undefined variables).
pub fn all_diagnostics(src: &str, index: &SymbolIndex, builtins: &BuiltinSets) -> Vec<Diagnostic> {
    let mut diags = parse_diagnostics(src);
    // Only add semantic diagnostics if parsing succeeded (no syntax errors).
    if diags.is_empty() {
        diags.extend(semantic_diagnostics(src, index, builtins));
    }
    diags
}

/// Report warnings for references to names that have no definition.
pub fn semantic_diagnostics(
    src: &str,
    index: &SymbolIndex,
    builtins: &BuiltinSets,
) -> Vec<Diagnostic> {
    let mut diags = Vec::new();
    for (name, spans) in &index.references {
        // Skip if there's a definition for this name.
        if index.definitions.contains_key(name) {
            continue;
        }
        // Skip builtins.
        if builtins.is_known(name) {
            continue;
        }
        for span in spans {
            let range = Range {
                start: offset_to_position(src, span.start),
                end: offset_to_position(src, span.end),
            };
            diags.push(Diagnostic {
                range,
                severity: Some(DiagnosticSeverity::WARNING),
                source: Some("ezc".into()),
                message: format!("undefined name `{name}`"),
                ..Default::default()
            });
        }
    }
    diags
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
                code_action_provider: Some(CodeActionProviderCapability::Simple(true)),
                selection_range_provider: Some(SelectionRangeProviderCapability::Simple(true)),
                ..Default::default()
            },
            server_info: Some(ServerInfo {
                name: "ezc-lsp".into(),
                version: Some(env!("CARGO_PKG_VERSION").into()),
            }),
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
        let (token, _span) = match index.token_at(offset) {
            Some(pair) => pair,
            None => return Ok(None),
        };

        // Check for operator/builtin docs first.
        if let Some(md) = token_docs(token) {
            return Ok(Some(Hover {
                contents: HoverContents::Markup(MarkupContent {
                    kind: MarkupKind::Markdown,
                    value: md.into(),
                }),
                range: None,
            }));
        }

        // Variable hover: show definition count, reference count, and kind.
        let name = match token {
            Token::Bind(n) | Token::Recall(n) => Some(n.as_str()),
            Token::Ident(n) if !self.builtins.is_known(n) => Some(n.as_str()),
            _ => None,
        };

        if let Some(name) = name {
            let def_count = index.definitions.get(name).map(|v| v.len()).unwrap_or(0);
            let ref_count = index.references.get(name).map(|v| v.len()).unwrap_or(0);
            let is_fn = index
                .definitions
                .get(name)
                .and_then(|spans| spans.first())
                .map(|s| index.is_function_def(s))
                .unwrap_or(false);

            let kind = if is_fn { "block" } else { "value" };
            let def_str = if def_count == 0 {
                "**undefined**".to_string()
            } else {
                format!(
                    "{def_count} definition{}",
                    if def_count == 1 { "" } else { "s" }
                )
            };

            let md = format!(
                "**`{name}`** — {kind}\n\n{def_str}, {ref_count} reference{}",
                if ref_count == 1 { "" } else { "s" }
            );
            return Ok(Some(Hover {
                contents: HoverContents::Markup(MarkupContent {
                    kind: MarkupKind::Markdown,
                    value: md,
                }),
                range: None,
            }));
        }

        Ok(None)
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
            Some(n) => n.to_owned(),
            None => return Ok(None),
        };

        // Search current document first, then all open documents.
        let mut locations: Vec<Location> = Vec::new();

        let def_sites = index.definition_sites(&name);
        if !def_sites.is_empty() {
            let li = LineIndex::new(src);
            for span in def_sites {
                locations.push(Location {
                    uri: uri.clone(),
                    range: span_to_range(&li, span),
                });
            }
        } else {
            // Not found locally — search other open documents.
            for (doc_uri, (doc_src, doc_index)) in docs.iter() {
                if doc_uri == uri {
                    continue;
                }
                let defs = doc_index.definition_sites(&name);
                if !defs.is_empty() {
                    let li = LineIndex::new(doc_src);
                    for span in defs {
                        locations.push(Location {
                            uri: doc_uri.clone(),
                            range: span_to_range(&li, span),
                        });
                    }
                }
            }
        }

        if locations.is_empty() {
            return Ok(None);
        }

        Ok(Some(if locations.len() == 1 {
            GotoDefinitionResponse::Scalar(locations.into_iter().next().expect("len == 1"))
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

        // Search across all open documents.
        let mut locations: Vec<Location> = Vec::new();
        for (doc_uri, (doc_src, doc_index)) in docs.iter() {
            let occurrences = doc_index.all_occurrences(&name);
            if !occurrences.is_empty() {
                let li = LineIndex::new(doc_src);
                for span in &occurrences {
                    locations.push(Location {
                        uri: doc_uri.clone(),
                        range: span_to_range(&li, span),
                    });
                }
            }
        }

        if locations.is_empty() {
            Ok(None)
        } else {
            // Sort: current file first, then by position.
            locations.sort_by(|a, b| {
                let a_current = a.uri == *uri;
                let b_current = b.uri == *uri;
                b_current
                    .cmp(&a_current)
                    .then_with(|| a.uri.cmp(&b.uri))
                    .then_with(|| a.range.start.cmp(&b.range.start))
            });
            Ok(Some(locations))
        }
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

        // Build edits across all open documents. Each token's text includes the
        // sigil (@name, $name, or bare name), so we reconstruct the replacement
        // with the appropriate sigil.
        let mut changes: HashMap<Url, Vec<TextEdit>> = HashMap::new();

        for (doc_uri, (doc_src, doc_index)) in docs.iter() {
            let li = LineIndex::new(doc_src);
            let mut edits: Vec<TextEdit> = Vec::new();

            for (tok, span) in &doc_index.tokens {
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

            if !edits.is_empty() {
                changes.insert(doc_uri.clone(), edits);
            }
        }

        if changes.is_empty() {
            return Ok(None);
        }

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
                        _ => continue,
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
                    // Resolve relative to the document's directory.
                    let target = uri
                        .to_file_path()
                        .ok()
                        .and_then(|file| file.parent().map(|dir| dir.join(path)))
                        .and_then(|resolved| Url::from_file_path(resolved).ok());

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
            // Count references across all open documents.
            let mut ref_count = 0;
            for (_doc_uri, (_doc_src, doc_index)) in docs.iter() {
                ref_count += doc_index.references.get(name).map(|v| v.len()).unwrap_or(0);
            }

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
                        command: EZC_SHOW_REFERENCES_COMMAND.into(),
                        arguments: Some(references_code_lens_arguments(uri, range.start)),
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

        // ── Stack state hints (end of each line) ──────────────────────────
        let stack_states = compute_stack_states(src, 6);
        for (line, display) in &stack_states {
            if *line < params.range.start.line || *line > params.range.end.line {
                continue;
            }
            // Position at end of the line's content
            let line_end = if (*line as usize + 1) < li.line_count() {
                li.line_start(*line as usize + 1).saturating_sub(1)
            } else {
                src.len()
            };
            // Find actual end of content (before trailing whitespace/newline)
            let line_start = li.line_start(*line as usize);
            let line_text = &src[line_start..line_end];
            let trimmed_len = line_text.trim_end().len();

            hints.push(InlayHint {
                position: Position {
                    line: *line,
                    character: trimmed_len as u32,
                },
                label: InlayHintLabel::String(format!("  {display}")),
                kind: None,
                text_edits: None,
                tooltip: Some(InlayHintTooltip::String(
                    "Stack state after this line".into(),
                )),
                padding_left: Some(true),
                padding_right: None,
                data: None,
            });
        }

        // ── Reference count hints (after @name definitions) ───────────────
        for (name, def_spans) in &index.definitions {
            let ref_count = index.references.get(name).map(|v| v.len()).unwrap_or(0);
            let kind_label = if def_spans.iter().any(|s| index.is_function_def(s)) {
                "block"
            } else {
                "value"
            };

            for def_span in def_spans {
                let (line, col) = li.line_col(def_span.end);
                let hint_line = line as u32;
                if hint_line < params.range.start.line || hint_line > params.range.end.line {
                    continue;
                }

                let label = format!(
                    " ← {ref_count} ref{}, {kind_label}",
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

    // ── Code Actions ───────────────────────────────────────────────────────

    async fn code_action(&self, params: CodeActionParams) -> Result<Option<CodeActionResponse>> {
        let uri = &params.text_document.uri;

        let docs = self.documents.read().await;
        let (src, index) = match docs.get(uri) {
            Some(pair) => pair,
            None => return Ok(None),
        };

        let li = LineIndex::new(src);
        let mut actions: Vec<CodeActionOrCommand> = Vec::new();

        // Collect all defined names for fuzzy matching.
        let defined: Vec<&str> = index.definitions.keys().map(|s| s.as_str()).collect();
        let all_known: Vec<&str> = defined
            .iter()
            .copied()
            .chain(self.builtins.all_names().iter().copied())
            .collect();

        // For each diagnostic about an undefined variable, suggest close matches.
        for diag in &params.context.diagnostics {
            // Check if the diagnostic range corresponds to an undefined reference
            let start_offset = position_to_offset(src, diag.range.start);
            if let Some(name) = index.name_at(start_offset) {
                // Only suggest for names that have no definition
                if !index.definitions.contains_key(name) && !self.builtins.is_known(name) {
                    let suggestions = find_close_matches(name, &all_known, 3);
                    for suggestion in suggestions {
                        let title = format!("Did you mean `{suggestion}`?");
                        // Build a text edit replacing the reference with the suggestion
                        let edits = build_rename_edits(src, &li, index, name, suggestion, uri);

                        if !edits.is_empty() {
                            let mut changes = HashMap::new();
                            changes.insert(uri.clone(), edits);

                            actions.push(CodeActionOrCommand::CodeAction(CodeAction {
                                title,
                                kind: Some(CodeActionKind::QUICKFIX),
                                diagnostics: Some(vec![diag.clone()]),
                                edit: Some(WorkspaceEdit {
                                    changes: Some(changes),
                                    ..Default::default()
                                }),
                                is_preferred: Some(false),
                                ..Default::default()
                            }));
                        }
                    }
                }
            }
        }

        if actions.is_empty() {
            Ok(None)
        } else {
            Ok(Some(actions))
        }
    }

    // ── Selection Ranges ───────────────────────────────────────────────────

    async fn selection_range(
        &self,
        params: SelectionRangeParams,
    ) -> Result<Option<Vec<SelectionRange>>> {
        let uri = &params.text_document.uri;

        let docs = self.documents.read().await;
        let (src, index) = match docs.get(uri) {
            Some(pair) => pair,
            None => return Ok(None),
        };

        let li = LineIndex::new(src);
        let mut result: Vec<SelectionRange> = Vec::new();

        for pos in &params.positions {
            let offset = position_to_offset(src, *pos);
            let ranges = compute_selection_ranges(src, &li, &index.tokens, offset);
            result.push(ranges);
        }

        Ok(Some(result))
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

// ── Code action helpers ──────────────────────────────────────────────────────

/// Simple edit distance (Levenshtein) for short strings.
fn edit_distance(a: &str, b: &str) -> usize {
    let a: Vec<char> = a.chars().collect();
    let b: Vec<char> = b.chars().collect();
    let (m, n) = (a.len(), b.len());
    let mut dp = vec![vec![0usize; n + 1]; m + 1];
    #[allow(clippy::needless_range_loop)]
    for i in 0..=m {
        dp[i][0] = i;
    }
    #[allow(clippy::needless_range_loop)]
    for j in 0..=n {
        dp[0][j] = j;
    }
    #[allow(clippy::needless_range_loop)]
    for i in 1..=m {
        for j in 1..=n {
            let cost = if a[i - 1] == b[j - 1] { 0 } else { 1 };
            dp[i][j] = (dp[i - 1][j] + 1)
                .min(dp[i][j - 1] + 1)
                .min(dp[i - 1][j - 1] + cost);
        }
    }
    dp[m][n]
}

/// Find the closest matching names for a given identifier, ranked by edit distance.
/// Returns up to `max` suggestions. Only includes names within distance 3.
pub fn find_close_matches<'a>(name: &str, candidates: &[&'a str], max: usize) -> Vec<&'a str> {
    let mut scored: Vec<(&str, usize)> = candidates
        .iter()
        .filter(|&&c| c != name)
        .map(|&c| (c, edit_distance(name, c)))
        .filter(|&(_, d)| d <= 3)
        .collect();
    scored.sort_by_key(|&(_, d)| d);
    scored.into_iter().take(max).map(|(s, _)| s).collect()
}

/// Build text edits to rename all occurrences of `old_name` to `new_name`.
fn build_rename_edits(
    src: &str,
    li: &LineIndex,
    index: &SymbolIndex,
    old_name: &str,
    new_name: &str,
    _uri: &Url,
) -> Vec<TextEdit> {
    let mut edits = Vec::new();
    for (tok, span) in &index.tokens {
        let replacement = match tok {
            Token::Bind(n) if n == old_name => format!("@{new_name}"),
            Token::Recall(n) if n == old_name => format!("${new_name}"),
            Token::Ident(n) if n == old_name => new_name.to_string(),
            _ => continue,
        };
        edits.push(TextEdit {
            range: span_to_range(li, span),
            new_text: replacement,
        });
    }
    let _ = src; // used for span_to_range via li
    edits
}

// ── Selection range helpers ──────────────────────────────────────────────────

/// Compute nested selection ranges for a byte offset. Returns ranges from
/// innermost (current token) to outermost (entire file), linked via `parent`.
fn compute_selection_ranges(
    src: &str,
    li: &LineIndex,
    tokens: &[(Token, ByteRange<usize>)],
    offset: usize,
) -> SelectionRange {
    let mut ranges: Vec<Range> = Vec::new();

    // 1. Current token
    if let Some((_, span)) = tokens
        .iter()
        .find(|(_, span)| span.start <= offset && offset < span.end)
    {
        ranges.push(span_to_range(li, span));
    }

    // 2. Enclosing blocks — find all bracket pairs containing offset, inner to outer.
    let enclosing = find_enclosing_brackets(tokens, offset);
    for (open_span, close_span) in &enclosing {
        let combined = open_span.start..close_span.end;
        ranges.push(span_to_range(li, &combined));
    }

    // 3. Current line
    let (line, _) = li.line_col(offset);
    let _line_start = li.line_start(line);
    let line_end = if line + 1 < li.line_count() {
        li.line_start(line + 1).saturating_sub(1)
    } else {
        src.len()
    };
    let line_range = Range {
        start: Position {
            line: line as u32,
            character: 0,
        },
        end: offset_to_position(src, line_end),
    };
    ranges.push(line_range);

    // 4. Entire file
    let file_range = Range {
        start: Position {
            line: 0,
            character: 0,
        },
        end: offset_to_position(src, src.len()),
    };
    ranges.push(file_range);

    // Deduplicate consecutive equal ranges.
    ranges.dedup();

    // Build nested SelectionRange from innermost to outermost.
    build_selection_range_chain(&ranges)
}

/// Find all bracket pairs (open_span, close_span) that enclose the given offset,
/// ordered from innermost to outermost.
fn find_enclosing_brackets(
    tokens: &[(Token, ByteRange<usize>)],
    offset: usize,
) -> Vec<(ByteRange<usize>, ByteRange<usize>)> {
    // Build a stack of open brackets, then match closers.
    let mut pairs: Vec<(ByteRange<usize>, ByteRange<usize>)> = Vec::new();
    let mut stack: Vec<(Token, ByteRange<usize>)> = Vec::new();

    for (tok, span) in tokens {
        match tok {
            Token::OpenParen | Token::OpenBracket | Token::OpenBrace => {
                stack.push((tok.clone(), span.clone()));
            }
            Token::CloseParen | Token::CloseBracket | Token::CloseBrace => {
                let expected = match tok {
                    Token::CloseParen => Token::OpenParen,
                    Token::CloseBracket => Token::OpenBracket,
                    Token::CloseBrace => Token::OpenBrace,
                    _ => continue,
                };
                if let Some(pos) = stack.iter().rposition(|(t, _)| *t == expected) {
                    let (_, open_span) = stack.remove(pos);
                    // Does this pair enclose the offset?
                    if open_span.start <= offset && offset <= span.end {
                        pairs.push((open_span, span.clone()));
                    }
                }
            }
            _ => {}
        }
    }

    // Sort innermost first: smallest span first.
    pairs.sort_by_key(|(a, b)| std::cmp::Reverse(a.start + (b.end - a.start)));
    pairs.sort_by_key(|(a, b)| b.end - a.start);
    pairs
}

/// Chain a list of ranges into a nested `SelectionRange` (innermost first).
fn build_selection_range_chain(ranges: &[Range]) -> SelectionRange {
    // Build from outermost (last) to innermost (first).
    let mut current: Option<SelectionRange> = None;
    for range in ranges.iter().rev() {
        current = Some(SelectionRange {
            range: *range,
            parent: current.map(Box::new),
        });
    }
    current.unwrap_or(SelectionRange {
        range: Range::default(),
        parent: None,
    })
}

// ── Extractable formatting/folding helpers ───────────────────────────────────

/// Format ezc source code with normalized whitespace. Testable standalone function.
pub fn format_source(src: &str) -> String {
    let tokens: Vec<(Token, ByteRange<usize>)> = match lexer::lex(src) {
        Ok(toks) => toks.into_iter().map(|(t, s)| (t, s.into_range())).collect(),
        Err(_) => return src.to_string(), // can't format broken source
    };

    let li = LineIndex::new(src);
    let mut formatted_lines: Vec<String> = Vec::new();
    let line_count = li.line_count();

    for line_num in 0..line_count {
        let line_start = li.line_start(line_num);
        let line_end = if line_num + 1 < line_count {
            li.line_start(line_num + 1) - 1
        } else {
            src.len()
        };

        let raw_line = &src[line_start..line_end];

        let comment_start = find_comment_start(raw_line, line_start, &tokens);
        let (code_part, comment_part) = match comment_start {
            Some(offset) => {
                let rel = offset - line_start;
                (&raw_line[..rel], Some(raw_line[rel..].trim_end()))
            }
            None => (raw_line, None),
        };

        let line_tokens: Vec<&str> = tokens
            .iter()
            .filter(|(_, span)| {
                let (tok_line, _) = li.line_col(span.start);
                tok_line == line_num && span.start < line_start + code_part.len()
            })
            .map(|(_, span)| &src[span.start..span.end])
            .collect();

        let mut line_out = if line_tokens.is_empty() {
            String::new()
        } else {
            line_tokens.join(" ")
        };

        if let Some(comment) = comment_part {
            if !line_out.is_empty() {
                line_out.push(' ');
            }
            line_out.push_str(comment);
        }

        let trimmed = line_out.trim_end().to_string();
        formatted_lines.push(trimmed);
    }

    while formatted_lines
        .last()
        .map(|l| l.is_empty())
        .unwrap_or(false)
    {
        formatted_lines.pop();
    }

    let mut formatted = formatted_lines.join("\n");
    formatted.push('\n');
    formatted
}

/// Compute folding ranges from source code. Testable standalone function.
pub fn compute_folding_ranges(src: &str) -> Vec<FoldingRange> {
    let tokens: Vec<(Token, ByteRange<usize>)> = match lexer::lex(src) {
        Ok(toks) => toks.into_iter().map(|(t, s)| (t, s.into_range())).collect(),
        Err(_) => return vec![],
    };

    let li = LineIndex::new(src);
    let mut ranges: Vec<FoldingRange> = Vec::new();

    // Delimiter-based folding
    let mut stack: Vec<(u32, &Token)> = Vec::new();
    for (tok, span) in &tokens {
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
                    _ => continue,
                };
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

    // Comment-block folding
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

    ranges
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

#[cfg(test)]
mod reference_code_lens_tests {
    use super::{references_code_lens_arguments, EZC_SHOW_REFERENCES_COMMAND};
    use tower_lsp::lsp_types::{Position, Url};

    /// Regression: VS Code `editor.action.findReferences` rejects JSON args from the server
    /// (string URI + LSP `Position` are not `URI.isUri` / `Position.isIPosition`). The extension
    /// command `ezc.showReferences` must receive this stable shape.
    #[test]
    fn references_code_lens_arguments_match_extension_contract() {
        let uri: Url = "file:///tmp/x.ezc".parse().unwrap();
        let pos = Position {
            line: 2,
            character: 5,
        };
        let args = references_code_lens_arguments(&uri, pos);
        assert_eq!(args.len(), 2);
        assert_eq!(
            args[0],
            serde_json::Value::String("file:///tmp/x.ezc".into())
        );
        assert_eq!(args[1], serde_json::json!({ "line": 2, "character": 5 }));
        assert_eq!(EZC_SHOW_REFERENCES_COMMAND, "ezc.showReferences");
    }
}
