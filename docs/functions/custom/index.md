---
layout: docs
title: Custom Functions
---

In EZC, you can define your own functions:

```fortran
[SQUARE] a
    return (a^2)
[/SQUARE]

var (@SQUARE 2)
```

That code prints out `4.000...`.

The format is:

```fortran
[$NAME$] $PARAMS...$
    # do stuff here
    return (VALUE)
[/$NAME$]
```

The `$NAME$` is how you call it later, $PARAMS...$ can be a list of names. These are the values you pass it.

You start it and end it with `[]` brackets with a name inside of it.

Params can be anything, and when you pass values (such as `2` in our example), you can use those. They are ordered.

`return (value)` does what you'd expect. It yields the value inside the parenthesis.
