from shared_data import *

# we use other blocks here
from libBasic import *

"""

    Loops lib for EZC

    This includes all loops, and labels and goto

"""

char_st = {
    "label": "Label",
    "goto": "Goto",

    "if": "If",
    "for": "For",

    "fi": "EndLoop",
    "rof": "EndLoop"   
}

char_op = {
    
}

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