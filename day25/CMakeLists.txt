cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

project([aoc2017-day25])

add_library(libday25
        utils.cpp
        tokenizer.cpp
        program.cpp
        parser.cpp
        executor.cpp
        ast_executor.cpp
        bytecode_executor.cpp
        jit.cpp)

add_executable(day25 day25.cpp)
target_link_libraries(day25 PUBLIC libday25)

add_executable(jit-demo jit-demo.cpp)
target_link_libraries(jit-demo PUBLIC libday25)