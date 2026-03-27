/// Debug Adapter Protocol (DAP) message types.
///
/// The full DAP specification is at
/// <https://microsoft.github.io/debug-adapter-protocol/specification>.
///
/// We represent the wire protocol using serde structs. Inbound messages are
/// deserialized from JSON; outbound messages are serialized to JSON.
use serde::{Deserialize, Serialize};
use serde_json::Value;

// ── Inbound (client → server) ─────────────────────────────────────────────

/// A raw DAP request, before the command-specific arguments are parsed.
#[derive(Debug, Deserialize)]
pub struct RawRequest {
    pub seq: i64,
    pub command: String,
    pub arguments: Option<Value>,
}

/// Helper: deserialize typed arguments from a raw request.
pub fn parse_args<T: for<'de> Deserialize<'de>>(req: &RawRequest) -> Option<T> {
    req.arguments
        .as_ref()
        .and_then(|v| serde_json::from_value(v.clone()).ok())
}

// ── Arguments structs ─────────────────────────────────────────────────────

#[derive(Debug, Deserialize, Default)]
pub struct InitializeArgs {
    #[serde(rename = "clientID")]
    pub client_id: Option<String>,
    #[serde(rename = "clientName")]
    pub client_name: Option<String>,
    #[serde(rename = "adapterID", default)]
    pub adapter_id: String,
    pub locale: Option<String>,
    #[serde(rename = "linesStartAt1")]
    pub lines_start_at_1: Option<bool>,
    #[serde(rename = "columnsStartAt1")]
    pub columns_start_at_1: Option<bool>,
    #[serde(rename = "supportsVariableType")]
    pub supports_variable_type: Option<bool>,
    #[serde(rename = "supportsRunInTerminalRequest")]
    pub supports_run_in_terminal_request: Option<bool>,
    #[serde(rename = "supportsInvalidatedEvent")]
    pub supports_invalidated_event: Option<bool>,
}

#[derive(Debug, Deserialize)]
pub struct LaunchArgs {
    /// Absolute path to the `.ezc` source file.
    pub program: String,
    /// If `true`, halt before executing the first instruction.
    #[serde(rename = "stopOnEntry", default)]
    pub stop_on_entry: bool,
    /// If `true`, run without the debugger (no breakpoints, no stops).
    #[serde(rename = "noDebug", default)]
    pub no_debug: bool,
}

#[derive(Debug, Deserialize)]
pub struct SetBreakpointsArgs {
    pub source: SourceArg,
    pub breakpoints: Option<Vec<SourceBreakpoint>>,
}

#[derive(Debug, Deserialize)]
pub struct SourceArg {
    pub path: Option<String>,
    pub name: Option<String>,
    #[serde(rename = "sourceReference")]
    pub source_reference: Option<i64>,
}

#[derive(Debug, Deserialize)]
pub struct SourceBreakpoint {
    pub line: usize,
    pub column: Option<usize>,
    pub condition: Option<String>,
    #[serde(rename = "hitCondition")]
    pub hit_condition: Option<String>,
    #[serde(rename = "logMessage")]
    pub log_message: Option<String>,
}

#[derive(Debug, Deserialize)]
pub struct SetExceptionBreakpointsArgs {
    pub filters: Vec<String>,
}

#[derive(Debug, Deserialize)]
pub struct StackTraceArgs {
    #[serde(rename = "threadId")]
    pub thread_id: i64,
    #[serde(rename = "startFrame")]
    pub start_frame: Option<usize>,
    pub levels: Option<usize>,
}

#[derive(Debug, Deserialize)]
pub struct ScopesArgs {
    #[serde(rename = "frameId")]
    pub frame_id: usize,
}

#[derive(Debug, Deserialize)]
pub struct VariablesArgs {
    #[serde(rename = "variablesReference")]
    pub variables_reference: usize,
    pub filter: Option<String>,
    pub start: Option<usize>,
    pub count: Option<usize>,
    pub format: Option<Value>,
}

