
import ezcompiler
from ezcompiler import INTEGER, VARIABLE, EQUALS, operator_tiers, LPAREN, RPAREN
from ezcompiler.tlex import Lexer

class EZC2PC(object):
    def __init__(self, lines):
        self.lexer = None
        self.srclines = lines
        self.linenum = 0

    def next_line(self):
        #self.lines = []
        # set current token to the first token taken from the input
        if self.linenum < len(self.srclines):
            self.varnum = 0
            self.lines = []            
            self.line = self.srclines[self.linenum]
            self.lexer = Lexer(self.line)
            self.current_token = self.lexer.get_next_token()
            self.linenum += 1

    def new_tmpvar(self):
        self.tmpvar = "tmp{0}".format(self.varnum)
        self.varnum += 1

    def error(self):
        raise Exception('Invalid syntax')

    def eat(self, token_type):
        # compare the current token type with the passed token
        # type and if they match then "eat" the current token
        # and assign the next token to the self.current_token,
        # otherwise raise an exception.
        if self.current_token.type == token_type:
            self.current_token = self.lexer.get_next_token()
        else:
            self.error()

    def rside(self):
        while self.current_token.type == EQUALS:
            self.eat(EQUALS)
        return self.expr()

    def factor(self):
        """factor : INTEGER | LPAREN expr RPAREN"""
        
        token = self.current_token
        if token.type == INTEGER:
            self.eat(INTEGER)
            return token.value
        elif token.type == VARIABLE:
            self.eat(VARIABLE)
            return token.value
        elif token.type == LPAREN:
            self.eat(LPAREN)
            result = self.expr()
            self.eat(RPAREN)
            return result

    def term(self):
        """term : factor ((MUL | DIV) factor)*"""
        result = self.factor()

        while self.current_token.type in operator_tiers[1] or self.current_token.type in operator_tiers[0]:
            token = self.current_token
            self.new_tmpvar()
            line = "{0}({1}, {2}, {3});"
            self.eat(token.type)
            self.lines.append(line.format(ezcompiler.op_to_name(token.type), self.tmpvar, result, self.factor()))
            result = self.tmpvar
        return result


    def expr(self):
        """Arithmetic expression parser / interpreter.

        calc> 7 + 3 * (10 / (12 / (3 + 1) - 1))
        22

        expr   : term ((PLUS | MINUS) term)*
        term   : factor ((MUL | DIV) factor)*
        factor : INTEGER | LPAREN expr RPAREN
        """

        result = self.term()
        var = []
        fresult = None

        if self.current_token.type == EQUALS:
            fresult = self.rside()


        while self.current_token.type in operator_tiers[2]:
            token = self.current_token
            self.new_tmpvar()
            line = "{0}({1}, {2}, {3});"
            self.eat(token.type)
            self.lines.append(line.format(ezcompiler.op_to_name(token.type), self.tmpvar, result, self.term()))
            result = self.tmpvar

        if fresult:
            self.lines[-1] = self.lines[-1].replace(fresult, result)
        else:
            fresult = result
        return fresult

    def get_pc(self):
        res = []
        for i in range(0, len(self.srclines)):
            self.next_line()
            self.expr()
            res.append(self.lines)
        return res

def main(argv):
    import argparse
    import ezlogging

    parser = argparse.ArgumentParser(description='EZC 2 Pseudocode')

    parser.add_argument('files', metavar='files', type=str, nargs='*', default=[], help='files to compile')
    parser.add_argument('-o', default="a.out", help='Output file')

    parser.add_argument('-file', default="{0}.pso", type=str, help='File struct')
    args = parser.parse_args(argv)
    SLOC = 0
    for cfile in args.files:
        if cfile.endswith(".ezc"):
            fp = open(args.file.format(cfile), "w+")
            lines = open(cfile).read().split("\n")
            PCgen = EZC2PC(lines)
            fp.write("! SLOC: {0}\n".format(SLOC))
            for x in PCgen.get_pc():
                for xx in x:
                    fp.write(xx + "\n")
                SLOC += 1

            fp.close()

if __name__ == "__main__":
    import sys
    main(sys.argv[1:])