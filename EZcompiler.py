import EZlogger as log
import math
from shared_data import *
import shared_data

default_libs = ["libBasic", "libLoops", "libMath"]
char_st = {

}

char_op = {

}


def import_libs_code(imports):
    r = ""
    for lib in imports:
        r += "global %s; import %s; " % (lib, lib)
    return r

def build_classes(imports):
    global char_st
    global char_op
    char_st = dict()
    char_op = dict()
    for l in imports:
        st_evals = eval("%s.char_st.items()" % (l))
        op_evals = eval("%s.char_op.items()" % (l))
        st_imports = dict()
        op_imports = dict()

        for st in st_evals:
            st_imports[st[0]] = "%s.%s" % (l, st[1])

        for op in op_evals:
            op_imports[op[0]] = "%s.%s" % (l, op[1])

        char_st = dict(char_st.items() + st_imports.items())
        char_op = dict(char_op.items() + op_imports.items())


def compile_str(lines, imports):
    imports = imports + default_libs
    exec(import_libs_code(imports))
    
    build_classes(imports)

    statements = []
    out = libs + start
    statement_str = ""

    for line in lines:
        try:
            v = pile_str(line)
            if v:
                statements.append(v)
        except Exception as e:
            log.err("GenericLine", str(e))

    CONSTS_ST = [
        eval("%s(assign=\"prec\", args=[\"128\"])" % (char_st["setprec"])),
        eval("%s(assign=\"ZERO\", args=[\"0.0\"])" % (char_st["set"])),
        eval("%s(assign=\"ONE\", args=[\"1.0\"])" % (char_st["set"])),
        eval("%s(assign=\"TWO\", args=[\"2.0\"])" % (char_st["set"])),
        eval("%s(assign=\"TEN\", args=[\"10.0\"])" % (char_st["set"]))
        ]

    for st in statements:
        if isinstance(st, eval(char_st["setprec"])):
            print "Found"
            CONSTS_ST[0] = st
            statements.remove(st)

    statements = CONSTS_ST[1:] + statements

    out += CONSTS_ST[0].get_st() + "\n"

    for st in statements:
        statement_str += st.get_st() + "\n"

    var_names.remove("prec")

    for v in var_names:
        out += get_var_init(v) + "\n"

    out = out + statement_str

    for v in var_names:
        out += get_var_free(v) + "\n"

    out += end
    return out

def pile_str(line):
    if "#" in line:
        line = line[:line.index("#")]
    line = clean_line(line)
    if line == "":
        return
    for char in char_op:
        if " " + char + " " in line and "=" in line:
            _data = clean_line(line.replace(char, ""))
            assign, args = parse_function(_data)
            a, b = args
            exec("R=%s(assign, a, b)" % (char_op[char]))
            return R
    for char in char_st:
        if (char == line) or (char + " " in line):
            if (line.index(char) == 0) or (" %s " % (char) in line):
                rm_func = line.replace(char, "", 1) 
                assign, args = parse_function(rm_func)
                exec("R=%s(assign=assign, args=args)" % (char_st[char]))
                return R
    if "=" in line:
        assign, args = parse_function(line)
        exec("R=%s(assign=assign, args=args)" % (char_st["set"]))
        return R