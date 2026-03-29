import * as path from "path";
import * as vscode from "vscode";
import {
  LanguageClient,
  LanguageClientOptions,
  ServerOptions,
  TransportKind,
  Trace,
} from "vscode-languageclient/node";
import { ezcLog, ezcLspTrace, logAlways, logDapVerbose } from "./ezcOutput";
import { resolveEzcTool } from "./ezcPaths";

let client: LanguageClient | undefined;

function ezcConfig(): vscode.WorkspaceConfiguration {
  return vscode.workspace.getConfiguration();
}

function dapTraceEnabled(): boolean {
  return ezcConfig().get<boolean>("ezc.trace.dap") ?? true;
}

function syncLspTrace(): void {
  if (!client) {
    return;
  }
  const level = ezcConfig().get<string>("ezc.trace.server") ?? "off";
  if (level === "verbose") {
    client.setTrace(Trace.Verbose);
  } else if (level === "messages") {
    client.setTrace(Trace.Messages);
  } else {
    client.setTrace(Trace.Off);
  }
}

// ── Activation ───────────────────────────────────────────────────────────

export function activate(context: vscode.ExtensionContext): void {
  const lspPathCfg = ezcConfig().get<string>("ezc.lsp.path") ?? "";
  const lspResolved = resolveEzcTool(context, {
    configuredPath: lspPathCfg,
    debugBinaryName: "ezc-lsp",
    cliSubcommand: "lsp",
  });
  logAlways(
    `Language server (${lspResolved.via}): ${quoteCmd(lspResolved.command, lspResolved.args)}`,
  );

  const serverOptions: ServerOptions = {
    command: lspResolved.command,
    args: lspResolved.args,
    transport: TransportKind.stdio,
  };

  const clientOptions: LanguageClientOptions = {
    documentSelector: [{ scheme: "file", language: "ezc" }],
    synchronize: {
      fileEvents: vscode.workspace.createFileSystemWatcher("**/*.ezc"),
    },
    outputChannel: ezcLog,
    traceOutputChannel: ezcLspTrace,
  };

  client = new LanguageClient(
    "ezc-lsp",
    "EZC Language Server",
    serverOptions,
    clientOptions,
  );

  syncLspTrace();
  void client.start().then(
    () => logAlways("Language server started"),
    (err: unknown) => {
      logAlways(`Language server failed to start: ${String(err)}`);
      ezcLog.show(true);
    },
  );
  context.subscriptions.push(client);

  context.subscriptions.push(
    vscode.workspace.onDidChangeConfiguration((e) => {
      if (e.affectsConfiguration("ezc.trace.server")) {
        syncLspTrace();
      }
    }),
  );

  // CodeLens → find references (JSON args are not valid for built-in command).
  context.subscriptions.push(
    vscode.commands.registerCommand(
      "ezc.showReferences",
      async (uriArg: unknown, posArg: unknown) => {
        try {
          const uriStr = typeof uriArg === "string" ? uriArg : String(uriArg);
          const uri = vscode.Uri.parse(uriStr);

          // Ensure the document is open (findReferences needs it).
          const doc = await vscode.workspace.openTextDocument(uri);
          await vscode.window.showTextDocument(doc, { preserveFocus: true });

          let line = 0;
          let character = 0;
          if (posArg && typeof posArg === "object") {
            const p = posArg as Record<string, unknown>;
            if (typeof p.line === "number") line = p.line;
            if (typeof p.character === "number") character = p.character;
          }
          const position = new vscode.Position(line, character);

          await vscode.commands.executeCommand(
            "vscode.executeReferenceProvider",
            uri,
            position,
          ).then((locations) => {
            // Show peek view with results
            void vscode.commands.executeCommand(
              "editor.action.showReferences",
              uri,
              position,
              locations ?? [],
            );
          });
        } catch (err) {
          logAlways(`showReferences error: ${String(err)}`);
        }
      },
    ),
  );

  // ── DAP ────────────────────────────────────────────────────────────────

  context.subscriptions.push(
    vscode.debug.registerDebugAdapterTrackerFactory("ezc", {
      createDebugAdapterTracker(
        session: vscode.DebugSession,
      ): vscode.DebugAdapterTracker {
        return {
          onWillStartSession(): void {
            logAlways(`Debug session starting: ${session.name} (${session.id})`);
          },
          onWillReceiveMessage(message: unknown): void {
            if (dapTraceEnabled()) {
              logDapVerbose(`client → DA: ${JSON.stringify(message)}`);
            }
          },
          onDidSendMessage(message: unknown): void {
            if (dapTraceEnabled()) {
              logDapVerbose(`DA → client: ${JSON.stringify(message)}`);
            }
          },
          onWillStopSession(): void {
            logAlways(`Debug session stopping: ${session.name}`);
          },
          onError(error: Error): void {
            // VS Code often reports "read error" when stdin closes after disconnect — not a bug.
            if (isBenignDebugAdapterIoError(error.message)) {
              if (dapTraceEnabled()) {
                logDapVerbose(`adapter I/O closed: ${error.message}`);
              }
              return;
            }
            logAlways(`Debug adapter error: ${error.message}`);
            ezcLog.show(true);
          },
          onExit(code: number | undefined, signal: string | undefined): void {
            logAlways(
              `Debug adapter process exited (code=${code ?? "null"}, signal=${signal ?? ""})`,
            );
          },
        };
      },
    }),
  );

  const configProvider = new EzcDebugConfigurationProvider();
  context.subscriptions.push(
    vscode.debug.registerDebugConfigurationProvider("ezc", configProvider),
  );

  const adapterFactory = new EzcDebugAdapterDescriptorFactory(context);
  context.subscriptions.push(
    vscode.debug.registerDebugAdapterDescriptorFactory("ezc", adapterFactory),
  );

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

  context.subscriptions.push(
    vscode.commands.registerCommand("ezc.showOutput", () => {
      ezcLog.show(true);
    }),
  );
}

