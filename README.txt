The Amplifier Project

Founded by liftA42 on Oct 30, 2017. Licensed under MIT.

What I want to do in this project:
* Finite state automata implementation. Including DFA and NFA as the base, and
utils to manipulate them, such as generating DFA with a given NFA.
* Regex parsing engine based on above automata. Accepts PCRE string and
generates corresponding NFA or DFA.
* String util library based on above regex library.
* Pushdown automata implementation.
* BNF parsing library based on above automata.
* A programming language called Diode, which naturally use CPS for function
calls.
* A minimal compiler for Diode, and its standard library.
* Some small programs written in Diode, such as a 2048-like game.

The project will be divided into three modules:
* auto: including FSA and PDA implementation
* naive: some general foundation of project, such as utils, testing and error
handling functions
* re: regex and string library
* bnf: BNF parsing library
* diode: the Diode Programming Language's compiler and standard library
* d048: a 2048 game written in Diode
