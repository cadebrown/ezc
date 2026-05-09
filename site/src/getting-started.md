# Getting started

## Install

ezc is a Rust project. Build from source:

```bash
git clone https://github.com/cadebrown/ezc
cd ezc
cargo install --path crates/ezc-cli
```

This installs the `ezc` binary. Verify:

```bash
ezc -e "3 4 +"
# → 7
```

## Your first program

Create `hello.ezc`:

```ezc
"Hello, ezc!" wl
```

Run it:

```bash
ezc hello.ezc
```

`wl` writes a string with a newline. The string is pushed onto the stack,
then `wl` pops and prints it.

You can also evaluate a string directly:

```bash
ezc -e "3 4 +"
# → 7
```

Or check for syntax errors without running:

```bash
ezc -c hello.ezc
```

## The REPL

Run `ezc` with no arguments to start the interactive REPL:

```bash
ezc
```

You'll see the stack update after each line:

```
> 3 4 +
[7]
> 2 *
[14]
```

Press `Ctrl+D` to exit. Use `--no-tui` for a plain reedline interface
without the boxed UI.

## Editor support

ezc ships with first-class support for VS Code, Neovim, Zed, and Helix.
See the [editor setup](editors.md) page for installation instructions.

## Next steps

- [**Stack basics**](tutorial/basics.md) — push, pop, operators
- [**Variables**](tutorial/variables.md) — `@bind` and `$recall`
- [**Functions**](tutorial/functions.md) — blocks, `!`, defining your own
