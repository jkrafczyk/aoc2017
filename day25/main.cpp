#include "day25.hpp"
#include <fstream>
#include <iostream>
#include <numeric>

using std::cout;
using std::endl;
using std::flush;
using std::map;
using std::ofstream;
using std::ostream;
using std::string;

using namespace day25;

struct Arguments {
    string program;
    string executor;
    enum { NONE, RUN, BENCHMARK, GENERATE_C } action;
};

int usage(string cmd) {
    auto last_slash = cmd.find_last_of('/');
    if (last_slash != string::npos) {
        cmd = cmd.substr(last_slash + 1);
    }
    cout << "Usage: " << cmd << " run program executor" << endl
         << "   or: " << cmd << " benchmark program [executor]" << endl
         << "   or: " << cmd << " generate-c program" << endl
         << endl
         << "Available executors: " << endl;
    for (auto it : list_executors()) {
        cout << " * " << it << endl;
    }
    return 1;
}

Arguments parse_args(int argc, char **argv) {
    Arguments result;
    if (argc < 2) {
        return result;
    }

    for (int i = 1; i < argc; i++) {
        string arg = argv[i];
        if (i == 1) {
            if (arg == "run") {
                result.action = Arguments::RUN;
            } else if (arg == "generate-c") {
                result.action = Arguments::GENERATE_C;
            } else if (arg == "benchmark") {
                result.action = Arguments::BENCHMARK;
            } else {
                return result;
            }
        } else if (i == 2) {
            result.program = arg;
        } else if (i == 3 && (result.action == Arguments::RUN ||
                              result.action == Arguments::BENCHMARK)) {
            auto executors = list_executors();
            if (find(executors.begin(), executors.end(), arg) !=
                executors.end()) {
                result.executor = arg;
            } else {
                result.action = Arguments::NONE;
                return result;
            }
        } else {
            result.action = Arguments::NONE;
            return result;
        }
    }

    return result;
}

int generate_c(Program program, ostream &cfile) {
    map<string, int> state_map;
    for (auto state : program.states) {
        state_map[state.first] = state_map.size();
    }

    cfile << "#include <time.h>" << endl;
    cfile << "#include <stdio.h>" << endl;
    cfile << "#include <string.h>" << endl;
    cfile << "char tape[" << program.checksum_delay << "];" << endl;

    cfile << "void run() {" << endl;
    cfile << "  unsigned current_state = " << state_map[program.initial_state]
          << ";" << endl;
    cfile << "  unsigned long current_offset = 0;" << endl;
    cfile << "  for (unsigned long steps = 0; steps < "
          << program.checksum_delay << "; steps++) {" << endl;
    cfile << "    switch(current_state) {" << endl;
    for (auto state : program.states) {
        cfile << "      case " << state_map[state.first] << ":" << endl;
        cfile << "        if (tape[current_offset] == 0) {" << endl;
        cfile << "          tape[current_offset] = "
              << state.second.actions[0].write_value << ";" << endl;
        cfile << "          current_offset = (current_offset + sizeof(tape) + "
              << state.second.actions[0].move_direction << ") % sizeof(tape);"
              << endl;
        cfile << "          current_state = "
              << state_map[state.second.actions[0].next_state] << ";" << endl;
        cfile << "        } else {" << endl;
        cfile << "          tape[current_offset] = "
              << state.second.actions[1].write_value << ";" << endl;
        cfile << "          current_offset = (current_offset + sizeof(tape) + "
              << state.second.actions[1].move_direction << ") % sizeof(tape);"
              << endl;
        cfile << "          current_state = "
              << state_map[state.second.actions[1].next_state] << ";" << endl;
        cfile << "        }" << endl;
        cfile << "        break;" << endl;
    }
    cfile << "    } //switch" << endl;
    cfile << "  } //for" << endl;
    cfile << "}; //run" << endl;
    cfile << "int main() {" << endl;
    cfile << "  //Benchmark:" << endl;
    cfile << "  clock_t start_ts = clock();" << endl;
    cfile << "  for (int i=0; i<25; i++) {" << endl;
    cfile << "    run();" << endl;
    cfile << "  }" << endl;
    cfile << "  clock_t end_ts = clock();" << endl;
    cfile << "  double duration = end_ts - start_ts;" << endl;
    cfile << "  duration /= CLOCKS_PER_SEC;" << endl;
    cfile << "  duration *= 1000;" << endl;
    cfile << R"(  printf("Time per iteration: %lfms\n", duration/25);)" << endl;
    cfile << R"(  printf("Total executed steps: )"
          << program.checksum_delay * 25 << R"(\n");)" << endl;
    cfile << "  printf(\"%lf steps/ms\\n%lf us/steps\\n\", "
          << program.checksum_delay * 25 << "/ duration, 1000 * duration / "
          << program.checksum_delay * 25 << ");" << endl;
    cfile << "//Actual execution:" << endl;
    cfile << "    memset(tape, 0, sizeof(tape));" << endl;
    cfile << "    run();" << endl;
    cfile << "  unsigned checksum = 0;" << endl;
    cfile << "  for (unsigned long i=0; i<sizeof(tape); i++) {" << endl;
    cfile << "    checksum += tape[i];" << endl;
    cfile << "  } //for" << endl;
    cfile << R"(  printf("Checksum: %i\n", checksum);)" << endl;
    cfile << "  return 0;" << endl;
    cfile << "} //main" << endl;
    return 0;
}

