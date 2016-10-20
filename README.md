# EZC

A intermediate language which is transpiled to C. Supports multiprecision easily.

Made for people who don't want to use low level memory management, but want the speed it provides.

# Support

Tested on Ubuntu 15.04, should work for all Linux/Unix distros.

Not tested on OSX, but should work

# Installation

The only things you'll need are `cc` (which most should have), `/usr/bin/python` (most should have), and `mpfr` (look up Installation for your operating system)

# Running

use it like: `./ezcc $file $file1 . . . -o $output`. Then, run `./$output`

# Structure

This is **not a general purpose language**

As of now, there are no methods, no string variables, only multiprecision floats.
## Statements

Comments are denoted by `#`, and multiline comments look like:
```
###

Comment

###
```

## Variables

Variables are always multiprecision floats

To set the precision for the whole file, use
`prec 1024`
or whatever number of bits to use (default is 128)

To initialize a variable, use:
`E = 2.718281828459045`

The length of digits is not limited by a double.

You can also copy a variable like:

`other_var = E`

## Methods

A method does not return anything. 

The most basic are: `echo`, and `var`

Use echo to print text, like

```
echo Hello World
> Hello World
```

Use var to print variable info (at full precision)

```
var E
> E = 2.718281828459045 . . .
```

## Operations

Operations are (at this point) take two operands, and store in the output

Operations are `+`, `-` ,`*`, `/`, `^`

You use it like:

`sum = sum + a`

It sets `sum` to `sum` + `a`


## Blocks (if, for, etc)

Blocks are denoted by their name, and the end is their name backwards. You can use `break` to escape at any time

Example:
```
if var1 < var2
    echo var1 Less than var2
fi
```

Currently, `if`, and `for` are supported

### If

You can use all comparing operaters (<, >, =<, >= ,==)

if example:
```
if a > b
    echo a is larger than b
fi
``` 

`for` is used in the following syntax:

```
for $variable START END [STEP]

rof
```

$variable is a new variable that has not been initialized, `STAR` and `END` and `STEP` can be constants (literally writing out digits, or they can be variable names)

if nothing is entered for `STEP`, it defaults to 1.0

You can use a higher `START` than `END`, and a negative step value.

so, 
```
for i 0.0 10.0

rof
```

and 
```
for i a b .1

rof
```

```
for i 30 -20 -1.0

rof
```

Are all valid

For more, see the wiki

## Examples

Check [examples](https://github.com/ChemicalDevelopment/ezc/tree/master/examples)

# Tutorials

Coming soon!
