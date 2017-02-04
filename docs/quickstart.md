# Quick Start

First, [install](./#/installing) EZC.

Now, there are a few ways to use ezc

## File

You can create files and compile those to runnable executables.

For example, open `~/Documents/`, and create a file called `test.ezc` inside it.

Now, write this:

``` bash
prec $1
x = prompt 'Enter a number'
printf 'The square root of your number is \n'
var (sqrt x)
```

<button class="btn" data-clipboard-text="prec $1
x = prompt 'Enter a number'
printf 'The square root of your number is \n'
var (sqrt x)">
    Copy to clipboard
</button>


Now, back in your terminal, make sure you are in the same directory as the file you've created, and run (when it asks for a number, type one and hit `Enter`)

``` bash
 $ ezcc test.ezc -o test.o -v5
   > EZCC
    > Version: X.X.X
    > Date: 20XX-01-26 21:12:21 -0500

  > Authors
    > Cade Brown <cade@cade.site>

  > Compiling
    > cc  /tmp/rpthB.c -lm  -lmpfr -lgmp  -o test.o

 $ ./test.o
 Enter a number: 2 <Enter>

 The square root of your number is 1.41421356237309504879
```

## Passing as an argument

You don't need a file to compile, use:

``` bash
 $ ezcc -c "var (sqrt 2)"
 1.41421356237309504879
```

This compiles whatever you put right after the `-c` argument, and then executes it, showing you the result.

This is good for quick prototyping.

# Utilities

First, [install utilities](/#/installing?id=utils)

Now, you should be able to run `add 2 3`, which results in `5.000...`

The full list of utils can be found [in the github repo](https://github.com/ChemicalDevelopment/ezc/tree/master/utils)

## Chaining

To chain together utilities, use shell substition. Wrap things in `$()` together.

For example: `add $(pi) $(e)` will add e and pi, resulting in `5.8598744...`

## Precision

You can also use a precision: `sqrt 2 1000` prints the square root of 2 to 1000 digits.

Simply append the number of digits after the last argument

If you'd like to set a default precision, run:

```bash
export EZC_PREC=100
```
<button class="btn" data-clipboard-text="export EZC_PREC=100">
    Copy to clipboard
</button>

