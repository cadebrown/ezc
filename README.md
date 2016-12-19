# EZC

[![Build Status](https://travis-ci.org/ChemicalDevelopment/ezc.svg?branch=master)](https://travis-ci.org/ChemicalDevelopment/ezc)

Current version: 3.0.0

A intermediate language which is transpiled to C. Multiprecision is built into the language.

Made for people who don't want to use low level memory management, but want the speed it provides.

Includes commandline utilities which are located in /usr/bin (such as `sqrt`, `mul`, `pow`, etc.)

You can think of it like a calculator language, but with more functions and more digits

[Chemical Development Docs](http://chemicaldevelopment.us/docs/ezc/) is the documentation for EZC

![Screenshot](screenshots/pi_basic.png)

# Installation

## Global (default)

(this requires `sudo` rights)

This should work for all systems with `curl`, `cc`, and `make`, and has been tested on Ubuntu15/16, OSX (El Capitan), Debian 8, Raspbian (Raspberry pi), and confirmed working.

To install, run copypaste this into your terminal and then hit enter:

`cd /tmp/ && curl https://github.com/ChemicalDevelopment/ezc/archive/master.zip -L > ezc.zip && unzip ezc.zip && cd ezc-master && sudo make global`

This will prompt for your password, then finish in about 5-10 minutes

After it has finished, you can test and make sure that it worked by running: `ezcc --help` (to view the compiler help messages), and `pi 10000` (for 10000 digits of pi).

If any errors are produced, please open an [issue](https://github.com/ChemicalDevelopment/ezc/issues)

## Local

if for some reason you don't have sudo rights, you can also install locally:

`cd /tmp/ && curl https://github.com/ChemicalDevelopment/ezc/archive/master.zip -L > ezc.zip && unzip ezc.zip && cd ezc-master && make` (this will take about 5-10 minutes)

go in the install directory (`cd ~/ezc/`), run `ls` to view all the programs associated with it.

Run `./ezcc --help` (to view compiler help), and `./pi 10000` (for 10000 digits of pi)

If any errors are produced, please open an [issue](https://github.com/ChemicalDevelopment/ezc/issues)


## Using a package manager

For some platforms (Debian Based, Fedora Based, OSX, FreeBSD) you can run:

`cd /tmp/ && curl https://github.com/ChemicalDevelopment/ezc/archive/master.zip -L > ezc.zip && unzip ezc.zip && cd ezc-master && make local-noreq`

This uses a package manager to download MPFR and GMP so that it doesn't need to be compiled. This will take about 15 - 20 seconds to install, but only for these platforms. If you use a different one, it defaults to the above option.

(**note for OSX users** this method requires [homebrew](http://brew.sh/) to install)

If any errors are produced, please open an [issue](https://github.com/ChemicalDevelopment/ezc/issues)

# Building

You just need `cc` (or another c compiler), and `git` (you can download zip as well)

For all OSs:

First, clone this repository:

`git clone https://github.com/ChemicalDevelopment/ezc.git`

Then, run `./ezcc.py -h` and assure that no errors were produced. If the were, please create and [Issue](https://github.com/ChemicalDevelopment/ezc/issues)

Now, run `sudo make global`. If you get permissions errors, run `make`

(if you use the second one, run `cd ~/ezc/`, and when I run `ezcc`, you run `./ezcc`. When I run anything, you just add `./` before it)

To test it, run `ezcc -c "var (sqrt 2)"` (or `./ezcc -c "var (sqrt 2)`) 

After this, run `sqrt 2` and it should print out digits of square root of two (1.4142135623730...), and it should be equivelant to your output with `ezcc`


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

Or, to a thousand digits, 

`add $(e 1000) $(pi 1000)`

The list of all utilities is located in this repo in ./utils/

On your installed system, the compiled versions are listed in `/usr/bin/$UTIL`, and should be accessible with just running the name of the util (i.e. `sqrt 2`)


# Examples

To compute pi, simply run:

`ezcc -c "var (pi)"`

or, 

`echo "var (pi)" | ezcc -e`


Using `-c` or `-e` means you don't need a file, but c reads from the next argument, and e reads from stdin

You can also use a shebang, namely:

`#!/usr/bin/ezc -runfile`

or, to run locally

`#!./ezcc.py -runfile`

See the `examples` folder for a number of examples


# Syntax highlighting

For Sublime text, use the included `ezc.tmLanguage` file.

For Visual Studio Code, run `CTRL+SHIFT+P` and type in `install extensions`. Search for `ezc`. Click install, and when you restart code, all .ezc files will have formatting

For any other text editor, look up how to install .tmLanguage files (most support tmLanguage)

## Documentation

[Chemical Development Docs](http://chemicaldevelopment.us/docs/ezc/) is the documentation for EZC,

and a tutorial is located at [docs tutorial](http://chemicaldevelopment.us/docs/ezc/tutorials)
