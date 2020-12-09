#include "ast_executor.hpp"
#include <numeric>

using std::accumulate;
namespace day25 {
AstExecutor::AstExecutor(Program program)
    : m_program(program), m_memory(program.checksum_delay), m_offset(0),
      m_state(program.initial_state) {}

AstExecutor::~AstExecutor() {}

void AstExecutor::reset() {
    for (unsigned long i = 0; i < m_memory.size(); i++) {
        m_memory[i] = 0;
    }
    m_offset = 0;
    m_state = m_program.initial_state;
}

void AstExecutor::step() {
    const auto &state = m_program.states.at(m_state);
    const auto &action = state.actions.at(m_memory[m_offset]);
    m_memory[m_offset] = action.write_value;
    if (action.move_direction < 0 && m_offset == 0) {
        m_offset = m_program.checksum_delay - 1;
    } else {
        m_offset =
            (m_offset + action.move_direction) % m_program.checksum_delay;
    }
    m_state = action.next_state;
}

uint32_t AstExecutor::diagnostic_checksum() {
    auto checksum = accumulate(m_memory.begin(), m_memory.end(), 0L);
    return checksum;
}
} // namespace day25