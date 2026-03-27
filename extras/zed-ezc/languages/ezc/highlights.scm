; Tree-sitter highlight queries for EZC — Zed editor
; Zed uses its own scope names (not the same as TextMate or Neovim).

(comment) @comment
(string) @string
(string (escape_sequence)) @string.escape

(integer) @number
(hex_integer) @number
(typed_integer) @number
(float) @number

(type_constructor) @type

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
(op_cond) @keyword
(op_ternary) @keyword
(op_compose) @operator
(op_loop) @keyword
(op_map) @keyword
(op_filter) @keyword
(op_fold) @keyword

(op_eq) @operator
(op_ne) @operator
(op_lt) @operator
(op_gt) @operator
(op_lte) @operator
(op_gte) @operator

(bind name: (identifier)) @variable
(recall name: (identifier)) @variable

["(" ")" "[" "]" "{" "}"] @punctuation.bracket
