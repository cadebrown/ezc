# Chapter 2: Values And Literals

EZC has a small value model. Keeping it small is intentional: simple values make stack flow predictable.

## 2.1 Integers

All numeric arithmetic in EZC is integer arithmetic.

```ezc run
20 6 / prt
```

```ezc run
20 6 % prt
```

## 2.2 Text Objects

Use either double quotes or single quotes.

```ezc run
"hello, world" prt
```

```ezc run
'single quoted text' prt
```

Both forms create text values; choose what reads best for your content.

## 2.3 Truthy And Falsy Basics

Many control words rely on truthiness.

- `0` is falsy
- non-zero integers are truthy
- empty text is falsy
- non-empty text is truthy

```ezc run
0 not prt
```

```ezc run
"" not prt
```

```ezc run
"ok" not prt
```

## 2.4 Symbolic Data In Stack Literals

Inside `[]`, bare words become symbol values instead of executable operators.

```ezc run
[alpha beta 7 "msg"] prt
```

This is useful for carrying labels and lightweight records as data.

## 2.5 Mixed Literals

Literals compose freely.

```ezc run
[42 "answer" [x y] (1 2 +)] prt
```

The block inside the stack literal is stored as data until explicitly executed.

## Chapter Checkpoint

You should now be comfortable with:

- numeric and text literal syntax
- truthiness intuition for condition-building
- symbolic values through stack literals
