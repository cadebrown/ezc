import * as path from "path";
import * as vscode from "vscode";
import {
  LanguageClient,
  LanguageClientOptions,
  ServerOptions,
  TransportKind,
} from "vscode-languageclient/node";

let client: LanguageClient | undefined;

// ── Activation ───────────────────────────────────────────────────────────

export function activate(context: vscode.ExtensionContext) {
  const config = vscode.workspace.getConfiguration("ezc");
  const fs = require("fs");
  const devDir = path.resolve(context.extensionPath, "../../target/debug");

  // Resolve LSP binary: prefer standalone ezc-lsp, fall back to ezc lsp
  let lspPath: string = config.get("lsp.path") ?? "";
  let lspArgs: string[] = [];
  if (!lspPath) {
    const devLsp = path.join(devDir, "ezc-lsp");
    if (fs.existsSync(devLsp)) {
      lspPath = devLsp;
      console.log(`[ezc] Using dev LSP: ${devLsp}`);
    } else {
      // Try standalone ezc-lsp on PATH, fall back to ezc lsp subcommand
      lspPath = "ezc-lsp";
      // If ezc-lsp isn't installed, this will fail and we could try "ezc" + ["lsp"]
    }
  }

  // Resolve DAP binary: prefer standalone ezc-dap, fall back to ezc dap
  let dapPath: string = config.get("dap.path") ?? "";
  let dapArgs: string[] = [];
  if (!dapPath) {
    const devDap = path.join(devDir, "ezc-dap");
    if (fs.existsSync(devDap)) {
      dapPath = devDap;
      console.log(`[ezc] Using dev DAP: ${devDap}`);
    } else {
      dapPath = "ezc-dap";
    }
  }

  // ── LSP client ───────────────────────────────────────────────────────

  const serverOptions: ServerOptions = {
    command: lspPath,
    args: lspArgs,
    transport: TransportKind.stdio,
  };

  const clientOptions: LanguageClientOptions = {
    documentSelector: [{ scheme: "file", language: "ezc" }],
    synchronize: {
      fileEvents: vscode.workspace.createFileSystemWatcher("**/*.ezc"),
    },
  };

  client = new LanguageClient(
    "ezc-lsp",
    "EZC Language Server",
    serverOptions,
    clientOptions,
  );

  client.start();
  context.subscriptions.push(client);

  // ── DAP: debug configuration provider ────────────────────────────────

  const configProvider = new EzcDebugConfigurationProvider();
  context.subscriptions.push(
    vscode.debug.registerDebugConfigurationProvider("ezc", configProvider),
  );

  // ── DAP: debug adapter descriptor factory ────────────────────────────

  const adapterFactory = new EzcDebugAdapterDescriptorFactory(dapPath);
  context.subscriptions.push(
    vscode.debug.registerDebugAdapterDescriptorFactory("ezc", adapterFactory),
  );

  // ── Commands ─────────────────────────────────────────────────────────

  // "Debug Current File" command — launches a debug session for the active editor
  context.subscriptions.push(
    vscode.commands.registerCommand("ezc.debugFile", async () => {
      const editor = vscode.window.activeTextEditor;
      if (!editor) {
        vscode.window.showWarningMessage("No active editor");
        return;
      }
      const filePath = editor.document.uri.fsPath;
      if (!filePath.endsWith(".ezc")) {
        vscode.window.showWarningMessage("Active file is not an EZC file");
        return;
      }
      await vscode.debug.startDebugging(undefined, {
        type: "ezc",
        request: "launch",
        name: `Debug ${path.basename(filePath)}`,
        program: filePath,
        stopOnEntry: true,
      });
    }),
  );
}

export function deactivate(): Thenable<void> | undefined {
  return client?.stop();
}

// ── DAP classes ──────────────────────────────────────────────────────────

/**
 * Resolves and augments launch configurations for EZC debug sessions.
 *
 * When the user presses F5 on a `.ezc` file without an explicit
 * `launch.json`, this provider fills in sensible defaults so the session
 * starts immediately.
 */
class EzcDebugConfigurationProvider
  implements vscode.DebugConfigurationProvider
{
  /** Called when VS Code needs a list of initial configurations (for launch.json). */
  provideDebugConfigurations(
    _folder: vscode.WorkspaceFolder | undefined,
  ): vscode.DebugConfiguration[] {
    return [
      {
        type: "ezc",
        request: "launch",
        name: "Debug Current File",
        program: "${file}",
        stopOnEntry: true,
      },
      {
        type: "ezc",
        request: "launch",
        name: "Run Current File (no debug)",
        program: "${file}",
        stopOnEntry: false,
        noDebug: true,
      },
    ];
  }

  /**
   * Called every time a debug session is about to start.
   * Fills in `program` from the active editor if it's missing.
   */
  resolveDebugConfiguration(
    _folder: vscode.WorkspaceFolder | undefined,
    config: vscode.DebugConfiguration,
  ): vscode.ProviderResult<vscode.DebugConfiguration> {
    // If launched without a config (F5 on a .ezc file), fill in defaults
    if (!config.type && !config.request && !config.name) {
      const editor = vscode.window.activeTextEditor;
      if (editor && editor.document.languageId === "ezc") {
        config.type = "ezc";
        config.name = "Debug Current File";
        config.request = "launch";
        config.program = editor.document.uri.fsPath;
        config.stopOnEntry = true;
      }
    }

    if (!config.program) {
      const editor = vscode.window.activeTextEditor;
      if (editor && editor.document.languageId === "ezc") {
        config.program = editor.document.uri.fsPath;
      }
    }

    if (!config.program) {
      return vscode.window
        .showInformationMessage("Cannot find a program to debug")
        .then(() => undefined);
    }

    return config;
  }
}

/**
 * Tells VS Code how to launch the EZC debug adapter process.
 *
 * We use `DebugAdapterExecutable` (stdio transport), which spawns
 * `ezc debug` as a subprocess and communicates over its stdin/stdout.
 * This is the same pattern as `ezc lsp` for the language server.
 */
class EzcDebugAdapterDescriptorFactory
  implements vscode.DebugAdapterDescriptorFactory
{
  constructor(private readonly dapPath: string) {}

  createDebugAdapterDescriptor(
    _session: vscode.DebugSession,
  ): vscode.DebugAdapterDescriptor {
    const configuredPath =
      vscode.workspace.getConfiguration("ezc").get<string>("dap.path") ??
      this.dapPath;
    // Standalone binary — no subcommand args needed
    return new vscode.DebugAdapterExecutable(configuredPath, []);
  }
}
