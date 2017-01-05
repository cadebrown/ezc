---
layout: docs
title: Variable Initialization
---

### Constant

To initialize a variable to a constant, simply type out the constant:

`x = 3.14159265358979`

### Variable

You can also use another variable, such as:

`x = y`


### Command Line Arguments

Additionally, you can read in values from the commandline, using `$`. `$1` reads in the first argument, `$2` reads the second, and so one. 

<pre>
# Prints sum of first two args
a = $1 : b = $2

sum = a + b

var sum
</pre>

When compiled, as `sum.o`, you can use it like so:

<pre>
> ./sum.o 1.0 2.0
> sum : 3.00000000 . . .
</pre>