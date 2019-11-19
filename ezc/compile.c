
#include "ezc-impl.h"

#define IS_DIGIT(_c) ((_c) >= '0' && (_c) <= '9')
#define IS_WHITE(_c) ((_c) == ' ' || (_c) == '\t' || (_c) == '\n')
#define IS_IDENT_START(_c) ((_c) >= 'a' && (_c) <= 'z')
#define IS_IDENT_MIDDLE(_c) (IS_IDENT_START(_c) || (_c) == '_')

// compile into a list of instructions
int ezc_compile(ezcp* ret, ezc_str src_name, ezc_str src) {
    ezc_str_copy(&ret->src_name, src_name);
    ezc_str_copy(&ret->src, src);

    ret->body = EZCI_NONE;
    ret->body.type = EZCI_BLOCK;
    ret->body._block.children = NULL;
    ret->body._block.n = 0;

    // list of pointers to blocks
    struct ezci_block** blocks = ezc_malloc(sizeof(struct ezci_block*));
    int block_idx = 0;
    blocks[0] = &ret->body._block;

    int line = 0, col = 0;
    int start_line, start_col;

    char* str = ret->src._, *stop = ret->src._ + ret->src.len;
    char c;

    #define _c_meta (ezcim){ .line = start_line, .col = start_col, .len = (int)(str - tokstart), .prog = ret }
    #define _c_adv { if (*str == '\n') { col = 0; line++; } else { col++; } str++; }

    // add it to the current level of parenthesis
    #define _c_add(_inst) { \
        blocks[block_idx]->children = ezc_realloc(blocks[block_idx]->children, sizeof(ezci) * (++(blocks[block_idx]->n))); \
        blocks[block_idx]->children[blocks[block_idx]->n-1] = _inst;  \
    }

    #define casec(_c, _type) else if (c == _c) {  _c_adv; ezci new_op = (ezci){ .type = _type, .meta = _c_meta }; _c_add(new_op); }
    #define cases(_str, _type) else if (strncmp(_str, str, strlen(_str)) == 0) { int _i; for (_i = 0; _i < strlen(_str); ++_i) { _c_adv }; ezci new_op = (ezci){ .type = _type, .meta = _c_meta }; _c_add(new_op); }

    char* tokstart = NULL;

    while (str < stop) {
        start_line = line;
        start_col = col;
        tokstart = str;
        c = *str;
        if (IS_WHITE(c)) {
            while (IS_WHITE(*str)) {
                _c_adv;
            }
        } else if (c == '{') {
            _c_adv;

            ezci new_block = EZCI_NONE;
            new_block.type = EZCI_BLOCK;
            new_block._block.n = 0;
            new_block._block.children = NULL;
            new_block.meta = _c_meta;

            int inserted_idx = block_idx;

            _c_add(new_block);

            block_idx++;
            blocks = ezc_realloc(blocks, sizeof(struct ezci_block*) * (block_idx+1));
            blocks[block_idx] = &(blocks[inserted_idx]->children[blocks[inserted_idx]->n-1]._block);

        } else if (c == '}') {

            _c_adv;

            if (block_idx < 0) {
                ezc_error("Extra '}'");
                ezc_printmeta(_c_meta);
                exit(1);
            }

            //printf("finished [%d]\n", blocks[block_idx]->n);
            
            block_idx--;

        } else if (c == '#') {
            _c_adv;
            while (*str != '\0' && *str != '\n') {
                _c_adv;
            }
            _c_adv;
            // ignore, because its a comment
        } 

        cases("==", EZCI_EQ)
        cases("<>", EZCI_SWAP)
        casec('!', EZCI_BANG)
        casec('`', EZCI_DEL)
        casec(':', EZCI_COPY)
        casec('+', EZCI_ADD)
        casec('-', EZCI_SUB)
        casec('*', EZCI_MUL)
        casec('/', EZCI_DIV)
        casec('%', EZCI_MOD)
        casec('^', EZCI_POW)
        cases("_", EZCI_UNDER)
        cases("@", EZCI_PEEK)
        cases("|", EZCI_WALL)
        else if (IS_DIGIT(c)) {
            char* start = str;
            bool had_dot = false;
            ezc_int val = 0;
            ezc_real rval = 0.0;
            ezc_real point = 1.0;
            while (IS_DIGIT(*str) || (!had_dot && *str == '.')) {
            //printf("HERE: %s\n", str);
                // switch over
                if (*str == '.') {
                    had_dot = true;
                    rval = (ezc_real)val;
                } else if (had_dot) {
                    rval = rval + (point /= 10) * (*str - '0');
                } else {
                    val = val * 10 + (*str - '0');
                }
                _c_adv;
            }

            if (had_dot) {
                ezci new_real = (ezci) { .type = EZCI_REAL, ._real = rval };
                _c_add(new_real);
            } else {
                ezci new_int = (ezci) { .type = EZCI_INT, ._int = val };
                _c_add(new_int);
            }

        } else if (c == '"') {
            _c_adv;
            ezc_str parsed = EZC_STR_NULL;
            ezc_str_from(&parsed, "", 0);

            while (*str != '\0' && *str != '"') {
                ezc_str_concatc(&parsed, *str);
                _c_adv;
            }

            if (*str == '"') {
                _c_adv;
            } else {
                ezc_error("Expected Ending '\"'");
                ezc_printmeta(_c_meta);
                return 1;
            }

            ezci new_str = (ezci) { .type = EZCI_STR, ._str = parsed };
            _c_add(new_str);

        } else if (IS_IDENT_START(c)) {
            char* start = str;
            int len = 0;
            while (*str != '\0' && IS_IDENT_MIDDLE(*str)) {
                len++;
                _c_adv;
            }
            ezc_str parsed = EZC_STR_NULL;
            ezc_str_from(&parsed, start, len);
            ezci new_str = (ezci) { .type = EZCI_STR, ._str = parsed };

            _c_add(new_str);
        } else {
            ezc_error("Invalid Character: '%c'", c);
            ezc_printmeta(_c_meta);
            return 1;
        }
    }

    ezc_free(blocks);

    if (block_idx != 0) {
        ezc_error("Unbalanced {}'s");
        ezc_printmeta(_c_meta);
        return 1;
    } else {
        return 0;
    }
}


