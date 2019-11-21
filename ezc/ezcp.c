


#include "ezc-impl.h"


#define IS_DIGIT(_c) ((_c) >= '0' && (_c) <= '9')
#define IS_WHITE(_c) ((_c) == ' ' || (_c) == '\t' || (_c) == '\n')
#define IS_IDENT_START(_c) (((_c) >= 'a' && (_c) <= 'z') || ((_c) >= 'A' && (_c) <= 'Z'))
#define IS_IDENT_MIDDLE(_c) (IS_IDENT_START(_c) || (_c) == '_')

// compile into a list of instructions
void ezcp_init(ezcp* ret, ezc_str src_name, ezc_str src) {
    ezc_str_copy(&ret->src_name, src_name);
    ezc_str_copy(&ret->src, src);

    ret->body = EZCI_EMPTY;
    ret->body.type = EZCI_BLOCK;
    ret->body._block.children = NULL;
    ret->body._block.n = 0;

    ezci** blocks = ezc_malloc(sizeof(ezci*));
    int block_idx = 0;
    blocks[0] = &ret->body;

    // list of pointers to blocks
    //struct ezci_block** blocks = ezc_malloc(sizeof(struct ezci_block*));
    //int block_idx = 0;
    //blocks[0] = &ret->body._block;

    int line = 0, col = 0;
    int start_line, start_col;

    char* str = ret->src._, *stop = ret->src._ + ret->src.len;
    char c;

    #define GEN_INST(_type) ((ezci){ .type = _type, .m_line = start_line, .m_col = start_col, .m_len = (int)(str - tokstart), .m_prog = ret })

    #define SCAN_ADVANCE() { if (*str == '\n') { col = 0; line++; } else { col++; } str++; }

    // add it to the current level of parenthesis
    #define ADD_INST(_inst) { \
        blocks[block_idx]->_block.children = ezc_realloc(blocks[block_idx]->_block.children, sizeof(ezci) * (++(blocks[block_idx]->_block.n))); \
        blocks[block_idx]->_block.children[blocks[block_idx]->_block.n-1] = _inst;  \
    }

    #define CASE_STR(_str, _type) else if (strncmp(_str, str, strlen(_str)) == 0) { int _i; for (_i = 0; _i < strlen(_str); ++_i) { SCAN_ADVANCE(); }; ADD_INST(GEN_INST(_type)); }

    char* tokstart = NULL;

    while (str < stop) {
        start_line = line;
        start_col = col;
        tokstart = str;
        c = *str;
        if (IS_WHITE(c)) {
            while (IS_WHITE(*str)) {
                SCAN_ADVANCE();
            }
        } else if (c == '{') {
            SCAN_ADVANCE();

            ezci new_block = GEN_INST(EZCI_BLOCK);
            new_block._block.n = 0;
            new_block._block.children = NULL;

            int inserted_idx = block_idx;

            ADD_INST(new_block);

            block_idx++;
            blocks = ezc_realloc(blocks, sizeof(struct ezci_block*) * (block_idx+1));
            blocks[block_idx] = &(blocks[inserted_idx]->_block.children[blocks[inserted_idx]->_block.n-1]);

        } else if (c == '}') {

            SCAN_ADVANCE();

            if (block_idx < 0) {
                ezc_error("Extra '}'");
                //ezc_printmeta(_c_meta);
                exit(1);
            }

            //printf("finished [%d]\n", blocks[block_idx]->n);
            
            block_idx--;

        } else if (c == '#') {
            SCAN_ADVANCE();
            while (*str != '\0' && *str != '\n') {
                SCAN_ADVANCE();
            }
            SCAN_ADVANCE();
            // ignore, because its a comment
        } 
        CASE_STR("==", EZCI_EQ)
        CASE_STR("<>", EZCI_SWAP)
        CASE_STR("!", EZCI_EXEC)
        CASE_STR("`", EZCI_DEL)
        CASE_STR(":", EZCI_COPY)
        CASE_STR("_", EZCI_UNDER)
        CASE_STR("$", EZCI_GET)
        CASE_STR("+", EZCI_ADD)
        CASE_STR("-", EZCI_SUB)
        CASE_STR("*", EZCI_MUL)
        CASE_STR("/", EZCI_DIV)
        CASE_STR("%", EZCI_MOD)
        CASE_STR("^", EZCI_POW)
        CASE_STR("|", EZCI_WALL)
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
                SCAN_ADVANCE();
            }

            if (had_dot) {
                ezci new_real = GEN_INST(EZCI_REAL);
                new_real._real = rval;
                ADD_INST(new_real);
            } else {
                ezci new_int = GEN_INST(EZCI_INT);
                new_int._int = val;
                ADD_INST(new_int);
            }

        } else if (c == '"') {
            SCAN_ADVANCE();
            ezc_str parsed = EZC_STR_NULL;
            ezc_str_copy_cp(&parsed, "", 0);

            while (*str != '\0' && *str != '"') {
                ezc_str_append_c(&parsed, *str);
                SCAN_ADVANCE();
            }

            if (*str == '"') {
                SCAN_ADVANCE();
            } else {
                ezc_error("Expected Ending '\"'");
                ezc_printmeta(GEN_INST(EZCI_NONE));
                return;
            }

            ezci new_str = GEN_INST(EZCI_STR);
            new_str._str = parsed;
            ADD_INST(new_str);
            //(ezci) { .type = EZCI_STR, ._str = parsed };
            //_c_add(new_str);

        } else if (IS_IDENT_START(c)) {
            char* start = str;
            int len = 0;
            while (*str != '\0' && IS_IDENT_MIDDLE(*str)) {
                len++;
                SCAN_ADVANCE();
            }
            ezc_str parsed = EZC_STR_NULL;
            ezc_str_copy_cp(&parsed, start, len);
            ezci new_str = GEN_INST(EZCI_STR);
            new_str._str = parsed;
            ADD_INST(new_str);
        } else {
            ezc_error("Invalid Character: '%c'", c);
            ezc_printmeta(GEN_INST(EZCI_NONE));
            return;
        }
    }

    ezc_free(blocks);

    if (block_idx != 0) {
        ezc_error("Unbalanced {}'s");
        ezc_printmeta(GEN_INST(EZCI_NONE));
    }

    return;
}




