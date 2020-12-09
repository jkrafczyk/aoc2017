#include "day25.hpp"
#include "jit.hpp"
#include <iostream>
#include <fstream>

using std::string;
using std::ofstream;
using std::cout;
using std::endl;
using namespace day25;

/*
 * State machine register allocation:
 *   rax: Whatever is currently useful
 *   rcx: Remaining number of requested iterations (via call argument)
 *   rdx: Pointer to current state
 *   rbx: [Reserved, needs to be restored!]
 *   rdi: Tape base address
 *   rsp: Reserved!
 *   rbp: Reserved!
 */

void compile_state_action(Jit &jit, State &state, StateAction &action) {
    jit.emit_symbol("state_" + state.name + "_" + std::to_string(action.slot_condition));
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
    //stateX0:
    compile_state_action(jit, state, state.actions[0]);
    //stateX1:
    compile_state_action(jit, state, state.actions[1]);
}

void function_prelude(Jit &jit, const string &name) {
    jit.emit_symbol(name);
    jit.emit_symbol("_" + name + "_start");
    jit.emit_push(Register::RBP);
    jit.emit_mov(Register::RBP, Register::RSP);
    //This really should be 'sub rsp, 0x10', but it isn't.
    jit.emit_add(Register::RSP, -0x10);
}

void function_epilogue(Jit &jit, const string &name)  {
    jit.emit_symbol("_" + name + "end");
    jit.emit_add(Register::RSP, 0x10);
    jit.emit_pop(Register::RBP);
    jit.emit_ret();
}

int main(int argc, char** argv) {
    uint64_t output = 666;
    Jit jit;

    jit.emit_symbol("output", &output);
    function_prelude(jit, "main");
    //Get address of 'output' variable:
    jit.emit_mov(Register::RAX, jit.symbol("output"));
    //Put function argument into output variable
    jit.emit_mov(Indirect(Register::RAX), Register::RDI);
    //Return value '23'
    jit.emit_mov(Register::RAX, 23);
    function_epilogue(jit, "main");
    jit.finalize_code();

    auto result = jit.call("main", 80085);
    cout << "Call returned: " << result << endl
         << "'output' is now: " << output << endl;

    {
        auto memory = jit.dump_memory();
        ofstream file("func.bin");
        for (auto byte: memory) {
            file.write((char *) &byte, 1);
        }
    }

//    auto result = jit.call("main", 42);
//    cout << "Result: " << (void*)result << endl;
//    cout << "Side-effect: " << output << endl;
    return 0;
}