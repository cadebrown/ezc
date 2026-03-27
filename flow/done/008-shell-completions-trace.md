# 008: Shell Completions & Trace Mode

Status: done
Created: 2026-03-27
Updated: 2026-03-27

## Context

Two small DX improvements wired into the CLI.

## Shell completions

`ezc completions <shell>` generates shell completions for bash, zsh, fish,
elvish, or PowerShell using `clap_complete`. Output the result to the
appropriate completion directory for your shell:

```bash
# bash
ezc completions bash >> ~/.bash_completion

# zsh
ezc completions zsh > ~/.zsh/completions/_ezc

# fish
ezc completions fish > ~/.config/fish/completions/ezc.fish
```

## Trace mode

`ezc --trace <file>` (or `ezc -e "..." --trace`) prints the stack state after
each expression to stderr in a tabular format, then prints the final stack to
stdout as usual:

```
   1 │ 3                    → [3]
   1 │ 4                    → [3 4]
   1 │ +                    → [7]
   1 │ 2                    → [7 2]
   1 │ *                    → [14]
14
```

Implemented in `crates/ezc-cli/src/commands/run.rs::execute_trace_src()` using
`Engine::eval_one` directly (same method used by the DAP stepper).

## Verification

- `echo "3 4 + 2 *" | ezc --trace` → correct per-step output
- `ezc completions bash | head` → valid bash completion script
