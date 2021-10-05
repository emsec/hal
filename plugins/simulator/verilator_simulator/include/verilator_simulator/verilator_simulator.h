#pragma once

#include "hal_core/defines.h"
#include "netlist_simulator_controller/simulation_engine.h"

#include <set>
#include <vector>
#include <filesystem>

namespace hal {
class Netlist;
class Gate;
class GateType;
class Net;

namespace verilator_simulator {
    namespace converter {
        bool convert_gate_library_to_verilog(const Netlist* nl, std::filesystem::path path);

        std::set<GateType*> get_gate_gate_types_from_netlist(const Netlist* nl);

        std::vector<std::string> get_parameters_for_gate(const GateType* gt);

        std::string get_prologue_for_gate_type(const GateType* gt);

        // gate functionality
        std::string get_function_for_gate(const GateType* gt);
        std::string get_function_for_lut(const GateType* gt);
        std::string get_function_for_combinational_gate(const GateType* gt);
        std::string get_function_for_ff(const GateType* gt);
        std::string get_function_for_latch(const GateType* gt);

        std::string get_epilogue_for_gate_type(const GateType* gt);

        std::vector<std::string> get_vector_for_const_char(const char** txt);
    }

    class VerilatorEngine : public SimulationEngineScripted {
        // path to VCD file with results when simulation done
        std::string m_result_filename;

        static const int s_command_lines;

    public:
        VerilatorEngine()
            : SimulationEngineScripted("verilator_simulator")
        {
            ;
        }
        std::unique_ptr<Netlist> m_partial_netlist;

        bool setSimulationInput(SimulationInput* simInput) override;
        std::string resultFilename() const override { return m_result_filename; }
        int numberCommandLines() const override;
        std::vector<std::string> commandLine(int lineIndex) const override;
        void done() override;
    };

} // namespace verilator_simulator

} // namespace hal
