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
