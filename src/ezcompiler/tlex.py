###                     EZC src/ezcompiler/tlex.py v@VERSION@
#
#  EZC is free software; you are free to modify and/or redistribute it under the terms of the GNU GPLv3. See 'LICENSE' for more details.
#
#  Contains lexer utilities used by parser.py to generate ASTs.
#
#  TODO:
#    * Parse statements which don't begin with `x=`, like `f(x)`
#
#
###

import ezc

import ezcompiler
from token import Token

class Lexer(object):
    def __init__(self, text):
        self.text = text
        self.pos = 0
        self.current_char = self.text[self.pos]

    # todo add more explicit error messages
    def error(self, got=None, expect=None):
        ezc.err("Invalid Character (tlex.py)", ezcompiler.InvalidCharacter(self.pos, self.text, got, expect))

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


    def function(self):
        result = ''
        self.advance()
        while self.current_char is not None and self.current_char.isdigit():
            result += self.current_char
            self.advance()
        return int(result)


    def constant(self):
        result = ''
        while self.current_char is not None and self.current_char.isdigit() or self.current_char == ".":
            result += self.current_char
            self.advance()
        return result

    def _id(self):
        result = ''
        while self.current_char is not None and self.current_char.replace('"', "").isalnum():
            result += self.current_char
            self.advance()

        token = ezcompiler.RESERVED_KEYWORDS.get(result, Token(ezcompiler.ID, result, self.pos))
        return token

    def get_next_token(self):
        while self.current_char is not None:

            if self.current_char.isspace():
                self.skip_whitespace()
                continue

            if self.peek() == '"':
                start = self.pos
                res = ""
                l_char = ""
                self.advance()
                while self.current_char is not None and l_char != "\\" and self.current_char != '"':
                    res += self.current_char
                    self.advance()
                self.advance()
                return Token(ezcompiler.STRING, res, start)

            if self.peek() == ',':
                self.advance()
                return Token(ezcompiler.COMMA, ',', self.pos)

            if self.current_char.isalpha():
                num = 1
                tres = ""
                while self.peek(num)[-1].isalpha() or self.peek(num)[-1].isspace():
                    if num >= len(self.text):
                        break
                    tres += self.peek(num)[-1]
                    num += 1
                if self.peek(num)[-1] == "(":
                    self.advance(num)
                    return Token(ezcompiler.FUNCTION, tres, self.pos-1)
                return self._id()

            if self.current_char.isdigit() or self.current_char == ".":
                return Token(ezcompiler.CONSTANT, self.constant(), self.pos)

            # avoids equality (like ==)
            if  self.peek() == "=" and self.peek(2) != "==":
                self.advance()
                return Token(ezcompiler.ASSIGN, '=', self.pos-1)

            if self.peek() == ';':
                self.advance()
                return Token(ezcompiler.SEMI, ';', self.pos-1)


            if self.peek() == '+':
                self.advance()
                return Token(ezcompiler.ADD, '+', self.pos-1)

            if self.peek() == '-':
                self.advance()
                return Token(ezcompiler.SUB, '-', self.pos-1)

            if self.peek() == '*':
                self.advance()
                return Token(ezcompiler.MUL, '*', self.pos-1)

            if self.peek() == '/':
                self.advance()
                return Token(ezcompiler.DIV, '/', self.pos-1)

            if self.peek() == '(':
                self.advance()
                return Token(ezcompiler.LPAREN, '(', self.pos-1)

            if self.peek() == ')':
                self.advance()
                return Token(ezcompiler.RPAREN, ')', self.pos-1)

            if self.peek() == '.':
                self.advance()
                return Token(ezcompiler.DOT, '.', self.pos-1)

            self.error(got=ezcompiler.UNKNOWN, expect=None)

        return Token(ezcompiler.EOF, None, self.pos)
