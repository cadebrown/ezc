--- nvim-ezc: Neovim plugin for the EZC language
---
--- Provides:
---   - Filetype detection (.ezc → filetype=ezc)
---   - Tree-sitter parser registration (requires tree-sitter-ezc)
---   - LSP client setup (uses ezc lsp)
---   - nvim-dap setup (uses ezc debug)
---   - ftplugin settings (comment string, indent)
---
--- Usage (lazy.nvim):
---   { "cadebrown/nvim-ezc", ft = "ezc", dependencies = { "neovim/nvim-lspconfig" } }
---
--- Or without a plugin manager, clone this repo to:
---   ~/.local/share/nvim/site/pack/ezc/start/nvim-ezc/
--- and call require("ezc").setup() in your init.lua.

local M = {}

-- ── Default options ───────────────────────────────────────────────────────

local defaults = {
  --- Path to the ezc binary. Defaults to "ezc" (must be in PATH).
  cmd = "ezc",
  --- Enable LSP client (requires ezc in PATH).
  lsp = true,
  --- Enable nvim-dap configuration (requires nvim-dap to be loaded first).
  dap = true,
  --- Enable tree-sitter highlighting (requires nvim-treesitter + tree-sitter-ezc).
  treesitter = true,
  --- Extra options passed to vim.lsp.start on attach.
  lsp_settings = {},
}

-- ── Setup ─────────────────────────────────────────────────────────────────

--- Configure the EZC plugin.
---
---@param opts table|nil Optional overrides for default options.
function M.setup(opts)
  local config = vim.tbl_deep_extend("force", defaults, opts or {})

  -- Filetype detection
  vim.filetype.add({
    extension = { ezc = "ezc" },
  })

  -- ftplugin settings
  vim.api.nvim_create_autocmd("FileType", {
    pattern = "ezc",
    callback = function()
      vim.bo.commentstring = "# %s"
      vim.bo.shiftwidth = 2
      vim.bo.tabstop = 2
      vim.bo.expandtab = true
    end,
  })

  -- LSP
  if config.lsp then
    vim.api.nvim_create_autocmd("FileType", {
      pattern = "ezc",
      callback = function(ev)
        vim.lsp.start(vim.tbl_extend("force", {
          name = "ezc-lsp",
          cmd = { "ezc-lsp" },
          root_dir = vim.fs.root(ev.buf, { ".git", "Cargo.toml" }),
          settings = config.lsp_settings,
        }, {}))
      end,
    })
  end

  -- DAP
  if config.dap then
    M._setup_dap(config)
  end

  -- Tree-sitter parser registration
  if config.treesitter then
    M._register_treesitter()
  end
end

-- ── DAP configuration ─────────────────────────────────────────────────────

function M._setup_dap(config)
  local ok, dap = pcall(require, "dap")
  if not ok then
    return -- nvim-dap not installed; skip silently
  end

  -- Adapter: spawn ezc-dap on stdio
  dap.adapters.ezc = {
    type = "executable",
    command = "ezc-dap",
    name = "ezc",
  }

  -- Default launch configuration
  dap.configurations.ezc = {
    {
      type = "ezc",
      request = "launch",
      name = "Debug current file",
      program = function()
        return vim.fn.expand("%:p")
      end,
      stopOnEntry = true,
    },
    {
      type = "ezc",
      request = "launch",
      name = "Run current file (no debug)",
      program = function()
        return vim.fn.expand("%:p")
      end,
      stopOnEntry = false,
      noDebug = true,
    },
  }
end

-- ── Tree-sitter ───────────────────────────────────────────────────────────

function M._register_treesitter()
  local ok, parsers = pcall(require, "nvim-treesitter.parsers")
  if not ok then
    return -- nvim-treesitter not installed; skip silently
  end

  -- Register the EZC parser if it isn't already known.
  -- Users can install it with :TSInstall ezc once the parser is published,
  -- or manually by pointing parser_install_dir to the tree-sitter-ezc build.
  local parser_config = parsers.get_parser_configs()
  if not parser_config.ezc then
    parser_config.ezc = {
      install_info = {
        url = "https://github.com/cadebrown/tree-sitter-ezc",
        files = { "src/parser.c" },
        branch = "main",
        generate_requires_npm = false,
        requires_generate_from_grammar = false,
      },
      filetype = "ezc",
    }
  end
end

-- ── Keybindings helper ───────────────────────────────────────────────────

--- Register default debug keybindings for EZC buffers.
--- Call this from your own config if you want the suggested bindings:
---
---   require("ezc").setup()
---   require("ezc").set_dap_keymaps()
---
function M.set_dap_keymaps()
  local dap_ok, dap = pcall(require, "dap")
  if not dap_ok then
    return
  end
  local dapui_ok, dapui = pcall(require, "dapui")

  vim.api.nvim_create_autocmd("FileType", {
    pattern = "ezc",
    callback = function(ev)
      local buf = ev.buf
      local opts = { buffer = buf, silent = true }
      vim.keymap.set("n", "<F5>",  dap.continue,          vim.tbl_extend("force", opts, { desc = "Debug: Continue" }))
      vim.keymap.set("n", "<F10>", dap.step_over,         vim.tbl_extend("force", opts, { desc = "Debug: Step Over" }))
      vim.keymap.set("n", "<F11>", dap.step_into,         vim.tbl_extend("force", opts, { desc = "Debug: Step Into" }))
      vim.keymap.set("n", "<F12>", dap.step_out,          vim.tbl_extend("force", opts, { desc = "Debug: Step Out" }))
      vim.keymap.set("n", "<leader>db", dap.toggle_breakpoint, vim.tbl_extend("force", opts, { desc = "Debug: Toggle Breakpoint" }))
      vim.keymap.set("n", "<leader>dB", function()
        dap.set_breakpoint(vim.fn.input("Condition: "))
      end, vim.tbl_extend("force", opts, { desc = "Debug: Conditional Breakpoint" }))
      vim.keymap.set("n", "<leader>dl", function()
        dap.set_breakpoint(nil, nil, vim.fn.input("Log message: "))
      end, vim.tbl_extend("force", opts, { desc = "Debug: Logpoint" }))
      if dapui_ok then
        vim.keymap.set("n", "<leader>du", dapui.toggle, vim.tbl_extend("force", opts, { desc = "Debug: Toggle UI" }))
      end
    end,
  })
end

return M
