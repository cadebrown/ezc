import * as vscode from "vscode";

/** Main EZC log — View → Output → pick "EZC". */
export const ezcLog = vscode.window.createOutputChannel("EZC");

/** LSP wire trace when `ezc.trace.server` is messages/verbose. */
export const ezcLspTrace = vscode.window.createOutputChannel("EZC LSP");

export function logAlways(message: string): void {
  const line = `[${timestamp()}] ${message}`;
  ezcLog.appendLine(line);
}

export function logDapVerbose(message: string): void {
  ezcLog.appendLine(`[DAP ${timestamp()}] ${message}`);
}

function timestamp(): string {
  return new Date().toISOString();
}
