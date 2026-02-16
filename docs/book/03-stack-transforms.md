# Chapter 3: Stack Transforms

This chapter is the mechanical core of EZC style: move values into the shape your next operator expects.

Core words:

- `dup` (alias `,`) duplicate top item
- `del` (alias `.`) delete top item
- `swp` (alias `~`) swap top two items
- `ovr` (alias `_`) copy second item to top

## 3.1 Duplicate And Reuse

```ezc run
5 dup * prt
```

## 3.2 Swap To Fix Order

```ezc run
10 20 swp prt prt
```

Because `prt` consumes, this prints `10` then `20` after the swap.

## 3.3 Over For Fan-Out

`ovr` is useful when one value must feed multiple operations.

```ezc run
3 4 ovr + + prt
```

Trace:

- `[3 4]`
- `ovr` -> `[3 4 3]`
- `+` -> `[3 7]`
- `+` -> `[10]`

## 3.4 Alias Pass

Same operations, fewer characters:

```ezc run
9 , * prt
```

```ezc run
7 8 _ + + prt
```

```ezc run
1 2 ~ prt prt
```

## 3.5 Control Your Stack Budget

`del` is not optional cleanup. It is a design tool.

```ezc run
1 2 3 . prt prt
```

Removing temporary values early prevents accidental coupling later.

## Chapter Checkpoint

You should be able to rewrite a stack pipeline using only `dup`, `del`, `swp`, and `ovr` to satisfy operand order requirements.
