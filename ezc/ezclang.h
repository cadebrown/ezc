#ifndef __EZCLANG_H__
#define __EZCLANG_H__

#ifndef CALL_FUNCTION
#define CALL_FUNCTION "!"
#endif

#ifndef CAST
#define CAST "->"
#endif

#ifndef ESC
#define ESC '\\'
#endif

#ifndef COMMENT
#define COMMENT "#"
#endif

#ifndef SEPARATOR
#define SEPARATOR ','
#endif

#ifndef INTERACTIVE_PROMPT
#define INTERACTIVE_PROMPT " %> "
#endif

#ifndef BLOCK_START
#define BLOCK_START "{"
#endif

#ifndef BLOCK_END
#define BLOCK_END "}"
#endif

#define CHAR0 '\0'

#define SPACE ' '

#define NEWLINE '\n'
#define NEWLINE_STR "\n"

#define BUILTIN_ADD "+"
#define BUILTIN_SUB "-"
#define BUILTIN_MUL "*"
#define BUILTIN_DIV "/"

#define BUILTIN_POW "^"

#define BUILTIN_FLOOR "_"

#define BUILTIN_NEG "~"

#define BUILTIN_COPY ":"

#define BUILTIN_DICTGET "$"
#define BUILTIN_DICTSET "$="

#define BUILTIN_NULL "null"


#define IS_BUILTIN(c) (ISLIM(c, BUILTIN_ADD) || ISLIM(c, BUILTIN_SUB) || ISLIM(c, BUILTIN_COPY) || ISLIM(c, BUILTIN_NEG) || ISLIM(c, BUILTIN_MUL) || ISLIM(c, BUILTIN_DIV) || ISLIM(c, BUILTIN_POW) || ISLIM(c, BUILTIN_FLOOR) || ISLIM(c, BUILTIN_DICTGET) || ISLIM(c, BUILTIN_DICTSET) || ISLIM(c, BUILTIN_NULL))


#include <string.h>


#define __MIN(a, b) ((a) > (b) ? (b) : (a))
#define ISLIM(c, v) (__MIN(strlen(c), strlen(v)) == strlen(v) && strncmp(c, v, __MIN(strlen(c), strlen(v))) == 0)

#define __IS_SPECIAL(c) (ISLIM(c, CALL_FUNCTION) || ISLIM(c, CAST) || ISLIM(c, COMMENT) || *c == ESC || *c == CHAR0 || *c == SPACE || *c == SEPARATOR || IS_BUILTIN(c))
#define IS_SPECIAL(c) __IS_SPECIAL((c))

#define __IS_QUOTE(c) (*c == '\'' || *c == '"')
#define IS_QUOTE(c) __IS_QUOTE((c))

#endif
