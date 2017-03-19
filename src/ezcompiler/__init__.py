
EQUALS    =    'EQUALS'
CONSTANT  =  'CONSTANT'
GETARG    =    'GETARG'
VARIABLE  =  'VARIABLE'
FUNCTION  =  'FUNCTION'
KEYWORD   =   'KEYWORD'
STRING    =    'STRING'
TUPLE     =     'TUPLE'
ADD       =         '+'
SUB       =         '-'
MUL       =         '*'
DIV       =         '/'
POW       =         '^'
ET        =        '=='
GT        =         '>'
LT        =         '<'

LPAREN    =         '('
RPAREN    =         ')'
EOF       =       'EOF'


protected_words = ('rof', 'fi', )

operators = (ADD, SUB, MUL, DIV, POW, GT, LT, ET)

operator_tiers = (
	(POW, GT, LT, ET, ), 
	(MUL, DIV, ), 
	(ADD, SUB, )
)

_op_name_dict = {
	POW: "POW",
	MUL: "MUL",
	DIV: "DIV",
	ADD: "ADD",
	SUB: "SUB",

	GT: "GT",
	LT: "LT",
	ET: "ET",

	EQUALS: "SET",
}

_macros = {
	"$": "GETARG",
}

def op_to_name(op):
	if op not in _op_name_dict.keys():
		raise Exception('Invalid operator: {0}'.format(op))
	return _op_name_dict[op]