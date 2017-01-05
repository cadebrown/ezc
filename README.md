# EZC

[![Build Status](https://travis-ci.org/ChemicalDevelopment/ezc.svg?branch=master)](https://travis-ci.org/ChemicalDevelopment/ezc)

Tested platforms:
  * Debian 8
  * OSX (El Capitan)
  * Raspbian

A intermediate language which is transpiled to C. Multiprecision is built into the language.

Made for people who don't want to use low level memory management, but want the speed it provides.

Includes commandline utilities (such as `sqrt`, `mul`, `pow`, etc.)

You can think of it like a calculator language, but with more functions and more digits

[EZC Documentation](chemicaldevelopment.us/ezc/)

# Installation

If any errors are produced, please open an [issue](https://github.com/ChemicalDevelopment/ezc/issues)

## Global (default)

(this requires `sudo` rights)

To install, copypaste this into your terminal and then hit enter:

`curl chemdev.space/ezc.sh -L | bash`

This will prompt for your password, then finish in about 5-10 minutes

Now, you should be able to run `ezcc -h` from anywhere

## Local

if for some reason you don't have sudo rights, you can also install locally:

`curl chemdev.space/ezc-local.sh -L | bash`

EZC should be installed in `~/ezc/`


## Using a package manager

For some platforms (Debian Based, Fedora Based, OSX, FreeBSD) you can run:

`cd /tmp/ && curl https://github.com/ChemicalDevelopment/ezc/archive/master.zip -L > ezc.zip && unzip ezc.zip && cd ezc-master && make local-noreq`

This uses a package manager to download MPFR and GMP so that it doesn't need to be compiled. This will take about 15 - 20 seconds to install, but only for these platforms. If you use a different one, it defaults to the above option.

(**note for OSX users** this method requires [homebrew](http://brew.sh/) to install)

If any errors are produced, please open an [issue](https://github.com/ChemicalDevelopment/ezc/issues)

# Building

You just need `cc`, `python`, `curl`, and `git` (you can download zip as well)

First, clone this repository:

`git clone http://github.chemicaldevelopment.us/ezc`

Then, run `python src/ezcc.py -h` and assure that no errors were produced. If the were, please create and [Issue](https://github.com/ChemicalDevelopment/ezc/issues)

Now, run `make`

To test it, run `cd ezc`, then `./ezcc -c "var (sqrt 2)"` 

After this, run `./sqrt 2` and it should print out digits of square root of two (1.4142135623730...)


# Examples

To compute pi, simply run:

`./ezcc -c "var (pi)"`

or, 

`echo "var (pi)" | ./ezcc -e`


Using `-c` or `-e` means you don't need a file, but c reads from the next argument, and e reads from stdin

You can also use a shebang, namely:

`#!/usr/bin/ezc -runfile`

or, to run locally

`#!./ezcc -runfile`

See the `example.ezc` folder for a number of examples


# Environment variables

You can change the way EZC runs.

Run `export EZC_PREC=1000` to set the number of digits to 1000 by default.

Run `export EZC_DEG=1` to use degree mode for trig functions. To disable this, run `export EZC_DEG=0`

# Utilities

Commandline utilities are included, including all arithmetic (`add`, `sub`, etc.)

As well as basic trig and inverses (of `sin`, `cos`, and `tan`) and others like `fact` (factorial), `gamma`, `zeta`, `pi`, and `e`

You can specify the number of arguments these functions take in EZC, and an optional additional argument which specifies how many digits to compute.

For example, you can run `pi` in bash and it will print out 60 digits. You can run `pi 10000` to print out 10000 digits.

You can also link these through bash execution, so:

`sin $(pi)`

prints out `0.0000...`

If the number of digits is not present, it defaults to the length of the longest argument.

So, to find e + pi, simply run

`add $(e) $(pi)`

Or, to a thousand digits, 

`add $(e 1000) $(pi 1000)`

The list of all utilities is located in this repo in ./utils/

On your installed system, the compiled versions are listed in `/usr/bin/$UTIL`, and should be accessible with just running the name of the util (i.e. `sqrt 2`)

# Syntax highlighting

For Sublime text, use the included `ezc.tmLanguage` file.

For Visual Studio Code, run `CTRL+SHIFT+P` and type in `install extensions`. Search for `ezc`. Click install, and when you restart code, all .ezc files will have formatting

For any other text editor, look up how to install .tmLanguage files (most support tmLanguage)

## Documentation

[Chemical Development Docs](http://chemicaldevelopment.us/ezc/) is the documentation for EZC,

and tutorials are located at [docs tutorial](http://chemicaldevelopment.us/ezc/tutorials)
