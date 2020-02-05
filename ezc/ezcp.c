// ezc/ezcp.c - the source code for EZC programs, basically the parser
//                & compiler 
//
// Most errors cause a shutdown currently, which is a TODO to make a better
//   error handling system
//
// @author   : Cade Brown <cade@chemicaldevelopment.us>
// @license  : WTFPL (http://www.wtfpl.net/)
// @date     : 2019-11-20
//

#include "ezc-impl.h"

// the characters representings digits in different bases
static const char digitstr[] = EZC_DIGIT_STR;

// returns the digit index from a given char
// i.e. '1' returns 1, 'a' returns 10, etc
static int dig_from_chr(char c) {
    int i;
    for (i = 0; i < sizeof(digitstr) - 1; ++i) {
        if (digitstr[i] == c) return i;
    }
    return -1;
}

// macro to test if a character is a digit
#define IS_DIGIT_BASE(_c, _base) (dig_from_chr(_c) >= 0 && dig_from_chr(_c) < (_base))
// macro to test if a character is whitespace (and can be ignored)
#define IS_WHITE(_c) ((_c) == ' ' || (_c) == '\t' || (_c) == '\n')
// returns true if the character starts an identifier
#define IS_IDENT_START(_c) (((_c) >= 'a' && (_c) <= 'z') || ((_c) >= 'A' && (_c) <= 'Z'))
// returns true if the character could come at some point in the middle of an identifier
#define IS_IDENT_MIDDLE(_c) (IS_IDENT_START(_c) || (_c) == '_')