#[derive(Debug, Deserialize)]
pub struct EvaluateArgs {
    pub expression: String,
    #[serde(rename = "frameId")]
    pub frame_id: Option<usize>,
    pub context: Option<String>,
    pub format: Option<Value>,
}

#[derive(Debug, Deserialize)]
pub struct ContinueArgs {
    #[serde(rename = "threadId")]
    pub thread_id: i64,
    #[serde(rename = "singleThread", default)]
    pub single_thread: bool,
}

#[derive(Debug, Deserialize)]
pub struct NextArgs {
    #[serde(rename = "threadId")]
    pub thread_id: i64,
    pub granularity: Option<String>,
}

#[derive(Debug, Deserialize)]
pub struct StepInArgs {
    #[serde(rename = "threadId")]
    pub thread_id: i64,
    #[serde(rename = "targetId")]
    pub target_id: Option<i64>,
    pub granularity: Option<String>,
}

#[derive(Debug, Deserialize)]
pub struct StepOutArgs {
    #[serde(rename = "threadId")]
    pub thread_id: i64,
    pub granularity: Option<String>,
}

#[derive(Debug, Deserialize)]
pub struct PauseArgs {
    #[serde(rename = "threadId")]
    pub thread_id: i64,
}

#[derive(Debug, Deserialize)]
pub struct DisconnectArgs {
    #[serde(rename = "restart", default)]
    pub restart: bool,
    #[serde(rename = "terminateDebuggee")]
    pub terminate_debuggee: Option<bool>,
}

#[derive(Debug, Deserialize)]
pub struct TerminateArgs {
    #[serde(rename = "restart", default)]
    pub restart: bool,
}

// ── Outbound response/event helpers ──────────────────────────────────────

/// Build a success response body.
pub fn success_response(seq: &mut i64, request_seq: i64, command: &str, body: Value) -> Value {
    *seq += 1;
    serde_json::json!({
        "seq": *seq,
        "type": "response",
        "request_seq": request_seq,
        "success": true,
        "command": command,
        "body": body,
    })
}

/// Build an error response body.
pub fn error_response(seq: &mut i64, request_seq: i64, command: &str, message: &str) -> Value {
    *seq += 1;
    serde_json::json!({
        "seq": *seq,
        "type": "response",
        "request_seq": request_seq,
        "success": false,
        "command": command,
        "message": message,
    })
}

/// Build an event body.
pub fn event(seq: &mut i64, event: &str, body: Value) -> Value {
    *seq += 1;
    serde_json::json!({
        "seq": *seq,
        "type": "event",
        "event": event,
        "body": body,
    })
}

// ── Capabilities ──────────────────────────────────────────────────────────

/// Serialize the server's capability declaration (sent in the `initialize`
/// response body).
pub fn capabilities() -> Value {
    serde_json::json!({
        "supportsConfigurationDoneRequest": true,
        "supportsConditionalBreakpoints": true,
        "supportsLogPoints": true,
        "supportsEvaluateForHovers": true,
        "supportsTerminateRequest": true,
        "supportsRestartRequest": false,
        "supportsStepBack": false,
        "supportsSetVariable": false,
        "supportsSingleThreadExecutionRequests": false,
        "exceptionBreakpointFilters": [
            {
                "filter": "all",
                "label": "All Exceptions",
                "description": "Break on any EZC runtime error (type mismatch, stack underflow, etc.)",
                "default": false,
            }
        ],
    })
}

// ── Variable reference scheme ─────────────────────────────────────────────
//
// We use a flat store (Vec<Vec<DapVariable>>) that is rebuilt on each pause:
//   index 0: unused (DAP reserves 0 for "no children")
//   index 1: "Variables" scope — all named bindings
//   index 2: "Stack" scope — the value stack
//   index 3+: dynamically allocated for nested list/block children

