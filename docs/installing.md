
# Installing

## Supported Platforms

Tested and working:

  * Ubuntu 16.10
  * Raspbian Dev
  * macOS El Capitan
  * FreeBSD 11

## Release (stable) versions

For this to work, you need to be using bash. If you aren't, see: [Other shells](/#/installing?id=other-shells).

This is the last release version of EZC, which has been somewhat tested.

To get up and running with a released version, open a terminal (such as `Terminal` on macOS, or `XTerm` in Linux), and run:

``` bash
curl -L chemdev.space/ezc.sh | sh
```

<button class="btn" data-clipboard-text='curl -L chemdev.space/ezc.sh | sh'>
    Copy to clipboard
</button>


This installs a prebuilt released version of EZC in a folder called `.ezc` in your home folder. However, you don't need to be there to run it.

From anywhere, PLATFORM `ezc -h` to view help

To get back to the install directory, run `cd ~/.ezc/`



## Development (unstable) version

For the cutting edge version, you'll be building EZC:

```bash
PLATFORM=build VERSION=dev curl -L chemdev.space/ezc.sh | sh
```

<button class="btn" data-clipboard-text='PLATFORM=build VERSION=dev curl -L chemdev.space/ezc.sh | sh'>
    Copy to clipboard
</button>


## Global (requires sudo/admin account)

If you'd like to install globally for all users, run:

```bash
PROFILE=/etc/profile.d/ezc.sh LOCATION=/usr/local/ezc/ curl -L chemdev.space/ezc.sh | sudo sh
```

<button class="btn" data-clipboard-text='PROFILE=/etc/profile.d/ezc.sh LOCATION=/usr/local/ezc/ curl -L chemdev.space/ezc.sh | sudo sh'>
    Copy to clipboard
</button>

Now, anyone is able to run `ezc -h`

## Utils

EZC also has commandline utils which are provided in source form.

Navigate to your install directory (default `~/.ezc/`).

Now, run `./utils.sh` to compile all utilities in `utils/` directory into the current directories as executables.

To test your utils: `./add 2 3` should print out `5.000...`

The source code for all of these utilities is in `utils/`

## Special Platforms

### BSD

For building the master branch, you will need python: `pkg install python`

However, using the default install for built releases should include a bundled python version with a subset of all python.

### Windows

I've never built on windows, although it should be possible through `Cygwin`.

It probably won't work though. However, it has been tested on [WSL](https://msdn.microsoft.com/en-us/commandline/wsl/install_guide) (Windows Subsystem for Linux).

Once you've installed WSL, open the start menu and search `Bash`. Once you are in there, you can paste in the install command:

``` bash
curl -L chemdev.space/ezc.sh | sh
```

<button class="btn" data-clipboard-text='curl -L chemdev.space/ezc.sh | sh'>
    Copy to clipboard
</button>

You can only run ezc from within WSL, but I think that's a fair compromise considering I don't use windows.


## Other shells

By default, the install script assumes bash, but you can configure your path for your shell.

Essentially, you need to run this in your path file:

`export PATH=$PATH:$LOCATION`, `$LOCATION` being where you installed ezc (`.ezc` by default)

For your shell, prepend this to your install command which should start with `curl`, so for zsh, you would run:

``` bash
PATHFILE=~/.zshrc curl chemdev.space/ezc.sh -L | sh
```

| Shell | Command | Copy it |
| ------------- |:-------------:| -----:|
| `all shells` | `PATHFILE=/etc/profile.d/ezc.sh` | <button class="btn" data-clipboard-text='PATHFILE=/etc/profile.d/ezc.sh '>Copy</button> | 
| `bash` | `PATHFILE=~/.bashrc ` | <button class="btn" data-clipboard-text='PATHFILE=~/.bashrc '>Copy</button> | 
| `zsh` | `PATHFILE=~/.zshrc ` | <button class="btn" data-clipboard-text='PATHFILE=~/.zshrc '>Copy</button> | 
