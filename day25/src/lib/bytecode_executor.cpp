#include "bytecode_executor.hpp"
#include <cstring>

namespace day25 {
BytecodeExecutor::BytecodeExecutor(Program program) : m_program(program) {
    m_memory = new uint8_t[m_program.checksum_delay];
    m_code = new uint16_t[m_program.states.size()];

    // Compile the turing machine
    // 1: Map state names to indexes in program memory

    for (auto state : m_program.states) {
        m_state_map[state.first] = m_state_map.size();
    }

    if (m_program.states.size() > 32) {
        throw new std::runtime_error(
            "Bytecode interpreter only works with up to 32 states!");
    }

    // 2: Encode states into bytecode
    for (auto state : m_program.states) {
        m_code[m_state_map.at(state.first)] = encode_state(state.second);
    }

    reset();
}

void BytecodeExecutor::reset() {
    memset(m_memory, 0, m_program.checksum_delay);
    m_state = m_state_map[m_program.initial_state];
    m_memory_offset = 0;
}

void BytecodeExecutor::step() {
    auto bytecode = m_code[m_state];
    auto slot = m_memory[m_memory_offset];
    uint8_t encoded_action;
    if (slot == 0) {
        encoded_action = bytecode & 0xff;
    } else {
        encoded_action = (bytecode >> 8) & 0xff;
    }
    uint8_t write_contents;
    int8_t move_direction;
    uint8_t next_state;
    decode_action(encoded_action, write_contents, move_direction, next_state);
    m_memory[m_memory_offset] = write_contents;
    m_memory_offset =
        (m_memory_offset + m_program.checksum_delay + move_direction) %
        (m_program.checksum_delay);
    m_state = next_state;
}

uint32_t BytecodeExecutor::diagnostic_checksum() {
    uint32_t checksum = 0;
    for (uint32_t i = 0; i < m_program.checksum_delay; i++) {
        checksum += m_memory[i];
    }
    return checksum;
}

BytecodeExecutor::~BytecodeExecutor() {
    delete[] m_memory;
    delete[] m_code;
}

uint16_t BytecodeExecutor::encode_state(const day25::State &state) {
    // Encoding:
    // Bits    Contents
    // 0..7    op-if-slot-is-zero
    // 8..15   op-if-slot-is-one
    return encode_action(state.actions.at(1)) << 8 |
           encode_action(state.actions.at(0));
}

uint8_t BytecodeExecutor::encode_action(const day25::StateAction &action) {
    // Operation encoding:
    // Bits    Contents
    // 0       write_value
    // 1       move_direction (1 for '+', 0 for '-')
    // 2..6    next_state
    // 7       reserved
    uint8_t result = 0;
    result |= (action.write_value & 0x01);
    result |= (action.move_direction == 1 ? 1 : 0) << 1;
    result |= (m_state_map.at(action.next_state) & 0x1f) << 2;
    uint8_t write_contents;
    int8_t move_direction;
    uint8_t next_state;
    decode_action(result, write_contents, move_direction, next_state);
    if (write_contents != action.write_value ||
        move_direction != action.move_direction ||
        next_state != m_state_map.at(action.next_state)) {
        throw new std::runtime_error(
            "Bug: decoding instruction does not yield original encoder input.");
    }
    return result;
}

void BytecodeExecutor::decode_action(uint8_t &encoded, uint8_t &write_contents,
                                     int8_t &move_direction,
                                     uint8_t &next_state) {
    write_contents = encoded & 0x01;
    move_direction = ((encoded >> 1) & 0x01) ? 1 : -1;
    next_state = (encoded >> 2);
}
} // namespace day25