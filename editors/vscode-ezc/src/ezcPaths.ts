/**
 * Resolve ezc-lsp / ezc-dap executables. Installed extensions live under
 * ~/.vscode/extensions/... — `../../target/debug` from there is wrong, so we
 * also check workspace `target/debug` and fall back to `ezc lsp` / `ezc dap`.
 */

import * as fs from "fs";
import * as path from "path";
import * as vscode from "vscode";

export interface ResolvedExecutable {
  command: string;
  args: string[];
  /** How this resolution was chosen (for logging). */
  via: string;
}

export function resolveEzcTool(
  context: vscode.ExtensionContext,
  options: {
    /** `ezc.lsp.path` or `ezc.dap.path` from settings. */
    configuredPath: string;
    /** File name in target/debug, e.g. `ezc-lsp`. */
    debugBinaryName: string;
    /** Subcommand for `ezc` CLI, e.g. `lsp` or `dap`. */
    cliSubcommand: string;
  },
): ResolvedExecutable {
  const trimmed = options.configuredPath.trim();
  if (trimmed) {
    return { command: trimmed, args: [], via: "ezc.*.path setting" };
  }

  const devDir = path.resolve(context.extensionPath, "../../target/debug");
  const devBinary = path.join(devDir, options.debugBinaryName);
  if (fs.existsSync(devBinary)) {
    return {
      command: devBinary,
      args: [],
      via: `monorepo target (${devBinary})`,
    };
  }

  for (const folder of vscode.workspace.workspaceFolders ?? []) {
    const p = path.join(folder.uri.fsPath, "target/debug", options.debugBinaryName);
    if (fs.existsSync(p)) {
      return { command: p, args: [], via: `workspace target (${p})` };
    }
  }

  // `cargo install` of ezc-cli provides `ezc`, not always a standalone `ezc-dap` on PATH.
  return {
    command: "ezc",
    args: [options.cliSubcommand],
    via: `PATH: ezc ${options.cliSubcommand}`,
  };
}
