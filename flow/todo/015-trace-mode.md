# 015: Trace Mode

Status: todo
Created: 2026-03-26

## Context

`ezc run --trace` shows stack state after each expression.

## Plan

Add --trace flag to `ezc run`. After each expression print `[expr] -> [stack]`.
Also expose as `:trace on` in REPL.
