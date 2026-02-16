use std::{fs, path::Path};

use ezc::{ezcbc::Value, run_source};

fn read_fixture(name: &str) -> String {
    let path = Path::new(env!("CARGO_MANIFEST_DIR"))
        .join("test")
        .join(name);
    fs::read_to_string(&path).unwrap_or_else(|err| {
        panic!("failed to read fixture {}: {err}", path.display());
    })
}

fn read_demo(name: &str) -> String {
    let path = Path::new(env!("CARGO_MANIFEST_DIR"))
        .join("demo")
        .join(name);
    fs::read_to_string(&path).unwrap_or_else(|err| {
        panic!("failed to read demo {}: {err}", path.display());
    })
}

#[test]
fn arithmetic_fixture_runs() {
    let src = read_fixture("arithmetic.ezc");
    let result = run_source("test/arithmetic.ezc", &src).expect("program should run");
    assert_eq!(result.stack, vec![Value::Int(20)]);
    assert_eq!(result.stdout, "");
}

#[test]
fn block_exec_fixture_runs() {
    let src = read_fixture("block_exec.ezc");
    let result = run_source("test/block_exec.ezc", &src).expect("program should run");
    assert_eq!(result.stack, vec![]);
    assert_eq!(result.stdout, "25");
}

#[test]
fn stack_literal_fixture_runs() {
    let src = read_fixture("stack_literal.ezc");
    let result = run_source("test/stack_literal.ezc", &src).expect("program should run");

    let [Value::Stack(values)] = result.stack.as_slice() else {
        panic!("expected a single stack literal value");
    };

    assert_eq!(values[0], Value::Symbol("a".to_string()));
    assert_eq!(values[1], Value::Int(1));
    assert_eq!(values[2], Value::Stack(vec![Value::Int(2), Value::Int(3)]));
    assert!(matches!(values[3], Value::Block(_)));
}

#[test]
fn conditional_true_fixture_runs() {
    let src = read_fixture("conditional_true.ezc");
    let result = run_source("test/conditional_true.ezc", &src).expect("program should run");
    assert_eq!(result.stack, vec![Value::Int(111)]);
}

#[test]
fn conditional_false_fixture_runs() {
    let src = read_fixture("conditional_false.ezc");
    let result = run_source("test/conditional_false.ezc", &src).expect("program should run");
    assert_eq!(result.stack, vec![Value::Int(222)]);
}

#[test]
fn loop_fixture_runs() {
    let src = read_fixture("loop_countdown.ezc");
    let result = run_source("test/loop_countdown.ezc", &src).expect("program should run");
    assert_eq!(result.stack, vec![Value::Int(0)]);
    assert_eq!(result.stdout, "3\n2\n1");
}

#[test]
fn hello_demo_runs() {
    let src = read_demo("hello.ezc");
    let result = run_source("demo/hello.ezc", &src).expect("program should run");
    assert_eq!(result.stdout, "hello, world");
    assert_eq!(result.stack, vec![]);
}

#[test]
fn gcd_demo_runs() {
    let src = read_demo("gcd.ezc");
    let result = run_source("demo/gcd.ezc", &src).expect("program should run");
    assert_eq!(result.stdout, "gcd(252, 105):\n21");
    assert_eq!(result.stack, vec![]);
}

#[test]
fn fib_demo_runs() {
    let src = read_demo("fib.ezc");
    let result = run_source("demo/fib.ezc", &src).expect("program should run");
    assert_eq!(
        result.stdout,
        "fibonacci numbers below 100:\n1\n1\n2\n3\n5\n8\n13\n21\n34\n55\n89"
    );
    assert_eq!(result.stack, vec![]);
}

#[test]
fn powers_of_two_demo_runs() {
    let src = read_demo("powers_of_two.ezc");
    let result = run_source("demo/powers_of_two.ezc", &src).expect("program should run");
    assert_eq!(
        result.stdout,
        "powers of two below 1024:\n1\n2\n4\n8\n16\n32\n64\n128\n256\n512"
    );
    assert_eq!(result.stack, vec![]);
}

#[test]
fn triangular_demo_runs() {
    let src = read_demo("triangular.ezc");
    let result = run_source("demo/triangular.ezc", &src).expect("program should run");
    assert_eq!(
        result.stdout,
        "triangular numbers 1..10:\n1\n3\n6\n10\n15\n21\n28\n36\n45\n55"
    );
    assert_eq!(result.stack, vec![]);
}

#[test]
fn factorial_demo_runs() {
    let src = read_demo("factorial.ezc");
    let result = run_source("demo/factorial.ezc", &src).expect("program should run");
    assert_eq!(result.stdout, "10!:\n3628800");
    assert_eq!(result.stack, vec![]);
}
