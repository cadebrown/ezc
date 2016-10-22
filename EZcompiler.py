import EZlogger as log
import math
from shared_data import *
import shared_data

default_libs = ["libBasic", "libLoops", "libMath"]

char_st = {

}

char_op = {

}

multiline_comment = False

line_ct = 1


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

def transpile_sts(list_st):
    r = ""
    for st in list_st:
        r += "\t\n" + st.get_st()
    return r

def compile_lines(lines, imports):
    imports = imports + default_libs
    exec(import_libs_code(imports))
    
    build_classes(imports)

    statements = []
    out = libs + start
    statement_str = ""

    for line in lines:
        v = pile_str(line)
        if v:
            if isinstance(v, list):
                for s in v:
                    statements.append(s)
            else:
                statements.append(v)

    CONSTS_ST = [
        eval("%s(assign=\"prec\", args=[\"128\"])" % (char_st["setprec"])),
        eval("%s(assign=\"ZERO\", args=[\"0.0\"])" % (char_st["set"])),
        eval("%s(assign=\"ONE\", args=[\"1.0\"])" % (char_st["set"])),
        eval("%s(assign=\"TWO\", args=[\"2.0\"])" % (char_st["set"])),
        eval("%s(assign=\"TEN\", args=[\"10.0\"])" % (char_st["set"]))
    ]

    for st in statements:
        if isinstance(st, eval(char_st["setprec"])):
            CONSTS_ST[0] = st
            statements.remove(st)

    statements = CONSTS_ST[1:] + statements

    out += "\n\t" + CONSTS_ST[0].get_st()

    statement_str += transpile_sts(statements)

    var_names.remove("prec")

    for v in var_names:
        out += "\n\t" + get_var_init(v)

    out = out + statement_str

    for v in var_names:
        out += "\n\t" + get_var_free(v)

    out += end
    return out

def parse_oper(line):
    for item in char_op:
        if " " + item + " " in line and "=" in line:
            _data = clean_line(line.replace(item, ""))
            assign, args = parse_function(_data)
            a, b = args
            exec("R = %s(assign, a, b)" % (char_op[item]))
            return R

def parse_st(line):
    for item in char_st:
        if (line == item) or (line.startswith(item) and ("%s " % (item)) in line) or (" %s " % (item) in line):
            rm_func = line.replace(item, "", 1)
            assign, args = parse_function(rm_func)
            exec("R = %s(assign=assign, args=args)" % (char_st[item]))
            return R

def parse_default(line):
    if "=" in line:
        assign, args = parse_function(line)
        exec("R = %s(assign=assign, args=args)" % (char_st["set"]))
        return R

def pile_str(line, level=0):
    global line_ct
    global multiline_comment
    R = None
    if line.startswith("###"):
        multiline_comment = not multiline_comment
    
    read_line = not (line.startswith("#") or multiline_comment)

    if read_line:
        try:
            if "#" in line:
                line = line[:line.index("#")]
            line = clean_line(line)

            if ";" in line:
                Rs = []
                for sl in line.split(";"):
                    Rs.append(pile_str(clean_line(sl), level + 1))
                return Rs

            R = parse_oper(line)
            if not R:
                R = parse_st(line)
            if not R:
                R = parse_default(line)
        except Exception as e:
            log.err("Transpiling", ["Line %s: " % (line_ct), "  %s" % (line)])
    if level == 0:
        line_ct += 1
    return R