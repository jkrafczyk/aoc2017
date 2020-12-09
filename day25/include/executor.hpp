#pragma once
#include <cstdint>
#include <list>
#include <memory>
#include <string>

namespace day25 {
struct Program;

class Executor {
  public:
    virtual ~Executor() {}
    virtual void step() = 0;
    virtual void reset() = 0;
    virtual uint32_t diagnostic_checksum() = 0;
};

std::list<std::string> list_executors();
std::shared_ptr<Executor> get_executor(const std::string &name, Program p);
} // namespace day25