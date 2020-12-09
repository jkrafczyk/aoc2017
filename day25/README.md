# The excesses of day 25

This directory contains a solution (or set of solutions) for [day 25 of Advent of Code 2017](https://adventofcode.com/2017/day/25), written in C++.

Specifically, it contains a parser for the program specification in "The Halting Problem", combined with several possibilities to run a program:

* `AstExecutor` basically walks the syntax tree of the program to run it.
* `BytecodeExecutor` translates the program into a short bytecode array and runs that, not using the syntax tree at all during execution. It should be faster by a few orders of magnitude.
* For comparison, a utility to translate programs into C code and write it to a file is included. That file can then be compiled with any C compiler.
* A Just-in-Time compiler is planned. 

## Usage

To compile the application, run:
```
mkdir build
cd build
cmake ..
make
cd ..
```

Afterwards, run `build/day25` to get a short help message.

To just get the result, use `build/day25 run real-input bytecode`. This will take the program from the file `real-input` and run it with the bytecode-based runtime (as apposed to `ast`, the tree-walker runtimer).

To benchmark all available runtimes, use `build/day25 benchmark real-input`.

To convert a Program to C sourcecode, use `build/day25 generate-c real-input`. The result will be written to the file `generated-program.c`, can be compiled with `gcc -o generated-program generated-program.c`, and then run with `./generated-program`. It will both run a short benchmark, and output the result for the day.

## File overview

* `CMakeLists.txt` describes the build process for CMake.  
* The `tokenizer`, `parser` and `program` files contain classes related to parsing the turing machine language and representing parsed programs in-memory.
* `executor.cpp` and `executor.hpp` contain the base classes for everything that can run programs directly in-memory (as apposed to generating C source code)
* Any `something_executor` file contains files related to one executor/runtime implementation.
* `jit.hpp` and `jit.cpp` are utilities for creating executable amd64/IA-32E/x64 programs in-memory.

## Useful resources

* A description of the SystemV AMD64 ABI (calling conventions): https://en.wikipedia.org/wiki/X86_calling_conventions#System_V_AMD64_ABI
* Agner Fog's 'objconv' disassembler: https://www.agner.org/optimize/#objconv
* Skeeto's article on writing a JIT for a toy language: https://nullprogram.com/blog/2015/03/19/
* People claim that https://www.stephendiehl.com/posts/monads_machine_code.html is relevant, too.