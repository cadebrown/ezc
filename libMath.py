from shared_data import *

"""

    Math lib for EZC

    Contains operators, like +, -, /, ...

    Also, most math functions, like log, logb, exp, sqrt, int, etc...

"""

# lib setup
lib_header = """


"""

# operators
char_op = {
    "+": "Add_Op",
    "-": "Sub_Op",
    "*": "Mul_Op",
    "/": "Div_Op",
    "^": "Pow_Op"
}

# strings for functions
char_st = {
    "add": "Add",
    "sub": "Sub",
    "mul": "Mul",
    "div": "Div",
    "pow": "Pow",

    "int": "Int",

    "sqrt": "Sqrt",
    "sin": "Sin",
    "asin": "Asin",
    "cos": "Cos",
    "acos": "Acos",
    "exp": "Exp",
    "log": "Log",
    "logb": "LogB"
}

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

class Asin(Statement):
    def get_st(self):
        return "mpfr_asin(%s, %s, GMP_RNDN);\n" % (self.assign, self.args[0])

class Cos(Statement):
    def get_st(self):
        return "mpfr_cos(%s, %s, GMP_RNDN);\n" % (self.assign, self.args[0])

class Acos(Statement):
    def get_st(self):
        return "mpfr_acos(%s, %s, GMP_RNDN);\n" % (self.assign, self.args[0])

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

"""

     Operators

"""

class Add_Op(Operator):
    def get_st(self):
        return Add(self.assign, [self.a, self.b]).get_st()

class Sub_Op(Operator):
    def get_st(self):
        return Sub(self.assign, [self.a, self.b]).get_st()

class Mul_Op(Operator):
    def get_st(self):
        return Mul(self.assign, [self.a, self.b]).get_st()

class Div_Op(Operator):
    def get_st(self):
        return Div(self.assign, [self.a, self.b]).get_st()

class Pow_Op(Operator):
    def get_st(self):
        return Pow(self.assign, [self.a, self.b]).get_st()