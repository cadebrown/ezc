
import ezcompiler
from ezcompiler import EQUALS, CONSTANT, VARIABLE, FUNCTION, TUPLE, LPAREN, RPAREN, KEYWORD, EOF, ADD, SUB, MUL, DIV, POW, GT, LT, ET
from ezcompiler.token import Token


class Lexer(object):
    def __init__(self, text):
        # client string input, e.g. "4 + 2 * 3 - 6 / 2"
        self.text = text
        # self.pos is an index into self.text
        self.pos = 0
        self.current_char = None
        if self.pos < len(self.text):
            self.current_char = self.text[self.pos]

    def error(self):
        raise Exception('Invalid character')

    def advance(self):
        """Advance the `pos` pointer and set the `current_char` variable."""
        self.pos += 1
        if self.pos > len(self.text) - 1:
            self.current_char = None  # Indicates end of input
        else:
            self.current_char = self.text[self.pos]

    def skip_whitespace(self):
        while self.current_char is not None and self.current_char.isspace():
            self.advance()

    def function(self):
        """Return a valid variable name"""
        result = ''
        while self.current_char.isalpha():
            result += self.current_char
            self.advance()
        self.skip_whitespace()
        if self.current_char == LPAREN:
            self.advance()
        else:
            raise Exception("Expected '('")
        return result

    def variable(self):
        """Return a valid variable name"""
        result = ''
        while self.current_char is not None and self.current_char.isalpha():
            result += self.current_char
            self.advance()
        return result

    def constant(self):
        """Return a (multidigit) integer consumed from the input."""
        result = ''
        while self.current_char is not None and (self.current_char.isdigit() or self.current_char == "."):
            result += self.current_char
            self.advance()
        return result

    def get_next_token(self):
        """Lexical analyzer (also known as scanner or tokenizer)

        This method is responsible for breaking a sentence
        apart into tokens. One token at a time.
        """
        while self.current_char is not None:

            if self.current_char.isspace():
                self.skip_whitespace()
                continue
            
            for macro in ezcompiler._macros:
                self.text = self.text.replace(macro, ezcompiler._macros[macro])

            for word in ezcompiler.protected_words:
                if self.text[self.pos:].startswith(word):
                    return Token(KEYWORD, word)

            if self.current_char.isalpha():
                c_ptr = self.pos
                while self.text[c_ptr].isalpha() or self.text[c_ptr].isspace():
                    c_ptr += 1
                if self.text[c_ptr] == "(":
                    ret = Token(FUNCTION, self.function())
                    return ret
            
            if self.current_char.isalpha():
                return Token(VARIABLE, self.variable())

            if self.current_char.isdigit() or self.current_char == ".":
                return Token(CONSTANT, self.constant())

            if self.current_char == ',':
                self.advance()
                return Token(TUPLE, ',')

            if self.current_char == GT:
                self.advance()
                return Token(GT, '>')

            if self.current_char == LT:
                self.advance()
                return Token(LT, '<')

            if self.text[self.pos:self.pos+2] == ET:
                self.advance()
                self.advance()
                return Token(ET, '==')

            if self.current_char == '+':
                self.advance()
                return Token(ADD, '+')

            if self.current_char == '=':
                self.advance()
                return Token(EQUALS, '=')

            if self.current_char == '-':
                self.advance()
                return Token(SUB, '-')

            if self.current_char == '*':
                self.advance()
                return Token(MUL, '*')

            if self.current_char == '/':
                self.advance()
                return Token(DIV, '/')

            if self.current_char == '^':
                self.advance()
                return Token(POW, '^')

            if self.current_char == '(':
                self.advance()
                return Token(LPAREN, '(')

            if self.current_char == ')':
                self.advance()
                return Token(RPAREN, ')')

            self.error()

            self.error()

        return Token(EOF, None)
