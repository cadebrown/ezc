//! Integration tests for the ezc language server's core logic.
//!
//! Tests the `SymbolIndex`, `BuiltinSets`, formatting, folding, and code action
//! helpers without running the actual LSP server.

use ezc_lsp::symbols::{BuiltinSets, SemanticClass, SymbolIndex};

// ── SymbolIndex: definitions ─────────────────────────────────────────────────

#[test]
fn symbol_index_single_definition() {
    let builtins = BuiltinSets::new();
    let src = "(, *) @square  5 $square !";
    let index = SymbolIndex::build(src, &builtins);
    assert_eq!(index.definition_sites("square").len(), 1);
}

#[test]
fn symbol_index_all_occurrences() {
    let builtins = BuiltinSets::new();
    let src = "(, *) @square  5 $square !";
    let index = SymbolIndex::build(src, &builtins);
    let occ = index.all_occurrences("square");
    assert_eq!(occ.len(), 2, "should find @square and $square");
}

#[test]
fn symbol_index_multiple_definitions() {
    let builtins = BuiltinSets::new();
    let src = "1 @x 2 @x $x";
    let index = SymbolIndex::build(src, &builtins);
    assert_eq!(index.definition_sites("x").len(), 2);
    assert_eq!(index.all_occurrences("x").len(), 3);
}

// ── SymbolIndex: references ──────────────────────────────────────────────────

#[test]
fn bare_word_ref_not_builtin() {
    let builtins = BuiltinSets::new();
    let src = "(, *) @square  5 square";
    let index = SymbolIndex::build(src, &builtins);
    // bare "square" is a user-defined name, should be in references
    assert!(
        index.references.contains_key("square"),
        "bare user ident should be tracked as reference"
    );
}

#[test]
fn builtin_not_indexed_as_ref() {
    let builtins = BuiltinSets::new();
    let src = "[1 2 3] len";
    let index = SymbolIndex::build(src, &builtins);
    assert!(
        !index.references.contains_key("len"),
        "builtin 'len' should NOT appear in references"
    );
}

#[test]
fn recall_is_reference() {
    let builtins = BuiltinSets::new();
    let src = "42 @x $x $x";
    let index = SymbolIndex::build(src, &builtins);
    let refs = index.references.get("x").expect("should have refs for x");
    assert_eq!(refs.len(), 2, "two $x recalls");
}

// ── SymbolIndex: is_function_def ─────────────────────────────────────────────

#[test]
fn is_function_def_after_close_paren() {
    let builtins = BuiltinSets::new();
    let src = "(, *) @square  42 @x";
    let index = SymbolIndex::build(src, &builtins);

    let sq_defs = index.definition_sites("square");
    assert!(!sq_defs.is_empty());
    assert!(
        index.is_function_def(&sq_defs[0]),
        "@square is preceded by ) → function def"
    );

    let x_defs = index.definition_sites("x");
    assert!(!x_defs.is_empty());
    assert!(
        !index.is_function_def(&x_defs[0]),
        "@x is preceded by 42 → NOT function def"
    );
}

// ── SymbolIndex: name_at ─────────────────────────────────────────────────────

#[test]
fn name_at_bind() {
    let builtins = BuiltinSets::new();
    let src = "5 @x $x";
    let index = SymbolIndex::build(src, &builtins);

    // "@x" starts at offset 2, ends at 4
    assert_eq!(index.name_at(2), Some("x"), "offset 2 is inside @x");
    assert_eq!(index.name_at(3), Some("x"), "offset 3 is inside @x");
}

#[test]
fn name_at_recall() {
    let builtins = BuiltinSets::new();
    let src = "5 @x $x";
    let index = SymbolIndex::build(src, &builtins);

    // "$x" starts at offset 5, ends at 7
    assert_eq!(index.name_at(5), Some("x"), "offset 5 is inside $x");
    assert_eq!(index.name_at(6), Some("x"), "offset 6 is inside $x");
}

