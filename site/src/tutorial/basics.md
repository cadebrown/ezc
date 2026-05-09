# Stack basics

ezc has a single, global stack. Every value you write is pushed onto it.
Every operator pops its arguments off and pushes its result back.

## Pushing values

```ezc
3
```

That program pushes the integer `3`. Running it leaves a stack of `[3]`.

Multiple values stack up:

```ezc
1 2 3
```

The stack is `[1 2 3]` — `3` is on top.

## Operators

`+` pops two numbers and pushes their sum:

```ezc
3 4 +
```

Step by step:

1. `3` → stack `[3]`
2. `4` → stack `[3 4]`
3. `+` pops `3` and `4`, pushes `7` → stack `[7]`

This is reverse Polish notation. There are no parentheses for grouping
arithmetic, and no operator precedence — the order is determined entirely
by the stack.

```ezc
3 4 + 2 *
```

That's `(3 + 4) * 2 = 14`. Try it.

## All the arithmetic operators

```ezc
10 3 +    # add → 13
10 3 -    # subtract → 7
10 3 *    # multiply → 30
10 3 /    # divide → 3 (integer division)
10 3 %    # modulo → 1
2 10 ^    # power → 1024
```

## Comparisons push 0 or 1

There is no separate boolean type. Comparisons push `1` for true, `0`
for false:

```ezc
3 4 <     # → 1
4 3 <     # → 0
3 3 ==    # → 1
```

## Stack manipulation

Sometimes you want to reorder, duplicate, or drop values:

| op  | effect      | description |
|-----|-------------|-------------|
| `,` | `a → a a`   | dup: copy the top |
| `;` | `a →`       | drop: discard the top |
| `~` | `a b → b a` | swap: exchange the top two |
| `_` | `a b → a b a` | over: copy the second to the top |

Try it:

```ezc
5 ,       # → [5 5]
5 ;       # → []
1 2 ~     # → [2 1]
1 2 _     # → [1 2 1]
```

## Comments

Anything after `#` on a line is a comment:

```ezc
3 4 +     # this is a comment, the result is 7
```

## What's next

- [**Variables and scopes**](variables.md) — naming values
