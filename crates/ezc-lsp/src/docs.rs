//! Operator and built-in documentation for hover and completion.

use ezc::token::{Op, Token};
use tower_lsp::lsp_types::{
    CompletionItem, CompletionItemKind, Documentation, InsertTextFormat, MarkupContent, MarkupKind,
};

/// Return markdown documentation for a token, or `None` if no docs exist.
pub fn token_docs(token: &Token) -> Option<&'static str> {
    Some(match token {
        // ── Arithmetic ───────────────────────────────────────────────────────
        Token::Op(Op::Add) => {
            "**`+`** — Add\n\n\
             ```\na b → (a+b)\n```\n\n\
             Pop two numbers of the same family, push their sum. Promotes to the wider type.\n\n\
             **Example**: `3 4 +` → `7`"
        }
        Token::Op(Op::Sub) => {
            "**`-`** — Subtract\n\n\
             ```\na b → (a-b)\n```\n\n\
             Pop two numbers of the same family, push `a - b`. Promotes to the wider type.\n\n\
             **Example**: `10 3 -` → `7`"
        }
        Token::Op(Op::Mul) => {
            "**`*`** — Multiply\n\n\
             ```\na b → (a*b)\n```\n\n\
             Pop two numbers of the same family, push their product.\n\n\
             **Example**: `3 4 *` → `12`"
        }
        Token::Op(Op::Div) => {
            "**`/`** — Divide\n\n\
             ```\na b → (a/b)\n```\n\n\
             Pop two numbers, push `a / b`. Integer division for integral types; \
             floating-point division for floats. Error on divide by zero.\n\n\
             **Example**: `10 2 /` → `5`"
        }
        Token::Op(Op::Mod) => {
            "**`%`** — Modulo\n\n\
             ```\na b → (a%b)\n```\n\n\
             Pop two numbers, push `a mod b`. Error on divide by zero.\n\n\
             **Example**: `10 3 %` → `1`"
        }
        Token::Op(Op::Pow) => {
            "**`^`** — Power\n\n\
             ```\na b → (a^b)\n```\n\n\
             Pop two numbers, push `a` raised to the power of `b`.\n\n\
             **Example**: `2 10 ^` → `1024`"
        }

        // ── Comparison ───────────────────────────────────────────────────────
        Token::Eq => {
            "**`==`** — Equal\n\n\
             ```\na b → (0 or 1)\n```\n\n\
             Push `1` if `a == b`, else `0`.\n\n\
             **Example**: `3 3 ==` → `1`"
        }
        Token::NotEq => {
            "**`!=`** — Not Equal\n\n\
             ```\na b → (0 or 1)\n```\n\n\
             Push `1` if `a != b`, else `0`.\n\n\
             **Example**: `3 4 !=` → `1`"
        }
        Token::Lt => {
            "**`<`** — Less Than\n\n\
             ```\na b → (0 or 1)\n```\n\n\
             Push `1` if `a < b`, else `0`.\n\n\
             **Example**: `3 4 <` → `1`"
        }
        Token::Gt => {
            "**`>`** — Greater Than\n\n\
             ```\na b → (0 or 1)\n```\n\n\
             Push `1` if `a > b`, else `0`.\n\n\
             **Example**: `4 3 >` → `1`"
        }
        Token::LtEq => {
            "**`<=`** — Less Than or Equal\n\n\
             ```\na b → (0 or 1)\n```\n\n\
             Push `1` if `a <= b`, else `0`.\n\n\
             **Example**: `3 3 <=` → `1`"
        }
        Token::GtEq => {
            "**`>=`** — Greater Than or Equal\n\n\
             ```\na b → (0 or 1)\n```\n\n\
             Push `1` if `a >= b`, else `0`.\n\n\
             **Example**: `4 3 >=` → `1`"
        }

        // ── Stack manipulation ───────────────────────────────────────────────
        Token::Comma => {
            "**`,`** — Dup\n\n\
             ```\na → a a\n```\n\n\
             Duplicate the top of the stack.\n\n\
             **Example**: `5 ,` → `5 5`"
        }
        Token::Tilde => {
            "**`~`** — Swap\n\n\
             ```\na b → b a\n```\n\n\
             Exchange the top two stack elements.\n\n\
             **Example**: `1 2 ~` → `2 1`"
        }
        Token::Underscore => {
            "**`_`** — Over\n\n\
             ```\na b → a b a\n```\n\n\
             Copy the second element to the top of the stack.\n\n\
             **Example**: `1 2 _` → `1 2 1`"
        }
        Token::Semicolon => {
            "**`;`** — Drop\n\n\
             ```\na → \n```\n\n\
             Discard the top of the stack silently.\n\n\
             **Example**: `1 2 ;` → `1`"
        }

        // ── I/O ──────────────────────────────────────────────────────────────
        Token::Colon => {
            "**`:`** — Write\n\n\
             ```\na → \n```\n\n\
             Print the top of the stack followed by a newline, then consume it.\n\n\
             **Example**: `\"hello\" :` → prints `hello`"
        }
        Token::Dot => {
            "**`.`** — Read\n\n\
             ```\n → line\n```\n\n\
             Read a line from stdin and push it as a string.\n\n\
             **Example**: `.` → pushes whatever the user types"
        }

        // ── Control flow ─────────────────────────────────────────────────────
        Token::Bang => {
            "**`!`** — Execute / Splat\n\n\
             ```\nblock → (result of block)\n```\n\n\
             **Block**: Pop a block and evaluate its body in the current scope.\n\n\
             **List**: Pop a list and push all its elements onto the stack (splat).\n\n\
             **String**: Pop a string, evaluate it as EZC source code.\n\n\
             **Example**: `(2 *) !` executes the block; `[1 2 3] !` pushes `1 2 3`"
        }
        Token::Question => {
            "**`?`** — Conditional Execute\n\n\
             ```\ncond (block) → (result of block)  (if cond truthy)\n             →                  (if cond falsy)\n```\n\n\
             Pop block, pop cond. If cond is truthy, execute the block.\n\n\
             **Example**: `1 (42) ?` → `42`; `0 (42) ?` → *(empty)*"
        }
        Token::DoubleQuestion => {
            "**`??`** — If-Else\n\n\
             ```\ncond (then) (else) → (result of then)  (if cond truthy)\n                   → (result of else)  (if cond falsy)\n```\n\n\
             Pop else-block, then-block, cond. Execute the matching branch.\n\n\
             **Example**: `1 (\"yes\") (\"no\") ??` → `\"yes\"`"
        }

        // ── Higher-order ─────────────────────────────────────────────────────
        Token::Amp => {
            "**`&`** — Loop\n\n\
             ```\ncond_block body_block → \n```\n\n\
             Repeatedly execute `body_block` while `cond_block` leaves a truthy value. \
             Both blocks are popped before the loop starts.\n\n\
             **Example**: `1 @i  (i 5 <) (i 1 + @i) &` — counts from 1 to 5"
        }
        Token::AmpBang => {
            "**`&!`** — Map\n\n\
             ```\nlist block → new_list\n```\n\n\
             Apply `block` to each element of `list`, return a new list of results.\n\n\
             **Example**: `[1 2 3] (2 *) &!` → `[2 4 6]`"
        }
        Token::AmpQuestion => {
            "**`&?`** — Filter\n\n\
             ```\nlist block → new_list\n```\n\n\
             Keep elements of `list` where `block` leaves a truthy value.\n\n\
             **Example**: `[1 2 3 4 5] (2 %) &?` → `[1 3 5]`"
        }
        Token::AmpSlash => {
            "**`&/`** — Fold / Reduce\n\n\
             ```\nlist init block → result\n```\n\n\
             Reduce `list` left-to-right. For each element, push `accumulator` and \
             `element`, run `block`, use the result as the new accumulator.\n\n\
             **Example**: `[1 2 3 4 5] 0 (+) &/` → `15`"
        }

        // ── Compose ──────────────────────────────────────────────────────────
        Token::Pipe => {
            "**`|`** — Compose / Concatenate\n\n\
             ```\na b → (a|b)\n```\n\n\
             - **Blocks**: concatenate into a new block\n\
             - **Lists**: concatenate into a new list\n\
             - **Strings**: concatenate into a new string\n\n\
             **Example**: `(1 +) (2 *) |` → block equivalent to `(1 + 2 *)`"
        }

        // ── Type constructors ─────────────────────────────────────────────────
        Token::Ident(name) => match name.as_str() {
            "int" => {
                "**`int`** — Convert to arbitrary-precision integer\n\n\
                 **Example**: `3.14f64 int` → `3`"
            }
            "str" => {
                "**`str`** — Convert to string\n\n\
                 Converts numbers to their decimal string representation. \
                 Strings pass through unchanged.\n\n\
                 **Example**: `42 str` → `\"42\"`"
            }
            "bin" => {
                "**`bin`** — Convert to binary blob\n\n\
                 Converts a string to its UTF-8 bytes. Binary blobs pass through.\n\n\
                 **Example**: `\"hi\" bin` → binary `[68 69]`"
            }
            "f16" => "**`f16`** — Convert to 16-bit float\n\n**Example**: `42 f16`",
            "f32" => "**`f32`** — Convert to 32-bit float\n\n**Example**: `3 f32` → `3.0f32`",
            "f64" => "**`f64`** — Convert to 64-bit float\n\n**Example**: `3 f64` → `3.0`",
            "u8"  => "**`u8`** — Convert to unsigned 8-bit integer (0–255)\n\n**Example**: `200 u8`",
            "u16" => "**`u16`** — Convert to unsigned 16-bit integer",
            "u32" => "**`u32`** — Convert to unsigned 32-bit integer",
            "u64" => "**`u64`** — Convert to unsigned 64-bit integer",
            "u128" => "**`u128`** — Convert to unsigned 128-bit integer",
            "u256" => "**`u256`** — Convert to unsigned 256-bit integer",
            "i8"  => "**`i8`** — Convert to signed 8-bit integer (−128–127)",
            "i16" => "**`i16`** — Convert to signed 16-bit integer",
            "i32" => "**`i32`** — Convert to signed 32-bit integer",
            "i64" => "**`i64`** — Convert to signed 64-bit integer",
            "i128" => "**`i128`** — Convert to signed 128-bit integer",
            "i256" => "**`i256`** — Convert to signed 256-bit integer",
            // I/O builtins
            "rl" => {
                "**`rl`** — Read Line\n\n\
                 ```\n → str\n```\n\n\
                 Read a full line from stdin (without trailing newline), push as string."
            }
            "wl" => {
                "**`wl`** — Write Line\n\n\
                 ```\nstr → \n```\n\n\
                 Write a string to stdout followed by a newline, consume it."
            }
            "rb" => {
                "**`rb`** — Read Byte\n\n\
                 ```\n → int\n```\n\n\
                 Read one byte from stdin, push as integer."
            }
            "wb" => {
                "**`wb`** — Write Byte\n\n\
                 ```\nint → \n```\n\n\
                 Write one byte (from integer value) to stdout."
            }
            // Collection builtins
            "len" => "**`len`** — Length\n\nPush the length of a list, string, or binary.",
            "nth" => "**`nth`** — Index\n\n`list n nth` → element at index n.",
            "tl" => "**`tl`** — Tail\n\nRemove the first element of a list.",
            "rev" => "**`rev`** — Reverse\n\nReverse a list or string.",
            "srt" => "**`srt`** — Sort\n\nSort a list.",
            "take" => "**`take`** — Take\n\n`list/str n take` → first n elements/chars.",
            "skip" => "**`skip`** — Skip\n\n`list/str n skip` → everything after first n.",
            "zip" => "**`zip`** — Zip\n\n`list list zip` → list of pairs.",
            "range" => "**`range`** — Range\n\n`start end range` → `[start start+1 ... end-1]`.",
            "typeof" => "**`typeof`** — Type\n\nPush the type name as a string.",
            "cut" => "**`cut`** — Split\n\n`str delim cut` → list of parts.",
            "cat" => "**`cat`** — Join\n\n`list delim cat` → joined string.",
            // Control flow
            "if" => "**`if`** — Conditional\n\n`cond (block) if` → execute if truthy. Alias for `?`.",
            "ifel" => "**`ifel`** — If-Else\n\n`cond (then) (else) ifel` → execute one branch. Alias for `??`.",
            "each" => "**`each`** — Iterate\n\n`list/n (block) each` → run block for each element.",
            "loop" => "**`loop`** — While Loop\n\n`(cond) (body) loop` → loop while cond truthy. Alias for `&`.",
            "import" => "**`import`** — Import\n\n`\"path\" import` → load and eval a file. Checks embedded std modules first.",
            "words" => "**`words`** — List Definitions\n\nPush a list of all defined names.",
            // Higher-order aliases
            "map" => "**`map`** — Map\n\n`list/n (block) map` → apply block to each, collect results. Alias for `&!`.",
            "fil" => "**`fil`** — Filter\n\n`list/n (block) fil` → keep truthy results. Alias for `&?`.",
            "red" => "**`red`** — Reduce/Fold\n\n`list init (block) red` → fold. Alias for `&/`.",
            _ => return None,
        },

        // Variables: show what they do generically
        Token::Bind(name) => {
            // Dynamic docs for bind — we can't easily return an owned string here,
            // so fall through and let the caller handle it.
            let _ = name;
            return None;
        }
        Token::Recall(name) => {
            let _ = name;
            return None;
        }

        _ => return None,
    })
}

