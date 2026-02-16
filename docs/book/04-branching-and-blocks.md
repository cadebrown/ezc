# Chapter 4: Branching And Blocks

EZC control flow stays value-oriented.

- `( ... )` creates a delayed block value
- `!` executes the top block
- `?` selects between two values

## 4.1 Delayed Execution

A block is just data until executed.

```ezc run
(6 dup * prt)
```

Execute it with `!`:

```ezc run
(6 dup * prt)!
```

## 4.2 Reusing Blocks

Because blocks are values, you can duplicate and execute multiple times.

```ezc run
(2 3 + prt) dup ! !
```

## 4.3 Value-Level Conditional Selection

`a b c ?` means:

- keep `a` when `c` is truthy
- keep `b` when `c` is falsy

```ezc run
"yes" "no" 1 ? prt
```

```ezc run
"yes" "no" 0 ? prt
```

## 4.4 Building Conditions Inline

`?` works cleanly with comparison words.

```ezc run
"bigger" "smaller" 9 5 > ? prt
```

```ezc run
100 200 4 4 = ? prt
```

## 4.5 Keep Condition Construction Explicit

Do not hide branch logic. Build condition values directly on stack near `?`.

This makes control flow easy to audit and easy to test.

## Chapter Checkpoint

You should now understand EZC branching as value selection, not statement branching.
