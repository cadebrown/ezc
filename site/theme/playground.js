// ── ezc WASM playground integration ─────────────────────────────────────
//
// Loads the ezc-web WASM module on demand and wires up:
//   1. Inline Run buttons on every <pre><code class="language-ezc"> block
//   2. The full-page playground (#ezc-full-playground)
//
// The WASM bundle is in /pkg/ relative to the site root.

(function () {
  let wasmReady = null;

  // mdbook serves the book at the root of the site, so /pkg/ is correct
  // for both local serve and GitHub Pages (when deployed at the root path).
  function pkgUrl(file) {
    // Find the site root by counting how deep we are.
    const path = window.location.pathname;
    const depth = (path.match(/\//g) || []).length - 1;
    const prefix = depth > 0 ? "../".repeat(depth) : "./";
    return prefix + "pkg/" + file;
  }

  async function loadWasm() {
    if (wasmReady) return wasmReady;
    wasmReady = (async () => {
      const mod = await import(pkgUrl("ezc_web.js"));
      await mod.default({ module_or_path: pkgUrl("ezc_web_bg.wasm") });
      return mod;
    })();
    // If init fails, clear wasmReady so a retry can re-attempt instead of
    // permanently sticking on a rejected promise.
    wasmReady.catch(() => { wasmReady = null; });
    return wasmReady;
  }

  function renderStack(stack) {
    if (!stack || stack.length === 0) {
      return '<span class="ezc-stack-empty">(stack empty)</span>';
    }
    return stack
      .map((v) => {
        const type = inferType(v);
        return `<span class="ezc-stack-item">${escapeHtml(v)}<span class="ezc-type">${type}</span></span>`;
      })
      .join("");
  }

  function inferType(v) {
    if (/^-?\d+$/.test(v)) return "int";
    if (/^-?\d*\.\d+/.test(v)) return "float";
    if (v.startsWith('"') && v.endsWith('"')) return "str";
    if (v.startsWith("[") && v.endsWith("]")) return "list";
    if (v.startsWith("(") && v.endsWith(")")) return "block";
    if (/^-?\d+(u|i)\d+$/.test(v)) return "num";
    if (/^-?[\d.]+f\d+$/.test(v)) return "float";
    return "value";
  }

  function escapeHtml(s) {
    return String(s)
      .replace(/&/g, "&amp;")
      .replace(/</g, "&lt;")
      .replace(/>/g, "&gt;")
      .replace(/"/g, "&quot;");
  }

  // ── Inline Run buttons ────────────────────────────────────────────────
  function setupInlineRunners() {
    const blocks = document.querySelectorAll("pre > code.language-ezc");
    blocks.forEach((code) => {
      const pre = code.parentElement;
      // Skip if the block is marked no-run or is inside the full playground.
      if (pre.dataset.norun !== undefined) return;
      if (pre.classList.contains("ezc-norun")) return;
      if (pre.closest("#ezc-full-playground")) return;
      // Skip if already wired (idempotent).
      if (pre.parentElement.classList.contains("ezc-code-wrap")) return;

      // Build wrapper structure:
      //   <div.ezc-code-wrap>
      //     <pre>...code...</pre>
      //     <div.ezc-run-row><button>Run ▶</button></div>
      //     <div.ezc-output> (hidden until first run)
      //   </div>
      const wrap = document.createElement("div");
      wrap.className = "ezc-code-wrap";
      pre.parentElement.insertBefore(wrap, pre);
      wrap.appendChild(pre);

      const row = document.createElement("div");
      row.className = "ezc-run-row";
      wrap.appendChild(row);

      const btn = document.createElement("button");
      btn.className = "ezc-run-btn";
      btn.type = "button";
      btn.textContent = "Run ▶";
      btn.title = "Run this snippet";
      btn.setAttribute("aria-label", "Run this ezc snippet");
      row.appendChild(btn);

      const output = document.createElement("div");
      output.className = "ezc-output";
      output.style.display = "none";
      output.setAttribute("role", "status");
      output.setAttribute("aria-live", "polite");
      wrap.appendChild(output);

      const run = async () => {
        btn.disabled = true;
        btn.textContent = "Running…";
        output.style.display = "block";
        output.classList.remove("ezc-output-error");
        output.innerHTML = '<span class="ezc-stack-empty">loading…</span>';
        try {
          const wasm = await loadWasm();
          const result = wasm.run(code.textContent);
          if (result && result.ok) {
            output.innerHTML =
              '<span class="ezc-output-arrow">→</span>' +
              renderStack(result.stack);
          } else {
            output.classList.add("ezc-output-error");
            output.textContent = "✗ " + (result?.error ?? "unknown error");
          }
        } catch (e) {
          output.classList.add("ezc-output-error");
          output.textContent = "✗ failed to load runtime: " + e;
        } finally {
          btn.disabled = false;
          btn.textContent = "Run ▶";
        }
      };

      btn.addEventListener("click", run);
    });
  }

  // ── Full-page playground ──────────────────────────────────────────────
  function setupFullPlayground() {
    const root = document.getElementById("ezc-full-playground");
    if (!root) return;

    root.innerHTML = `
      <div class="ezc-playground-controls">
        <button data-action="run">Run ▶</button>
        <button data-action="trace">Trace</button>
        <button data-action="share">Share</button>
        <button data-action="clear">Clear</button>
      </div>
      <div class="ezc-playground">
        <textarea spellcheck="false" placeholder="# Type ezc here. Ctrl+Enter to run."></textarea>
        <div class="ezc-playground-output">
          <div class="ezc-playground-pane" data-pane="stack">
            <h4>Stack</h4>
            <div data-content="stack"><span class="ezc-stack-empty">(empty)</span></div>
          </div>
          <div class="ezc-playground-pane" data-pane="output">
            <h4>Output</h4>
            <div data-content="output"><span class="ezc-stack-empty">(none)</span></div>
          </div>
          <div class="ezc-playground-pane" data-pane="trace" style="display:none">
            <h4>Trace</h4>
            <div data-content="trace"></div>
          </div>
        </div>
      </div>
    `;

    const ta = root.querySelector("textarea");
    const stackOut = root.querySelector('[data-content="stack"]');
    const outputOut = root.querySelector('[data-content="output"]');
    const traceOut = root.querySelector('[data-content="trace"]');
    const tracePane = root.querySelector('[data-pane="trace"]');

    // Restore from URL hash.
    if (location.hash.length > 1) {
      try {
        ta.value = decodeURIComponent(atob(location.hash.slice(1)));
      } catch (_) {
        ta.value = decodeURIComponent(location.hash.slice(1));
      }
    }

    // Tab indents instead of changing focus.
    ta.addEventListener("keydown", (e) => {
      if (e.key === "Tab") {
        e.preventDefault();
        const start = ta.selectionStart, end = ta.selectionEnd;
        ta.value = ta.value.slice(0, start) + "  " + ta.value.slice(end);
        ta.selectionStart = ta.selectionEnd = start + 2;
      } else if (e.key === "Enter" && (e.ctrlKey || e.metaKey)) {
        e.preventDefault();
        runProgram();
      }
    });

    async function runProgram() {
      stackOut.innerHTML = '<span class="ezc-stack-empty">loading…</span>';
      tracePane.style.display = "none";
      try {
        const wasm = await loadWasm();
        const result = wasm.run(ta.value);
        if (result.ok) {
          stackOut.innerHTML = renderStack(result.stack);
          outputOut.innerHTML = '<span class="ezc-stack-empty">(none)</span>';
        } else {
          stackOut.innerHTML = `<span style="color:#ff8a8a">✗ ${escapeHtml(result.error)}</span>`;
        }
      } catch (e) {
        stackOut.innerHTML = `<span style="color:#ff8a8a">✗ ${escapeHtml(String(e))}</span>`;
      }
    }

    // Group source lines into bracket-balanced chunks. A chunk is a
    // sequence of lines whose bracket depth returns to (or stays at) 0
    // by the end. Inside a string literal or `# comment`, brackets don't
    // count. This way trace can step over multi-line `(...)`, `[...]`,
    // and `{...}` constructs without splitting them apart.
    function balancedChunks(src) {
      const chunks = [];
      let buf = [];
      let startLine = 1;
      let depth = 0;
      let inString = false;
      let lineNum = 0;
      const lines = src.split("\n");
      for (const line of lines) {
        lineNum++;
        if (buf.length === 0) startLine = lineNum;
        buf.push(line);
        // Walk this line, updating string/comment/depth state.
        for (let i = 0; i < line.length; i++) {
          const ch = line[i];
          if (inString) {
            if (ch === "\\") { i++; continue; }
            if (ch === '"') inString = false;
          } else {
            if (ch === "#") break; // rest of line is comment
            if (ch === '"') { inString = true; continue; }
            if (ch === "(" || ch === "[" || ch === "{") depth++;
            else if (ch === ")" || ch === "]" || ch === "}") {
              if (depth > 0) depth--;
            }
          }
        }
        // String literals can't span newlines in ezc; reset just in case.
        inString = false;
        if (depth === 0 && buf.some((l) => l.trim() && !l.trim().startsWith("#"))) {
          chunks.push({ startLine, text: buf.join("\n") });
          buf = [];
        }
      }
      // Trailing unbalanced content — emit if there's executable code so the
      // user sees the error. Skip if it's only comments.
      if (
        buf.length &&
        buf.some((l) => l.trim() && !l.trim().startsWith("#"))
      ) {
        chunks.push({ startLine, text: buf.join("\n") });
      }
      return chunks;
    }

    async function traceProgram() {
      tracePane.style.display = "block";
      traceOut.innerHTML = '<span class="ezc-stack-empty">loading…</span>';
      try {
        const wasm = await loadWasm();
        const chunks = balancedChunks(ta.value);
        if (chunks.length === 0) {
          traceOut.innerHTML = '<span class="ezc-stack-empty">(no executable code)</span>';
          return;
        }
        const engine = new wasm.EzcEngine();
        const rows = [];
        for (const { startLine, text } of chunks) {
          const display = text.trim().split("\n").join(" ⏎ ");
          const r = engine.eval(text);
          if (r && r.ok) {
            rows.push(`<div><span style="color:var(--sidebar-fg,#888)">${startLine}:</span> ${escapeHtml(display)}  <span class="ezc-output-arrow">→</span> ${renderStack(r.stack)}</div>`);
          } else {
            rows.push(`<div><span style="color:#ff8a8a">${startLine}: ✗ ${escapeHtml(r?.error ?? "unknown error")}</span></div>`);
            break;
          }
        }
        traceOut.innerHTML = rows.join("");
      } catch (e) {
        traceOut.innerHTML = `<span style="color:#ff8a8a">✗ ${escapeHtml(String(e))}</span>`;
      }
    }

    function shareProgram() {
      const encoded = btoa(encodeURIComponent(ta.value));
      const url = location.origin + location.pathname + "#" + encoded;
      navigator.clipboard.writeText(url);
      const btn = root.querySelector('[data-action="share"]');
      const orig = btn.textContent;
      btn.textContent = "Copied!";
      setTimeout(() => { btn.textContent = orig; }, 1200);
    }

    root.addEventListener("click", (e) => {
      const action = e.target.closest("[data-action]")?.dataset.action;
      if (action === "run") runProgram();
      else if (action === "trace") traceProgram();
      else if (action === "share") shareProgram();
      else if (action === "clear") {
        ta.value = "";
        stackOut.innerHTML = '<span class="ezc-stack-empty">(empty)</span>';
        outputOut.innerHTML = '<span class="ezc-stack-empty">(none)</span>';
        tracePane.style.display = "none";
      }
    });
  }

  // Run setup once the page is ready.
  function init() {
    setupInlineRunners();
    setupFullPlayground();
  }

  if (document.readyState === "loading") {
    document.addEventListener("DOMContentLoaded", init);
  } else {
    init();
  }
})();
