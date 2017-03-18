
EQUALS    =    'EQUALS'
INTEGER   =   'INTEGER'
VARIABLE  =  'VARIABLE'
ADD       =         '+'
SUB       =         '-'
MUL       =         '*'
DIV       =         '/'
POW       =         '^'

LPAREN    =         '('
RPAREN    =         ')'
EOF       =       'EOF'


operators = (ADD, SUB, MUL, DIV, POW)

operator_tiers = (
	(POW, ), 
	(MUL, DIV, ), 
	(ADD, SUB, )
)

_op_name_dict = {
	POW: "POW",
	MUL: "MUL",
	DIV: "DIV",
	ADD: "ADD",
	SUB: "SUB",

	EQUALS: "SET",
}

def op_to_name(op):
	if op not in _op_name_dict.keys():
		raise Exception('Invalid operator: {0}'.format(op))
	return _op_name_dict[op]
