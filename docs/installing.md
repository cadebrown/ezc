
## Installing

### Release (stable) versions

To get up and running with a released version, open a terminal (such as `Terminal` on macOS, or `XTerm` in Linux), and run:

```bash
LOCATION=~/.ezc/
mkdir $LOCATION; cd $LOCATION
curl chemdev.space/ezc.sh -L | bash; cd ezc
echo "export PATH=\$PATH:$LOCATION" >> ~/.bashrc; source ~/.bashrc
```

<button class="btn" data-clipboard-text='LOCATION=~/.ezc/; mkdir $LOCATION; cd $LOCATION; curl chemdev.space/ezc.sh -L | bash; cd ezc; echo "export PATH=\$PATH:$LOCATION" >> ~/.bashrc; source ~/.bashrc'>
    Copy to clipboard
</button>

This installs a prebuilt released version of EZC in a folder called `.ezc` in your home folder. However, you don't need to be there to run it.

From anywhere, type `ezcc --help` to view help

To get back to the install directory, run `cd ~/.ezc/`

### Development version

To install the current version (most recent) that is unstable, and probably has bugs, you'll visit the [travis CI site](https://travis-ci.org/ChemicalDevelopment/ezc)

Look under `Build Jobs`, and click your platform.

Now, scroll down in the page until it says 

```bash
Download the file here:
https://transfer.sh/XXXXX/ezc-PLATFORM.tar.xz
```

Copy that link, and then type:

```bash
LINK="PASTEHERE"
cd ~; curl $LINK > ezc.tar.xz; untar ezc.tar.xz; mv ezc .ezc; cd .ezc
```

<button class="btn" data-clipboard-text='LINK="PASTEHERE"; cd ~; curl $LINK > ezc.tar.xz; tar xfv ezc.tar.xz; mv ezc .ezc; cd .ezc'>
    Copy to clipboard
</button>


replacing `PASTEHERE` with your link you copied.


### Global (requires sudo/admin account)

If you'd like to install globally for all users, run:

```bash
curl chemdev.space/build-ezc-global.sh -L | bash
```

<button class="btn" data-clipboard-text='curl chemdev.space/build-ezc-global.sh -L | bash'>
    Copy to clipboard
</button>


This will build the development branch for all users, and will also build MPFR and GMP from source.
