# EZC

## What is EZC?

EZC is an RPN language that supports multiprecision, a live interpereter, and more all without the end user needing to know memory management. EZC also includes lots of mathematical functions, even at high precisions (using MPFR).


Online documentation: http://chemicaldevelopment.us/ezc/


## INSTALLING

### Release

These are found from the releases page, and can be installed like this (uses [cmake](https://cmake.org/runningcmake/) as a buildsystem):

```
mkdir build && cd build
cmake ..
make -j4
make install
```
(you may need to run `sudo make install` if you need priviledges to install in `/usr/local`).


### Development Versions

Just clone this repo:

`git clone https://github.com/ChemicalDevelopment/ezc.git`, then follow the `Release` installation.


## Usage

Run the program to view help:

`ec -h`

If you are building from a `build/` directory, replace `ec` with `./ezc/ec`

And it will output the help message.

## Examples

Here are some quick examples:

```
 $ ec -e'cade is a boss'

final results
-------------
globals[0]:
stack[4]:
  0: 'cade':str
  1: 'is':str
  2: 'a':str
  3: 'boss':str

```

Use a function like `FUNCTION!`

```
 $ ec -e'cade is a boss concat! dump!'

globals[0]:
stack[3]:
  0: 'cade':str
  1: 'is':str
  2: 'aboss':str

```

Some functions are implemented as repeaters, using `FUNCTION&!` (which will repeatedly call the function).

```
 $ ec -e'cade is a boss concat&! dump!'

globals[0]:
stack[1]:
  0: 'cadeisaboss':str

```


## TODOs
				 
  * Add checks


## AUTHORS

  * Cade Brown <cade@cade.site>

