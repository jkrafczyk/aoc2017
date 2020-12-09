#pragma once
#include "executor.hpp"
#include "program.hpp"
#include <string>

namespace day25 {
Program load_file(const std::string &filename);
}