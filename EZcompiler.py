import EZlogger as log
import math

libs = """#include <stdio.h>
#include <stdlib.h>
#include <mpfr.h>
#include <math.h>
"""

start="""
int main(int argc, char *argv[]) {
int _prec, _pprec;
"""

end="""
}
"""

char_oper = {
    "+": "Add",
    "-": "Sub",
    "*": "Mul",
    "/": "Div",
    "^": "Pow"
}

char_st = {
    "prec": "Prec",
    
    "echo": "Echo",
    "var": "Var",

    "add": "Add",
    "sub": "Sub",
    "mul": "Mul",
    "div": "Div",
    "pow": "Pow",

    "int": "Int",

    "sqrt": "Sqrt",
    "sin": "Sin",
    "cos": "Cos",
    "exp": "Exp",
    "log": "Log",
    "logb": "LogB",

    "label": "Label",
    "goto": "Goto",

    "if": "If",
    "for": "For",
    "fi": "EndLoop",
    "rof": "EndLoop"   
}

st_must_be_first = [
    "if",
    "fi"
]


var_names = set()

def get_var_init(var):
    return "mpfr_t %s; mpfr_init(%s);" % (var, var)

def get_var_free(var):
    return "mpfr_clear(%s);" % (var)

def compile_str(lines):
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

    #out += prec.init_st()
    CONSTS_ST = [Prec("prec", ["64"]), Set("ZERO", ["0"]), Set("ONE", ["1"]), Set("TWO", ["2"]), Set("TEN", ["10"])]

    if isinstance(statements[0], Prec):
        CONSTS_ST[0] = statements[0]
        statements = statements[1:]
    else:
        out += DEFAULT_PREC_ST.get_st()

    statements = CONSTS_ST[1:] + statements
    
    for st in statements:
        statement_str += st.get_st() + "\n"

    out += CONSTS_ST[0].get_st() + "\n"

    for v in var_names:
        if v != "prec":
            out += get_var_init(v) + "\n"

    out = out + statement_str

    for v in var_names:
        out += get_var_free(v) + "\n"

    out += end
    return out

def clean_line(v):
    v = v.lstrip()
    v = v.rstrip()
    return v

def split_l(aa):
    r = []
    for a in aa:
        a = clean_line(a)
        if a and a != "":
            r.append(a)
    return r

def parse_function(l):
    l = clean_line(l)
    if "=" in l:
        assign, args = l.split("=")
        return clean_line(assign), split_l(args.split(" "))
    else:
        args = split_l(l.split(" "))
        return "", args

def pile_str(line):
    if "#" in line:
        line = line[:line.index("#")]
    line = clean_line(line)
    if line == "":
        return
    for char in char_oper:
        if char in line and "=" in line:
            _data = clean_line(line.replace(char, ""))
            assign, args = parse_function(_data)
            a, b = args
            exec("R=%s_Op(assign, a, b)" % (char_oper[char]))
            return R
    for char in char_st:
        if char in line:
            go_on = True
            if char in st_must_be_first:
                go_on = line.startswith(char)
            if go_on:
                rm_func = line.replace(char, "", 1) 
                assign, args = parse_function(rm_func)
                exec("R=%s(assign=assign, args=args)" % (char_st[char]))
                return R
    if "=" in line:
        assign, args = parse_function(line.replace(char, ""))
        return Set(assign, args)


def is_const(str):
    return str[0].isdigit() or str[0] == "." or str[0] == "-"

def is_pos_int(str):
    return str[0].isdigit()

"""

    Statement

"""

class Statement():
    def __init__(self, assign, args):
        if assign and assign != "":
            var_names.add(assign)
        self.assign = assign
        self.args = args

class Prec(Statement):
    def get_st(self):
        print self.assign
        c_str = self.args[0]
        if "$" in c_str:
            num = self.args[0].replace("$", "")
            c_str = "strtol(argv[%s], NULL, 10)" % (num) 
        return "_prec = %s; mpfr_set_default_prec(_prec); _pprec = (int)(_prec * log(2.0) / log(10.0)); mpfr_t prec; mpfr_init(prec); " % (c_str) + Set("prec", self.args[:1]).get_st()

class Echo(Statement):
    def get_st(self):
        return "printf(\"%s\\n\");\n" % (" ".join(self.args))
    
class Var(Statement):
    def get_st(self):
        self.var = self.args[0]
        return "mpfr_printf(\"%s : %%.*Rf \\n\", _pprec, %s);\n" % (self.var, self.var)

class Set(Statement):
    def get_st(self):
        var_names.add(self.assign)
        r = ""
        if is_const(self.args[0]):
            r += "mpfr_set_str(%s, \"%s\", 10, GMP_RNDN);\n" % (self.assign, " ".join(self.args))
        elif self.args[0].startswith("$"):
            num = self.args[0].replace("$", "")
            r += "if (argc > %s) { mpfr_set_str(%s, argv[%s], 10, GMP_RNDN); } else { mpfr_set_str(%s, \"0.0\", 10, GMP_RNDN);\n }" % (num, self.assign, num, self.assign)
        else:
            r += "mpfr_set(%s, %s, GMP_RNDN);" % (self.assign, self.args[0])
        return r

