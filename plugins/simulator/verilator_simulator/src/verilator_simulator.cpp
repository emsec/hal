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
#include "hal_core/utilities/log.h"

#include <filesystem>
#include <sstream>
#include <vector>

namespace hal {

namespace verilator_simulator {

    namespace converter {
        std::vector<std::string> get_vector_for_const_char(const char** txt)
        {
            std::vector<std::string> retval;
            const char* s = *txt;
            while (s)
            {
                retval.push_back(std::string(s));
                s++;
            }
            return retval;
        }
    }

    /// subsequent code is only meant to illustrate use of classes

    const int VerilatorEngine::s_command_lines = 3;

    void VerilatorEngine::setSimulationInput(SimulationInput* simInput)
    {
        // most of the work is done here, setup Verilator files using the following information:


        simInput->get_gates();      // returns unordered set of gates which are part of simulation
        simInput->get_clocks();     // returns vector of clocks setup for simulation
        simInput->get_input_nets(); // returns vector of input nets
        simInput->get_simulation_net_events(); // returns vector of net events (values [0,1,x,z] for net ids, duration of simulation step
    }

    int VerilatorEngine::numberCommandLines() const
    {
        return s_command_lines;
    }

    std::vector<std::string> VerilatorEngine::commandLine(int lineIndex) const
    {
        // returns commands to be executed
        switch (lineIndex)
        {
        case 0:
        {
            const char* cl[] = {"verilator", "-I.", "-Wall", "-Wno-fatal", "--MMD", "-trace", "-y", "xilinx_unisim_includes/",
                                "--Mdir", "obj_dir", "--exe", "-cc", "-DSIM_VERILATOR", "--trace-depth", "1",
                                "generic_tb.cpp", "counter.v", nullptr};
            return converter::get_vector_for_const_char(cl);
        }
            break;
        case 1:
        {
            const char* cl[] = {"make", "--no-print-directory", "-C", "obj_dir/", "-f", "Vcounter.mk", nullptr};
            return converter::get_vector_for_const_char(cl);
        }
            break;
        case 2:
        {
            const char* cl[] = {"verilator", "-I.", "-Wall", "-Wno-fatal", nullptr };
            return converter::get_vector_for_const_char(cl);
        }
            break;
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
