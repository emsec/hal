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
             * Get all used gate types in netlist.
             * 
             * @param[in] nl - Netlist for which the models shall be created
             * @returns Set of used gate types.
             */
            std::set<GateType*> get_gate_gate_types_from_netlist(const Netlist* nl);

            /**
             * Generate the parameters function for the simulation model of the gate.
             * 
             * @param[in] gt - The gate type to create the parameters for.
             * @returns Vector of all parameters that the gate type has.
             */
            std::vector<std::string> get_parameters_for_gate(const GateType* gt);

            /**
             * Generates the prologue for the simulation model.
             * 
             * @param[in] gt - The gate type to create the parameters for.
             * @returns The prologue for the simulation model.
             */
            std::string get_prologue_for_gate_type(const GateType* gt);

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
             * @param[in] gt - The gate type to create the epilogue for.
             * @returns The epilogue for the gate for the simulation model.
             */
            std::string get_epilogue_for_gate_type(const GateType* gt);
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
            int m_num_of_threads      = 4;
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
