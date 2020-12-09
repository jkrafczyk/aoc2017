#include "program.hpp"
#include <iostream>
using std::endl;
using std::ostream;
using std::string;

namespace day25 {
ostream &operator<<(ostream &os, const Program &program) {
  os << "Program:" << endl
     << "  Initial state: " << program.initial_state << endl
     << "  Checksum after " << program.checksum_delay << " steps" << endl
     << "  States:" << endl;

  for (auto state : program.states) {
    os << "    " << state.first << ":" << endl;
    for (auto action : state.second.actions) {
      os << "      If slot = " << action.first << ":" << endl;
      os << "        Write " << action.second.write_value << endl;
      os << "        Move " << action.second.move_direction << endl;
      os << "        Next state: " << action.second.next_state << endl;
    }
  }

  return os;
}
} // namespace day25