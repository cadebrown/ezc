---
layout: docs
title: Setting Variables
---

### Assignment

If you have a variable, `x`, you can assign a value by a constant, like `x = 3.05`.

You can also assign another variable, such as `x = y`.

Additionally, you can use `$` to get command line arguments, like `x = $1`

### Functions

A [function]({{site.ezc_docs}}/functions/) can be used to assign a value.

Functions now allow constants

This is valid:

<pre>
x = log 2.0
</pre>

And this is valid:

<pre>
y = 2.0
x = log y
</pre>

Some functions take multiple variables, like `log`

<pre>
x = log 2.0 4.0
</pre>

<pre>
a = 2.0
y = 4.0
x = log a y
</pre>

### Operators

[Operators]({{site.ezc_docs}}/operators/), like `+`, `-`, `*`, `/`, `**` and `^` can be used in shorthand.

Each Operator links to a function. The following two lines are equivalent: `z = x + y` and `z = add x y`

You can use operators in the middle of arguments.

`VARIABLE = OPA {OPERATOR} OPB`

which assigns the result to variable

Some operators can use only one, for example:

`a = b !`

for factorial

And some can use either one or two, such as:

`a = 1 ~ b` is equivalent to `a = ~ b`

To see a full list of functions, check the documentation on the [function list]({{site.ezc_docs}}/functions/list.html)