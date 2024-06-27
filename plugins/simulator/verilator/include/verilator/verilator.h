// MIT License
// 
// Copyright (c) 2019 Ruhr University Bochum, Chair for Embedded Security. All Rights reserved.
// Copyright (c) 2019 Marc Fyrbiak, Sebastian Wallat, Max Hoffmann ("ORIGINAL AUTHORS"). All rights reserved.
// Copyright (c) 2021 Max Planck Institute for Security and Privacy. All Rights reserved.
// Copyright (c) 2021 Jörn Langheinrich, Julian Speith, Nils Albartus, René Walendy, Simon Klix ("ORIGINAL AUTHORS"). All Rights reserved.
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once

#include "hal_core/defines.h"
#include "netlist_simulator_controller/simulation_engine.h"

#include <filesystem>
#include <set>
#include <vector>

namespace hal
{
    class Netlist;
    class Gate;
    class GateType;
    class Net;

    namespace verilator
    {
        namespace converter
        {
            /**
             * Generate verilog simulation models based on gate library provided in HAL.
             * 
             * @param[in] nl - Netlist for which the models shall be created
             * @param[in] verilator_sim_path - Path where the verilator simulation files are going to be stored.
             * @param[in] model_path - Path where the provided models are located.
             * @returns True if the model gerneration was successful, false otherwise.
             */
            bool convert_gate_library_to_verilog(const Netlist* nl, const std::filesystem::path verilator_sim_path, const std::filesystem::path model_path = "");

            /**
             * Gets the list of provided models, for which no simulation models have to generated.
             * The function also copies the provided models to the simulation folder.
             * 
             * @param[in] model_path - Path where the provided models are located.
             * @param[in] verilator_sim_path - Path where the models shall be copied to.
             * @returns Set of provided gate types.
             */
            std::set<std::string> get_provided_models(const std::filesystem::path model_path, const std::filesystem::path gate_definition_path);

            /**
             * Get all used gate types in netlist mapped to the appropriate variable name.
             * 
             * @param[in] nl - Netlist for which the models shall be created
             * @returns Map of used gate types and variable names.
             */
            std::unordered_map<GateType *, std::string> get_gate_gate_types_from_netlist(const Netlist* nl);

            /**
             * Generate the parameters function for the simulation model of the gate.
             * 
             * @param[in] gt - The gate type to create the parameters for.
             * @returns Vector of all parameters that the gate type has.
             */
            std::vector<std::string> get_parameters_for_gate(const GateType* gt);

            /**
             * Get the gate type name cleaned from characters that are not elegible in a variable name
             * @param gt - The gate type to get the name from
             * @return String with gate name suitable as variable name
             */
            std::string get_name_for_gate_type(const GateType* gt);

            /**
             * Generates the prologue for the simulation model.
             * 
             * @param[in] gt - The gate type to create the parameters for.
             * @returns The prologue for the simulation model.
             */
            std::string get_prologue_for_gate_type(const GateType* gt);

            /**
             * Translate a given Boolean function node into a human-readable string in Verilog format.
             *
             * @param[in] node The node of a Boolean function.
             * @param[in] operands The operands of the node .
             * @returns A string in Verilog format on success or an error message otherwise.
             */
            Result<std::string> verilog_function_printer(const BooleanFunction::Node& node, std::vector<std::string>&& operands);

            // gate functionality
            /**
             * Generates the functionality for the gate type.
             * 
             * @param[in] gt - The gate type to create the function for.
             * @returns The function of the gate for the simulation model.
             */
            std::string get_function_for_gate(const GateType* gt);

            /**
             * Generates the functionality for the LUT gate type.
             * 
             * @param[in] gt - The gate type to create the function for.
             * @returns The function of the LUT gate for the simulation model.
             */
            std::string get_function_for_lut(const GateType* gt);

            /**
             * Generates the functionality for the combinational gate.
             * 
             * @param[in] gt - The gate type to create the function for.
             * @returns The function of the combinational gate for the simulation model.
             */
            std::string get_function_for_combinational_gate(const GateType* gt);

            /**
             * Generates the functionality for the FF gate type.
             * 
             * @param[in] gt - The gate type to create the function for.
             * @returns The function of the FF gate for the simulation model.
             */
            std::string get_function_for_ff(const GateType* gt);

            /**
             * Generates the functionality for the latch gate type.
             * 
             * @param[in] gt - The gate type to create the function for.
             * @returns The function of the latch gate for the simulation model.
             */
            std::string get_function_for_latch(const GateType* gt);

            /**
             * Generates the epilogue for the gate type.
             * 
             * @returns The epilogue for the gate for the simulation model.
             */
            std::string get_epilogue_for_gate_type();
        }    // namespace converter

        class VerilatorEngine : public SimulationEngineScripted
        {
            // path to VCD file with results when simulation done is SimulationEngine::mResultFilename

            static const int s_command_lines;

        public:
            VerilatorEngine(const std::string& nam);
            std::unique_ptr<Netlist> m_partial_netlist;
            std::filesystem::path m_simulator_dir;

            std::string m_design_name;

            bool setSimulationInput(SimulationInput* simInput) override;
            int numberCommandLines() const override;
            std::vector<std::string> commandLine(int lineIndex) const override;
            bool finalize() override;

        private:
            bool write_testbench_files(SimulationInput* simInput);
            int m_num_of_threads = 4;
            std::string m_compiler;
        };

        class VerilatorEngineFactory : public SimulationEngineFactory
        {
        public:
            VerilatorEngineFactory() : SimulationEngineFactory("verilator")
            {
            }
            SimulationEngine* createEngine() const override;
        };
    }    // namespace verilator

}    // namespace hal
