
# ezc

`EZC` is an [RPN (Reverse Polish Notation)](https://en.wikipedia.org/wiki/Reverse_Polish_notation), stack-based language which emphasizes terseness, compactness, and innovative problem solving techniques. 

For example, [Euclid's GCD Algorithm](https://en.wikipedia.org/wiki/Euclidean_algorithm) is: `{:0== {\`} {<>_% gcd!} ifel!} gcd funcdef!`

Every function just operates on the stack, which is both dangerous, and useful at times. See more in the [examples folder](https://github.com/ChemicalDevelopment/ezc/tree/master/examples).

The code to print the square of a number is: `N 2^print!`, i.e. `5 2^print!` prints `25` to the screen

More complicated expressions as well, like `2 3 4*+ print!` results in `14`


## Building

To build `ezc`, just clone this [repo](https://github.com/chemicaldevelopment/ezc), or download a [release](https://github.com/ChemicalDevelopment/ezc/releases).

Then, `cd ezc`, or into the folder you downloaded it into.

Then `make`. This should build the `ec` binary in the `ec/` directory.

This can be executed like: `./ec/ec -h` for help, or run it with an expression like `./ec/ec -e '2 3+ print!'`, to test it out. That example should print out `5`.

To install, run `sudo make install`. By default, this installs to `/usr/local`, so you can now run `/usr/local/bin/ec`, or, provided your `PATH` variable is sane, just run `ec` in your shell.


## VSCode Extension

You can install the extension for vscode, by visiting: [https://marketplace.visualstudio.com/items?itemName=chemdev.ezc](https://marketplace.visualstudio.com/items?itemName=chemdev.ezc).

