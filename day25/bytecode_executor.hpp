#pragma once
#include "program.hpp"
#include "executor.hpp"

namespace day25 {
    class BytecodeExecutor: public virtual  Executor {
    public:
        BytecodeExecutor(Program program);
        virtual ~BytecodeExecutor();
        virtual void reset();
        virtual void step();
        virtual uint32_t diagnostic_checksum();
    private:
        const Program m_program;
        std::map<std::string, uint8_t> m_state_map;
        uint8_t m_state;
        uint32_t m_memory_offset;
        uint8_t *m_memory;
        uint16_t *m_code;

        uint16_t encode_state(const day25::State &state);
        uint8_t encode_action(const day25::StateAction &action);
        void decode_action(uint8_t& encoded, uint8_t &write_contents, int8_t &move_direction, uint8_t &next_state);
    };
}