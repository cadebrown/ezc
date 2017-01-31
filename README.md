# EZC

[![Build Status](https://travis-ci.org/ChemicalDevelopment/ezc.svg?branch=master)](https://travis-ci.org/ChemicalDevelopment/ezc)

To see usage and more, see [docs folder](./docs/), or the [docs website](http://chemicaldevelopment.us/ezc/)

This readme is more for developing and technical details

Tested platforms:
  * Debian 8
  * OSX (El Capitan)
  * Raspbian

A intermediate language which is transpiled to C. Multiprecision is built into the language.

Made for people who don't want to use low level memory management, but want the speed it provides.

Includes commandline utilities (such as `sqrt`, `mul`, `pow`, etc.)

You can think of it like a calculator language, but with more functions and more digits

[EZC Documentation](http://chemicaldevelopment.us/ezc/)

# Installation

If any errors are produced, please open an [issue](https://github.com/ChemicalDevelopment/ezc/issues)

Use the [installation doc page](http://chemicaldevelopment.us/ezc/installing) for most installing.


# Contributing

To contribute, clone the repo, then run:

`sudo make dev`

now, run `python src/ezcc.py example.py -run` to assure nothing is wrong.

Now, you are set up!


# Environment variables

You can change the way EZC runs.

Run `export EZC_PREC=1000` to set the number of digits to 1000 by default.

Run `export EZC_DEG=1` to use degree mode for trig functions. To disable this, run `export EZC_DEG=0`

# Syntax highlighting

For Sublime text, use the included `VSCODE-extension/ezc.tmLanguage` file.

For Visual Studio Code, run `CTRL+SHIFT+P` and type in `install extensions`. Search for `ezc`. Click install, and when you restart code, all .ezc files will have formatting

For any other text editor, look up how to install .tmLanguage files (most support tmLanguage)

## Documentation

[Chemical Development Docs](http://chemicaldevelopment.us/ezc/) is the documentation for EZC,

and tutorials are located at [docs tutorial](http://chemicaldevelopment.us/ezc/tutorials)

(coming soon)