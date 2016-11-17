# EZC

A intermediate language which is transpiled to C. Multiprecision is built into the language.

Made for people who don't want to use low level memory management, but want the speed it provides.

[Chemical Development Docs](http://chemicaldevelopment.us/docs/ezc/) is the documentation for EZC


[Screenshot](Screenshots/pi_basic.png)

# Installation

For any debian based system, simply go to the [latest release](https://github.com/ChemicalDevelopment/ezc/Releases/latest), and download the .deb file, and double click it

For other OSs, I'm working on packaging systems. For now, you can build them following below:

# Building

You'll need a few prerequisites: `cc`, and `mpfr`

For Debian/Ubuntu/MacOS, run `./install.sh`

For all OSs:

First, clone this repository:

`git clone https://github.com/ChemicalDevelopment/ezc.git`

Then, run `./ezcc` and assure that no errors were produced. If the were, please create and [Issue](https://github.com/ChemicalDevelopment/ezc/issues)

Now, run `sudo ./install-compiler.sh`. If you get permissions errors, run `./install-compiler.sh ~/ezc/ none`. If you use the second one, when I use `ezcc`, just replace it with `~/ezc/ezcc`

To test it, run `ezcc examples/pi.ezc -o pi.o` (or `~/ezc/ezcc examples/pi.ezc -o pi.o`) 

After this, run `./pi.o 1024` and it should print out 1024 bits of pi (3.14159265358979...)


# Support

Tested on Ubuntu 15.04, should work for all Linux/Unix distros.

Works on OSX

# Syntax highlighting

For Sublime text, use the included `ezc.tmLanguage` file.

For Visual Studio Code, run `CTRL+SHIFT+P` and type in `install extensions`. Search for `ezc`. Click install, and when you restart code, all .ezc files will have formatting

For any other text editor, look up how to install .tmLanguage files (most support tmLanguage)

# Documentation

[Chemical Development Docs](http://chemicaldevelopment.us/docs/ezc/) is the documentation for EZC

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

[Functions](http://chemicaldevelopment.us/docs/ezc/functions) for more about functions and statements

Check [examples](https://github.com/ChemicalDevelopment/ezc/tree/master/examples), 

or [docs examples](http://chemicaldevelopment.us/docs/ezc/examples) for the docs

# Tutorials

[Chemical Development Docs](http://chemicaldevelopment.us/docs/ezc/) is the documentation for EZC,

and a tutorial is located at [docs tutorial](http://chemicaldevelopment.us/docs/ezc/tutorials)
