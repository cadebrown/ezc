# EZC

Current version: 3.0.0

A intermediate language which is transpiled to C. Multiprecision is built into the language.

Made for people who don't want to use low level memory management, but want the speed it provides.

Includes commandline utilities which are located in /usr/bin (such as `sqrt`, `mul`, `pow`, etc.)

You can think of it like a calculator language, but with more functions and more digits

[Chemical Development Docs](http://chemicaldevelopment.us/docs/ezc/) is the documentation for EZC

![Screenshot](screenshots/pi_basic.png)

# Installation

For all (*nix) systems (Debian, Ubuntu, Raspbian, OSX, FreeBSD, . . .) and maybe (cygwin, mingw, etc...), run `git clone http://github.chemicaldevelopment.us/ezc && cd ezc && make && source ~/.bashrc` in the terminal to install. This only requires some sort of C compiler.

If that fails, please open an [issue](https://github.com/ChemicalDevelopment/ezc/issues)

# Building

You just need `gcc` (or another c compiler)

For all OSs:

First, clone this repository:

`git clone https://github.com/ChemicalDevelopment/ezc.git`

Then, run `./ezcc.py` and assure that no errors were produced. If the were, please create and [Issue](https://github.com/ChemicalDevelopment/ezc/issues)

Now, run `sudo ./install-all.sh`. If you get permissions errors, run `./install-all.sh ~/ezc/bin/ ~/ezc/src/`. If you use the second one, when I use `ezcc`, just replace it with `~/ezc/bin/ezcc`

To test it, run `ezcc utils/sqrt -o sqrt` (or `~/ezc/bin/ezc utils/sqrt -o sqrt`) 

After this, run `./sqrt 2 1000` and it should print out 1000 digits of square root of two (1.4142135623730...)

# Environment variables

You can change the way EZC runs.

Run `export EZC_PREC=1000` to set the number of digits to 1000 by default.

Run `export EZC_DEG=1` to use degree mode for trig functions. To disable this, run `export EZC_DEG=0`

# Utilities

As of v3, commandline utilities are included, including all arithmetic (`add`, `sub`, etc.)

As well as basic trig and inverses (of `sin`, `cos`, and `tan`) and others like `fact` (factorial), `gamma`, `zeta`, `pi`, and `e`

You can specify the number of arguments these functions take in EZC, and an optional additional argument which specifies how many digits to compute.

For example, you can run `pi` in bash and it will print out 60 digits. You can run `pi 10000` to print out 10000 digits.

You can also link these through bash execution, so:

`sin $(pi)`

prints out `0.0000...`

If the number of digits is not present, it defaults to the length of the longest argument.

So, to find e + pi, simply run

`add $(e) $(pi)`

Or, to a million digits, 

`add $(e 1000) $(pi 1000)`

The list of all utilities is located in this repo in ./utils/

On your installed system, the compiled versions are listed in `/usr/bin/$UTIL`


# Examples

To compute pi, simply run:

`echo "i = acos -1 : var i" | ezcc -e`

or, 

`ezcc -c "i = acos -1 : var i"`

Using `-c` or `-e` means you don't need a file, but c reads from the next argument, and e reads from stdin

You can also use a shebang, namely:

`#!/bin/ezc -runfile`

or, to run locally

`#!./ezcc.py -runfile`

See the `examples` folder for a number of examples


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

use it like: `./ezcc.py $file $file1 . . . -o $output`. Then, run `./$output`

# Documentation

[Functions](http://chemicaldevelopment.us/docs/ezc/functions) for more about functions and statements

Check [examples](https://github.com/ChemicalDevelopment/ezc/tree/master/examples), 

or [docs examples](http://chemicaldevelopment.us/docs/ezc/examples) for the docs

# Tutorials

[Chemical Development Docs](http://chemicaldevelopment.us/docs/ezc/) is the documentation for EZC,

and a tutorial is located at [docs tutorial](http://chemicaldevelopment.us/docs/ezc/tutorials)
