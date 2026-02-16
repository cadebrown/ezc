# Chapter 1: The Stack Model

EZC is postfix and concatenative. Every token either pushes a value or transforms values already on the stack.

If you keep one question in mind, keep this one:

- What does the stack look like before this word?
- What does it look like after this word?

## 1.1 Reading Left To Right

Numbers and literals go first. Operators consume from the top.

```ezc run
2 3 + prt
```

Step-by-step:

- `2` -> `[2]`
- `3` -> `[2 3]`
- `+` -> `[5]`
- `prt` prints `5` and removes it

## 1.2 Pipelines, Not Statements

EZC code is usually one long transformation pipeline.

```ezc run
2 3 + 4 * prt
```

That is `(2 + 3) * 4` in infix terms, but EZC keeps everything linear.

## 1.3 The Top Of Stack Is The Active Focus

When you read a program, your eyes should stay on the right edge of the current stack.

```ezc run
10 3 - 2 * prt
```

Useful habit:

- annotate stack state while learning
- erase annotations as patterns become automatic

## 1.4 Printing Vs Leaving Results

`prt` is for output. If you do not print, the value remains on the stack for later words.

```ezc run
7 8 +
```

The web runner will show the final stack if nothing is printed.

## 1.5 Comments Are Runtime-Ignored

Use `#` for notes and derivations.

```ezc run
2 3 +   # add first pair
4 *     # scale result
prt
```

## Chapter Checkpoint

By now you should be able to:

- read postfix arithmetic without converting to infix
- trace stack snapshots across a short pipeline
- decide whether to `prt` or keep values for more work
