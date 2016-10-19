import EZlogger as log

libs = """#include <stdio.h>
#include <stdlib.h>
#include <mpfr.h>
#include <math.h>
"""

start="""
int main(int argc, char *argv[]) {
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

char_oper_func = {
    "sin": "Sin"
}

char_block = {
    "if": "If",
    "break": "Break",
    "for": "Count",
}

char_cond = {
    ">": "Greater",
    "<": "Less",
    "==": "Equal"
}

char_func = {
    "echo": "PrintStr",
    "printstr": "PrintRaw",
    "var": "PrintVar",
    "printvar": "PrintRawVar",
}

def compile_str(lines):
    vars = [FloatVar("ZERO", "0.0"),FloatVar("ONE", "1.0"),FloatVar("TWO", "2.0"),FloatVar("TEN", "10.0")]
    procs = []
    prec = Prec(128)
    out = libs + start
    for line in lines:
        try:
            v = pile_str(line)
            if isinstance(v, FloatVar):
                vars.append(v)
            elif isinstance(v, Prec):
                prec = v
            elif v != None:
                procs.append(v)
        except:
            print "Generic Error Found With Statement: \n\t%s" % (line)

    out += prec.init_st()

    for v in vars:
        out += v.init_st()

    for p in procs:
        out += p.get_st()

    for v in vars:
        out += v.clear_st()

    out += end
    return out

def clean_line(v):
    v = v.lstrip()
    v = v.rstrip()
    return v
        
def pile_str(line):
    if "#" in line:
        line = line[0:line.index("#")]
    line = clean_line(line)
    if line.startswith("prec"):
        val = line[line.index("prec")+4:]
        line = clean_line(line)
        return Prec(val)
    elif line.startswith("set"):
        line = line[line.index("set")+3:]
        line = clean_line(line)
        name = line.split(" ")[0]
        st = line.split(" ")[1]
        return FloatVar(name, st)
    else:
        for char in char_func:
            if line.startswith(char):
                try:
                    line = line[len(char):]
                    line = clean_line(line)
                    op = line
                    exec("R=%s()" % (char_func[char]))
                    R.var = op
                    return R
                except e:
                    log.err("Parsing Function", line)
        for char in char_oper_func:
            if line.startswith(char):
                try:
                    line = line.replace(" ", "").replace("=", "")
                    line = line.split(char)
                    print line
                    assign_i = min(line.index(" "), line.index("="))
                    assign = line[:assign_i]
                    line = line[assign_i+1:]; line = clean_line(line)
                    a = line[]
                    line = line[len(char):]
                    line = clean_line(line)
                    op = line
                    exec("R=%s(assign=%s)" % (char_func[char]))
                    R.var = op
                    return R
                except e:
                    log.err("Parsing Function", line)
        for char in char_block:
            if line.startswith(char):
                try:
                    line = line[len(char):]
                    line = clean_line(line)
                    if char == "for":
                        args = line.split(" ")
                        var, min, max = args[:3]
                        step = 1.0
                        if len(args) > 3:
                            step = args[3]
                        exec("R=%s(min=\"%s\", max=\"%s\", var=\"%s\", step=\"%s\")" % (char_block[char], min, max, var, step))
                    elif char == "if":
                        for co in char_cond:
                            if co in line:
                                exec("cond=%s(\"%s\", \"%s\")" % (char_cond[co], line.split(co)[0], line.split(co)[1]))
                        exec("R=%s(cond=cond)" % (char_block[char]))
                    else:
                        exec("R=%s()" % (char_block[char]))
                    return R
                except e:
                    log.err("Parsing Block", line)
        for cr in char_block:
            char = cr[::-1]
            if line.startswith(char):
                return BlockEnd()
        for char in char_oper:
            if char in line:
                assign = line[0:line.index("=")]
                a = line[line.index("=")+1:line.index(char)]
                b = line[line.index(char)+len(char):]
                exec("R=%s(assign, a, b)" % (char_oper[char]))
                return R


def is_const(str):
    return str[0].isdigit() or str[0] == "."

#Class for float variables
class FloatVar():
    def __init__(self, name, val):
        self.name = name
        self.val = val

    def init_st(self):
        init_str = "\nmpfr_t %s; mpfr_init (%s);" % (self.name, self.name)
        main_str = "mpfr_set_str(%s, \"%s\", 10, GMP_RNDN);\n" % (self.name, self.val)
        if self.val.startswith("$"):
            anum = self.val.replace("$", "")
            main_str = "if (argc > %s) {" % (anum)
            main_str += "mpfr_set_str(%s, argv[%s], 10, GMP_RNDN);\n" % (self.name, anum)
            main_str += "} else {  mpfr_set_str(%s, \"0.0\", 10, GMP_RNDN);\n }" % (self.name)
        return init_str + main_str

    def clear_st(self):
        return "mpfr_clear(%s);" % (self.name)

#Class for precision
class Prec():
    def __init__(self, prec):
        self.prec = prec

    def init_st(self):
        import math
        pv = FloatVar("prec", self.prec)
        return pv.init_st() + "int _prec = %s; int _pprec = %s; mpfr_set_default_prec(_prec);\n" % (self.prec, str(int(math.log(2, 10) * int(self.prec))))


#Class for print statement
class PrintVar():
    def __init__(self, var=None):
        self.var = var

    def get_st(self):
        return "mpfr_printf(\"%s : %%.*Rf \\n\", _pprec, %s);\n" % (self.var, self.var)

#Class for print statement
class PrintRawVar():
    def __init__(self, var=None):
        self.var = var

    def get_st(self):
        return "mpfr_printf(\"%%.*Rf \\n\", _pprec, %s);\n" % (self.var)

#Class for print statement
class PrintStr():
    def __init__(self, var=None):
        self.var = var

    def get_st(self):
        return "mpfr_printf(\"%s\\n\");\n" % (self.var)

# No newline
class PrintRaw():
    def __init__(self, var=None):
        self.var = var

    def get_st(self):
        return "mpfr_printf(\"%s\");\n" % (self.var.replace("\"", ""))

"""

     Blocks

