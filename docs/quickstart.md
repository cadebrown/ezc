# Quick Start

First, [install](./#/installing) EZC.

Now, there are a few ways to use ezc

## File

You can create files and then run those files

For example, open `~/Documents/`, and create a file called `test.ezc` inside it.

Now, write this:

``` bash
'Square root of 2: ' print
2 sqrt print
```

<button class="btn" data-clipboard-text="'Square root of 2: ' print
2 sqrt print"
</button>


Now, back in your terminal, make sure you are in the same directory as the file you've created, and run (when it asks for a number, type one and hit `Enter`)

``` bash
 $ ec -f test.ezc

 Square root of 2: 1.41421356237309504879
```

## Passing as an argument

You don't need a file to run, use:

``` bash
 $ ec -e "2 sqrt"
 1.41421356237309504879
```

This compiles whatever you put right after the `-e` argument, and then executes it, showing you the result.

This is good for quick prototyping, but you can also compute and see results in realtime:

## Running an interactive shell

You can run an interactive shell as well:

``` bash
  $ ec -

  > 
```

Now, you can enter expressions, functions, and run `q` or `quit` to end.

