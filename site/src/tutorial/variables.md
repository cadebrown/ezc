# Variables and scopes

## Binding with `@`

`@name` pops the top of the stack and binds it to a name:

```ezc
42 @x
```

After this, the stack is empty and `x` holds `42`.

## Recalling with `$`

`$name` pushes the value of a name onto the stack:

```ezc
42 @x
$x $x +
```

That's `42 + 42`, leaving `[84]`.

## Rebinding is allowed

Each `@x` rebinds. The new value replaces the old:

```ezc
1 @x
$x        # → 1
2 @x
$x        # → 2
```

## Scopes

`{ ... }` introduces a local scope. Bindings made inside the braces
disappear when the scope ends:

```ezc
10 @x
{ 99 @x $x }    # local x = 99
$x              # outer x is still 10
```

The block above leaves `[99 10]` on the stack — first the `99` recalled
inside the scope, then the outer `x`.

## Bare names

A bare identifier (no sigil) is treated like a recall, but if the value
is a block it executes. We'll see that in the [next chapter](functions.md).

## Why both `@` and `$`?

The sigils make data flow obvious. Reading `@x` you immediately know
something is being stored; reading `$x` you know something is being
fetched. There's no ambiguity about which way the value moves.

## What's next

- [**Blocks and functions**](functions.md) — defining reusable code
