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
import math

def isint(val, bits=pasm.MAX_WORD_BITS, signed=False):
    try:
        x = int(val)
        if signed:
            return x < 2 ** (bits - 1) and x > - (2 ** (bits - 1))
        else:
            return x < 2 ** bits
    except:
        return False

def ADD(*args):
    if len(args) == 2:
        if isint(args[0]) and isint(args[1]):
            tres = int(args[0]) + int(args[1])
            if isint(tres):
                return tres
    return None

def U_ADD(*args):
    if len(args) == 1:
        if isint(args[0]):
            tres = +int(args[0])
            if isint(tres):
                return tres
    return None


def SUB(*args):
    if len(args) == 2:
        if isint(args[0]) and isint(args[1]):
            tres = int(args[0]) - int(args[1])
            if isint(tres):
                return tres
    return None

def U_SUB(*args):
    if len(args) == 1:
        if isint(args[0]):
            tres = -int(args[0])
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

def F_sqrt(*args):
    if len(args) == 1:
        if isint(args[0]):
            tres = int(math.sqrt(int(args[0])))
            if isint(tres) and tres ** 2 == int(args[0]):
                return tres
    return None



fold_func = {
    "ADD": ADD,
    "U_ADD": U_ADD,
    "SUB": SUB,
    "U_SUB": U_SUB,
    "MUL": MUL,
    "DIV": DIV,
    "F_sqrt": F_sqrt
}

