#include "verilator_simulator/verilator_simulator.h"

#include "hal_core/netlist/boolean_function.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/gate_library/enums/pin_type.h"
#include "hal_core/netlist/gate_library/gate_library.h"
#include "hal_core/netlist/gate_library/gate_type.h"
#include "hal_core/netlist/gate_library/gate_type_component/ff_component.h"
#include "hal_core/netlist/gate_library/gate_type_component/gate_type_component.h"
#include "hal_core/netlist/gate_library/gate_type_component/init_component.h"
#include "hal_core/netlist/gate_library/gate_type_component/latch_component.h"
#include "hal_core/netlist/gate_library/gate_type_component/lut_component.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/netlist/netlist_utils.h"
#include "hal_core/utilities/log.h"
#include "hal_core/utilities/utils.h"

#include "netlist_simulator_controller/simulation_input.h"
#include "verilator_simulator/templates.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

namespace hal {
namespace verilator_simulator {
    namespace converter {
        std::vector<std::string> get_vector_for_const_char(const char** txt)
        {
            std::vector<std::string> retval;
            const char* s = *txt;
            while (s) {
                retval.push_back(std::string(s));
                s++;
            }
            return retval;
        }

    }
    std::string get_makefile(const Netlist* netlist)
    {
        std::string makefile_template = get_makefile_template();

        utils::replace(makefile_template, std::string("<top_module>"), netlist->get_design_name());

        return makefile_template;
    }

    std::string get_testbench_h(const std::vector<SimulationInput::Clock>& clocks)
    {
        std::string testbench_h_template = get_testbench_h_template();

        SimulationInput::Clock slowest_clock = clocks.at(0);

        // check if all start at zero
        for (const auto& clock : clocks) {
            if (!clock.start_at_zero) {
                log_error("verilator_simulator", "unsupported reached: clocks have to start at zero for this engine");
            }
        }

        // identify slowest clock
        for (const auto& clock : clocks) {
            if (slowest_clock.switch_time > clock.switch_time) {
                slowest_clock = clock;
            }
        }

        // clock defines
        std::stringstream clock_defines;
        std::stringstream clock_inits;
        std::stringstream clock_advances;
        std::stringstream clock_falling_edge;
        std::stringstream sim_clock_tick;
        std::stringstream mintime_clocks;
        mintime_clocks << "\t\tunsigned mintime = m_" << slowest_clock.clock_net->get_name() << ".time_to_tick();" << std::endl;

        for (const auto& clock : clocks) {
            std::string clock_name = clock.clock_net->get_name();
            clock_defines << "\tTBCLOCK m_" << clock_name << ";" << std::endl;
            clock_inits << "\t\tm_" << clock_name << ".init(" << clock.switch_time * 2 << ");" << std::endl;
            clock_advances << "\t\tm_core->" << clock_name << " = m_" << clock_name << ".advance(mintime);" << std::endl;

            clock_falling_edge << "\t\tif (m_" << clock_name << ".falling_edge() {" << std::endl;
            clock_falling_edge << "\t\t\tm_changed = true;" << std::endl;
            clock_falling_edge << "\t\t\tsim_" << clock_name << "_tick();" << std::endl;
            clock_falling_edge << "\t\t}" << std::endl;

            sim_clock_tick << "\tvirtual void sim_" << clock_name << "_tick(void){" << std::endl;
            sim_clock_tick << "\t\t\tm_changed = false;" << std::endl;
            sim_clock_tick << "\t\t}" << std::endl;

            if (clock.clock_net == slowest_clock.clock_net) {
                continue;
            }

            mintime_clocks << "\t\tif (m_" << clock_name << ".time_to_tick() < mintime) {" << std::endl;
            mintime_clocks << "\t\t\tmintime = m_" << clock_name << ".time_to_tick()" << std::endl;
            mintime_clocks << "\t\t}" << std::endl;
        }

        utils::replace(testbench_h_template, std::string("<CLOCK_DEFINES>"), clock_defines.str());
        utils::replace(testbench_h_template, std::string("<CLOCKS_INITS>"), clock_inits.str());
        utils::replace(testbench_h_template, std::string("<MINTIME_CLOCKS>"), mintime_clocks.str());
        utils::replace(testbench_h_template, std::string("<CLOCK_ADVANCES>"), clock_advances.str());
        utils::replace(testbench_h_template, std::string("<CLOCK_FALLING_EDGE>"), clock_falling_edge.str());
        utils::replace(testbench_h_template, std::string("<<SIM_CLOCK_TICK>"), sim_clock_tick.str());
        return testbench_h_template;
    }

    /// subsequent code is only meant to illustrate use of classes

    const int VerilatorEngine::s_command_lines = 3;

    bool VerilatorEngine::setSimulationInput(SimulationInput* simInput)
    {
        const std::vector<const Gate*> simulation_gates(simInput->get_gates().begin(), simInput->get_gates().end());
        m_partial_netlist = netlist_utils::get_partial_netlist(simulation_gates.at(0)->get_netlist(), simulation_gates);

        std::filesystem::path simulator_dir = "/tmp/hal/simulator/";
        std::filesystem::create_directory(simulator_dir);

        std::vector<SimulationInput::Clock> clocks = simInput->get_clocks();

        // set inputs in testbench
        std::string testbench_cpp = get_testbench_cpp_template();

        // todo

        // write all files
        std::ofstream makefile_file(simulator_dir / "Makefile");
        makefile_file << get_makefile(m_partial_netlist.get());
        makefile_file.close();

        std::ofstream testbench_h_file(simulator_dir / "testbench.h");
        testbench_h_file << get_testbench_h(clocks);
        testbench_h_file.close();

        std::ofstream testbench_clock_h_file(simulator_dir / "tbclock.h");
        testbench_clock_h_file << get_testbench_clock_h_template();
        testbench_clock_h_file.close();

        std::ofstream testbench_cpp_file(simulator_dir / "testbench.cpp");
        testbench_cpp_file << testbench_cpp;
        testbench_cpp_file.close();

        if (!converter::convert_gate_library_to_verilog(m_partial_netlist.get(), simulator_dir)) {
            log_error("verilator_simulator", "could not create gate definitions in verilog");
        };

        return true; // everything ok
    }

    int VerilatorEngine::numberCommandLines() const
    {
        return s_command_lines;
    }

    std::vector<std::string> VerilatorEngine::commandLine(int lineIndex) const
    {
        // returns commands to be executed
        switch (lineIndex) {
        case 0: {
            const char* cl[] = { "verilator", "-I.", "-Wall", "-Wno-fatal", "--MMD", "-trace", "-y", "xilinx_unisim_includes/",
                "--Mdir", "obj_dir", "--exe", "-cc", "-DSIM_VERILATOR", "--trace-depth", "1",
                "generic_tb.cpp", "counter.v", nullptr };
            return converter::get_vector_for_const_char(cl);
        } break;
        case 1: {
            const char* cl[] = { "make", "--no-print-directory", "-C", "obj_dir/", "-f", "Vcounter.mk", nullptr };
            return converter::get_vector_for_const_char(cl);
        } break;
        case 2: {
            const char* cl[] = { "make", "run", nullptr };
            return converter::get_vector_for_const_char(cl);
        } break;
        default:
            break;
        }

        return std::vector<std::string>();
    }

    void VerilatorEngine::done()
    {
        // could set m_result_filename (VCD results) here
    }

} // namespace verilator_simulator
}
