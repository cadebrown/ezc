# Flow: Structured Development Workflow

Flow is a document-driven workflow for tracking features from plan through completion.
Each feature lives in its own file — agents and humans can pick up any item independently.

## Stages

```
todo ──► work ──► done
 plan    active   complete
         impl     + lessons
```

## Directory Structure

- `flow/todo/` -- Planned work with detailed implementation steps. Ready to be picked up.
- `flow/work/` -- Currently in progress. One developer/agent per item.
- `flow/done/` -- Completed. Includes a "Lessons Learned" section.

## Document Format

Filename: `NNN-slug.md` where NNN is a zero-padded sequence number.

```markdown
# NNN: Title

Status: todo | work | done
Created: YYYY-MM-DD
Updated: YYYY-MM-DD

## Context

Why this is being done and what outcome it produces.

## Plan

Step-by-step implementation plan. Detailed enough for an agent to execute
independently without additional context.

## Verification

How to confirm the work is complete (commands, tests, expected output).

## Status Log (added at work stage)

- YYYY-MM-DD: Started
- YYYY-MM-DD: Blocked on X

## Lessons Learned (added at done stage)

What was surprising, what changed from the plan, what to do differently.
```

## Rules

1. Items in `flow/todo/` must have a complete Plan section before moving to `work/`.
2. One item in `flow/work/` per person/agent at a time.
3. Done items are never deleted — they are project memory.
4. All flow changes are committed alongside the code changes they describe.

## Stage Transitions

- **todo -> work**: Pick up the item. Move file, update status. Add Status Log section.
- **work -> done**: Tests pass, work complete. Move file, update status. Add Lessons Learned.

## For AI Agents

1. Check `flow/work/` first — don't start duplicate work.
2. Find your item in `flow/todo/`, read the full Plan section before touching code.
3. Update the Status Log as you go.
4. Move to `flow/done/` when tests pass and the Verification section checks out.
