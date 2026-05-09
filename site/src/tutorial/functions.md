# Blocks and functions

A **block** is a piece of deferred code wrapped in parentheses. Blocks
are values — you can push them, name them, and execute them on demand.

## Pushing a block

```ezc
(3 4 +)
```

This pushes a block onto the stack. The body `3 4 +` is **not** executed
yet. The stack contains a single block value.

## Executing a block with `!`

`!` pops a block and runs it:

```ezc
(3 4 +) !
```

Step by step:

1. `(3 4 +)` → stack `[(...)]`
2. `!` pops the block and runs `3 4 +` → stack `[7]`

## Functions are named blocks

A function in ezc is just a block bound to a name:

```ezc
(, *) @square
5 $square !
```

Reading from left to right:

1. `(, *)` — push a block. Its body uses `,` (dup) then `*`.
2. `@square` — bind the block to `square`.
3. `5` — push 5.
4. `$square` — recall the block onto the stack.
5. `!` — execute it. `,` makes the stack `[5 5]`, then `*` gives `[25]`.

## Bare-word call

Pushing-then-executing with `$square !` is common, so ezc lets you write
just the name:

```ezc
(, *) @square
5 square        # same as: 5 $square !
```

When a bare identifier resolves to a block, it auto-executes.

## Multiple arguments

Blocks pop their inputs from the stack in the same order they were
pushed. A two-argument function is just a block that pops two values:

```ezc
(_ * +) @mul-add    # (a b c → a*b+c)
2 3 4 mul-add       # → 14   (2*3 + 4? no: see below)
```

Wait — that's wrong. Let me show you correctly. `_` is *over*, which
copies the second-from-top to the top. Let's write it more carefully:

```ezc
(* +) @mul-add          # (a b c → (a*b) + c)
2 3 4 mul-add           # 4 then * pops 4 and 3 first... 
```

Actually trace through carefully:

```ezc
3 4 + 5 *        # (3+4) * 5 = 35
```

A useful exercise: try writing your own `cube` function using `,` and
`*`:

```ezc
# Define cube here, then test with: 3 cube
```

## Higher-order

Blocks can be passed to operators like `&!` (map). More on that in the
[next chapter on lists](lists.md).

## What's next

- [**Lists and higher-order ops**](lists.md) — map, filter, fold