// initializes a program from a source name and a source string
void ezcp_init(ezcp* ret, ezc_str src_name, ezc_str src) {
    // make copies of the strings
    ezc_str_copy(&ret->src_name, src_name);
    ezc_str_copy(&ret->src, src);

    // start the program off with a blanked body
    ret->body = EZCI_EMPTY;
    ret->body.type = EZCI_BLOCK;
    ret->body._block.children = NULL;
    ret->body._block.n = 0;

    // list of pointers to the hierarchical block parsing structure.
    // This is for nested blocks, like in this example:
    // 1 2 {test1} {test2 {test3 {test4}}}
    // the `blocks` variable would be of size 4.
    // At the start, blocks would be:
    // [ret->body]
    //then, once the first '{' is parsed,
    // [ret->body {test1...}]
    // , but after each '}', the top of the blocks is popped off,
    // since that is no longer where newly parsed instructions should be placed
    // at the point of parsing `test4`, the blocks should have:
    // [ret->body {test2} {test3} {...}]
    // since it is 3 deep past the global block
    ezci** blocks = ezc_malloc(sizeof(ezci*));
    int block_idx = 0;
    blocks[0] = &ret->body;

    // current line/column
    int line = 0, col = 0;
    // where the current token started
    int start_line, start_col;
    char* tokstart;

    // current parse position, and where to stop parsing
    char* str = ret->src._, *stop = ret->src._ + ret->src.len;

    // advances the scanner a single character ,updaing the line and
    //   column variables if newlines are encountered
    #define SCAN_ADVANCE() { if (*str == '\n') { col = 0; line++; } else { col++; } str++; }

    // generates an instruction of a given type, with all the m_* fields (i.e.
    //   the meta fields) filled out according to the parser.
    // NOTE: This should be called after all the calls to SCAN_ADVANCE(), since
    //   it uses the current string - the start of the token to compute length.
    #define GEN_INST(_type) ((ezci){ .type = _type, .m_line = start_line, .m_col = start_col, .m_len = (int)(str - tokstart), .m_prog = ret })

    // This adds a given instruction to the current block, using the `blocks`
    //   variable, thus adding it to the inner-most scope, i.e. within
    //   all the {}'s correctly
    #define ADD_INST(_inst) { \
        blocks[block_idx]->_block.children = ezc_realloc(blocks[block_idx]->_block.children, sizeof(ezci) * (++(blocks[block_idx]->_block.n))); \
        blocks[block_idx]->_block.children[blocks[block_idx]->_block.n-1] = _inst;  \
    }

    // an `else if` block describing a string literal mapping to a builtin VM
    //   instruction
    #define BUILTIN_CASE(_str, _type) else if (strncmp(_str, str, strlen(_str)) == 0) { int _i; for (_i = 0; _i < strlen(_str); ++_i) { SCAN_ADVANCE(); }; ADD_INST(GEN_INST(_type)); }

    while (str < stop) {
        // set the variables describing the start of the current token
        tokstart = str;
        start_line = line;
        start_col = col;

        // get the character we're staring at
        char c = *str;
        if (IS_WHITE(c)) {
            SCAN_ADVANCE();
            // always skip whitespace
            while (IS_WHITE(*str)) {
                SCAN_ADVANCE();
            }
        } else if (c == '{') {
            SCAN_ADVANCE();
            // add on a new block onto the list of blocks,
            // to handle multiple levels of scope within blocks
            ezci new_block = GEN_INST(EZCI_BLOCK);
            new_block._block.n = 0;
            new_block._block.children = NULL;

            // where is it being inserted at in the array of the current top block?
            int inserted_idx = block_idx;

            // add it to its parent's scope
            ADD_INST(new_block);
            // increment the index so newer blocks will fall farther up in the
            //   hierarchy
            block_idx++;

            // reallocate the blocks, and add the newly generated block
            //   to the list of precedent blocks
            blocks = ezc_realloc(blocks, sizeof(struct ezci_block*) * (block_idx+1));
            blocks[block_idx] = &(blocks[inserted_idx]->_block.children[blocks[inserted_idx]->_block.n-1]);

        } else if (c == '}') {
            SCAN_ADVANCE();
            // if they are unbalanced, print an error
            if (block_idx < 0) {
                ezc_error("Extra '}'");
                ezc_printmeta(GEN_INST(EZCI_NONE));
                exit(1);
            }
            
            // just pop off the current block scope
            block_idx--;

        } else if (c == '#') {
            SCAN_ADVANCE();
            while (*str != '\0' && *str != '\n') {
                SCAN_ADVANCE();
            }
            SCAN_ADVANCE();
            // ignore, because its a comment
        }
        // builtins and operators/stuff
        BUILTIN_CASE("==", EZCI_EQ)
        BUILTIN_CASE("<>", EZCI_SWAP)
        BUILTIN_CASE("!", EZCI_EXEC)
        BUILTIN_CASE("`", EZCI_DEL)
        BUILTIN_CASE(":", EZCI_COPY)
        BUILTIN_CASE("_", EZCI_UNDER)
        BUILTIN_CASE("$", EZCI_GET)
        BUILTIN_CASE("+", EZCI_ADD)
        BUILTIN_CASE("-", EZCI_SUB)
        BUILTIN_CASE("*", EZCI_MUL)
        BUILTIN_CASE("/", EZCI_DIV)
        BUILTIN_CASE("%", EZCI_MOD)
        BUILTIN_CASE("^", EZCI_POW)
        BUILTIN_CASE("|", EZCI_WALL)
        else if (c == '.' || IS_DIGIT_BASE(c, 10)) {
            // if its a digit, it means it starts a numeric constant
            char* start = str;
            // whether or not its had a `.` in the constant
            //   (determines whether its an int or real)
            bool had_dot = false;
            // the base to parse, defaults to base 10
            int base = 10;
            
            // value as an int
            ezc_int ival = 0;
            // value as a real
            ezc_real rval = 0.0;

            // base^-i th power, for parsing the fractional
            //   part of real numbers
            ezc_real b2ni = 1.0;
            while (IS_DIGIT_BASE(*str, base) || (!had_dot && *str == '.')) {
                // if its a dot, it means its a real constant,
                //   rather than an integer
                if (*str == '.') {
                    had_dot = true;
                    // start computing real value
                    rval = (ezc_real)ival;
                } else if (had_dot) {
                    rval = rval + (b2ni /= base) * (*str - '0');
                } else {
                    ival = ival * base + (*str - '0');
                }

                // advance to the next character
                SCAN_ADVANCE();
            }

            if (had_dot) {
                // add a literal real instruction
                ezci new_real = GEN_INST(EZCI_REAL);
                new_real._real = rval;
                ADD_INST(new_real);
            } else {
                // add a literal int instruction
                ezci new_int = GEN_INST(EZCI_INT);
                new_int._int = ival;
                ADD_INST(new_int);
            }

        } else if (c == '"') {
            SCAN_ADVANCE();

            // start with parsed
            ezc_str parsed = EZC_STR_NULL;

            while (*str != '\0' && *str != '"') {
                if (*str == '\\') {
                    // escape character
                    SCAN_ADVANCE();
                    // now, do escape code:
                    if (*str == 'n') {
                        ezc_str_append_c(&parsed, '\n');
                    } else if (*str == 't') {
                        ezc_str_append_c(&parsed, '\t');
                    } else if (*str == '0') {
                        ezc_str_append_c(&parsed, '\0');
                    } else {
                        ezc_error("Invalid escape code: '\\%c'", *str);
                        ezc_printmeta(GEN_INST(EZCI_NONE));
                        return;
                    }

                } else {
                    // just append the character
                    ezc_str_append_c(&parsed, *str);
                }
                SCAN_ADVANCE();
            }

            // ensure it was terminated
            if (*str == '"') {
                SCAN_ADVANCE();
            } else {
                ezc_error("Expected Ending '\"'");
                ezc_printmeta(GEN_INST(EZCI_NONE));
                return;
            }

            // add the string
            ezci new_str = GEN_INST(EZCI_STR);
            new_str._str = parsed;
            ADD_INST(new_str);

        } else if (IS_IDENT_START(c)) {
            // parse an identifier
            char* start = str;
            int len = 0;
            while (*str != '\0' && IS_IDENT_MIDDLE(*str)) {
                len++;
                SCAN_ADVANCE();
            }
            // parse from the entire source (which should have no \\ codes,
            //   so should be linear copying)
            ezc_str parsed = EZC_STR_NULL;
            ezc_str_copy_cp(&parsed, start, len);

            // add it as an instruction
            ezci new_str = GEN_INST(EZCI_STR);
            new_str._str = parsed;
            ADD_INST(new_str);
        } else {
            // something wrong happened
            ezc_error("Invalid Character: '%c'", c);
            ezc_printmeta(GEN_INST(EZCI_NONE));
            return;
        }
    }

    // free our hierarchy of blocks (but not the blocks themselves)
    ezc_free(blocks);

    // make sure the blocks were balanced
    if (block_idx != 0) {
        ezc_error("Unbalanced {}'s");
        ezc_printmeta(GEN_INST(EZCI_NONE));
        return;
    }

    return;
}

