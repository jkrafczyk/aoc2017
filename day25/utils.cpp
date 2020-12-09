#include "day25.hpp"
#include "parser.hpp"
#include "tokenizer.hpp"
#include <fstream>
#include <regex>

using std::ifstream;
using std::string;

namespace day25 {
Program load_file(const std::string &filename) {
    ifstream ifs(filename);
    if (ifs.fail() || ifs.bad()) {
        throw new std::runtime_error("Could not open file " + filename);
    }
    Tokenizer t(ifs);
    Parser p(t);
    auto state = p.parse();
    if (state.error) {
        string msg = "Error trying to compile program " + filename + "\n";
        // msg += state.
        throw new std::runtime_error(msg);
    }
    return p.program();
}
} // namespace day25