// ── Completion items ──────────────────────────────────────────────────────────

fn op_item(label: &str, detail: &str, doc: &str) -> CompletionItem {
    CompletionItem {
        label: label.into(),
        kind: Some(CompletionItemKind::OPERATOR),
        detail: Some(detail.into()),
        documentation: Some(Documentation::MarkupContent(MarkupContent {
            kind: MarkupKind::Markdown,
            value: doc.into(),
        })),
        insert_text: Some(label.into()),
        insert_text_format: Some(InsertTextFormat::PLAIN_TEXT),
        ..Default::default()
    }
}

fn type_item(label: &str, detail: &str) -> CompletionItem {
    CompletionItem {
        label: label.into(),
        kind: Some(CompletionItemKind::TYPE_PARAMETER),
        detail: Some(detail.into()),
        insert_text: Some(label.into()),
        insert_text_format: Some(InsertTextFormat::PLAIN_TEXT),
        ..Default::default()
    }
}

fn builtin_item(label: &str, detail: &str) -> CompletionItem {
    CompletionItem {
        label: label.into(),
        kind: Some(CompletionItemKind::FUNCTION),
        detail: Some(detail.into()),
        insert_text: Some(label.into()),
        insert_text_format: Some(InsertTextFormat::PLAIN_TEXT),
        ..Default::default()
    }
}

