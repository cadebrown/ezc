
import ezcompiler
from ezcompiler import CONSTANT, VARIABLE, STRING, EQUALS, TUPLE, operator_tiers, LPAREN, RPAREN
from ezcompiler.tlex import Lexer

SUB_OPT_STR = "E2P"
SUB_OPTS = ["o"]
SUB_NAME = "EZC 2 PC"

class EZC2PC(object):
    def __init__(self, lines):
        self.lexer = None
        self.srclines = lines
        self.linenum = 0
        self.varnum = 0
        

    def next_line(self):
        #self.lines = []
        # set current token to the first token taken from the input
        if self.linenum < len(self.srclines):
            
            self.lines = []            
            self.line = self.srclines[self.linenum]
            self.lexer = Lexer(self.line)
            self.current_token = self.lexer.get_next_token()
            self.linenum += 1

    def new_tmpvar(self):
        self.tmpvar = "tmp{0}".format(self.varnum)
        self.varnum += 1
        return self.tmpvar

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
        if token.type == CONSTANT:
            self.eat(CONSTANT)
            return token.value
        elif token.type == VARIABLE:
            self.eat(VARIABLE)
            return token.value
        elif token.type == STRING:
            self.eat(STRING)
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
            rettmp = self.new_tmpvar()
            line = "{1}={0}({2}, {3});"
            self.eat(token.type)
            self.lines.append(line.format(ezcompiler.op_to_name(token.type), rettmp, result, self.factor()))
            result = rettmp
        return result

    def func(self):
        result = self.term()
        if self.current_token.type in (ezcompiler.FUNCTION, ):
            line = "{0}={1}({2});"
            fname = self.current_token.value
            rettmp = self.new_tmpvar()

            self.eat(ezcompiler.FUNCTION)
            var = []
            while self.current_token.type in (ezcompiler.LPAREN, ezcompiler.CONSTANT, ezcompiler.STRING, ezcompiler.VARIABLE, ezcompiler.FUNCTION):
                if self.current_token.type == ezcompiler.FUNCTION:
                    result = self.func()
                else:
                    result = self.expr()
                self.eat(self.current_token.type)
                
                var += [str(result)]
            result = rettmp
            self.lines.append(line.format(rettmp, fname, ", ".join(var)))
        return result

    def changeAssignment(self, text, v):
        c_ptr = 0
        tor = ""
        while c_ptr < len(text) and text[c_ptr].isdigit() or text[c_ptr].isalpha():
            tor += text[c_ptr]
            c_ptr += 1
        tor = tor + "="
        return text.replace(tor, v + "=", 1)

    def expr(self):
        """Arithmetic expression parser / interpreter.

        calc> 7 + 3 * (10 / (12 / (3 + 1) - 1))
        22

        expr   : term ((PLUS | MINUS) term)*
        term   : factor ((MUL | DIV) factor)*
        factor : INTEGER | LPAREN expr RPAREN
        """

        if self.current_token.type == ezcompiler.KEYWORD:
            self.lines.append(self.current_token.value + ";")
            return 
        result = self.func()
        var = []
        fresult = None

        if self.current_token.type == EQUALS:
            fresult = self.rside()
        while self.current_token.type in operator_tiers[2]:
            token = self.current_token
            rettmp = self.new_tmpvar()
            line = "{1}={0}({2}, {3});"
            self.eat(token.type)
            self.lines.append(line.format(ezcompiler.op_to_name(token.type), rettmp, result, self.expr()))
            result = rettmp

        self.result = result
        self.fresult = fresult

        if fresult:
            if len(self.lines) > 0:
                self.lines[-1] = self.changeAssignment(self.lines[-1], result)
        else:
            fresult = result
        return result

    def get_pc(self):
        res = []
        for i in range(0, len(self.srclines)):
            self.next_line()
            self.expr()
            if len(self.lines) > 0 and self.result and not self.fresult:
                self.lines[-1] = self.lines[-1].replace(self.result+"=", "", 1)
            if len(self.lines) == 0:
                self.lines = [self.line]

            res.append(self.lines)
        return res

def main(argv):
    import argparse
    import ezlogging

    parser = argparse.ArgumentParser(description=SUB_NAME)

    parser.add_argument('files', metavar='files', type=str, nargs='*', default=[], help='files to compile')

    parser.add_argument('-o', default="{0}.pc", type=str, help='File struct')
    args = parser.parse_args(argv)
    SLOC = 0
    for cfile in args.files:
        if cfile.endswith(".ezc"):
            fp = open(args.o.format(cfile), "w+")
            lines = open(cfile).read().split("\n")
            PCgen = EZC2PC(lines)
            for x in PCgen.get_pc():
                if x:
                    fp.write("! SLOC: {0}\n".format(SLOC))
                    for xx in x:
                        fp.write(xx + "\n")
                    SLOC += 1
                    fp.write("\n")

            fp.close()

if __name__ == "__main__":
    import sys
    main(sys.argv[1:])