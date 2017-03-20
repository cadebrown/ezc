###             EZC src/ezcompiler/__init__.py v@VERSION@
#
#  EZC is free software; you are free to modify and/or redistribute it under the terms of the GNU GPLv3. See 'LICENSE' for more details.
#
#  Keywords, tokens, and the like for EZC
#
#  TODO:
#
#
###


from token import Token

INTEGER             = 'INTEGER'
PLUS                = 'PLUS'
MINUS               = 'MINUS'
MUL                 = 'MUL'
DIV                 = 'DIV'
LPAREN              = '('
RPAREN              = ')'
ID                  = 'ID'
ASSIGN              = 'ASSIGN'
BEGIN               = 'BEGIN'
END                 = 'END'
SEMI                = 'SEMI'
DOT                 = 'DOT'
EOF                 = 'EOF'

RESERVED_KEYWORDS = {
    'function': Token('FUNCTION', 'FUNCTION')
}