#[test]
fn name_at_whitespace_returns_none() {
    let builtins = BuiltinSets::new();
    let src = "5 @x $x";
    let index = SymbolIndex::build(src, &builtins);

    assert_eq!(index.name_at(1), None, "offset 1 is whitespace");
    assert_eq!(index.name_at(4), None, "offset 4 is whitespace");
}

#[test]
fn name_at_number_returns_none() {
    let builtins = BuiltinSets::new();
    let src = "42 @x";
    let index = SymbolIndex::build(src, &builtins);

    assert_eq!(index.name_at(0), None, "offset 0 is number '4'");
}

// ── SymbolIndex: defined_names ───────────────────────────────────────────────

#[test]
fn defined_names_source_order() {
    let builtins = BuiltinSets::new();
    let src = "(, *) @square\n42 @x\n(, square square) @fourth";
    let index = SymbolIndex::build(src, &builtins);

    let names: Vec<&str> = index.defined_names().iter().map(|(n, _)| *n).collect();
    assert_eq!(names.len(), 3);
    assert_eq!(names[0], "square");
    assert_eq!(names[1], "x");
    assert_eq!(names[2], "fourth");
}

#[test]
fn defined_names_empty_source() {
    let builtins = BuiltinSets::new();
    let src = "1 2 +";
    let index = SymbolIndex::build(src, &builtins);
    assert!(index.defined_names().is_empty());
}

// ── SymbolIndex: token_at ────────────────────────────────────────────────────

#[test]
fn token_at_returns_correct_token() {
    let builtins = BuiltinSets::new();
    let src = "3 4 +";
    let index = SymbolIndex::build(src, &builtins);

    // "3" at offset 0
    let (tok, _) = index.token_at(0).expect("token at offset 0");
    assert!(
        matches!(tok, ezc::token::Token::Int(v) if v == "3"),
        "expected Int(3)"
    );

    // "+" at offset 4
    let (tok, _) = index.token_at(4).expect("token at offset 4");
    assert!(
        matches!(tok, ezc::token::Token::Op(ezc::token::Op::Add)),
        "expected Op(Add)"
    );
}

// ── BuiltinSets ──────────────────────────────────────────────────────────────

#[test]
fn builtin_known_builtins() {
    let builtins = BuiltinSets::new();
    assert!(builtins.is_known("len"), "len is a builtin");
    assert!(builtins.is_known("int"), "int is a type constructor");
    assert!(builtins.is_known("if"), "if is control flow");
    assert!(!builtins.is_known("myFunc"), "myFunc is not known");
}

#[test]
fn builtin_prelude_names() {
    let builtins = BuiltinSets::new();
    // Prelude defines things like "dvb" (decimal-verbose), etc.
    // Any @name in the prelude should be known.
    // We can't assert specific names without reading the prelude, but
    // is_known should return true for prelude-defined names.
    // At minimum, the prelude should define some names.
    assert!(
        !builtins.prelude_names.is_empty(),
        "prelude should define at least one name"
    );
}

#[test]
fn builtin_classify_type() {
    let builtins = BuiltinSets::new();
    assert_eq!(builtins.classify("int"), SemanticClass::Type);
    assert_eq!(builtins.classify("f64"), SemanticClass::Type);
    assert_eq!(builtins.classify("u8"), SemanticClass::Type);
    assert_eq!(builtins.classify("str"), SemanticClass::Type);
}

#[test]
fn builtin_classify_keyword() {
    let builtins = BuiltinSets::new();
    assert_eq!(builtins.classify("if"), SemanticClass::Keyword);
    assert_eq!(builtins.classify("ifel"), SemanticClass::Keyword);
    assert_eq!(builtins.classify("loop"), SemanticClass::Keyword);
    assert_eq!(builtins.classify("each"), SemanticClass::Keyword);
    assert_eq!(builtins.classify("import"), SemanticClass::Keyword);
}

