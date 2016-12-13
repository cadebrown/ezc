"""

Links libraries, variables, and statements

"""


import re
import shared

libs = []

def register_lib(lib):
	global libs
	libs.append(lib)
	shared.start += str(lib)
	for f in lib.funcs:
		shared.functions[f] = lib.funcs[f]
	for o in lib.opers:
		shared.operators[o] = lib.opers[o]

def get_var_inits(exc=[]):
	init_st = "\n\tmpfr_t %%s; mpfr_init(%%s);" % ()
	ret = ""
	for var in shared.var_set.difference(set(exc)):
		ret += init_st % (var, var ) 
	return ret

def set_prec(st):
	shared.prec = st

def get_lib_code():
	global libs
	res = ""
	for l in libs:
		res += l.text
	return res

def reset_vars():
	shared.var_set = set()

def get_prec_init():
	return "\n\t%s" % (shared.prec)