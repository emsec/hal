#include "verilator/verilator.h"

#include <sstream>

namespace hal
{
    namespace verilator
    {
        const std::string get_makefile_template()
        {
            std::stringstream makefile_template;
            makefile_template << "#!/bin/bash\n"
                              << "\n"
                              << "#### CONFIG ####\n"
                              << "design_name=\"V<design_name>\"\n"
                              << "verilator_base_command=\"verilator -I. -Wall -Wno-fatal --MMD -trace-fst <num_of_trace_threads> -y gate_definitions/ --Mdir obj_dir <num_of_binary_trace_threads> "
                                 "--noassert --exe -cc -DSIM_VERILATOR --trace-depth 1\"\n"
                              << "num_of_build_threads=<num_of_build_threads>\n"
                              << "final_compile_base_command=\'g++  -Llib   testbench.o verilated.o verilated_fst_c.o <verilated_threads.o> \'\"$design_name\"\'__ALL.a   -lz  -pthread -lpthread "
                                 "-latomic   -o \'\"$design_name\"\' -lm -lstdc++\'\n"
                              << "\n"
                              << "#### DO NOT TOUCH ####\n"
                              << "echo \"Automated Simulation Script\"\n"
                              << "echo\n"
                              << "\n"
                              << "declare -a parts\n"
                              << "\n"
                              << "echo \"collecting files\"\n"
                              << "# collect all cpp files\n"
                              << "for entry in *.cpp\n"
                              << "do\n"
                              << "    if [[ $entry == *\"part\"* ]]; then\n"
                              << "        parts+=(\"${entry/.cpp/\"\"}\")\n"
                              << "    fi\n"
                              << "done\n"
                              << "\n"
                              << "\n"
                              << "echo \"found ${#parts[@]} part file(s)\"\n"
                              << "echo\n"
                              << "\n"
                              << "echo \"starting verilator...\"\n"
                              << "\n"
                              << "# building verilator command\n"
                              << "verilator_command=\"$verilator_base_command\"\n"
                              << "\n"
                              << "for entry in \"${parts[@]}\"\n"
                              << "do\n"
                              << "    verilator_command+=\" $entry.cpp\"\n"
                              << "done\n"
                              << "\n"
                              << "verilator_command+=\" testbench.cpp chip.v\"\n"
                              << "\n"
                              << "$verilator_command\n"
                              << "ret_code=$?\n"
                              << "if [ $ret_code != 0 ]; then\n"
                              << "    printf \'error in verilator, %d\\n\' $ret_code\n"
                              << "    exit $ret_code\n"
                              << "fi\n"
                              << "\n"
                              << "make -j$num_of_build_threads --no-print-directory -C obj_dir/ -f $design_name.mk\n"
                              << "\n"
                              << "ret_code=$?\n"
                              << "\n"
                              << "#fallback: try building $design_name with dynamic library\n"
                              << "if [ $ret_code != 0 ]; then\n"
                              << "    printf \'error in compilation, using fallback, return code: %d\\n\' $ret_code\n"
                              << "    mkdir obj_dir/lib\n"
                              << "    export LD_LIBRARY_PATH=obj_dir/lib/:$LD_LIBRARY_PATH\n"
                              << "\n"
                              << "    final_compile_command=\"$final_compile_base_command\"\n"
                              << "\n"
                              << "    for entry in \"${parts[@]}\"\n"
                              << "    do\n"
                              << "        o_file=\"obj_dir/$entry.o\"\n"
                              << "        so_file=\"obj_dir/lib/lib$entry.so\"\n"
                              << "\n"
                              << "        if test -f \"$o_file\"; then\n"
                              << "            gcc -shared -o $so_file $o_file\n"
                              << "        else \n"
                              << "            echo \"missing .o file, fallback failed, abort...\"\n"
                              << "            exit ret_code\n"
                              << "        fi\n"
                              << "\n"
                              << "        final_compile_command+=\" -l$entry\"\n"
                              << "\n"
                              << "    done\n"
                              << "    cd obj_dir\n"
                              << "    $final_compile_command\n"
                              << "    cd ..\n"
                              << "    ret_code=$?\n"
                              << "    if [ $ret_code != 0 ]; then\n"
                              << "        echo \"compile with dynamic link failed, exiting now...\"\n"
                              << "        exit $ret_code\n"
                              << "    fi\n"
                              << "fi\n"
                              << "\n"
                              << "obj_dir/$design_name";
            return makefile_template.str();
        }

        const std::string get_testbench_cpp_template()
        {
            std::stringstream testbench_cpp_template;
            testbench_cpp_template << "#include <stdlib.h>\n"
                                   << "#include <iostream>\n"
                                   << "#include <verilated.h>\n"
                                   << "#include <verilated_fst_c.h>\n"
                                   << "#include \"V<top_system>.h\"\n"
                                   << "#include \"testbench.h\"\n"
                                   << "\n"
                                   << "#define MAX_SIM_TIME 20\n"
                                   << "vluint64_t sim_time = 0;\n"
                                   << "\n"
                                   << "double sc_time_stamp(){\n"
                                   << "\treturn sim_time;\n"
                                   << "}\n"
                                   << "\n"
                                   << "int main(int argc, char** argv, char** env) {\n"
                                   << "    V<top_system> *dut = new V<top_system>;\n"
                                   << "\n"
                                   << "    Verilated::traceEverOn(true);\n"
                                   << "    VerilatedFstC *m_trace = new VerilatedFstC;\n"
                                   << "\n"
                                   << "\n"
                                   << "    dut->trace(m_trace, 1);\n"
                                   << "    m_trace->open(\"waveform.fst\");\n"
                                   << "\n"
                                   << "<insert_trace_here>\n"
                                   << "\n"
                                   << "\tm_trace->dump(sim_time);\n"
                                   << "    m_trace->close();\n"
                                   << "    delete dut;\n"
                                   << "    exit(EXIT_SUCCESS);\n"
                                   << "}";
            return testbench_cpp_template.str();
        }
        const std::string get_partial_testbench_cpp_template()
        {
            std::stringstream partial_testbench_cpp_template;
            partial_testbench_cpp_template << "#include <stdlib.h>\n"
                                           << "#include <verilated.h>\n"
                                           << "#include <verilated_fst_c.h>\n"
                                           << "#include \"V<top_system>.h\"\n"
                                           << "\n"
                                           << "void <part_xy>(V<top_system>* dut, VerilatedFstC* m_trace, vluint64_t& sim_time) {\n"
                                           << "<insert_trace_here>\n"
                                           << "}";
            return partial_testbench_cpp_template.str();
        }

        const std::string get_testbench_h_template()
        {
            std::stringstream partial_testbench_h_template;
            partial_testbench_h_template << "#include <stdlib.h>\n"
                                         << "#include <verilated.h>\n"
                                         << "#include <verilated_fst_c.h>\n"
                                         << "#include \"V<top_system>.h\"\n"
                                         << "\n"
                                         << "<insert_partial_testbench_here>\n";
            return partial_testbench_h_template.str();
        }

    }    // namespace verilator
}    // namespace hal