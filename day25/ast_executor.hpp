#pragma once
#include "executor.hpp"
#include "program.hpp"
#include <vector>
#include <string>

namespace day25 {
    class AstExecutor: public virtual Executor {
    public:
        AstExecutor(Program program);
        virtual ~AstExecutor();
        virtual void step();
        virtual void reset();
        virtual uint32_t diagnostic_checksum();
    protected:
        const Program m_program;
        std::vector<char> m_memory;
        uint32_t m_offset;
        std::string m_state;
    };
}