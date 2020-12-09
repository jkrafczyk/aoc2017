cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

project([aoc2017-day25])

add_library(d25
        src/lib/utils.cpp
        src/lib/tokenizer.cpp
        src/lib/program.cpp
        src/lib/parser.cpp
        src/lib/executor.cpp
        src/lib/ast_executor.cpp
        src/lib/bytecode_executor.cpp
        src/lib/jit.cpp
        src/lib/jit_executor.cpp)

#Enable loads of warnings, but accept C99 extensions like designated initializers:
target_compile_options(d25 PRIVATE -Wall -Wextra -pedantic -Wno-c99-extensions)
target_include_directories(d25 PRIVATE include)

add_executable(day25 src/app/main.cpp)
target_link_libraries(day25 PUBLIC d25)
target_include_directories(day25 PRIVATE include)

add_executable(jit-demo src/app/jit-demo.cpp)
target_link_libraries(jit-demo PUBLIC d25)