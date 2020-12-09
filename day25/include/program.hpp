#pragma once

#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace day25 {
    /**
     * Represents what to do in a specific state for a specific current tape value.
     *
     * This corresponds to the "If the current value is ..." lines in a program and the actions below them.
     */
struct StateAction {
    // TODO: Would be nice to have the original tokens available here, for
    // better error messages in post-processing passes.
    //! Only apply this action if the current tape slot value is this.
    unsigned slot_condition;
    //! Write this value to the current tape slot when running this \ref StateAction.
    unsigned write_value;
    //! Move the tape this many slots. (-1 or +1)
    int move_direction;
    //! Continue with this state afterwards.
    std::string next_state;
};

/**
 * Describes a possible state of the turing machine.
 *
 * Corresponds to the 'In state ...' lines in a program and everything below them.
 */
struct State {
    // TODO: Would be nice to have the original token available here, for better
    // error messages in post-processing passes.
    //! Name of this state
    std::string name;
    //! Map from (current tape value) to (\ref StateAction to perform for this value). Typically contains the keys 0 and 1.
    std::map<unsigned, StateAction> actions;
};

/**
 * Describes an entire program, including initialization and all states.
 */
struct Program {
    //! Name of the state the turing machine should start in.
    std::string initial_state;
    //! Number of steps to run before calculating the checksum.
    uint32_t checksum_delay;
    //! Map from (state name) to \ref State
    std::map<std::string, State> states;
};

std::ostream &operator<<(std::ostream &os, const Program &program);
} // namespace day25