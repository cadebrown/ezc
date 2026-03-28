; Tree-sitter highlight queries for EZC — Helix editor
; Place this at: ~/.config/helix/runtime/queries/ezc/highlights.scm

(comment) @comment
(string) @string
(string (escape_sequence)) @constant.character.escape

(integer) @constant.numeric.integer
(hex_integer) @constant.numeric.integer
(typed_integer) @constant.numeric.integer
(float) @constant.numeric.float

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
(op_write) @special
(op_read) @special
(op_execute) @keyword.operator
(op_cond) @keyword.control.conditional
(op_ternary) @keyword.control.conditional
(op_compose) @operator
(op_loop) @keyword.control.repeat
(op_map) @keyword.operator
(op_filter) @keyword.operator
(op_fold) @keyword.operator
(op_eq) @operator
(op_neq) @operator
(op_lt) @operator
(op_gt) @operator
(op_lteq) @operator
(op_gteq) @operator

(bind name: (identifier)) @variable
(recall name: (identifier)) @variable

(block ["(" ")"] @punctuation.bracket)
(list ["[" "]"] @punctuation.bracket)
(scope ["{" "}"] @punctuation.bracket)
