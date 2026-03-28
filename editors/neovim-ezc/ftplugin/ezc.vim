" ftplugin/ezc.vim — buffer-local settings for EZC source files

if exists("b:did_ftplugin")
  finish
endif
let b:did_ftplugin = 1

setlocal commentstring=#\ %s
setlocal shiftwidth=2
setlocal tabstop=2
setlocal expandtab
setlocal iskeyword+=-,@-@

" Disable automatic comment continuation on newline
setlocal formatoptions-=r
setlocal formatoptions-=o
