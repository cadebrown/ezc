# Standard library

The standard library lives in `std/`. The **prelude** is loaded
automatically before every program. Other modules are loaded with
`import`.

## Prelude (auto-loaded)

### Stack combinators

| name | stack effect      | description |
|------|-------------------|-------------|
| `dup`  | `a → a a`         | named alias of `,` |
| `drop` | `a →`             | named alias of `;` |
| `swap` | `a b → b a`       | named alias of `~` |
| `over` | `a b → a b a`     | named alias of `_` |
| `nip`  | `a b → b`         | drop second |
| `tuck` | `a b → b a b`     | dup top, hide under second |
| `dup2` | `a b → a b a b`   | dup pair |
| `rot`  | `a b c → b c a`   | rotate three |
| `id`   | `a → a`           | identity (no-op) |

### Predicates

| name   | stack effect | description |
|--------|--------------|-------------|
| `zero` | `a → bool`   | is zero |
| `even` | `a → bool`   | is even |
| `odd`  | `a → bool`   | is odd |
| `ltz`  | `a → bool`   | less than zero |
| `gtz`  | `a → bool`   | greater than zero |
| `dvb`  | `a b → bool` | a divisible by b |

### Logic

| name  | stack effect | description |
|-------|--------------|-------------|
| `not` | `a → bool`   | logical negation |
| `and` | `a b → bool` | logical and |
| `or`  | `a b → bool` | logical or |

### Math

| name   | stack effect    | description |
|--------|-----------------|-------------|
| `inc`  | `a → a+1`       | increment |
| `dec`  | `a → a-1`       | decrement |
| `sq`   | `a → a*a`       | square |
| `neg`  | `a → -a`        | negate |
| `abs`  | `a → |a|`       | absolute value |
| `sum`  | `[list] → n`    | sum of a list |
| `prod` | `[list] → n`    | product of a list |
| `iota` | `n → list`      | `[0..n-1]` |
| `min`  | `a b → m`       | minimum |
| `max`  | `a b → m`       | maximum |

### Collections

| name    | stack effect       | description |
|---------|--------------------|-------------|
| `hd`    | `[list] → first`   | head |
| `flat`  | `[[..]] → [..]`    | flatten one level |
| `apply` | `[args] (block) →` | splat args, then execute block |

### Misc

| name   | description |
|--------|-------------|
| `dfl`  | `val fallback dfl → val if truthy, fallback otherwise` |
| `peek` | `a → a` (also prints `a`) — debugging aid |

## `std/math.ezc`

```ezc
"std/math.ezc" import
```

| name   | stack effect       | description |
|--------|--------------------|-------------|
| `fact` | `n → n!`           | factorial |
| `fib`  | `n → fib(n)`       | nth Fibonacci |
| `gcd`  | `a b → g`          | Euclidean GCD |
| `pow2` | `b e → b^e`        | integer power for any width |
| `clamp`| `v lo hi → v'`     | clamp to range |
| `sgn`  | `n → -1\|0\|1`     | signum |

## `std/str.ezc`

```ezc
"std/str.ezc" import
```

| name     | stack effect          | description |
|----------|-----------------------|-------------|
| `rep`    | `s n → s'`            | repeat string n times |
| `padl`   | `s w c → s'`          | pad-left to width with char |
| `has`    | `s sub → bool`        | contains substring |
| `starts` | `s pre → bool`        | starts with prefix |
| `ends`   | `s suf → bool`        | ends with suffix |