"""

class Block():
    def __init__(self, cond=None, var=None, min=None, max=None, step=None):
        self.var = var
        self.cond = cond
        self.max = max
        self.min = min
        self.step = step
    
    def get_st(self):
        return "{\n"

class BlockEnd(Block):
    def get_st(self):
        return "}\n"

class Break(Block):
    def get_st(self):
        return "break;\n"

class If(Block):
    def get_st(self):
        return "if (%s) {\n" % (self.cond.get_st())

class Count(Block):
    def get_st_min(self):
        if is_const(self.min):
            minv = FloatVar(self.var + "_min", self.min)
            return (minv.init_st(), "mpfr_set(%s, %s_min, GMP_RNDN)" % (self.var, self.var))
        else:
            return ("", "mpfr_set(%s, %s, GMP_RNDN)" % (self.var, self.min))

    def get_st_max(self):
        if is_const(self.max):
            maxv = FloatVar(self.var + "_max", self.max)
            return (maxv.init_st(), "mpfr_cmp(%s, %s_max) <= 0" % (self.var, self.var))
        else:
            return ("", "mpfr_cmp(%s, %s) <= 0" % (self.var, self.max))

    def get_st_step(self):
        if is_const(self.step):
            stepv = FloatVar(self.var + "_step", self.step)
            return (stepv.init_st(), "mpfr_add(%s, %s, %s_step, GMP_RNDN)" % (self.var, self.var, self.var))
        else:
            return ("", "mpfr_add(%s, %s, %s, GMP_RNDN)" % (self.var, self.var, self.step))

    def get_st(self):
        vari = FloatVar(self.var, "0.0")
        minv = self.get_st_min()
        maxv = self.get_st_max()
        stepv = self.get_st_step()
        return vari.init_st() + minv[0] + maxv[0] + stepv[0] + "for (%s; %s; %s) {\n" % (minv[1], maxv[1], stepv[1])

"""

    Conditions

"""

class Condition():
    def __init__(self, a, b):
        self.a = a
        self.b = b
    
class Greater(Condition):
    def get_st(self):
        return "mpfr_cmp(%s, %s) > 0" % (self.a, self.b)

class Less(Condition):
    def get_st(self):
        return "mpfr_cmp(%s, %s) < 0" % (self.a, self.b)

class Equal(Condition):
    def get_st(self):
        return "mpfr_cmp(%s, %s) == 0" % (self.a, self.b)

"""

     Operations

"""

class Operation():
    def __init__(self, assign, a, b):
        self.assign = assign
        self.a = a
        self.b = b

class Add(Operation):
    def get_st(self):
        return "mpfr_add(%s, %s, %s, GMP_RNDN);\n" % (self.assign, self.a, self.b)

class Sub(Operation):
    def get_st(self):
        return "mpfr_sub(%s, %s, %s, GMP_RNDN);\n" % (self.assign, self.a, self.b)

class Mul(Operation):
    def get_st(self):
        return "mpfr_mul(%s, %s, %s, GMP_RNDN);\n" % (self.assign, self.a, self.b)

class Div(Operation):
    def get_st(self):
        return "mpfr_div(%s, %s, %s, GMP_RNDN);\n" % (self.assign, self.a, self.b)

class Pow(Operation):
    def get_st(self):
        return "mpfr_pow(%s, %s, %s, GMP_RNDN);\n" % (self.assign, self.a, self.b)


"""

    Operation Functions

"""

class OpFunc():
    def __init__(self, assign, a):
        self.assign = assign
        self.a = a

class Sin(OpFunc):
    def get_st(self):
        return "mpfr_sin(%s, %s, GMP_RNDN);\n" % (self.assign, self.a)