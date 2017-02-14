# -*- coding: utf-8 -*-

import compiling.resolve

import parsing


def reset():
	global start, main, end, default_file, user_funcs
	start = ""
	main =  """

int main(int argc, char *argv[]) {
	ezc_init(argc, argv);
	
"""
	end = """
	return 0;
}
"""
	default_file = ""
	user_funcs = ""

printf_convert_fmt= {
	"F": "%.*Rf",
	"sf": "%.*Re",
	"f": "%.*Rf",
	"Z": "%s"
}
printf_convert_args = {
	"F": resolve.printf_F,
	"sf": resolve.printf_sf,
	"f": resolve.printf_f,
	"Z": resolve.printf_Z
}


var = set()
var_lvl = {}
c_lvl = 0

_var = set()
_var_lvl = {}
_c_lvl = 0


is_func = False

"""Variables registered"""

not_vars = []
"""Specific to not use tmp vars (user functions)"""

protected_words = ["RETURN", "NaN", "INF", "NINF", "set", "mpfr_t"]
"""Constants"""

functions = "getprec,time,wait,stime,etime,svar,binomcoef,binompdf,binomcdf,binomcdf_1,normalpdf,normalcdf,erf,free,prompt,if,else,file,fi,for,rof,prec,add,sub,mul,div,pow,mod,\",mpz,var,rawvar,intvar,rawintvar,set,sqrt,\\√,cbrt,min,max,near,trunc,rand,fact,echo,printf,hypot,exp,log,agm,gamma,factorial,zeta,\\ζ,pi,deg,rad,sin,cos,tan,asin,acos,atan,csc,sec,cot,acsc,asec,acot,sinh,cosh,tanh,asinh,acosh,atanh,csch,sech,coth,acsch,asech,acoth".split(",")
"""Callable functions"""

operators = "~,^,choose,*,/,%,+,-,~,?".split(",")
"""Callable operators"""

order_op = [group.split(",") for group in "?,~,choose,,^,,*,/,%,,+,-".split(",,")]
"""Order of operators"""

op_map_funcs = {
	"+": "add",
	"-": "sub",
	"*": "mul",
	"/": "div",
	"^": "pow",
	"%": "mod",
	"~": "near",
	"?": "rand",
	"choose": "binomcoef",
}
"""Maps operators to functions"""

operators_translate_funcs = {
	"__default__": resolve.get_function_translate
}
"""Translates operators to functions"""

functions_alias = {
	"ζ": "zeta",
	"\"": "echo",
	"√": "sqrt"
}
"""Aliases for functions"""

functions_translate_funcs = {
	"__default__": resolve.c_call,
	"echo": resolve.c_echo,
	"printf": resolve.c_printf,
	"prompt": resolve.c_prompt,
	"near": resolve.c_call_optional_call,
	"log": resolve.c_call_optional_call,
	"rand": resolve.c_call_optional_call,
	"prec": resolve.c_prec,
	"if": resolve.c_if_call,
	"for": resolve.c_for_call,
	"fi": resolve.c_endblock,
	"else": resolve.c_elseblock,
	"rof": resolve.c_endblock,
	"file": resolve.c_file,
	"pi": resolve.c_noarg_call
}
"""Type of call for each function"""


type_resolve_dict = {
	"function": resolve.get_function_translate,
	"operator": resolve.get_operator_translate,
	"user_function": resolve.get_user_func_translate
}
"""Types to resolve function"""


includes = """

#include "EZC.h"

#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <mpfr.h>
#include <math.h>

"""

user_funcs=""""""

start=""""""

main = """"""

end = """"""

is_func = False
current_func = None

default_file=""

reset()

needed_var = 0

full_line = ""

parsing.parser.init(functions, operators, order_op)
