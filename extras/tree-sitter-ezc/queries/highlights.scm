; EZC tree-sitter highlight queries

; Comments
(comment) @comment.line

; Strings
(string) @string
(escape_sequence) @string.escape

; Numeric literals
(integer) @number
(hex_integer) @number
(float) @number.float
(typed_integer) @number
(typed_float) @number.float

; Variables
(bind) @variable.parameter
(recall) @variable

; Arithmetic operators
(op_add) @operator
(op_sub) @operator
(op_mul) @operator
(op_div) @operator
(op_mod) @operator
(op_pow) @operator

; Comparison operators
(op_eq) @operator
(op_neq) @operator
(op_lt) @operator
(op_gt) @operator
(op_lteq) @operator
(op_gteq) @operator

; Control operators
(op_execute) @keyword.operator
(op_cond) @keyword.operator
(op_ternary) @keyword.operator
(op_loop) @keyword.repeat
(op_map) @keyword.operator
(op_filter) @keyword.operator
(op_fold) @keyword.operator

; Stack operators
(op_swap) @operator
(op_dup) @operator
(op_drop) @operator
(op_over) @operator

; I/O operators
(op_write) @keyword.operator
(op_read) @keyword.operator

; Compose
(op_compose) @operator

; Type constructors
(type_constructor) @type.builtin

; Built-in I/O functions
(builtin) @function.builtin

; Identifiers
(identifier) @variable

; Delimiters
"(" @punctuation.bracket
")" @punctuation.bracket
"[" @punctuation.bracket
"]" @punctuation.bracket
"{" @punctuation.bracket
"}" @punctuation.bracket
