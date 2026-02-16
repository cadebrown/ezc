# Chapter 6: Composite Data

Not every value in EZC is an integer.
You can carry structured data directly on the stack.

## 6.1 Sub-Stacks

`[ ... ]` creates a stack value as one object.

```ezc run
[1 2 [3 4] "payload"] prt
```

Nested stacks are valid, and preserve exact structure.

## 6.2 Symbols As Data

Inside `[]`, bare words are symbols, not executable operators.

```ezc run
[swp del custom-tag] prt
```

Outside `[]`, those same words are interpreted as program words.

## 6.3 Blocks Inside Data

Blocks can be embedded in stack data and transported.

```ezc run
[(2 3 + prt) "deferred"] prt
```

The embedded block is not executed automatically.

## 6.4 Conditional Data Selection

Because `?` works on values, you can branch over any type, not just integers.

```ezc run
[1 2 3] [9 9 9] 1 ? prt
```

```ezc run
"left" "right" 0 ? prt
```

## 6.5 Designing Data Carriers

A useful EZC pattern is to keep algorithm state in plain stack values and attach labels in sub-stacks when needed.

Example shape ideas:

- `[n acc]` for numeric loops
- `["mode" state]` for branchable flow
- `[tag payload]` for tiny symbolic records

## Chapter Checkpoint

You now have enough value types to model small interpreters and symbolic transforms in pure EZC style.
