# 015: Trace Mode

Status: done
Created: 2026-03-26
Completed: 2026-04-09

## Context

`ezc run --trace` shows stack state after each expression.

## Plan

Add --trace flag to `ezc run`. After each expression print `[expr] -> [stack]`.
Also expose as `:trace on` in REPL.