#[test]
fn builtin_classify_function() {
    let builtins = BuiltinSets::new();
    assert_eq!(builtins.classify("len"), SemanticClass::Function);
    assert_eq!(builtins.classify("rev"), SemanticClass::Function);
    assert_eq!(builtins.classify("srt"), SemanticClass::Function);
}

#[test]
fn builtin_classify_user_variable() {
    let builtins = BuiltinSets::new();
    assert_eq!(builtins.classify("myFunc"), SemanticClass::Variable);
    assert_eq!(builtins.classify("foo"), SemanticClass::Variable);
}

#[test]
fn builtin_all_names() {
    let builtins = BuiltinSets::new();
    let names = builtins.all_names();
    assert!(names.contains(&"len"), "all_names should include builtins");
    assert!(
        names.contains(&"int"),
        "all_names should include type constructors"
    );
    assert!(
        names.contains(&"if"),
        "all_names should include control flow"
    );
}

// ── Formatting ───────────────────────────────────────────────────────────────

#[test]
fn format_normalize_spaces() {
    let src = "3   4    +";
    let formatted = ezc_lsp::format_source(src);
    assert_eq!(formatted, "3 4 +\n");
}

#[test]
fn format_preserves_comments() {
    let src = "3 4 + # this is a comment";
    let formatted = ezc_lsp::format_source(src);
    assert_eq!(formatted, "3 4 + # this is a comment\n");
}

#[test]
fn format_trailing_newline() {
    let src = "3 4 +";
    let formatted = ezc_lsp::format_source(src);
    assert!(
        formatted.ends_with('\n'),
        "formatted output should end with newline"
    );
}

#[test]
fn format_multiline() {
    let src = "1  2  +\n3  4  *";
    let formatted = ezc_lsp::format_source(src);
    assert_eq!(formatted, "1 2 +\n3 4 *\n");
}

#[test]
fn format_preserves_strings() {
    let src = "\"hello   world\"  :";
    let formatted = ezc_lsp::format_source(src);
    assert_eq!(formatted, "\"hello   world\" :\n");
}

#[test]
fn format_strips_trailing_blank_lines() {
    let src = "1 2 +\n\n\n";
    let formatted = ezc_lsp::format_source(src);
    assert_eq!(formatted, "1 2 +\n");
}

// ── Folding ──────────────────────────────────────────────────────────────────

#[test]
fn folding_multiline_block() {
    let src = "(\n  1 +\n  2 *\n)";
    let ranges = ezc_lsp::compute_folding_ranges(src);
    assert_eq!(ranges.len(), 1, "one fold for the block");
    assert_eq!(ranges[0].start_line, 0);
    assert_eq!(ranges[0].end_line, 3);
}

#[test]
fn folding_nested_blocks() {
    let src = "(\n  [\n    1 2 3\n  ]\n)";
    let ranges = ezc_lsp::compute_folding_ranges(src);
    assert_eq!(ranges.len(), 2, "folds for ( and [");
}

#[test]
fn folding_single_line_block_no_fold() {
    let src = "(1 2 +)";
    let ranges = ezc_lsp::compute_folding_ranges(src);
    assert!(
        ranges.is_empty(),
        "single-line blocks should not produce folds"
    );
}

#[test]
fn folding_comment_block() {
    let src = "# line 1\n# line 2\n# line 3\n1 2 +";
    let ranges = ezc_lsp::compute_folding_ranges(src);
    let comment_ranges: Vec<_> = ranges
        .iter()
        .filter(|r| r.kind == Some(tower_lsp::lsp_types::FoldingRangeKind::Comment))
        .collect();
    assert_eq!(comment_ranges.len(), 1, "consecutive comments fold");
    assert_eq!(comment_ranges[0].start_line, 0);
    assert_eq!(comment_ranges[0].end_line, 2);
}

// ── Code action helpers ──────────────────────────────────────────────────────

