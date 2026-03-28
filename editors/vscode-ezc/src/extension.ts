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
  let serverPath: string = config.get("server.path") ?? "ezc";

  // In development, try to find the debug build relative to the extension.
  if (serverPath === "ezc") {
    const fs = require("fs");
    const devBuild = path.resolve(context.extensionPath, "../../target/debug/ezc");
    if (fs.existsSync(devBuild)) {
      serverPath = devBuild;
      console.log(`[ezc] Using dev build: ${devBuild}`);
    }
  }

  // ── LSP client ───────────────────────────────────────────────────────

  const serverOptions: ServerOptions = {
    command: serverPath,
    args: ["lsp"],
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

  const adapterFactory = new EzcDebugAdapterDescriptorFactory(serverPath);
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
  constructor(private readonly serverPath: string) {}

  createDebugAdapterDescriptor(
    _session: vscode.DebugSession,
  ): vscode.DebugAdapterDescriptor {
    // Re-read the config each time in case the user changed it
    const configuredPath =
      vscode.workspace.getConfiguration("ezc").get<string>("server.path") ??
      this.serverPath;
    return new vscode.DebugAdapterExecutable(configuredPath, ["dap"]);
  }
}
