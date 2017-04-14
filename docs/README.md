# EZC 1.4.1

## What is EZC?

EZC is programming language meant to enable fast calculations of multiprecision floats, but with RPN syntax, and without memory management.

It is sort of like a calculator language, but with for, if, function definitions, and more advanced features.

The main advantage of EZC is the ability to change the precision, so the programmer is not limited to 64 bit, or roughly 20 digit double.

Also, the EZC std libraries have many functions dealing with more advanced functionality than even `math.h` or most system math libraries have.

EZC supports all trigonometric functions to full precision, many obscure functions (Gamma, Zeta, Bessel, etc), as well as extensions of common functions to all real numbers, such as factorial (via Gamma function).


### What can it do?

It can calculate sums and equations very fast.

For example, to calculate the square root of 2 to 100000000 digits (100 million, or 10^8)

 * EZC takes 10.25 s
 * python's Decimal class takes so long it wont finish
 * python package mpmath takes at least 2 minutes
 * gmpy2 seems to outperform at roughly 9.6 s

The base for EZC is written in C, and in fact, EZC is translated to C then compiled. Additionally, you can write c code and interface with that in EZC.

Also, many custom functions, such as binomcdf, normalcdf, etc are implemented in EZC.


### Who is it for?

This is primarily for programmers who want to [fold](https://en.wikipedia.org/wiki/Constant_folding) constants or equations, match correlations, or use RPN.

This is an easy way to prototype a summation, and then see if it matches up with your existing data, or even calculate a billion digits of a constant.

This language isn't for everyone. It is meant to be easy, but it is not meant for GUIs, elaborate printing, writing libraries in, or anything like that. It is meant to be a calculator like language which is performant, and easy to distribute on any operating system with python and a c compiler.


## How can I use it?

Well, just follow the [quick start guide](./#/quickstart), and go along!

It is mainly terminal based, but I plan to make an IDE in the future.

