###                     EZC src/ezcompiler/parser.py v@VERSION@
#
#  EZC is free software; you are free to modify and/or redistribute it under the terms of the GNU GPLv3. See 'LICENSE' for more details.
#
#  Parses, using a lexer, constants, and operations. Generates an AST to use
#
#  TODO:
#
#
###

import ezc

import ezcompiler
from ezcompiler import INTEGER, ID, ASSIGN, FUNCTION, COMMA, ADD, SUB, MUL, DIV, SEMI, LPAREN, RPAREN, EOF
from token import Var, Num, Assign, NoOp, UnaryOp, BinOp, Compound, Function

class Parser(object):
    def __init__(self, lexer):
        self.lexer = lexer
        self.current_token = self.lexer.get_next_token()

    # todo add more explicit error messages
    def error(self, got=None, expect=None):
        ezc.err("Invalid Character (parser.py)", ezcompiler.InvalidCharacter(self.current_token.char_at, self.lexer.text, got, expect))

    # go forward, incrementing lexer pointer
    def eat(self, token_type):
        if self.current_token.type == token_type:
            self.current_token = self.lexer.get_next_token()
        else:
            self.error(self.current_token.type, token_type)

    # the whole program is a compound statement
    def program(self):
        node = self.compound_statement()
        return node

    def compound_statement(self):
        nodes = self.statement_list()

        root = Compound()
        for node in nodes:
            root.children.append(node)

        return root

    def statement_list(self):
        node = self.statement()

        results = [node]

        while self.current_token.type == SEMI:
            self.eat(SEMI)
            results.append(self.statement())
        if self.current_token.type == ID:
            self.error(got=ID, expect=None)

        return results

    def statement(self):
        if self.current_token.type == ID:
            node = self.assignment_statement()
        else:
            node = self.empty()
        return node

    def assignment_statement(self):
        left = self.variable()
        token = self.current_token
        self.eat(ASSIGN)
        right = self.expr()
        node = Assign(left, token, right)
        return node

    def variable(self):
        node = Var(self.current_token)
        self.eat(ID)
        return node

    def empty(self):
        return NoOp()

    def expr(self):
        node = self.term()

        while self.current_token.type in (ADD, SUB, ):
            token = self.current_token
            if token.type == ADD:
                self.eat(ADD)
            elif token.type == SUB:
                self.eat(SUB)

            node = BinOp(left=node, op=token, right=self.term())

        return node

    def term(self):
        node = self.factor()

        while self.current_token.type in (MUL, DIV):
            token = self.current_token
            if token.type == MUL:
                self.eat(MUL)
            elif token.type == DIV:
                self.eat(DIV)

            node = BinOp(left=node, op=token, right=self.factor())

        return node


    def factor(self):
        token = self.current_token

        if token.type == ADD:
            self.eat(ADD)
            node = UnaryOp(token, self.factor())
            return node
        elif token.type == SUB:
            self.eat(SUB)
            node = UnaryOp(token, self.factor())
            return node
        elif token.type == INTEGER:
            self.eat(INTEGER)
            return Num(token)
        elif token.type == LPAREN:
            self.eat(LPAREN)
            node = self.expr()
            self.eat(RPAREN)
            return node
        elif token.type == FUNCTION:
            self.eat(FUNCTION)
            args = []
            while self.current_token.type != RPAREN:
                args.append(self.expr())
                if self.current_token.type != RPAREN:
                    self.eat(COMMA)
            self.eat(RPAREN)
            return Function(token, args)
        else:
            node = self.variable()
            return node

    def parse(self):
        node = self.program()
        if self.current_token.type != EOF:
            self.error(expect=EOF, got=self.current_token.type)

        return node
