# EZC

A intermediate language which is transpiled to C. Supports multiprecision easily.

Made for people who don't want to use low level memory management, but want the speed it provides.

[Chemical Development Docs](http://chemicaldevelopment.us/docs/ezc/) is the documentation for EZC

# Installation

First, clone this repository:

`git clone https://github.com/ChemicalDevelopment/ezc.git`

Then, run `./ezcc` and assure that no errors were produced. If the were, please create and [Issue](https://github.com/ChemicalDevelopment/ezc/issues)

Now, run `sudo ./install.sh`. If you get permissions errors, run `./install.sh ~/ezc/ none`. If you use the second one, when I use `ezcc`, just replace it with `~/ezc/ezcc`

To test it, run `ezcc examples/pi.ezc -o pi.o` (or `~/ezc/ezcc examples/pi.ezc -o pi.o`) 

After this, run `./pi.o 1024` and it should print out 1024 bits of pi (3.14159265358979...)


# Support

Tested on Ubuntu 15.04, should work for all Linux/Unix distros.

Not tested on OSX, but should work

# Documentation

[Chemical Development Docs](http://chemicaldevelopment.us/docs/ezc/) is the documentation for EZC

# Installation

The only things you'll need are `cc` (which most should have), `/usr/bin/python` (most should have), and `mpfr` (look up Installation for your operating system)

# Running

use it like: `./ezcc $file $file1 . . . -o $output`. Then, run `./$output`

# Structure

This is **not a general purpose language**

## Statements

Comments are denoted by `#`, and multiline comments look like:
```
###

Comment

###
```

[Chemical Development Docs](http://chemicaldevelopment.us/docs/ezc/) is the documentation for EZC

Check [examples](https://github.com/ChemicalDevelopment/ezc/tree/master/examples)

# Tutorials

[Chemical Development Docs](http://chemicaldevelopment.us/docs/ezc/) is the documentation for EZC

Coming soon!

