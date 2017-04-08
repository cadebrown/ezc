# Workflow

If you've installed EZC, you may want to start running it outside of the terminal.

For this tutorial, I will use VSCODE.

You can get it [here](https://code.visualstudio.com/)

## Set up

### VSCODE

To set up your workflow, install VS Code.

Now, open a folder and create a file `test.ezc` in it. Press `Ctrl+Shift+B`.

Click on `Configure Task Runner`, and then search `Others`

Now, delete all of the lines in the generated file, and paste this in:

``` json
{
	"version": "0.1.0",
	"command": "ezcc",
	"isShellCommand": true,
	"args": ["-run", "${workspaceRoot}/test.ezc"],
	"showOutput": "always"
}
```
<button class="btn" data-clipboard-text='{
	"version": "0.1.0",
	"command": "ezcc",
	"isShellCommand": true,
	"args": ["-run", "${workspaceRoot}/test.ezc"],
	"showOutput": "always"
}'>
    Copy to clipboard
</button>

### Optional: syntax highlighting

If you'd like syntax highlighting, just open vscode, press `Ctrl+P`, and run:

```
ext install ezc
```

<button class="btn" data-clipboard-text='ext install ezc'>
    Copy to clipboard
</button>


## Running

Now, to run your file, just use `Ctrl+Shift+B`

Viola!

Caveats:

  * things that require input, like `prompt` do not work

