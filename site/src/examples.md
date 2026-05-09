# Examples

A gallery of small programs. Every block is runnable — click **Run ▶**.

## FizzBuzz

```ezc
1 16 range
(
  , 15 % 0 ==
  ( ; "FizzBuzz" : )
  (
    , 3 % 0 ==
    ( ; "Fizz" : )
    (
      , 5 % 0 ==
      ( ; "Buzz" : )
      ( : )
      ??
    )
    ??
  )
  ??
) each
```

## Sum of squares

```ezc
10 iota (sq) map sum
```

## Factorial

```ezc
"std/math.ezc" import
20 fact
```

That's `2,432,902,008,176,640,000` — exact, because `int` is BigInt by
default.

## Fibonacci

```ezc
"std/math.ezc" import
0 30 range (fib) map
```

## Pythagorean triples up to 20

```ezc
1 21 range          # a in 1..20
(@a
  1 21 range        # b in 1..20
  (@b
    1 21 range      # c in 1..20
    (@c
      $a sq $b sq + $c sq ==
      ([$a $b $c] :) ?
    ) each
  ) each
) each
```

## Primes under 20

For each candidate `n`, count how many integers in `[2..n-1]` divide it.
If none do, `n` is prime.

```ezc
2 20 range
(@n
  2 $n range
  (@d $n $d % 0 == ) &?
  len 0 ==
  ($n :) ?
) each
```

## Mean of a list

```ezc
[42 7 19 88 3]
, sum ~ len /
```

## Reversing a string

```ezc
"hello" rev :
```

## Counting lines (REPL)

```ezc
0 @count
( . , "" != )         # while line not empty
( ; $count 1 + @count ) &
"Lines: " $count str | :
```

(That one needs stdin, so run it in the terminal with `ezc run`.)
