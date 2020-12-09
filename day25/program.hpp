#pragma once

#include <string>
#include <cstdint>
#include <map>
#include <vector>

namespace day25 {
    struct StateAction {
        //TODO: Would be nice to have the original tokens available here, for better error messages in post-processing passes.`
        unsigned slot_condition;
        unsigned write_value;
        int move_direction;
        std::string next_state;
    };

    struct State {
        //TODO: Would be nice to have the original token available here, for better error messages in post-processing passes.
        std::string name;
        std::map<unsigned, StateAction> actions;
    };

    struct Program {
        std::string initial_state;
        uint32_t checksum_delay;
        std::map <std::string, State> states;
    };

    std::ostream &operator<<(std::ostream &os, const Program &program);
}