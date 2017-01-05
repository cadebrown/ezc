---
layout: page
title: About
---

<p class="message">
  Any questions, comments, or concerns can be directed to <a href="mailto:info@chemicaldevelopment.us">info@chemicaldevelopment.us</a>
</p>

## About EZC

I first had the idea for EZC roughly in June 2016, and it was simple: 

Have a language with simple syntax, yet the ability to handle multiprecision (more precision than a double, or float) efficiently.


At the time, I was using GMP, MPFR, and MPC for quick computations (i.e. finding an infinite sum, or limit to a large precision). However, the problem with these libraries is that using them requires C synax, initialization, and some memory management. Not to mention the lack of operators, obfuscated function calls, and rounding.

With EZC, I've tried to simplify writing code that deals with multiprecision so that anyone can easily do it.

For example, to find a third to 1000 digits:

<pre>
prec 1000
var (1/3)
</pre>

This prints out `.3333...333`


