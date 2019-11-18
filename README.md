
# ezc

`EZC` is an [RPN (Reverse Polish Notation)](https://en.wikipedia.org/wiki/Reverse_Polish_notation), stack-based language which emphasizes terseness, compactness, and innovative problem solving techniques. 

For example, [Euclid's GCD Algorithm](https://en.wikipedia.org/wiki/Euclidean_algorithm) is: `{:0== {del!} {<>_% gcd!} ifel!} gcd funcdef!`

Parentheses are invalid characters in this language, as they are never needed for grouping expressions, due to the nature of RPN.



The code to print the square of a number is: `N 2^print!`, i.e. `5 2^print!` prints `25` to the screen

More complicated expressions as well, like `2 3 4*+ print!` results in `14`



## Building

To build `ezc`, just clone this [repo](https://github.com/chemicaldevelopment/ezc), or download a [release](https://github.com/ChemicalDevelopment/ezc/releases).

Then, `cd ezc`, or into the folder you downloaded it into.

Then `make`. This should build the `ec` binary in the `ec/` directory.

This can be executed like: `./ec/ec -h` for help, or run it with an expression like `./ec/ec -e '2 3+ print!'`, to test it out. That example should print out `5`.

In the future, you'll be able to run `make check`, but I haven't implemented this yet.





