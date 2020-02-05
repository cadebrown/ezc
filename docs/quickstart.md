# Quick Start

First, [install](./#/installing) EZC.

Now, there are a few ways to use ezc

## File

You can create files and then run those files

For example, open `~/Documents/`, and create a file called `test.ezc` inside it.

Now, write this:

``` bash
"11^2: " print!
11 2^ print!
```

Now, back in your terminal, make sure you are in the same directory as the file you've created, and run (when it asks for a number, type one and hit `Enter`)

``` bash
$ ec test.ezc

11^2:
121
```

If you are running without installing EZC, you need the entire path, i.e.:

``` bash
$ ../ezc/ec/ec test.ezc

11^2:
121
```


You don't need a file to run, use:

``` bash
$ ec -e "1 2+"

3
```

This compiles whatever you put right after the `-e` argument, and then executes it, showing you the result.

This is good for quick prototyping, but you can also compute and see results in realtime:

## Running an interactive shell

You can run an interactive shell as well:

``` bash
$ ec -i

 %> 
```

Now, you can enter expressions, functions, and run `exit!` to end

