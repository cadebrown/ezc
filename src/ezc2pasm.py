###                     EZC src/ezc2pasm.py v@VERSION@
#
#  EZC is free software; you are free to modify and/or redistribute it under the terms of the GNU GPLv3. See 'LICENSE' for more details.
#
#  Generates PASM (psuedo assembly) from EZC source files.
#
#  TODO:
#    * Unary operations support
#    * Functions (subroutines)
#
#  CREDITS:
#    * Ruslan Spivak (https://ruslanspivak.com/) for a great resource on ASTs/parsing
#
###

from ezcompiler.tlex import Lexer
from ezcompiler.parser import Parser
from ezcompiler.inter import NodeVisitor

from pasm import folding

SUB_OPT_STR = "PASM"
SUB_OPTS = ["o"]
SUB_NAME = "EZC to PASM"


class EZC2PASM(NodeVisitor):

    GLOBAL_SCOPE = {}

    def __init__(self, parser, **kwargs):
        self.parser = parser
        self.lines = []
        self.tmp_var = []
        self.args = dict(kwargs)

    def new_tmp_var(self):
        self.tmp_var.append("__tmp" + str(len(self.tmp_var)))
        return self.tmp_var[-1]

    def visit_BinOp(self, node):
        l_vis = self.visit(node.left)
        r_vis = self.visit(node.right)
        line = "{0}({1},{2},{3})"
        if self.args["folding"]:
            if node.op.type in folding.fold_func:
                tres = folding.fold_func[node.op.type](l_vis, r_vis)
                if tres:
                    return tres
        tvar = self.new_tmp_var()
        self.lines.append(line.format(node.op.type, tvar, l_vis, r_vis))
        return tvar

    def visit_Num(self, node):
        return node.value

    def visit_UnaryOp(self, node):
        op = node.op.type
        if op == PLUS:
            return +self.visit(node.expr)
        elif op == MINUS:
            return -self.visit(node.expr)

    def visit_Compound(self, node):
        for child in node.children:
            self.visit(child)

    def visit_Assign(self, node):
        var_name = node.left.value
        self.GLOBAL_SCOPE[var_name] = self.visit(node.right)

    def visit_Var(self, node):
        var_name = node.value
        val = self.GLOBAL_SCOPE.get(var_name)
        if val is None:
            raise NameError(repr(var_name))
        else:
            return val

    def visit_NoOp(self, node):
        pass

    def interpret(self):
        tree = self.parser.parse()
        if tree is None:
            return ''
        return self.visit(tree)

    def get_src(self):
        tree = self.parser.parse()
        if tree is None:
            return ''
        self.visit(tree)
        for assign in self.GLOBAL_SCOPE:
            self.lines.append("ASSIGN({0},{1})".format(assign, self.GLOBAL_SCOPE[assign]))
        return self.lines

def main(argv):
    import argparse
    import ezlogging

    parser = argparse.ArgumentParser(description=SUB_NAME)

    parser.add_argument('files', metavar='files', type=str, nargs='*', default=[], help='files to compile')

    parser.add_argument('-o', default="{0}.pasm", type=str, help='File struct')
    parser.add_argument('-c', default=None, type=str, help='Compile a string')

    OPT_ARGS={ "folding": True }

    args = parser.parse_args(argv)
    if args.c:
        fp = open(args.o.format("ezc2pasm-c"), "w+")
        text = args.c

        lexer = Lexer(text)
        parser = Parser(lexer)
        asmgen = EZC2PASM(parser, **OPT_ARGS)

        for x in asmgen.get_src():
            fp.write(x + "\n")
        fp.close()
    
    else:
        for cfile in args.files:
            if cfile.endswith(".ezc"):
                fp = open(args.o.format(cfile), "w+")
                text = open(cfile).read().replace("\n", ";\n")

                lexer = Lexer(text)
                parser = Parser(lexer)
                asmgen = EZC2PASM(parser, **OPT_ARGS)

                for x in asmgen.get_src():
                    fp.write(x + "\n")
                fp.close()

if __name__ == "__main__":
    import sys
    main(sys.argv[1:])