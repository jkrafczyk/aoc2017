#include "day25.hpp"
#include "jit.hpp"
#include "day25.hpp"
#include "jit_executor.hpp"
#include <fstream>
#include <iostream>

using std::cout;
using std::flush;
using std::endl;
using std::ofstream;
using std::string;
using namespace day25;

int main(int argc, char **argv) {
    auto program = load_file("../sample-input");
    auto executor = JitExecutor(program);
    {
        auto memory = executor.jit().dump_memory();
        ofstream file("func.bin");
        for (auto byte : memory) {
            file.write((char *)&byte, 1);
        }
    }

    for (uint64_t i = 0; i < program.checksum_delay; i++) {
        executor.step();
    }
    return 0;
}

int other_main(int argc, char **argv) {
    Jit jit;
    jit.add_constant("some_constant", "asdfzxcv");
    auto symbol = jit.symbol("some_constant");
    cout << symbol.address << endl;
//    jit.emit_mov(Register::RDX, Register::RAX);
//    jit.emit_mov(Register::RDX, Register::R10);
//    jit.emit_mov(Register::R12, Register::RAX);
//    jit.emit_mov(Register::R12, Register::R10);
    jit.emit_inc(Register::RDX);
    jit.emit_inc(Register::R10);
    jit.emit_dec(Register::RDX);
    jit.emit_dec(Register::R10);


//    jit.emit_mov(Register::R10, Indirect(Register::R9));
    jit.finalize_code();

    {
        auto memory = jit.dump_memory();
        ofstream file("func.bin");
        for (auto byte : memory) {
            file.write((char *)&byte, 1);
        }
    }
    return 0;
}

int old_main(int argc, char **argv) {
    uint64_t output = 666;
    Jit jit;

    jit.add_constant("format_string", "Jit says hi: %i!\n");
    jit.emit_symbol("output", &output);
    jit.emit_symbol("printf", (void*)printf);
    //jit.emit_symbol("format_string", format_string);
    jit.emit_function("main", 0, [](auto jit, auto name, auto end_label) {
        // Get address of 'output' variable:
        jit->emit_mov(Register::RAX, jit->symbol("output"));
        // Put function argument into output variable
        jit->emit_mov(Indirect(Register::RAX), Register::RDI);

        jit->emit_function_call(jit->symbol("printf"), 2, jit->symbol("format_string").address, 80085);
        // Return value '23'
        jit->emit_mov(Register::RAX, 23);
    });

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