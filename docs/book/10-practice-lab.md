# Chapter 10: Practice Lab

Use this chapter as a repetition loop. Write your own version first, then compare with reference answers.

## 10.1 Drills

1. Print absolute value of `-7`.
2. Print max of `9` and `4`.
3. Print sum of `1..6` using a `^` loop.
4. Print powers of two below `40`.
5. Print `"ok"` when `3 < 9`, else `"bad"`.

---

## 10.2 Reference: Absolute Value

```ezc run
-7 dup 0 swp - ovr 0 > ? prt
```

## 10.3 Reference: Max Of Two

```ezc run
9 4 ovr ovr > ? prt
```

## 10.4 Reference: Sum 1..6

```ezc run
1 0
(
  ovr +
  swp 1 + swp
  ovr 7 <
) ^
swp del prt
```

## 10.5 Reference: Powers Of Two

```ezc run
1
(
  dup prt
  2 *
  dup 40 <
) ^
del
```

## 10.6 Reference: Conditional Text

```ezc run
"ok" "bad" 3 9 < ? prt
```

## 10.7 Next Steps

After these drills, implement your own:

- FizzBuzz-style emitter with `?` and `%`
- prime candidate filter for a small range
- one-pass mini calculator over stack literals

Keep each program centered on explicit stack state transitions.
