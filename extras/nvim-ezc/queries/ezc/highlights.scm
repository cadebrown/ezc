; Tree-sitter highlight queries for EZC.
; Mirrors extras/tree-sitter-ezc/queries/highlights.scm but placed where
; Neovim's runtime path expects them.

(comment) @comment
(string) @string
(string (escape_sequence)) @string.escape

(integer) @number
(hex_integer) @number
(typed_integer) @number
(float) @number.float

(type_constructor) @type.builtin
(builtin) @function.builtin

(op_add) @operator
(op_sub) @operator
(op_mul) @operator
(op_div) @operator
(op_mod) @operator
(op_pow) @operator

(op_dup) @operator
(op_drop) @operator
(op_swap) @operator
(op_over) @operator
(op_write) @operator
(op_read) @operator

(op_execute) @keyword
(op_cond) @keyword.conditional
(op_ternary) @keyword.conditional
(op_compose) @operator
(op_loop) @keyword.repeat
(op_map) @function
(op_filter) @function
(op_fold) @function

(op_eq) @operator
(op_ne) @operator
(op_lt) @operator
(op_gt) @operator
(op_lte) @operator
(op_gte) @operator

(bind name: (identifier)) @variable
(recall name: (identifier)) @variable

(block ["(" ")"] @punctuation.bracket)
(list ["[" "]"] @punctuation.bracket)
(scope ["{" "}"] @punctuation.bracket)