/// Build the full completion list including variable names from the document.
pub fn completion_items(vars: &[String]) -> Vec<CompletionItem> {
    let mut items = vec![
        // ── Arithmetic ───────────────────────────────────────────────────────
        op_item("+", "a b → (a+b)", "Add two numbers"),
        op_item("-", "a b → (a-b)", "Subtract"),
        op_item("*", "a b → (a*b)", "Multiply"),
        op_item("/", "a b → (a/b)", "Divide"),
        op_item("%", "a b → (a%b)", "Modulo"),
        op_item("^", "a b → (a^b)", "Power"),
        // ── Comparison ──────────────────────────────────────────────────────
        op_item("==", "a b → 0|1", "Equal"),
        op_item("!=", "a b → 0|1", "Not equal"),
        op_item("<", "a b → 0|1", "Less than"),
        op_item(">", "a b → 0|1", "Greater than"),
        op_item("<=", "a b → 0|1", "Less than or equal"),
        op_item(">=", "a b → 0|1", "Greater than or equal"),
        // ── Stack ────────────────────────────────────────────────────────────
        op_item(",", "a → a a", "Dup: duplicate top"),
        op_item("~", "a b → b a", "Swap: exchange top two"),
        op_item("_", "a b → a b a", "Over: copy second to top"),
        op_item(";", "a → ", "Drop: discard top"),
        // ── I/O ──────────────────────────────────────────────────────────────
        op_item(":", "a → ", "Write: print top with newline"),
        op_item(".", " → str", "Read: read line from stdin"),
        // ── Control ──────────────────────────────────────────────────────────
        op_item(
            "!",
            "block → ...",
            "Execute block / splat list / eval string",
        ),
        op_item("?", "cond (block) →", "Conditional execute"),
        op_item("??", "cond (then) (else) →", "If-else execute"),
        op_item("&", "cond body → ", "Loop while condition truthy"),
        op_item(
            "&!",
            "list block → list",
            "Map: apply block to each element",
        ),
        op_item("&?", "list block → list", "Filter: keep truthy elements"),
        op_item(
            "&/",
            "list init block → result",
            "Fold/reduce with initial value",
        ),
        // ── Compose ──────────────────────────────────────────────────────────
        op_item(
            "|",
            "a b → (a|b)",
            "Compose/concatenate blocks, lists, or strings",
        ),
        // ── Type constructors ────────────────────────────────────────────────
        type_item("int", "→ arbitrary-precision integer"),
        type_item("str", "→ string"),
        type_item("bin", "→ binary blob"),
        type_item("f16", "→ f16"),
        type_item("f32", "→ f32"),
        type_item("f64", "→ f64"),
        type_item("u8", "→ u8"),
        type_item("u16", "→ u16"),
        type_item("u32", "→ u32"),
        type_item("u64", "→ u64"),
        type_item("u128", "→ u128"),
        type_item("u256", "→ u256"),
        type_item("i8", "→ i8"),
        type_item("i16", "→ i16"),
        type_item("i32", "→ i32"),
        type_item("i64", "→ i64"),
        type_item("i128", "→ i128"),
        type_item("i256", "→ i256"),
        // ── I/O builtins ─────────────────────────────────────────────────────
        builtin_item("rl", " → str  (read line)"),
        builtin_item("wl", "str →   (write line)"),
        builtin_item("rb", " → int  (read byte)"),
        builtin_item("wb", "int →   (write byte)"),
        // ── Collection builtins ──────────────────────────────────────────────
        builtin_item("len", "list/str/bin → n  (length)"),
        builtin_item("nth", "list n → elem  (index)"),
        builtin_item("tl", "list → list  (tail)"),
        builtin_item("rev", "list/str → list/str  (reverse)"),
        builtin_item("srt", "list → list  (sort)"),
        builtin_item("take", "list/str n → list/str  (first n)"),
        builtin_item("skip", "list/str n → list/str  (drop first n)"),
        builtin_item("zip", "list list → list  (zip pairs)"),
        builtin_item("range", "start end → list  (integer range)"),
        builtin_item("typeof", "a → str  (type name)"),
        builtin_item("cut", "str delim → list  (split)"),
        builtin_item("cat", "list delim → str  (join)"),
        // ── Control builtins ─────────────────────────────────────────────────
        builtin_item("each", "list/n (block) → ...  (iterate)"),
        builtin_item("loop", "(cond) (body) →  (while loop)"),
        builtin_item("import", "\"path\" →  (load file)"),
        builtin_item("words", " → list  (defined names)"),
        // ── Higher-order aliases ─────────────────────────────────────────────
        builtin_item("map", "list/n (block) → list  (map)"),
        builtin_item("fil", "list/n (block) → list  (filter)"),
        builtin_item("red", "list init (block) → result  (fold)"),
        // ── Logic ────────────────────────────────────────────────────────────
        builtin_item("and", "a b → 0|1  (logical and)"),
        builtin_item("or", "a b → 0|1  (logical or)"),
    ];

    // Variable completions from the document
    for name in vars {
        items.push(CompletionItem {
            label: format!("${name}"),
            kind: Some(CompletionItemKind::VARIABLE),
            detail: Some("variable".into()),
            insert_text: Some(name.clone()),
            insert_text_format: Some(InsertTextFormat::PLAIN_TEXT),
            sort_text: Some(format!("0_{name}")), // sort variables first
            ..Default::default()
        });
    }

    items
}
