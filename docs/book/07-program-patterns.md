# Chapter 7: Program Patterns

This chapter turns the primitives into full programs.
These are condensed versions of the demos in `demo/`.

## 7.1 Fibonacci Stream

State is `(a b)`.
Each step prints `b`, then advances to `(b, a+b)`.

```ezc run
0 1
(
  dup prt
  swp ovr +
  dup 50 <
) ^
del del
```

## 7.2 Euclidean GCD

State is `(a b)`.
Each step becomes `(b, a%b)` until remainder is zero.

```ezc run
252 105
(
  swp ovr %
  dup 0 = not
) ^
del prt
```

## 7.3 Factorial

State is `(n acc)`.
Multiply accumulator by `n`, increment `n`, continue while `n < 9`.

```ezc run
1 1
(
  ovr *
  swp 1 + swp
  ovr 9 <
) ^
swp del prt
```

## 7.4 Pattern Summary

Across these programs, the structure repeats:

1. initialize state values
2. write one block that advances state
3. emit one condition for `^`
4. clean up leftovers with `del`

When a program looks wrong, inspect state shape first.

## 7.5 Demo Files

For full commented versions, inspect:

- `demo/fib.ezc`
- `demo/gcd.ezc`
- `demo/factorial.ezc`
- `demo/triangular.ezc`
- `demo/powers_of_two.ezc`