#[test]
fn find_close_matches_basic() {
    let candidates = vec!["square", "sqrt", "string", "len"];
    let matches = ezc_lsp::find_close_matches("squre", &candidates, 3);
    assert!(
        !matches.is_empty(),
        "should find at least one close match for 'squre'"
    );
    assert_eq!(
        matches[0], "square",
        "closest match for 'squre' should be 'square'"
    );
}

#[test]
fn find_close_matches_no_match() {
    let candidates = vec!["x", "y", "z"];
    let matches = ezc_lsp::find_close_matches("abcdefghijklmnop", &candidates, 3);
    assert!(
        matches.is_empty(),
        "very different strings should produce no matches"
    );
}

#[test]
fn find_close_matches_exact_excluded() {
    let candidates = vec!["foo", "bar"];
    let matches = ezc_lsp::find_close_matches("foo", &candidates, 3);
    assert!(
        !matches.contains(&"foo"),
        "exact match should be excluded from suggestions"
    );
}

#[test]
fn find_close_matches_respects_max() {
    let candidates = vec!["aa", "ab", "ac", "ad"];
    let matches = ezc_lsp::find_close_matches("a", &candidates, 2);
    assert!(matches.len() <= 2, "should return at most max results");
}

// ── Integration flow tests ───────────────────────────────────────────────────

#[test]
fn goto_definition_flow() {
    let builtins = BuiltinSets::new();
    let src = "(, *) @square\n5 $square !";
    let index = SymbolIndex::build(src, &builtins);

    // Find "$square" — it starts after "5 " on line 2
    // Line 1: "(, *) @square\n" = 14 chars
    // Line 2: "5 $square !" — $square at offset 16
    let name = index.name_at(16).expect("should find name at offset 16");
    assert_eq!(name, "square");

    let defs = index.definition_sites(name);
    assert_eq!(defs.len(), 1, "one definition of square");
    // Definition @square spans within the first line
    assert!(defs[0].start < 14, "definition should be on the first line");
}

#[test]
fn rename_flow() {
    let builtins = BuiltinSets::new();
    let src = "(, *) @square  5 square";
    let index = SymbolIndex::build(src, &builtins);

    let occurrences = index.all_occurrences("square");
    assert_eq!(occurrences.len(), 2, "should find @square and bare square");
}

#[test]
fn document_symbols_flow() {
    let builtins = BuiltinSets::new();
    let src = "(, *) @square\n42 @x\n(, square square) @fourth";
    let index = SymbolIndex::build(src, &builtins);

    let names: Vec<&str> = index.defined_names().iter().map(|(n, _)| *n).collect();
    assert_eq!(names, vec!["square", "x", "fourth"]);

    // square is a function def, x is not, fourth is
    let sq = index.definition_sites("square");
    assert!(index.is_function_def(&sq[0]));

    let x = index.definition_sites("x");
    assert!(!index.is_function_def(&x[0]));

    let fourth = index.definition_sites("fourth");
    assert!(index.is_function_def(&fourth[0]));
}

#[test]
fn references_include_both_recall_and_bare() {
    let builtins = BuiltinSets::new();
    let src = "(, *) @f  $f  f";
    let index = SymbolIndex::build(src, &builtins);

    // @f is a definition, $f is a recall reference, bare f is an ident reference
    let refs = index
        .references
        .get("f")
        .expect("should have references for f");
    assert_eq!(refs.len(), 2, "should have $f and bare f as references");

    let all = index.all_occurrences("f");
    assert_eq!(all.len(), 3, "1 def + 2 refs = 3 total");
}

#[test]
fn empty_source_builds_without_panic() {
    let builtins = BuiltinSets::new();
    let index = SymbolIndex::build("", &builtins);
    assert!(index.tokens.is_empty());
    assert!(index.definitions.is_empty());
    assert!(index.references.is_empty());
    assert!(index.defined_names().is_empty());
    assert_eq!(index.name_at(0), None);
    assert_eq!(index.token_at(0), None);
}

