#pragma once
#include <cstdint>
#include <list>
#include <memory>
#include <string>

namespace day25 {
struct Program;

/**
 * Base class for everything that can run a \ref Program.
 */
class Executor {
  public:
    virtual ~Executor() {}
    //! Execute a single calculation step (one \ref StateAction).
    virtual void step() = 0;
    //! Reset the turing machine to its initial state.
    virtual void reset() = 0;
    //! Calculate the diagnostic checksum for the tape.
    virtual uint32_t diagnostic_checksum() = 0;
};

/** Return the names of all known executor types.
 * \relates Executor
 */
std::list<std::string> list_executors();

/** Create a new executor.
 *
 * \param type Type-name of the executor. Must be one of the values returned by \ref list_executors.
 * \param p The program to execute.
 * \relates Executor
*/
std::shared_ptr<Executor> get_executor(const std::string &type, Program p);
} // namespace day25