int run(Program program, const string &executor_name) {
    auto executor = get_executor(executor_name, program);
    cout << "Executing program." << endl;
    clock_t start_ts = clock();
    for (int i = 0; i < program.checksum_delay; i++) {
        executor->step();
    }
    clock_t end_ts = clock();
    double duration = end_ts - start_ts;
    duration *= 1000;
    duration /= CLOCKS_PER_SEC;
    cout << "Finished after " << duration << "ms" << endl;
    cout << "Diagnostic checksum: " << executor->diagnostic_checksum() << endl;
    return 0;
}

int benchmark(Program program, const string &executor_name,
              const string indent = "") {
    unsigned target_seconds = 20;
    auto target_clocks = CLOCKS_PER_SEC * target_seconds;

    if (executor_name == "") {
        int result = 0;
        cout << indent << "Benchmarking program with all executors..." << endl;
        for (auto name : list_executors()) {
            result |= benchmark(program, name, "    ");
            cout << endl;
        }
        return result;
    } else {
        cout << indent << "Benchmarking with executor " << executor_name
             << " for " << target_seconds << " seconds." << endl;
        auto executor = get_executor(executor_name, program);

        uint32_t iterations_per_block = 100000;
        uint32_t blocks_executed = 0;
        auto start_ts = clock();
        auto target_ts = start_ts + target_clocks;

        while (true) {
            auto now = clock();
            if (now > target_ts) {
                break;
            }
            for (uint32_t i = 0; i < iterations_per_block; i++) {
                executor->step();
            }
            blocks_executed++;
        }
        auto end_ts = clock();
        auto step_count = blocks_executed * iterations_per_block;
        double duration = end_ts - start_ts;
        duration *= 1000;
        duration /= CLOCKS_PER_SEC;
        cout << indent << "Took " << duration << "ms for " << step_count
             << " steps." << endl
             << indent << "  " << step_count / duration << " steps/ms" << endl
             << indent << "  " << 1000 * duration / step_count << " us/step"
             << endl;
        return 0;
    }
}

int main(int argc, char **argv) {
    auto args = parse_args(argc, argv);
    if (args.action == Arguments::NONE) {
        return usage(argv[0]);
    } else if (args.program.empty()) {
        return usage(argv[0]);
    } else if (args.action == Arguments::RUN &&
               (args.program.empty() || (args.executor.empty()))) {
        return usage(argv[0]);
    }

    auto program = load_file(args.program);

    if (args.action == Arguments::RUN) {
        return run(program, args.executor);
    } else if (args.action == Arguments::GENERATE_C) {
        string out_name = "generated-program.c";
        ofstream out_file;
        out_file.open(out_name);
        cout << "Writing program to file " << out_name << endl;
        return generate_c(program, out_file);
    } else if (args.action == Arguments::BENCHMARK) {
        return benchmark(program, args.executor);
    }

    return 0;
}