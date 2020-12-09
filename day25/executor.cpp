#include "executor.hpp"
#include "ast_executor.hpp"
#include "bytecode_executor.hpp"

#include <map>
using std::function;
using std::list;
using std::map;
using std::shared_ptr;
using std::string;

namespace day25 {
namespace {
typedef function<shared_ptr<Executor>(Program)> ExecutorFactory;

static map<string, ExecutorFactory> factories = {
    std::make_pair("ast",
                   [](auto p) { return std::make_shared<AstExecutor>(p); }),
    std::make_pair("bytecode", [](auto p) {
        return std::make_shared<BytecodeExecutor>(p);
    })};
} // namespace

shared_ptr<Executor> get_executor(const string &name, Program p) {
    return factories.at(name)(p);
}

list<string> list_executors() {
    list<string> names;
    for (auto it : factories) {
        names.push_back(it.first);
    }
    return names;
}
} // namespace day25