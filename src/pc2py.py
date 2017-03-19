
import ezcompiler
from ezcompiler import CONSTANT, VARIABLE, EQUALS, operator_tiers, LPAREN, RPAREN
from ezcompiler.tlex import Lexer

SUB_NAME = "PC 2 PY"
SUB_OPT_STR = "P2P"
SUB_OPTS = ["o"]

types = ["int", "flt"]

PY_LIB="""
import decimal
import sys

flt=decimal.Decimal

def prec(a):
    decimal.getcontext().prec = a

def __is_flt(a):
    return isinstance(a, flt)



def GETARG(x):
    return sys.argv[x]
def ADD(a, b):
    if __is_flt(a) or __is_flt(b):
        return decimal.getcontext().add(a, b)
    else:
        return a + b
def SUB(a, b):
    if __is_flt(a) or __is_flt(b):
        return decimal.getcontext().subtract(a, b)
    else:
        return a - b
def MUL(a, b):
    if __is_flt(a) or __is_flt(b):
        return decimal.getcontext().multiply(a, b)
    else:
        return a * b
def DIV(a, b):
    if __is_flt(a) or __is_flt(b):
        return decimal.getcontext().divide(a, b)
    else:
        return a / b
def POW(a, b):
    if __is_flt(a) or __is_flt(b):
        return decimal.getcontext().power(a, b)
    else:
        return a ** b

"""

var_list = { }

def is_assign(text):
    c_p = 0
    while c_p < len(text) and (text[c_p].isalpha() or text[c_p].isdigit()):
        c_p += 1
    return c_p < len(text) and text[c_p] == "="

class Statement():
    def __init__(self, line):
        if line == "":
            self.strval = line
        elif line.startswith("!"):
            self.strval = line.replace("!", "#", 1)
        else:
            if is_assign(line):
                self.assign = line[0:line.index("=")].strip()
                self.st = line[line.index("=")+1:].strip()
            else:
                self.assign = ""
                self.st = line
            if "(" in self.st:
                self.func = self.st[:self.st.index("(")].strip()
                self.args = self.st[self.st.index("(")+1:self.st.index(")")].split(",")
                self.args = [arg.strip() for arg in self.args]
            else:
                self.func = ""
                self.args = []
            if self.func == "print":
                line = line.replace(self.assign + "=", "", 1)
            self.strval = line

    def __str__(self):
        return self.strval
    
    def __repr__(self):
        return self.__str__()


class PC2PY(object):
    def __init__(self, lines):
        self.lines = lines
    
    def get_py(self):
        ret = [PY_LIB]
        for x in self.lines:
            st_x = Statement(x)
            ret.append(str(st_x))
        return ret

def main(argv):
    import argparse
    import ezlogging

    parser = argparse.ArgumentParser(description=SUB_NAME)

    parser.add_argument('files', metavar='files', type=str, nargs='*', default=[], help='files to compile')

    parser.add_argument('-o', default="{0}.py", type=str, help='File struct')
    args = parser.parse_args(argv)
    SLOC = 0
    for cfile in args.files:
        if cfile.endswith(".pc"):
            fp = open(args.o.format(cfile), "w+")
            lines = open(cfile).read().split("\n")
            PYgen = PC2PY(lines)
            for x in PYgen.get_py():
                if x:
                    #fp.write("! SLOC: {0}\n".format(SLOC))
                    #print x
                    fp.write(x + "\n")
                    #fp.write("\n")

            fp.close()

if __name__ == "__main__":
    import sys
    main(sys.argv[1:])