class Add(Statement):
    def get_st(self):
        return "mpfr_add(%s, %s, %s, GMP_RNDN);\n" % (self.assign, self.args[0], self.args[1])

class Sub(Statement):
    def get_st(self):
        return "mpfr_sub(%s, %s, %s, GMP_RNDN);\n" % (self.assign, self.args[0], self.args[1])

class Mul(Statement):
    def get_st(self):
        return "mpfr_mul(%s, %s, %s, GMP_RNDN);\n" % (self.assign, self.args[0], self.args[1])

class Div(Statement):
    def get_st(self):
        return "mpfr_div(%s, %s, %s, GMP_RNDN);\n" % (self.assign, self.args[0], self.args[1])

class Pow(Statement):
    def get_st(self):
        return "mpfr_pow(%s, %s, %s, GMP_RNDN);\n" % (self.assign, self.args[0], self.args[1])

class Int(Statement):
    def get_st(self):
        return "mpfr_trunc(%s, %s);\n" % (self.assign, self.args[0])

class Sqrt(Statement):
    def get_st(self):
        return "mpfr_sqrt(%s, %s, GMP_RNDN);\n" % (self.assign, self.args[0])

class Sin(Statement):
    def get_st(self):
        return "mpfr_sin(%s, %s, GMP_RNDN);\n" % (self.assign, self.args[0])

class Cos(Statement):
    def get_st(self):
        return "mpfr_cos(%s, %s, GMP_RNDN);\n" % (self.assign, self.args[0])

class Exp(Statement):
    def get_st(self):
        return "mpfr_exp(%s, %s, GMP_RNDN);\n" % (self.assign, self.args[0])

class Log(Statement):
    def get_st(self):
        return "mpfr_log(%s, %s, GMP_RNDN);\n" % (self.assign, self.args[0])

class LogB(Statement):
    def get_st(self):
        tmp_0_name ="tmp_0_" + str(abs(hash(self.assign)))
        tmp_1_name ="tmp_1_" + str(abs(hash(self.assign)))
        tmp_0_var = Set(tmp_0_name, "0.0").get_st()
        tmp_1_var = Set(tmp_1_name, "0.0").get_st()
        find_la = Log(tmp_0_name, self.args[1]).get_st()
        find_lb = Log(tmp_1_name, self.args[0]).get_st()
        find_div = Div(self.assign, (tmp_0_name, tmp_1_name)).get_st()
        return find_la + find_lb + find_div

class Label(Statement):
    def get_st(self):
        return "%s :;" % (self.args[0])

class Goto(Statement):
    def get_st(self):
        return "goto %s;" % (self.args[0])

class If(Statement):
    def get_st(self):
        return "if (mpfr_cmp(%s, %s) %s 0) {" % (self.args[0], self.args[2], self.args[1])

class For(Statement):
    def get_st(self):
        self.assign = self.args[0]
        self.args = self.args[1:]
        if len(self.args) <= 2:
            self.args.append("1.0")
        assigns = ""
        min_str = self.args[0]
        max_str = self.args[1]
        step_str = self.args[2]
        if is_const(self.args[0]):
            min_str = "%s_min" % (self.assign)
            assigns += Set(min_str, [self.args[0]]).get_st()
        if is_const(self.args[1]):
            max_str = "%s_max" % (self.assign)
            assigns += Set(max_str, [self.args[1]]).get_st()
        if is_const(self.args[2]):
            step_str = "%s_step" % (self.assign)
            assigns += Set(step_str, [self.args[2]]).get_st()

        set_str = "mpfr_set(%s, %s, GMP_RNDN)" % (self.assign, min_str)
        cmp_str = "mpfr_cmp(%s, %s)" % (self.assign, max_str)
        inc_st = "mpfr_add(%s, %s, %s, GMP_RNDN);" % (self.assign, self.assign, step_str)
        
        loop_var = Set(self.assign, self.args[0])
        do_name = "do_%s" % (self.assign)
        stop_do = "%s = 0;" % (do_name)
        init_st = "%s; int %s = 2;" % (set_str, do_name)
        loop_st = "while (%s) { " % (do_name)
        stop_st = "if (%s == 2) { %s = 1; } else { %s if (mpfr_cmp_ui(%s, 0) > 0) { if (%s >= 0) { %s } } else { if (%s <= 0) { %s } } }" % (do_name, do_name, inc_st, step_str, cmp_str, stop_do, cmp_str, stop_do)
        return assigns + init_st + loop_st + stop_st

class EndLoop(Statement):
    def get_st(self):
        return "}"

"""

     Operations

"""

class Operation():
    def __init__(self, assign, a, b):
        self.assign = assign
        self.a = a
        self.b = b

class Add_Op(Operation):
    def get_st(self):
        return Add(self.assign, [self.a, self.b]).get_st()

class Sub_Op(Operation):
    def get_st(self):
        return Sub(self.assign, [self.a, self.b]).get_st()

class Mul_Op(Operation):
    def get_st(self):
        return Mul(self.assign, [self.a, self.b]).get_st()

class Div_Op(Operation):
    def get_st(self):
        return Div(self.assign, [self.a, self.b]).get_st()

class Pow_Op(Operation):
    def get_st(self):
        return Pow(self.assign, [self.a, self.b]).get_st()