###                     EZC src/ezlogging/exc.py v@VERSION@
#
#  EZC is free software; you are free to modify and/or redistribute it under the terms of the GNU GPLv3. See 'LICENSE' for more details.
#
#  Handles common exceptions and helpful error messages
#
#  TODO:
#    * More common problems
#
###

import ezlogging
import ezcompiler as ec

desc = {
    ec.CONSTANT: "numeric constant",

    ec.COMMA: "separator (,)",

	ec.ASSIGN: "assignment (=)",

	ec.ADD: "operator (+)",
	ec.SUB: "operator (-)",

	ec.MUL: "operator (*)",
	ec.DIV: "operator (/)",

    ec.ID: "variable or function",
    ec.LPAREN: "begin group",
    ec.RPAREN: "end group",
    ec.EOF: "EOF (End Of File)",
    ec.FUNCTION: "function call",
    ec.UNKNOWN: "unknown symbol",
    None: "nothing",
}

special_expect_cases = {
    ec.COMMA: ["Try adding a comma between arguments."],
    ec.EOF: ["Try checking for extra characters on the end of the line."],
	ec.ASSIGN: ["Did you enter an expression on the left side?"],
}

special_got_cases = {
    ec.COMMA: ["Make sure this is inside of a function call.", "Are there two commas in a row?"],
    ec.ASSIGN: ["Make sure you don't have two assignments in the same."],
	ec.RPAREN: ["Make sure your parentheses are balanced."],
    ec.UNKNOWN: ["Try deleting this symbol."],
    ec.EOF: ["Make sure this line is complete."],
}

class NameNotFound(NameError):
    def __init__(self, val):
        super(NameError, self).__init__(val)

class InvalidCharacter(Exception):
    def __init__(self, col, _len, text, got=None, expect=None):
        ncolor = ezlogging.RESET + ezlogging.CYAN
        color = ezlogging.RESET + ezlogging.RED + ezlogging.BOLD + ezlogging.INVERT
        ogot = got
        oexpect = expect
        hcol = ezlogging.RESET + ezlogging.BOLD + ezlogging.BLUE
        expectcol = ezlogging.RESET + ezlogging.GREEN
        gotcol = ezlogging.RESET + ezlogging.LRED
        rs = []
        if col == len(text):
            rs.append(ncolor + text + color + " " + ezlogging.RESET)
        else:
            rs.append(ncolor + text[:col] + color + text[col:col+_len] + ncolor + text[col+_len:])
        rs.append(" " * (col) + color + "^" + ezlogging.RESET + " " * (len(text) - col))
        if got or expect:
            if oexpect in desc:
                expect = desc[oexpect]
            if ogot in desc:
                got = desc[ogot]
            rs.append("Expected {2}{0}{4}, but got {3}{1}{4}".format(expect, got, expectcol, gotcol, ezlogging.RESET))
            if ogot in special_got_cases:
                for hint in special_got_cases[ogot]:
                    rs.append("{1}[hint]:{2} {0}".format(hint, hcol, ncolor))
            if oexpect in special_expect_cases:
                for hint in special_expect_cases[oexpect]:
                    rs.append("{1}[hint]:{2} {0}".format(hint, hcol, ncolor))
        super(Exception, self).__init__("".join(["\n{0}".format(r) for r in rs]))
