#pragma once
#include "executor.hpp"
#include "program.hpp"
#include <string>

/**
 * \mainpage Advent of Code 2017, Day 25
 *
 * This project contains several interpreters for a simple turing machine language.
 *
 * <p>
 * For details, see the README.md on https://github.com/jkrafczyk/aoc2017/tree/master/day25
 * </p>
 */

/**
 * \defgroup parsing Parsing
 * \defgroup execution Execution
 * \defgroup jit JIT Machine Code Generation
 */

namespace day25 {
    /**
     * Construct a \ref Program from the contents of the specified file
     * \throws std::runtime_error If any error occurs during file i/o, tokenization or parsing.
     * \related Program
     * \ingroup parsing
     */
Program load_file(const std::string &filename);
}