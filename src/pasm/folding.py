###                     EZC src/pasm/folding.py v@VERSION@
#
#  EZC is free software; you are free to modify and/or redistribute it under the terms of the GNU GPLv3. See 'LICENSE' for more details.
#
#  Folds (i.e. evaluates before compile time) simple operations.
#
#  TODO:
#
#
###

import pasm
from fractions import gcd

def isint(val, bits=pasm.MAX_WORD_BITS):
    try:
        x = int(val)
        return x < 2 ** bits
    except:
        return False

def PLUS(*args):
    if len(args) == 2:
        if isint(args[0]) and isint(args[1]):
            tres = int(args[0]) + int(args[1])
            if isint(tres):
                return tres
    return None

def MINUS(*args):
    if len(args) == 2:
        if isint(args[0]) and isint(args[1]):
            tres = int(args[0]) - int(args[1])
            if isint(tres):
                return tres
    return None

def MUL(*args):
    if len(args) == 2:
        if isint(args[0]) and isint(args[1]):
            tres = int(args[0]) * int(args[1])
            if isint(tres):
                return tres
    return None

def DIV(*args):
    if len(args) == 2:
        if isint(args[0]) and isint(args[1]) and gcd(args[0], args[1]) == args[1]:
            tres = int(args[0]) / int(args[1])
            if isint(tres):
                return tres
    return None


fold_func = {
    "PLUS": PLUS,
    "MINUS": MINUS,
    "MUL": MUL,
    "DIV": DIV
}

