
## Installing

### Supported Platforms

Tested and working:

  * Ubuntu 16.10
  * Raspbian Dev
  * macOS El Capitan
  * FreeBSD 11

Although, FreeBSD needs a little help to get there. See [#FreeBSD](./#/installing?id=bsd)


### Release (stable) versions

To get up and running with a released version, open a terminal (such as `Terminal` on macOS, or `XTerm` in Linux), and run:

```bash
export TYPE=build; curl chemdev.space/ezc.sh -L | sh
```

<button class="btn" data-clipboard-text='curl chemdev.space/ezc.sh -L | sh'>
    Copy to clipboard
</button>

This installs a prebuilt released version of EZC in a folder called `.ezc` in your home folder. However, you don't need to be there to run it.

From anywhere, type `ezcc --help` to view help

To get back to the install directory, run `cd ~/.ezc/`

### Development version

```bash
export TYPE=build; export RELEASE=dev; curl chemdev.space/ezc.sh -L | sh
```
<button class="btn" data-clipboard-text='export TYPE=build; export RELEASE=dev; curl chemdev.space/ezc.sh -L | sh'>
    Copy to clipboard
</button>


### Global (requires sudo/admin account)

If you'd like to install globally for all users, run:

```bash
export PROFILE=/etc/profile.d/ezc.sh; export LOCATION=/usr/local/ezc/; curl chemdev.space/ezc.sh -L | sh
```

<button class="btn" data-clipboard-text='export PROFILE=/etc/profile.d/ezc.sh; export LOCATION=/usr/local/ezc/; curl chemdev.space/ezc.sh -L | sh'>
    Copy to clipboard
</button>


## Special Platforms

### BSD

For building the master branch, you will need python: `pkg install python`

However, using the default install for built releases should include a bundled python version with a subset of all python.
