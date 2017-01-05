---
layout: post
title: EZC Examples
---

To view digits of pi, run (from commandline):

```bash
./ezc -c "prec 1000 : var (pi)"
```

For digits of e, run:

```bash
./ezc -c "prec 1000 : var (exp 1)"
```

You can set degree or radian mode like so:

```bash
EZC_DEG=0 ./ezc -c "var (sin 90)"
```

(this prints out .89..., i.e. sin of 90 radians)

```bash
EZC_DEG=1 ./ezc -c "var (sin 90)"
```

(this prints out 1.0..., i.e. sin of 90 degrees)

Order of operations works as expected (with parenthetical wrapping), but with added operators.


So,

```bash
./ezc -c "var (2*3+-4*5)"
```

prints out -14

`2*3+-4*5` is the same as `(2*3)+((-4)*5)`

Operators are added for convenience

For example, you can use the `?` operators for random numbers.

The `?` operator is above exponents, (i.e. P?EMDAS)

See:

```bash
./ezc -c "var (-5?3)"
```

prints out a number >= -5 and < 3

```bash
./ezc -c "var (-10?1^2)"
```

gets a number between -10 and 1, but then squares that number, so the printed out number is >= 0, and < 100


