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

A function in ezc is just a block — the body operates on whatever is on
the stack. Whoever calls it puts the arguments there.

A two-argument adder:

```ezc
(+) @add
3 4 add        # → 7
```

A function that doubles its input, adds 1, then squares:

```ezc
(2 * 1 + sq) @f
3 f            # 3 → 6 → 7 → 49
```

Try writing a `cube` function. It needs to leave `n³` on the stack:

```ezc
# Define cube here, then test with: 3 cube
```

(One answer: `(, , * *) @cube` — dup twice, then multiply twice.)

## Higher-order

Blocks can be passed to operators like `&!` (map). More on that in the
[next chapter on lists](lists.md).

## What's next

- [**Lists and higher-order ops**](lists.md) — map, filter, fold
