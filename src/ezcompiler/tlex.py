###                     EZC src/ezcompiler/tlex.py v@VERSION@
#
#  EZC is free software; you are free to modify and/or redistribute it under the terms of the GNU GPLv3. See 'LICENSE' for more details.
#
#  Contains lexer utilities used by parser.py to generate ASTs.
#
#  TODO:
#    * Parse functions
#    * add peek(n) to non-destructively check next few characters
#    * Add custom exceptions, and use old exception format (showing which char): https://github.com/ChemicalDevelopment/ezc/blob/f125dacaee53ddcb97b6f1c4cf0c805dde925f12/src/parsing/__init__.py
#
###

import ezcompiler
from token import Token

class Lexer(object):
    def __init__(self, text):
        self.text = text
        self.pos = 0
        self.current_char = self.text[self.pos]

    def error(self):
        raise Exception('Invalid character')

    def advance(self, num=1):
        for i in range(0, num):
            self.pos += 1
            if self.pos > len(self.text) - 1:
                self.current_char = None
            else:
                self.current_char = self.text[self.pos]

    def peek(self, num=1):
        peek_pos = self.pos
        res = ""
        while peek_pos < self.pos + num:
            if peek_pos > len(self.text) - 1:
                break
            res += self.text[peek_pos]
            peek_pos += 1
        return res

    def skip_whitespace(self):
        while self.current_char is not None and self.current_char.isspace():
            self.advance()

    def integer(self):
        result = ''
        while self.current_char is not None and self.current_char.isdigit():
            result += self.current_char
            self.advance()
        return int(result)

    def _id(self):
        result = ''
        while self.current_char is not None and self.current_char.isalnum():
            result += self.current_char
            self.advance()

        token = ezcompiler.RESERVED_KEYWORDS.get(result, Token(ezcompiler.ID, result))
        return token

    def get_next_token(self):
        while self.current_char is not None:

            if self.current_char.isspace():
                self.skip_whitespace()
                continue

            if self.peek(4) == 'sqrt':
                self.advance(4)
                return Token(ezcompiler.SQRT, 'sqrt')

            if self.current_char.isalpha():
                return self._id()

            if self.current_char.isdigit():
                return Token(ezcompiler.INTEGER, self.integer())

            # avoids equality (like ==)
            if  self.peek() == "=" and self.peek(2) != "==":
                self.advance()
                return Token(ezcompiler.ASSIGN, '=')

            if self.peek() == ';':
                self.advance()
                return Token(ezcompiler.SEMI, ';')


            if self.peek() == '+':
                self.advance()
                return Token(ezcompiler.ADD, '+')

            if self.peek() == '-':
                self.advance()
                return Token(ezcompiler.SUB, '-')

            if self.peek() == '*':
                self.advance()
                return Token(ezcompiler.MUL, '*')

            if self.peek() == '/':
                self.advance()
                return Token(ezcompiler.DIV, '/')

            if self.peek() == '(':
                self.advance()
                return Token(ezcompiler.LPAREN, '(')

            if self.peek() == ')':
                self.advance()
                return Token(ezcompiler.RPAREN, ')')

            if self.peek() == '.':
                self.advance()
                return Token(ezcompiler.DOT, '.')

            self.error()

        return Token(ezcompiler.EOF, None)
