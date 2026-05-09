# I/O and modules

## Writing output

Three operators write to stdout:

| op   | type effect             | description |
|------|-------------------------|-------------|
| `:`  | `a →`                   | print top of stack with newline |
| `wl` | `a →`                   | named alias of `:`  |
| `wb` | `int →`                 | write a single byte |

```ezc
"Hello!" :       # prints: Hello!
42 :             # prints: 42
```

## Reading input

| op   | effect      | description |
|------|-------------|-------------|
| `.`  | `→ str`     | read a line from stdin |
| `rl` | `→ str`     | named alias of `.` |
| `rb` | `→ int`     | read a single byte |

```ezc
"What's your name? " :
. @name
"Hello, " $name | :
```

`|` (compose) concatenates strings, lists, or blocks.

## Imports

`import` loads another `.ezc` file (or an embedded standard module):

```ezc
"std/math.ezc" import
5 fact          # → 120
```

The path is a string. The first lookup is in the embedded modules
shipped with the compiler (`std/math.ezc`, `std/str.ezc`); if not found,
ezc reads from disk relative to the current directory.

Imports are guarded — importing the same path twice is a no-op.

## Putting it together

A complete program that prints the first ten factorials:

```ezc
"std/math.ezc" import
1 11 range
(@n
  $n str " factorial is " | $n fact str | :
) each
```

Note the `str` calls — `|` (compose) only joins values of the same kind
(string with string, list with list, block with block). To stitch
numbers into a sentence, convert them to strings first.

## Errors

Errors include the source span, file, line, and column:

```
error: stack underflow
   ╭─[example.ezc:3:5]
 3 │     +
   │     ┬
   │     ╰── operator + needs 2 values, found 1
   ╯
```

The DAP debugger lets you step through programs and inspect the stack
at every point — see the [editor setup](../editors.md) page.

## You're done with the tutorial

You've covered the whole language. Browse the reference for details:

- [**Operators**](../reference/operators.md)
- [**Builtins**](../reference/builtins.md)
- [**Type system**](../reference/types.md)
- [**Standard library**](../reference/stdlib.md)
