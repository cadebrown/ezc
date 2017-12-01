#ifndef __EZCLANG_H__
#define __EZCLANG_H__

#ifndef CALL_FUNCTION
#define CALL_FUNCTION '!'
#endif

#ifndef CAST
#define CAST ':'
#endif

#ifndef ESC
#define ESC '\\'
#endif

#ifndef SEPARATOR
#define SEPARATOR ','
#endif

#define CHAR0 '\0'

#define SPACE ' '

#define NEWLINE '\n'
#define NEWLINE_STR "\n"


#define __IS_SPECIAL(c) (c == CALL_FUNCTION || c == CAST || c == ESC || c == CHAR0 || c == SPACE || c == SEPARATOR)
#define IS_SPECIAL(c) __IS_SPECIAL((c))

#define __IS_QUOTE(c) (c == '\'' || c == '"')
#define IS_QUOTE(c) __IS_QUOTE((c))

#endif