export function deactivate(): Thenable<void> | undefined {
  return client?.stop();
}

/** Stdio adapter closed after stop/disconnect — VS Code still surfaces this as an error. */
function isBenignDebugAdapterIoError(message: string): boolean {
  const m = message.toLowerCase();
  return (
    m.includes("read error") ||
    m.includes("econnreset") ||
    m.includes("broken pipe") ||
    m.includes("socket hang up") ||
    (m.includes("closed") && m.includes("stream"))
  );
}

function quoteCmd(command: string, args: string[]): string {
  if (args.length === 0) {
    return command;
  }
  return `${command} ${args.map((a) => (/\s/.test(a) ? JSON.stringify(a) : a)).join(" ")}`;
}

// ── DAP classes ──────────────────────────────────────────────────────────

class EzcDebugConfigurationProvider
  implements vscode.DebugConfigurationProvider
{
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

  resolveDebugConfiguration(
    _folder: vscode.WorkspaceFolder | undefined,
    config: vscode.DebugConfiguration,
  ): vscode.ProviderResult<vscode.DebugConfiguration> {
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

    logAlways(`Resolved debug configuration program: ${config.program}`);
    return config;
  }
}

class EzcDebugAdapterDescriptorFactory
  implements vscode.DebugAdapterDescriptorFactory
{
  constructor(private readonly context: vscode.ExtensionContext) {}

  createDebugAdapterDescriptor(
    _session: vscode.DebugSession,
  ): vscode.DebugAdapterDescriptor {
    const dapPathCfg = ezcConfig().get<string>("ezc.dap.path") ?? "";
    const resolved = resolveEzcTool(this.context, {
      configuredPath: dapPathCfg,
      debugBinaryName: "ezc-dap",
      cliSubcommand: "dap",
    });
    logAlways(
      `Spawning debug adapter (${resolved.via}): ${quoteCmd(resolved.command, resolved.args)}`,
    );

    const trace = dapTraceEnabled();
    const env: { [key: string]: string } = {};
    for (const [k, v] of Object.entries(process.env)) {
      if (v !== undefined) {
        env[k] = v;
      }
    }
    if (trace) {
      env.RUST_LOG ??= "ezc_dap=debug,ezc=info";
    }

    const cwd =
      vscode.workspace.workspaceFolders?.[0]?.uri.fsPath ?? process.cwd();

    return new vscode.DebugAdapterExecutable(resolved.command, resolved.args, {
      env,
      cwd,
    });
  }
}
