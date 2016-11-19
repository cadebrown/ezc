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

def get_var_inits():
	init_st = "\n\tmpfr_t %%s; mpfr_init(%%s);" % ()
	ret = "\n\t%s" % (shared.prec)
	for var in shared.var_set:
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

