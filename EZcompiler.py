# logging errors
import EZlogger as log

# uses math logarithm
import math

# used for dynamic libraries
from shared_data import *

import libBasic, libLoops, libMath

# default libraries that are always included
default_libs = [
    "libBasic", 
    "libLoops", 
    "libMath"
]

# global var for keeping track
multiline_comment = False

# what line are we on?
line_ct = 1

# lists of statements and operators that are read in from libraries
char_st = { }
char_op = { }

# return the import code for dynamic libraries
def import_libs_code(imports):
    r = ""
    # loop through all libs
    for lib in imports:
        r += "global %s; import %s; libs += %s.lib_header; " % ((lib, ) * 3)
    return r

# add to char_st and char_op to define statements
def build_classes(imports):
    # define globals
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

# just takes a list of statements, and returns C code
def transpile_sts(list_st):
    r = ""
    i = 0
    for st in list_st:
        if isinstance(st.get_st(), str):
            r += "\n\t" + st.get_st()
        else:
            for rr in st.get_st():
                r += "\n\t" + rr
        i += 1
    return r

# compiles lines and imports into C
def compile_lines(lines, imports):
    # add in default libraries
    imports = imports + default_libs

    # import them
    exec(import_libs_code(imports))
    
    # build the classes from imports
    build_classes(imports)

    # start list of statements
    statements = []

    # out is C
    out = license + libs + start

    # string of statements
    statement_str = ""

    # loop through lines
    for line in lines:
        # pile the one string
        v = pile_str(line)
        # be sure to make sure it is valid
        if v:
            # if it's a list, append them one at a time
            if isinstance(v, list):
                for s in v:
                    statements.append(s)
            # add the only one
            else:
                statements.append(v)
    # default statements
    CONSTS_ST = [
        eval("%s(assign=\"prec\", args=[\"128\"])" % (char_st["setprec"])),
        eval("%s(assign=\"ZERO\", args=[\"0.0\"])" % (char_st["set"])),
        eval("%s(assign=\"ONE\", args=[\"1.0\"])" % (char_st["set"])),
        eval("%s(assign=\"TWO\", args=[\"2.0\"])" % (char_st["set"])),
        eval("%s(assign=\"TEN\", args=[\"10.0\"])" % (char_st["set"]))
    ]

    # loop through
    for st in statements:
        # if they are setting the precision, make sure we run that first
        if isinstance(st, eval(char_st["setprec"])):
            CONSTS_ST[0] = st
            statements.remove(st)

    # concat default and parsed
    statements = CONSTS_ST[1:] + statements

    # add in the original
    out += "\n\t" + CONSTS_ST[0].get_st()

    # just transpile all of them
    statement_str += transpile_sts(statements)

    # remove the keyword var
    var_names.remove("prec")

    # init vars at beginning
    for v in var_names:
        out += "\n\t" + get_var_init(v)

    # add all statements
    out = out + statement_str

    # clear all variables
    for v in var_names:
        out += "\n\t" + get_var_free(v)

    # add end
    out += end
    return out

# tryparses operator from line
def parse_oper(line):
    for item in char_op:
        if " " + item + " " in line and "=" in line:
            _data = clean_line(line.replace(item, ""))
            assign, args = parse_function(_data)
            if len(args) == 1:
                a, b = args[0], None
            else:
                a, b = args
            exec("R = %s(assign, a, b)" % (char_op[item]))
            return R

# tryparses statement from line
def parse_st(line):
    for item in char_st:
        if (line == item) or (line.startswith(item) and ("%s " % (item)) in line) or (" %s " % (item) in line):
            rm_func = line.replace(item, "", 1)
            assign, args = parse_function(rm_func)
            exec("R = %s(assign=assign, args=args)" % (char_st[item]))
            return R

# default parse for setting constant/other var
def parse_default(line):
    if " = " in line:
        assign, args = parse_function(line)
        exec("R = %s(assign=assign, args=args)" % (char_st["set"]))
        return R

# tries to pile a string, with level for multiline
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