/// A DAP `Variable` object.
#[derive(Debug, Clone, Serialize)]
pub struct DapVariable {
    pub name: String,
    pub value: String,
    #[serde(rename = "type")]
    pub var_type: String,
    #[serde(rename = "variablesReference")]
    pub variables_reference: i64,
    #[serde(rename = "namedVariables", skip_serializing_if = "Option::is_none")]
    pub named_variables: Option<i64>,
    #[serde(rename = "indexedVariables", skip_serializing_if = "Option::is_none")]
    pub indexed_variables: Option<i64>,
    #[serde(rename = "presentationHint", skip_serializing_if = "Option::is_none")]
    pub presentation_hint: Option<DapVariableHint>,
}

#[derive(Debug, Clone, Serialize)]
pub struct DapVariableHint {
    #[serde(skip_serializing_if = "Option::is_none")]
    pub kind: Option<String>,
    #[serde(skip_serializing_if = "Option::is_none")]
    pub attributes: Option<Vec<String>>,
    #[serde(skip_serializing_if = "Option::is_none")]
    pub visibility: Option<String>,
}

/// A DAP `StackFrame` object.
#[derive(Debug, Clone, Serialize)]
pub struct DapStackFrame {
    pub id: i64,
    pub name: String,
    #[serde(skip_serializing_if = "Option::is_none")]
    pub source: Option<DapSource>,
    pub line: i64,
    pub column: i64,
    #[serde(rename = "endLine", skip_serializing_if = "Option::is_none")]
    pub end_line: Option<i64>,
    #[serde(rename = "endColumn", skip_serializing_if = "Option::is_none")]
    pub end_column: Option<i64>,
    #[serde(rename = "presentationHint", skip_serializing_if = "Option::is_none")]
    pub presentation_hint: Option<String>,
}

/// A DAP `Source` object.
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct DapSource {
    #[serde(skip_serializing_if = "Option::is_none")]
    pub name: Option<String>,
    #[serde(skip_serializing_if = "Option::is_none")]
    pub path: Option<String>,
    #[serde(rename = "sourceReference", skip_serializing_if = "Option::is_none")]
    pub source_reference: Option<i64>,
}

/// A DAP `Scope` object.
#[derive(Debug, Clone, Serialize)]
pub struct DapScope {
    pub name: String,
    #[serde(rename = "presentationHint", skip_serializing_if = "Option::is_none")]
    pub presentation_hint: Option<String>,
    #[serde(rename = "variablesReference")]
    pub variables_reference: i64,
    #[serde(rename = "namedVariables", skip_serializing_if = "Option::is_none")]
    pub named_variables: Option<i64>,
    #[serde(rename = "indexedVariables", skip_serializing_if = "Option::is_none")]
    pub indexed_variables: Option<i64>,
    pub expensive: bool,
    #[serde(skip_serializing_if = "Option::is_none")]
    pub source: Option<DapSource>,
    #[serde(skip_serializing_if = "Option::is_none")]
    pub line: Option<i64>,
    #[serde(skip_serializing_if = "Option::is_none")]
    pub column: Option<i64>,
    #[serde(rename = "endLine", skip_serializing_if = "Option::is_none")]
    pub end_line: Option<i64>,
    #[serde(rename = "endColumn", skip_serializing_if = "Option::is_none")]
    pub end_column: Option<i64>,
}

/// A DAP `Breakpoint` object (sent in `setBreakpoints` response).
#[derive(Debug, Serialize)]
pub struct DapBreakpointResult {
    pub verified: bool,
    #[serde(skip_serializing_if = "Option::is_none")]
    pub message: Option<String>,
    #[serde(skip_serializing_if = "Option::is_none")]
    pub line: Option<i64>,
    #[serde(skip_serializing_if = "Option::is_none")]
    pub source: Option<DapSource>,
}

/// A DAP `Thread` object.
#[derive(Debug, Serialize)]
pub struct DapThread {
    pub id: i64,
    pub name: String,
}
