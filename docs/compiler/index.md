---
layout: docs
title: EZC Compiler Usage
---

If you have the github repo, you will run `python src/ezcc.py` for the compiler, but for built versions (and releases), use `./ezcc`

To view help, run `python src/ezcc.py -h` or `./ezcc -h`

That should print out something like:

```
usage: ezcc.py [-h] [-o O] [-v V] [-tmp TMP] [-cc CC] [-ccargs CCARGS]
               [-args ARGS] [-run] [-runfile] [-c C] [-e]
               [files [files ...]]

Compile EZC Language. vX.X.X http://github.chemicaldevelopment.us/ezc

positional arguments:
  files           files to compile

optional arguments:
 . . . 
```

So, you can run `python src/ezcc.py file.ezc -o file.out -v5 -cc gcc -run -args "1 2 3"` or `./ezcc file.ezc -o file.out -v5 -cc gcc -run -args "1 2 3"` to:

  * Compile file.ezc to executable file.out, with verbosity level 5, using `gcc`
  * Run the executable file.out with args 1 2 3, like `./file.out 1 2 3`


Or, just compile without running: `python src/ezcc.py file.ezc`

(this saves into `a.o` by default)

If you run into any difficulties, open an [issue](https://github.com/ChemicalDevelopment/ezc/issues) on github so that I can respond
