# 002: TextMate Grammar

Status: done
Created: 2026-03-26
Updated: 2026-03-26

## Context

Universal syntax highlighting for EZC. TextMate grammars are consumed natively
by VS Code, Sublime, Zed, and any other TextMate-compatible editor.

## Verification

Installed in VS Code extension and confirmed all token types highlight correctly.

## Lessons Learned

Had to recreate from scratch after worktree was lost. Updated for new operator
set (`,` dup, `;` drop, `:` write, `.` read, `&/` fold, `rl`/`wl`/`rb`/`wb` builtins).
