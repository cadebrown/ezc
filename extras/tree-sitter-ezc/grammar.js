/// <reference types="tree-sitter-cli/dsl" />

module.exports = grammar({
  name: "ezc",

  extras: ($) => [/\s/],

  rules: {
    source_file: ($) => repeat($._expression),

    _expression: ($) =>
      choice(
        $.comment,
        $.string,
        $.hex_integer,
        $.typed_float,
        $.float,
        $.typed_integer,
        $.integer,
        $.bind,
        $.recall,
        $.block,
        $.list,
        $.scope,
        // Multi-char operators before single-char (order matters)
        $.op_map,
        $.op_filter,
        $.op_fold,
        $.op_ternary,
        $.op_eq,
        $.op_neq,
        $.op_lteq,
        $.op_gteq,
        // Single-char operators
        $.op_add,
        $.op_sub,
        $.op_mul,
        $.op_div,
        $.op_mod,
        $.op_pow,
        $.op_execute,
        $.op_cond,
        $.op_compose,
        $.op_loop,
        $.op_lt,
        $.op_gt,
        $.op_swap,
        $.op_dup,
        $.op_drop,
        $.op_write,
        $.op_read,
        $.op_over,
        // Identifiers last (type constructors + builtins + bare words)
        $.type_constructor,
        $.builtin,
        $.identifier,
      ),

    // ── Comments ──────────────────────────────────────────────────────
    comment: (_$) => token(seq("#", /.*/)),

    // ── String literals ───────────────────────────────────────────────
    string: ($) =>
      seq('"', repeat(choice($.escape_sequence, $.string_content)), '"'),

    escape_sequence: (_$) => token.immediate(seq("\\", /[ntr\\"0]/)),
    string_content: (_$) => token.immediate(prec(-1, /[^"\\]+/)),

    // ── Numeric literals ──────────────────────────────────────────────
    hex_integer: (_$) =>
      token(
        seq(
          "0x",
          /[0-9a-fA-F]+/,
          optional(
            choice(
              "u8", "u16", "u32", "u64", "u128", "u256",
              "i8", "i16", "i32", "i64", "i128", "i256",
            ),
          ),
        ),
      ),

    // Float MUST come before typed_float so the suffix is consumed properly
    typed_float: (_$) =>
      token(
        choice(
          seq(
            /[0-9]+/,
            ".",
            /[0-9]+/,
            optional(seq(/[eE]/, optional(/[+-]/), /[0-9]+/)),
            choice("f16", "f32", "f64"),
          ),
          seq(/[0-9]+/, choice("f16", "f32", "f64")),
        ),
      ),

    float: (_$) =>
      token(
        seq(
          /[0-9]+/,
          ".",
          /[0-9]+/,
          optional(seq(/[eE]/, optional(/[+-]/), /[0-9]+/)),
        ),
      ),

    typed_integer: (_$) =>
      token(
        seq(
          /[0-9]+/,
          choice(
            "u8", "u16", "u32", "u64", "u128", "u256",
            "i8", "i16", "i32", "i64", "i128", "i256",
          ),
        ),
      ),

    integer: (_$) => token(/[0-9]+/),

    // ── Variables ─────────────────────────────────────────────────────
    bind: (_$) => token(seq("@", /[a-zA-Z_][a-zA-Z0-9_]*/)),
    recall: (_$) => token(seq("$", /[a-zA-Z_][a-zA-Z0-9_]*/)),

    // ── Containers ────────────────────────────────────────────────────
    block: ($) => seq("(", repeat($._expression), ")"),
    list: ($) => seq("[", repeat($._expression), "]"),
    scope: ($) => seq("{", repeat($._expression), "}"),

    // ── Multi-character operators ─────────────────────────────────────
    op_map: (_$) => "&!",
    op_filter: (_$) => "&?",
    op_fold: (_$) => "&/",
    op_ternary: (_$) => "??",
    op_eq: (_$) => "==",
    op_neq: (_$) => "!=",
    op_lteq: (_$) => "<=",
    op_gteq: (_$) => ">=",

    // ── Single-character operators ────────────────────────────────────
    op_add: (_$) => "+",
    op_sub: (_$) => "-",
    op_mul: (_$) => "*",
    op_div: (_$) => "/",
    op_mod: (_$) => "%",
    op_pow: (_$) => "^",
    op_execute: (_$) => "!",
    op_cond: (_$) => "?",
    op_compose: (_$) => "|",
    op_loop: (_$) => "&",
    op_lt: (_$) => "<",
    op_gt: (_$) => ">",
    op_swap: (_$) => "~",
    op_dup: (_$) => ",",
    op_drop: (_$) => ";",
    op_write: (_$) => ":",
    op_read: (_$) => ".",
    op_over: (_$) => "_",

    // ── Identifiers ───────────────────────────────────────────────────
    type_constructor: (_$) =>
      token(
        choice(
          "int",
          "u8", "u16", "u32", "u64", "u128", "u256",
          "i8", "i16", "i32", "i64", "i128", "i256",
          "f16", "f32", "f64",
          "str", "bin",
        ),
      ),

    builtin: (_$) => token(choice(
      // I/O
      "rl", "wl", "rb", "wb",
      // Collection
      "len", "nth", "tl", "rev", "srt", "take", "skip", "zip", "range", "typeof", "cut", "cat",
      // Control
      "each", "loop", "import", "words",
      // Higher-order aliases
      "map", "fil", "red",
      // Logic
      "and", "or",
    )),

    identifier: (_$) => token(/[a-zA-Z_][a-zA-Z0-9_]*/),
  },
});
