#include "jit_executor.hpp"
#include <cstdint>
#include <cstring>
#include <iostream>

using std::cout;
using std::endl;

namespace day25 {
    namespace {
        void compile_state_action(Jit *jit, const State &state, const StateAction &action, const std::string &end_label) {
            //Write value to tape:
            jit->emit_mov(Register::RAX, action.write_value);
            //"mov [R10 + R11], al"
            jit->emit(4, "\x43\x88\x04\x1A");
            //Store name of new state:
            jit->emit_mov(Register::RAX, jit->symbol("state_name_" + action.next_state));
            jit->emit_mov(Indirect(Register::R12), Register::RAX);
            //Store address of new state:
            jit->emit_mov(Register::RAX, jit->symbol("state_" + action.next_state));
            jit->emit_mov(Indirect(Register::R14), Register::RAX);
            //Move tape:
            if (action.move_direction > 0) {
                jit->emit_inc(Register::R10);
            } else {
                jit->emit_dec(Register::R10);
            }
            //return:
            jit->emit_jmp(jit->symbol(end_label));
        }

        void compile_state(Jit *jit, const State &state) {

            jit->emit_function("state_" + state.name, 0, [jit,state](auto _, auto _2, auto end_label) {
                auto if0 = "_state_" + state.name + "_if0";
                auto if1 = "_state_" + state.name + "_if1";
                auto cleanup = "_state_" + state.name + "_cleanup";
                auto fix_offset_underflow = "_state_" + state.name + "_fix_underflow";
                auto fix_offset_overflow = "_state_" + state.name + "_fix_overflow";
                auto offset_fixed = "_state_" + state.name + "_offset_fixed";
                //Local variables:
                //  R09 &tape_offset
                //  R10 tape_offset
                //  R11 &tape
                //  R12 &state_name
                //  R13 [unused]
                //  R14 &state_func
                //  R15 tape_size

                //Prepare locals
                jit->emit_mov(Register::R9, jit->symbol("tape_offset"));
                jit->emit_mov(Register::R10, Indirect(Register::R9));
                jit->emit_mov(Register::R11, jit->symbol("tape"));
                jit->emit_mov(Register::R12, jit->symbol("state_name"));
                jit->emit_mov(Register::R15, jit->symbol("tape_size"));
                jit->emit_mov(Register::R15, Indirect(Register::R15));
                jit->emit_mov(Register::R14, jit->symbol("state_func"));

                //Load state from tape
                jit->emit_mov(Register::RAX, 0);
                //"mov al, [R10 + R11]" (byte registers are not directly supported by the bytecode builder)
                jit->emit(4, "\x43\x8A\x04\x1A");
                jit->emit_cmp(Register::RAX, 0);
                jit->emit_jcc(Condition::NOT_EQUAL, jit->symbol(if1));

                //Behaviour for tape=0
                jit->emit_symbol(if0);
                compile_state_action(jit, state, state.actions.at(0), cleanup);

                //Behaviour for tape=1
                jit->emit_symbol(if1);
                compile_state_action(jit, state, state.actions.at(1), cleanup);

                jit->emit_symbol(cleanup);

                //Fixup tape offset in case of under/overflow
                jit->emit_cmp(Register::R10, 0);
                jit->emit_jcc(Condition::LESS, jit->symbol(fix_offset_underflow));
                jit->emit_cmp(Register::R10, Register::R15);
                jit->emit_jcc(Condition::ABOVE_EQUAL, jit->symbol(fix_offset_overflow));
                //No fixes required, skip.
                jit->emit_jmp(jit->symbol(offset_fixed));
                //Fix underflow
                jit->emit_symbol(fix_offset_underflow);
                jit->emit_mov(Register::R10, Register::R15);
                jit->emit_dec(Register::R10);
                jit->emit_jmp(jit->symbol(offset_fixed));
                //Fix overflow
                jit->emit_symbol(fix_offset_overflow);
                jit->emit_mov(Register::R10, 0);
                //Store new tape offset
                jit->emit_symbol(offset_fixed);
                jit->emit_mov(Indirect(Register::R9), Register::R10);
                //Return '0'
                jit->emit_mov(Register::RAX, 0);
            });
            jit->add_constant("state_name_" + state.name, state.name);
        }
    } // namespace

    JitExecutor::JitExecutor(Program program) : m_program(program), m_jit(new Jit), m_tape_size(m_program.checksum_delay) {
        m_tape = new uint8_t[m_tape_size];
        compile();
        reset();
    }

    JitExecutor::~JitExecutor() {
        delete[] m_tape;
        delete m_jit;
    }

    void JitExecutor::compile() {
        m_jit->emit_symbol("tape", m_tape);
        m_jit->emit_symbol("tape_size", &m_tape_size);
        m_jit->emit_symbol("tape_offset",&m_tape_offset);
        m_jit->emit_symbol("state_name", &m_state_name);
        m_jit->emit_symbol("state_func", &m_state_func);
        auto state = m_program.states.at(m_program.initial_state);
        for (auto it : m_program.states) {
            compile_state(m_jit, it.second);
        }
        m_jit->finalize_code();
    }

    void JitExecutor::dump_state() {
        cout << "State: " <<
             "idx=" << m_tape_offset << "; "
             << "state=" << m_state_name << "; "
             << "tape=";
        for (uint64_t i = 0; i < m_tape_size; i++) {
            cout << (int)(m_tape[i]);
        }
        cout << endl;
    }

    void JitExecutor::step() {
        m_state_func();
        //dump_state();
    }

    void JitExecutor::reset() {
        m_state_name = (char*)m_jit->symbol("state_name_" + m_program.initial_state).address;
        m_state_func = (void(*)()) (m_jit->symbol("state_" + m_program.initial_state).address);
        m_tape_offset = 0;
        memset((void*)(m_jit->symbol("tape").address), 0, m_program.checksum_delay);
        //dump_state();
    }

    uint32_t JitExecutor::diagnostic_checksum() {
        uint32_t checksum = 0;
        for (uint32_t i = 0; i < m_tape_size; i++) {
            checksum += m_tape[i];
        }
        return checksum;
    }
} // namespace day25