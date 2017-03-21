###                     EZC src/ezcompiler/__init__.py v@VERSION@
#
#  EZC is free software; you are free to modify and/or redistribute it under the terms of the GNU GPLv3. See 'LICENSE' for more details.
#
#  Keywords, tokens, and the like for EZC
#
#  TODO:
#    * Add to desc[] to print out more helpful messages
#
#
###

import ezlogging
from token import Token

UNKNOWN             = 'UNKNOWN'
CONSTANT            = 'CONSTANT'
STRING              = 'STRING'
COMMA               = 'COMMA'
FUNCTION            = 'FUNCTION'
ADD                 = 'ADD'
SUB                 = 'SUB'
MUL                 = 'MUL'
DIV                 = 'DIV'
LPAREN              = '('
RPAREN              = ')'
ID                  = 'ID'
ASSIGN              = 'ASSIGN'
BEGIN               = 'BEGIN'
END                 = 'END'
SEMI                = 'SEMI'
DOT                 = 'DOT'
EOF                 = 'EOF'

RESERVED_KEYWORDS = {
    #'function': Token('FUNCTION', 'FUNCTION')
}


desc = {
    "ID": "variable or function",
    ")": "end group",
    "(": "begin group",
    "EOF": "EOF (End Of File)",
    "FUNCTION": "function call",
    None: "nothing",
    "UNKNOWN": "unknown symbol",
}

class NameNotFound(NameError):
    def __init__(self, val):
        super(NameError, self).__init__(val)

class InvalidCharacter(Exception):
    def __init__(self, col, text, got=None, expect=None):
        ncolor = ezlogging.RESET + ezlogging.CYAN
        color = ezlogging.RESET + ezlogging.RED + ezlogging.BOLD + ezlogging.INVERT
        expectcol = ezlogging.RESET + ezlogging.GREEN
        gotcol = ezlogging.RESET + ezlogging.LRED
        rs = []
        if col == len(text):
            rs.append(ncolor + text + color + " " + ezlogging.RESET)
        else:
            rs.append(ncolor + text[:col] + color + text[col] + ncolor + text[col+1:])
        rs.append(" " * (col) + color + "^" + ezlogging.RESET + " " * (len(text) - col))
        if got or expect:
            if expect in desc:
                expect = desc[expect]
            if got in desc:
                got = desc[got]
            rs.append("Expected {2}{0}{4}, but got {3}{1}{4}".format(expect, got, expectcol, gotcol, ezlogging.RESET))
        super(Exception, self).__init__("".join(["\n{0}".format(r) for r in rs]))