#[test]
fn comment_only_source() {
    let builtins = BuiltinSets::new();
    let src = "# just a comment\n# another";
    let index = SymbolIndex::build(src, &builtins);
    assert!(index.tokens.is_empty());
    assert!(index.definitions.is_empty());
}

#[test]
fn complex_program_index() {
    let builtins = BuiltinSets::new();
    let src = r#"
# FizzBuzz
1 @i
($i 100 <=) (
    $i 15 % 0 == ($i 3 % 0 ==) ($i 5 % 0 ==)
    ("FizzBuzz" :) ("Fizz" :) ("Buzz" :) ($i :)
    $i 1 + @i
) &
"#;
    let index = SymbolIndex::build(src, &builtins);
    assert_eq!(
        index.definition_sites("i").len(),
        2,
        "i is defined twice (initial + increment)"
    );
    assert!(
        index.references.get("i").map(|v| v.len()).unwrap_or(0) >= 4,
        "i is referenced multiple times via $i"
    );
}

// ── Stack state computation ─────────────────────────────────────────

#[test]
fn test_stack_state_simple_arithmetic() {
    let states = ezc_lsp::compute_stack_states("3 4 +", 6);
    assert_eq!(states.len(), 1);
    assert_eq!(states[0].0, 0); // line 0
    assert!(states[0].1.contains("[7]"));
}

#[test]
fn test_stack_state_multiline() {
    let states = ezc_lsp::compute_stack_states("3 4 +\n2 *", 6);
    assert_eq!(states.len(), 2);
    assert!(states[0].1.contains("[7]")); // line 0
    assert!(states[1].1.contains("[14]")); // line 1
}

#[test]
fn test_stack_state_variable_binding() {
    let states = ezc_lsp::compute_stack_states("42 @x", 6);
    assert_eq!(states.len(), 1);
    assert!(states[0].1.contains("[]")); // stack empty after binding
}

#[test]
fn test_stack_state_error_stops() {
    let states = ezc_lsp::compute_stack_states("3 +\n2 *", 6);
    assert_eq!(states.len(), 1); // only line 0 (error), line 1 skipped
    assert!(states[0].1.contains("⚠"));
}

#[test]
fn test_stack_state_function_def_and_call() {
    let states = ezc_lsp::compute_stack_states("(, *) @sq\n5 sq", 6);
    assert_eq!(states.len(), 2);
    assert!(states[0].1.contains("[]")); // block bound, stack empty
    assert!(states[1].1.contains("[25]")); // 5 squared
}

#[test]
fn test_stack_state_empty_source() {
    let states = ezc_lsp::compute_stack_states("", 6);
    assert!(states.is_empty());
}

#[test]
fn test_stack_state_comment_only() {
    let states = ezc_lsp::compute_stack_states("# just a comment", 6);
    assert!(states.is_empty()); // comments produce no AST
}

#[test]
fn test_stack_state_truncation() {
    // Push many values, verify truncation
    let states = ezc_lsp::compute_stack_states("1 2 3 4 5 6 7 8 9 10", 3);
    assert_eq!(states.len(), 1);
    assert!(states[0].1.contains("...7")); // 7 values truncated
}

#[test]
fn test_stack_state_prelude_functions() {
    // Prelude functions like sq, neg, sum should work
    let states = ezc_lsp::compute_stack_states("7 sq", 10);
    assert_eq!(states.len(), 1);
    assert!(states[0].1.contains("[49]"));
}

#[test]
fn test_stack_state_io_noop() {
    // I/O operations (: . rl wl) should not block — uses NoopIo
    let states = ezc_lsp::compute_stack_states("42 :", 10);
    assert_eq!(states.len(), 1);
    // : consumes the value, stack should be empty
    assert!(states[0].1.contains("[]"));
}

#[test]
fn test_stack_state_multi_expr_single_line() {
    // Multiple expressions on one line — shows state after LAST expression
    let states = ezc_lsp::compute_stack_states("3 4 + 2 *", 10);
    assert_eq!(states.len(), 1);
    assert!(states[0].1.contains("[14]")); // not [7]
}

