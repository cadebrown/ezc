# Workflow

If you've installed EZC, you will want to start running it outside of the terminal.

For this tutorial, I will use VSCODE.

You can get it [here](https://code.visualstudio.com/)

## Set up

To set up your workflow, install VS Code.

Now, open your folder with `test.ezc` in it. Press `Ctrl+Shift+B`.

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

Now, to run your file, just use `Ctrl+Shift+B`


