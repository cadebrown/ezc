# Chapter 5: Loop Machines

`^` is EZC's loop operator.

It expects a block on top of the stack and repeatedly executes it.
After each iteration, the block must leave one condition value:

- truthy => continue
- falsy => stop

That condition value is consumed by `^`.

## 5.1 Countdown Pattern

```ezc run
5 (dup prt 1 - dup) ^ del
```

Pattern:

- state value stays on stack between iterations
- block computes next state
- block duplicates condition so one copy can become next state

## 5.2 Growth Loop Pattern

```ezc run
1 (dup prt 2 * dup 64 <) ^ del
```

This prints powers of two below `64`.

## 5.3 Accumulator Pattern

Sum `1..10` with state `(n acc)`.

```ezc run
1 0
(
  ovr +
  swp 1 + swp
  ovr 11 <
) ^
swp del prt
```

## 5.4 Loop Design Checklist

Before writing a loop, write these answers first:

1. What is my persistent state shape?
2. Which value is my continuation condition?
3. What temporary values must be deleted?

If those three are clear, `^` loops are usually short and stable.

## 5.5 Common Failure Mode

If a loop fails with stack underflow, the block probably did not leave a condition value for `^` to consume.

## Chapter Checkpoint

You should be able to design loops as explicit state machines.