#[test]
fn test_stack_state_list_operations() {
    let states = ezc_lsp::compute_stack_states("[1 2 3] (2 *) map", 10);
    assert_eq!(states.len(), 1);
    assert!(states[0].1.contains("[2 4 6]"));
}

#[test]
fn test_stack_state_string() {
    let states = ezc_lsp::compute_stack_states("\"hello\"", 10);
    assert_eq!(states.len(), 1);
    assert!(states[0].1.contains("\"hello\""));
}

#[test]
fn test_stack_state_scope() {
    // Scoped block: bindings don't leak
    let states = ezc_lsp::compute_stack_states("{ 10 @x $x 2 * }", 10);
    assert_eq!(states.len(), 1);
    assert!(states[0].1.contains("[20]"));
}

#[test]
fn test_stack_state_infinite_loop_limited() {
    // Infinite loop should hit step limit, not hang
    let states = ezc_lsp::compute_stack_states("1 (1) (1) &", 10);
    assert_eq!(states.len(), 1);
    assert!(states[0].1.contains("⚠")); // step limit error
}

// ── format_stack_display ────────────────────────────────────────────────────

#[test]
fn test_format_stack_display_empty() {
    let stack: Vec<&ezc::types::Value> = vec![];
    assert_eq!(ezc_lsp::format_stack_display(&stack, 10), "[]");
}

#[test]
fn test_format_stack_display_single() {
    let v = ezc::types::Value::int(42);
    let stack = vec![&v];
    assert_eq!(ezc_lsp::format_stack_display(&stack, 10), "[42]");
}

#[test]
fn test_format_stack_display_multiple() {
    let a = ezc::types::Value::int(1);
    let b = ezc::types::Value::int(2);
    let c = ezc::types::Value::int(3);
    let stack = vec![&a, &b, &c];
    assert_eq!(ezc_lsp::format_stack_display(&stack, 10), "[1 2 3]");
}

#[test]
fn test_format_stack_display_truncation() {
    let vals: Vec<ezc::types::Value> = (1..=10).map(|i| ezc::types::Value::int(i)).collect();
    let stack: Vec<&ezc::types::Value> = vals.iter().collect();
    let display = ezc_lsp::format_stack_display(&stack, 3);
    assert!(display.contains("...7"));
    assert!(display.contains("8 9 10"));
    assert!(!display.contains("1 2"));
}

// ── Folding: additional cases ───────────────────────────────────────────────

#[test]
fn folding_comment_lines() {
    let src = "# line 1\n# line 2\n# line 3\n42";
    let ranges = ezc_lsp::compute_folding_ranges(src);
    let comment_folds: Vec<_> = ranges
        .iter()
        .filter(|r| r.kind == Some(tower_lsp::lsp_types::FoldingRangeKind::Comment))
        .collect();
    assert!(!comment_folds.is_empty());
}

#[test]
fn folding_scope_block() {
    let src = "{\n  10 @x\n  $x\n}";
    let ranges = ezc_lsp::compute_folding_ranges(src);
    assert!(!ranges.is_empty());
}

// ── Formatting: additional cases ────────────────────────────────────────────

#[test]
fn format_empty_source() {
    assert_eq!(ezc_lsp::format_source(""), "\n");
}

#[test]
fn format_multiple_spaces_between_tokens() {
    assert_eq!(ezc_lsp::format_source("3    4     +"), "3 4 +\n");
}

#[test]
fn format_preserves_indentation_in_multiline() {
    let src = "(\n  1 +\n  2 *\n)";
    let formatted = ezc_lsp::format_source(src);
    // Should preserve the structure (indentation within blocks)
    assert!(formatted.contains("(\n"));
    assert!(formatted.contains(")"));
}

// ── SymbolIndex: additional edge cases ──────────────────────────────────────

