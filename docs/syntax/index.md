---
layout: docs
title: EZC Syntax
---

# Nesting

You can nest most everything in EZC, and it is expanded.

For example, the following two programs are equivalent:

```fortran
a = sqrt (2 + 3)
```

```fortran
b = 2 + 3
a = sqrt b
```


However, you should use parenthesis all the time.

This will print out the square root of 5: 

```fortran 
var (sqrt 2 + 3)
```

# Calling functions

Calling functions can be done.

Specifically, this is evaluated after parenthesis and operators.

Say, the `rand` function:

```fortran
a = rand 0 100
a = (rand 0,100)
```

are both equivalent

However, do not do:

```fortran
a = rand (0, 100)
```

Instead, wrap this:

```fortran
a = (rand 0, 100)
```

This is a design choice in EZC, that a function is not invoked by `name(args)`, but rather it is implied by `name args`, and can be wrapped for execution like `(name args)`

So, if you want to multiple two random numbers:

```fortran
a = (rand 0, 100) * (rand 0, 100)
```

Or, through operators:

```fortran
a = (0?100)*(0?100)
```

