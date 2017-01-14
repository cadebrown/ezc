---
layout: docs
title: Function List
---

Here is a full list of functions in ezC

| **Function** | **Explanation** |
| **Basic Functions** | |
| prec 256 | Sets global precision in bits   |
| x = set y | sets x to y   |
| z = min x y | minimum of x or y |
| z = max x y | maximum of x or y |
| z = near x y| nearest multiple of x to y|
| z = add x y | Adds x and y |
| z = sub x y | Subtracts y from x |
| z = mul x y | Multiplies x by y |
| z = div x y | Divides x by y |
| z = mod x y | Returns x modulo y |
| z = pow x y | x to the power of y |
| **Printing Functions** | |
| echo Hello | Prints out all arguments |
| var x | Prints variable value |
| intvar x | Prints out x, but as an int |
| file x a.txt | Prints the value of x into a.txt |
| **Math Functions**  | | 
| z = rand x [y=0] | uniform random number in [x, y)  |
| z = sqrt x | square root of x  |
| z = cbrt x  | cube root of x  |
| z = hypot x y  | hypoteneuse of triangle with sides x and y  |
| z = exp x | euler's number raised to x      |
| z = log x [y=e]  | logarithm of x      |
| z = agm x y   | arithmetic geometric mean |
| z = gamma x    | the gamma function at x |
| z = factorial x | factorial of x |
| z = zeta x  | Riemann zeta function of x |
| **Flow Blocks**  | |
| if z > x  | Conditional block that can use <, >, == |
| fi  | Ends if block |
| for i 0 10 2.5  | Starts i at 0, keeps adding 2.5 while i < 10 |
| rof  | Ends for block |

For trig functions, the list is quite verbose, so I'll just list them off:

sin, cos, tan, csc, sec, cot, sinh, cosh, tanh, csch, sech, coth

And inverses are available by prepending `a` to a function: like asin, acos, etc
