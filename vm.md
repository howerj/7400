# eForth for a tiny 16-bit Accumulator VM

This project contains a Virtual Machine for a 16-bit Accumulator based
CPU/system and a Forth interpreter for that system. It is designed so that
the CPU should be implementable in 7400 series logic circuits (and some
RAM/ROM).

To build:

	cc vm.c -o vm

To run:

	./vm vm.hex

Example Forth (white-space matters for all these examples, bit return after
each line):

	words

This adds two numbers using Reverse Polish Notation and prints the result:

	2 2 + . cr

To define the ISO standard hello world program:

	: hello cr ." Hello, World!" cr ;
	hello

Hitting CTRL-D will not work (the interpreter is set up to ignore -1 returned
by `fgetc` deliberately, for now). Instead type "bye" and hit enter to quit.
