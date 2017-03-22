###                     EZC src/ezc2pasm.py v@VERSION@
#
#  EZC is free software; you are free to modify and/or redistribute it under the terms of the GNU GPLv3. See 'LICENSE' for more details.
#
#  Generates PASM (psuedo assembly) from EZC source files.
#
#  TODO:
#
#
#  CREDITS:
#    * Ruslan Spivak (https://ruslanspivak.com/) for a great resource on ASTs/parsing
#
###

import ezc

import ezcompiler
from ezcompiler.tlex import Lexer
from ezcompiler.parser import Parser
from ezcompiler.inter import NodeVisitor

from ezlogging import log
from ezlogging import exc

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

    def visit_Function(self, node):
        viss = []
        for nd in node.args:
            viss.append(self.visit(nd))

        dfuncn = "F_{0}".format(node.value)
        dfunc =  "{0}({1}{2})"
        argf = "".join([",{0}".format(vis) for vis in viss])
        if self.args["folding"]:
            if dfuncn in folding.fold_func:
                tres = folding.fold_func[dfuncn](*viss)
                if tres:
                    return tres

        tvar = self.new_tmp_var()
        self.lines.append(dfunc.format(dfuncn, tvar, argf))
        return tvar

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
        vis = self.visit(node.expr)
        ufunc = "U_{0}".format(node.op.type)
        if self.args["folding"]:
            if ufunc in folding.fold_func:
                tres = folding.fold_func[ufunc](vis)
                if tres:
                    return tres
        tvar = self.new_tmp_var()
        line = "{0}({1},{2})"
        self.lines.append(line.format(ufunc, tvar, vis))
        return tvar   

    def visit_Compound(self, node):
        for child in node.children:
            self.visit(child)

    def visit_String(self, node):
        val = node.value
        return '"{0}"'.format(val)

    def visit_Assign(self, node):
        var_name = node.left.value
        self.GLOBAL_SCOPE[var_name] = self.visit(node.right)

    def visit_Var(self, node):
        var_name = node.value
        val = self.GLOBAL_SCOPE.get(var_name)
        if val is None:
            ezc.err("Undefined Variable", exc.NameNotFound(repr(var_name)))
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
        ofile = args.o.format("ezc2pasm-c")
        log.extra(SUB_NAME, "'{0}' -> {1}".format(args.c, ofile))
        
        fp = open(ofile, "w+")
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
                ofile = args.o.format(cfile)
                log.extra(SUB_NAME, "{0} -> {1}".format(cfile, ofile))
                
                fp = open(ofile, "w+")
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