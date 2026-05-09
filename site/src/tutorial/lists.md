# Lists and higher-order ops

`[...]` creates a list. Unlike blocks, list contents are evaluated
eagerly — each value is pushed onto a temporary stack and gathered into
a single list when the `]` is reached.

```ezc
[1 2 3]
```

That leaves `[[1 2 3]]` — a stack with one item, which is a list of
three integers.

## Building lists

```ezc
[1 2 +  3 4 +]    # → [[3 7]]
```

Expressions inside `[...]` execute, and the leftover values become the
list elements.

## `range` and `iota`

For numeric sequences:

```ezc
0 5 range      # → [[0 1 2 3 4]]
5 iota         # → [[0 1 2 3 4]]   (prelude: 0..n-1)
```

## Map (`&!`)

`&!` applies a block to every element of a list:

```ezc
[1 2 3 4] (sq) &!
```

`sq` is a prelude function (`(, *)`), squaring each element. Result:
`[[1 4 9 16]]`.

The named alias `map` does the same:

```ezc
[1 2 3 4] (sq) map
```

## Filter (`&?`)

`&?` keeps elements where the block evaluates to a truthy value:

```ezc
[1 2 3 4 5 6] (even) &?
```

Result: `[[2 4 6]]`.

## Fold (`&/`)

`&/` reduces a list with a block, given an initial accumulator:

```ezc
[1 2 3 4] 0 (+) &/
```

That's `((((0 + 1) + 2) + 3) + 4)` = `10`.

The prelude has `sum` and `prod` for the common cases:

```ezc
[1 2 3 4] sum     # → [10]
[1 2 3 4] prod    # → [24]
```

## Sum of squares

Combine map and sum:

```ezc
10 iota (sq) map sum
```

That's `0² + 1² + 2² + ... + 9² = 285`.

## List utilities

```ezc
[1 2 3] len               # → 3
[1 2 3] 0 nth             # → 1   (zero-indexed)
[1 2 3] rev               # → [[3 2 1]]
[3 1 2] srt               # → [[1 2 3]]
[1 2 3] [4 5 6] zip       # → [[[1 4] [2 5] [3 6]]]
[1 2 3 4 5] 2 take        # → [[1 2]]
[1 2 3 4 5] 2 skip        # → [[3 4 5]]
[[1 2] [3 4]] flat        # → [[1 2 3 4]]
```

## What's next

- [**Numeric types**](types.md) — int, u8…u256, i8…i256, f16/f32/f64
