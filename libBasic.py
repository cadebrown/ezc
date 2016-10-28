from shared_data import *

"""

    Basic lib for EZC

    This includes most setting variables, and printing, file ops, etc

"""

# operators
char_op = {

}

# statements
char_st = {
    "setprec": "Prec",

    "set": "Set",
    
    "echo": "Echo",
    "var": "Var",

    "file": "File"
}

# sets precision
class Prec(Statement):
    def get_st(self):
        c_str = "_prec = %s; " % (self.args[0])
        if "$" in c_str:
            num = self.args[0].replace("$", "")
            c_str = "if (argc > %s) { _prec = strtol(argv[%s], NULL, 10); } else { _prec = 128; } " % (num, num)
        return "%s mpfr_set_default_prec(_prec); _pprec = (int)(_prec * log(2.0) / log(10.0)); mpfr_t prec; mpfr_init(prec); mpfr_set_ui(prec, _prec, GMP_RNDN); " % (c_str)

# echo string
class Echo(Statement):
    def get_st(self):
        return "printf(\"%s\\n\");\n" % (" ".join(self.args))
    
# prints out var
class Var(Statement):
    def get_st(self):
        self.var = self.args[0]
        return "mpfr_printf(\"%s : %%.*Rf \\n\", _pprec, %s);\n" % (self.var, self.var)

# prints var to file
class File(Statement):
    def get_st(self):
        self.var = self.args[0]
        return "FILE *%s_fp = fopen(\"%s.txt\", \"w+\");\n mpfr_fprintf(%s_fp, \"%s : %%.*Rf\", _pprec, %s);" % (self.var, self.var, self.var, self.var, self.var)

# inits variable
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