#[test]
fn symbol_index_same_name_different_scopes() {
    let builtins = BuiltinSets::new();
    let src = "1 @x { 2 @x $x } $x";
    let index = SymbolIndex::build(src, &builtins);
    // Two definitions of x
    assert_eq!(index.definition_sites("x").len(), 2);
    // Two references ($x inside scope, $x outside)
    assert_eq!(index.references.get("x").map(|v| v.len()).unwrap_or(0), 2);
}

#[test]
fn symbol_index_prelude_function_not_user_ref() {
    let builtins = BuiltinSets::new();
    let src = "5 sq";
    let index = SymbolIndex::build(src, &builtins);
    // sq is a prelude name — should NOT be in references
    assert!(!index.references.contains_key("sq"));
}

#[test]
fn symbol_index_import_not_user_ref() {
    let builtins = BuiltinSets::new();
    let src = "\"std/math.ezc\" import";
    let index = SymbolIndex::build(src, &builtins);
    // import is a builtin keyword — should NOT be in references
    assert!(!index.references.contains_key("import"));
}

// ── BuiltinSets: completeness ───────────────────────────────────────────────

#[test]
fn builtin_sets_knows_all_type_constructors() {
    let b = BuiltinSets::new();
    for name in [
        "int", "str", "bin", "u8", "u16", "u32", "u64", "u128", "u256", "i8", "i16", "i32", "i64",
        "i128", "i256", "f16", "f32", "f64",
    ] {
        assert!(b.is_known(name), "{name} should be known");
        assert_eq!(
            b.classify(name),
            SemanticClass::Type,
            "{name} should be Type"
        );
    }
}

#[test]
fn builtin_sets_knows_all_control_flow() {
    let b = BuiltinSets::new();
    for name in ["if", "ifel", "loop", "each", "import"] {
        assert!(b.is_known(name), "{name} should be known");
        assert_eq!(
            b.classify(name),
            SemanticClass::Keyword,
            "{name} should be Keyword"
        );
    }
}

#[test]
fn builtin_sets_knows_all_builtins() {
    let b = BuiltinSets::new();
    for name in [
        "len", "nth", "tl", "rev", "srt", "take", "skip", "zip", "range", "typeof", "cut", "cat",
        "map", "fil", "red", "rl", "wl", "rb", "wb", "words", "depth", "clear", "and", "or",
    ] {
        assert!(b.is_known(name), "{name} should be known");
    }
}

#[test]
fn builtin_sets_knows_prelude_names() {
    let b = BuiltinSets::new();
    for name in [
        "not", "dvb", "even", "odd", "neg", "abs", "sq", "sum", "prod", "hd", "flat", "min", "max",
        "dfl", "inc", "dec", "peek",
    ] {
        assert!(b.is_known(name), "prelude name {name} should be known");
    }
}

// ── find_close_matches: additional cases ────────────────────────────────────

#[test]
fn find_close_matches_single_char_diff() {
    let matches = ezc_lsp::find_close_matches("sqare", &["square", "sq", "sqrt"], 3);
    assert!(matches.contains(&"square")); // edit distance 1
}

#[test]
fn find_close_matches_empty_candidates() {
    let matches = ezc_lsp::find_close_matches("foo", &[], 3);
    assert!(matches.is_empty());
}

// ── Docs: token_docs coverage ───────────────────────────────────────────────

#[test]
fn docs_exist_for_all_operators() {
    use ezc::token::{Op, Token};

    let tokens_to_check = vec![
        Token::Op(Op::Add),
        Token::Op(Op::Sub),
        Token::Op(Op::Mul),
        Token::Op(Op::Div),
        Token::Op(Op::Mod),
        Token::Op(Op::Pow),
        Token::Bang,
        Token::Question,
        Token::DoubleQuestion,
        Token::Pipe,
        Token::Amp,
        Token::AmpBang,
        Token::AmpQuestion,
        Token::AmpSlash,
        Token::Eq,
        Token::NotEq,
        Token::Lt,
        Token::Gt,
        Token::LtEq,
        Token::GtEq,
        Token::Tilde,
        Token::Comma,
        Token::Semicolon,
        Token::Underscore,
        Token::Colon,
        Token::Dot,
    ];

    for tok in tokens_to_check {
        assert!(
            ezc_lsp::docs::token_docs(&tok).is_some(),
            "No docs for token: {tok}"
        );
    }
}

