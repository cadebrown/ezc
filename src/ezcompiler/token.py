###                     EZC src/ezcompiler/token.py v@VERSION@
#
#  EZC is free software; you are free to modify and/or redistribute it under the terms of the GNU GPLv3. See 'LICENSE' for more details.
#
#  Simple definitions for the lexer/parser. AST definitions
#
#  TODO:
#
#
###

class Token(object):
    def __init__(self, type, value, char_at=-1):
        self.type = type
        self.value = value
        self.char_at = char_at

    def __str__(self):
        return 'Token({1}, {1})'.format(self.type, repr(self.value))

    def __repr__(self):
        return self.__str__()

class AST(object):
    pass


class BinOp(AST):
    def __init__(self, left, op, right):
        self.left = left
        self.token = self.op = op
        self.right = right


class Num(AST):
    def __init__(self, token):
        self.token = token
        self.value = token.value


class UnaryOp(AST):
    def __init__(self, op, expr):
        self.token = self.op = op
        self.expr = expr


class Compound(AST):
    def __init__(self):
        self.children = []


class Assign(AST):
    def __init__(self, left, op, right):
        self.left = left
        self.token = self.op = op
        self.right = right


class Var(AST):
    def __init__(self, token):
        self.token = token
        self.value = token.value

class String(AST):
    def __init__(self, token):
        self.token = token
        self.value = token.value


class Function(AST):
    def __init__(self, token, args):
        self.token = token
        self.value = token.value
        self.args = args


class NoOp(AST):
    pass
