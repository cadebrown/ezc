# EZC

## What is EZC?

EZC is an RPN language that supports multiprecision, a live interpereter, and more all without the end user needing to know memory management. EZC also includes lots of mathematical functions, even at high precisions (using MPFR).


Online documentation: http://chemicaldevelopment.us/ezc/


## INSTALLING

### Release

These are found from the releases page, and can be installed like this:

```
./configure
make
make install
```
(you may need to run `sudo make install` if you need priviledges to install in `/usr/local`).


Here are some more options:

```
./configure --with-gmp[=LOC] --prefix=PREFIX
make
make install
```

Where `[=LOC]` is optional, and points to the path where you installed gmp (`/usr/local` if you don't enter anything).

PREFIX is where the program installs to. So, if you run with `--prefix=$HOME`, you can now run `$HOME/bin/ec` to run the program.

### Development Versions

You will need automake/autoconf/etc (you probably already have this)

If you have these, you will run `autoreconf -i`, then follow the steps for a release (above).


## Usage

Run the program to view help:

` $ ec -h `

And it will output the help message.


## TODOs
				 
  * Add MPFR build with, and add mpfr (using `F` suffix), and then implement MPFR functions in plain C (for floats and ints), or perhaps cast those to mpfrs.
  * Add more functions to both int, and gmp_mpz s.
  * Add a manual, and update the docs site for new EZC


## AUTHORS

  * Cade Brown <cade@cade.site>

