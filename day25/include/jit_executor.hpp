#pragma once
#include "executor.hpp"
#include "jit.hpp"
#include "program.hpp"

namespace day25 {
    /**
     * Executes \ref Program "Programs" by translating them into machine-code in memory.
     *
     * \warning
     * This will crash in many scenarios:
     * Espcially, it requires the application to be running on a 64-bit X86 compatible CPU
     * (AMD64/X64/X86-64/IA32-E) <b>and</b> using the System V AMD64 ABI.
     * This typically means any 64-bit Linux on Intel/AMD CPUs, or MacOS on Intel CPUs.
     */
    class JitExecutor : public virtual Executor {
    public:
        JitExecutor(Program program);
        virtual ~JitExecutor() override;
        virtual void step() override;
        virtual void reset() override;
        virtual uint32_t diagnostic_checksum() override;
        Jit &jit() { return *m_jit; }
    private:
        const Program m_program;
        Jit *m_jit;
        uint8_t *m_tape;
        uint64_t m_tape_size;
        uint64_t m_tape_offset;
        char *m_state_name;
        void (*m_state_func)();
        void compile();
        void dump_state();
    };
} // namespace day25