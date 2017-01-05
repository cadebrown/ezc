---
layout: docs
title: EZC Functions
---

In ezC, functions are not declared, but are a set of keywords that can be applied like:

```fortran
x = log y
```

or

```fortran
echo Hello World
```


Similar to [operators](../operators/), functions take arguments, and store it in a variable, except for some functions like `echo`, `prec`, and `var`

In our examples, `x = log y`, it sets `x` to the natural logarithm (base e) of `y`

`echo Hello World` prints out Hello World to the console

`var x` prints out the name of the variable, `x`, and it's numerical value.

In fact, all operators are mapped to a function.

`x = a + b` is the same as `x = add a b`

`x = a ^ b` is the same as `x = pow a b`

etc.

Here's the global [list of functions](./all/)

You can also create your own functions, see [Custom Functions](./custom/)