#[test]
fn docs_exist_for_builtin_idents() {
    use ezc::token::Token;

    let ident_names = vec![
        "int", "str", "bin", "f32", "f64", "u8", "len", "nth", "rev", "srt", "typeof", "range",
        "each", "loop", "import", "words", "depth", "clear", "map", "fil", "red", "rl", "wl", "rb",
        "wb",
    ];

    for name in ident_names {
        let tok = Token::Ident(name.to_string());
        assert!(
            ezc_lsp::docs::token_docs(&tok).is_some(),
            "No docs for ident: {name}"
        );
    }
}

// ── Semantic diagnostics ───────────────────────────────────────────────────

#[test]
fn semantic_diag_undefined_recall() {
    let builtins = BuiltinSets::new();
    let src = "$undefined_name";
    let index = SymbolIndex::build(src, &builtins);
    let diags = ezc_lsp::semantic_diagnostics(src, &index, &builtins);
    assert_eq!(diags.len(), 1, "should warn about undefined $name");
    assert!(diags[0].message.contains("undefined_name"));
    assert_eq!(diags[0].severity, Some(tower_lsp::lsp_types::DiagnosticSeverity::WARNING));
}

#[test]
fn semantic_diag_defined_name_no_warning() {
    let builtins = BuiltinSets::new();
    let src = "42 @x $x";
    let index = SymbolIndex::build(src, &builtins);
    let diags = ezc_lsp::semantic_diagnostics(src, &index, &builtins);
    assert!(diags.is_empty(), "defined variables should not warn");
}

#[test]
fn semantic_diag_builtin_no_warning() {
    let builtins = BuiltinSets::new();
    let src = "[1 2 3] len";
    let index = SymbolIndex::build(src, &builtins);
    let diags = ezc_lsp::semantic_diagnostics(src, &index, &builtins);
    assert!(diags.is_empty(), "builtins should not warn");
}

#[test]
fn semantic_diag_prelude_no_warning() {
    let builtins = BuiltinSets::new();
    let src = "5 sq";
    let index = SymbolIndex::build(src, &builtins);
    let diags = ezc_lsp::semantic_diagnostics(src, &index, &builtins);
    assert!(diags.is_empty(), "prelude names should not warn");
}

#[test]
fn semantic_diag_multiple_undefined() {
    let builtins = BuiltinSets::new();
    let src = "$foo $bar";
    let index = SymbolIndex::build(src, &builtins);
    let diags = ezc_lsp::semantic_diagnostics(src, &index, &builtins);
    assert_eq!(diags.len(), 2, "should warn for each undefined name");
}

#[test]
fn semantic_diag_bare_ident_undefined() {
    let builtins = BuiltinSets::new();
    let src = "5 myfunc";
    let index = SymbolIndex::build(src, &builtins);
    let diags = ezc_lsp::semantic_diagnostics(src, &index, &builtins);
    assert_eq!(diags.len(), 1, "undefined bare ident should warn");
    assert!(diags[0].message.contains("myfunc"));
}

#[test]
fn all_diagnostics_skips_semantic_on_parse_error() {
    let builtins = BuiltinSets::new();
    // Unclosed paren causes parse error
    let src = "( 1 2 +";
    let index = SymbolIndex::build(src, &builtins);
    let diags = ezc_lsp::all_diagnostics(src, &index, &builtins);
    // Should only contain parse error, not semantic warnings
    assert!(
        diags.iter().all(|d| d.severity == Some(tower_lsp::lsp_types::DiagnosticSeverity::ERROR)),
        "should only have parse errors, not semantic warnings"
    );
}
