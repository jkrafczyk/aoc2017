#include "day25.hpp"
#include "jit.hpp"
#include <fstream>
#include <iostream>

using std::cout;
using std::flush;
using std::endl;
using std::ofstream;
using std::string;
using namespace day25;

/*
 * Register allocation order for called functions:
 *   rdi, rsi, rdx, rcx, r8, r9
 *   Return value in rax
 *   non-volatile registers: rbx, rsp, rbp, r12-r15
 * State machine register allocation:
 *   rax: Whatever is currently useful
 *   rcx: Remaining number of requested iterations (via call argument)
 *   rdx: Pointer to current state
 *   rbx: [Reserved, needs to be restored!]
 *   rdi: Tape base address
 *   rsp: Reserved!
 *   rbp: Reserved!
 *
 */

void compile_state_action(Jit &jit, State &state, StateAction &action) {
    jit.emit_symbol("state_" + state.name + "_" +
                    std::to_string(action.slot_condition));
    // mov [tape_base+cell_offset], action.write
    // Either:
    //   inc cell_offset
    //   mod tape_size
    // Or:
    //   add cell_offset, tape_size
    //   dec cell_offset
    //   mod tape_size
    // mov state, action.next_state
}

void compile_state(Jit &jit, State &state) {
    jit.emit_symbol("state_" + state.name);
    //  cmp [tape_base+cell_offset], 0
    //  jne stateX1
    // stateX0:
    compile_state_action(jit, state, state.actions[0]);
    // stateX1:
    compile_state_action(jit, state, state.actions[1]);
}

void say_hi(const char *arg) {
    cout << "Hi from the jit." << flush << endl;
    cout << "Passed argument was: " << (void*)arg << flush <<endl;
    cout << arg << flush << endl;
    cout << "Jit says bye." << flush << endl;
}

int main(int argc, char **argv) {
    uint64_t output = 666;
    const char *format_string = "jit says hi";

    Jit jit;

    jit.emit_symbol("output", &output);
    jit.emit_symbol("printf", (void*)printf);
    jit.emit_symbol("say_hi", (void*)say_hi);
    jit.emit_symbol("exit", (void*)exit);
    //jit.emit_symbol("format_string", format_string);
    jit.emit_function("main", 0, [format_string](auto jit, auto name, auto end_label) {
        // Get address of 'output' variable:
        jit->emit_mov(Register::RAX, jit->symbol("output"));
        // Put function argument into output variable
        jit->emit_mov(Indirect(Register::RAX), Register::RDI);

        jit->emit_function_call(jit->symbol("printf"), 1, format_string);
//        jit->emit_function_call(jit->symbol("exit"), 1, 0);
        // Return value '23'
        jit->emit_mov(Register::RAX, 23);
    });

    cout << "format_str: @" << (void*)format_string << " has contents " << format_string << endl;
    cout << "printf: @" << (void*)printf << endl;
    cout << "exit: @" << (void*)exit << endl;
    jit.emit_function_call(jit.symbol("exit"), 1, 0);
//    jit.emit_function_call(jit.symbol("printf"), 1, format_string);
    jit.finalize_code();

    {
        auto memory = jit.dump_memory();
        ofstream file("func.bin");
        for (auto byte : memory) {
            file.write((char *)&byte, 1);
        }
    }

    auto result = jit.call("main", 42);
    cout << "Result: " << (void *)result << endl;
    cout << "Side-effect: " << output << endl;
    return 0;
}