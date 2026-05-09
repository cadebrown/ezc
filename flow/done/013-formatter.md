# 013: Formatter (ezc fmt)

Status: done
Created: 2026-03-26
Completed: 2026-04-09

## Context

Canonical formatting for EZC. Requires lexer comment preservation first.

## Plan

1. Add Token::Comment to lexer (preserve comments)
2. Implement formatter: lex, re-emit with canonical spacing
3. Add `ezc fmt` and `ezc fmt --check` subcommands
