//  MIT License
//
//  Copyright (c) 2019 Ruhr University Bochum, Chair for Embedded Security. All Rights reserved.
//  Copyright (c) 2021 Max Planck Institute for Security and Privacy. All Rights reserved.
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in all
//  copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//  SOFTWARE.

#pragma once

#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/gate_library/enums/async_set_reset_behavior.h"
#include "hal_core/netlist/gate_library/gate_type.h"
#include "hal_core/netlist/net.h"
#include "netlist_simulator/simulation.h"

#include <map>
#include <unordered_set>

namespace hal
{
    class NetlistSimulator
    {
    public:
        /**
         * Add gates to the simulation set.
         * Only elements in the simulation set are considered during simulation.
         *
         * @param[in] gates - The gates to add.
         */
        void add_gates(const std::vector<Gate*>& gates);

        /**
         * Specify a net that carries the clock signal and set the clock frequency in hertz.
         *
         * @param[in] clock_net - The net that carries the clock signal.
         * @param[in] frequency - The clock frequency in hertz.
         * @param[in] start_at_zero - Initial clock state is 0 if true, 1 otherwise.
         */
        void add_clock_frequency(const Net* clock_net, u64 frequency, bool start_at_zero = true);

        /**
         * Specify a net that carries the clock signal and set the clock period in picoseconds.
         *
         * @param[in] clock_net - The net that carries the clock signal.
         * @param[in] period - The clock period from rising edge to rising edge in picoseconds.
         * @param[in] start_at_zero - Initial clock state is 0 if true, 1 otherwise.
         */
        void add_clock_period(const Net* clock_net, u64 period, bool start_at_zero = true);

        /**
         * Get all gates that are in the simulation set.
         *
         * @returns The simulation set.
         */
        const std::unordered_set<Gate*>& get_gates() const;

        /**
         * Get all nets that are considered inputs, i.e., not driven by a gate in the simulation set or global inputs.
         *
         * @returns The input nets.
         */
        const std::vector<const Net*>& get_input_nets() const;

        /**
         * Get all output nets of gates in the simulation set that have a destination outside of the set or that are global outputs.
         *
         * @returns The output nets.
         */
        const std::vector<const Net*>& get_output_nets() const;

        /**
         * Set the signal for a specific wire to control input signals between simulation cycles.
         *
         * @param[in] net - The net to set a signal value for.
         * @param[in] value - The value to set.
         */
        void set_input(const Net* net, BooleanFunction::Value value);

        /**
         * Load the specified initial value into the current state of all sequential elements.
         * 
         * @param[in] value - The initial value to load.
         */
        void load_initial_values(BooleanFunction::Value value);

        /**
         * Load the initial value specified within the netlist file into the current state of all sequential elements.
         * This is especially relevant for FPGA netlists, since these may provide initial values to load on startup.
         */
        void load_initial_values_from_netlist();

        /**
         * Simulate for a specific period, advancing the internal state.
         * Use 'set_input' to control specific signals.
         *
         * @param[in] picoseconds - The duration to simulate.
         */
        void simulate(u64 picoseconds);

        /**
         * Reset the simulator state, i.e., treat all signals as unknown.
         * Does not remove gates/nets from the simulation set.
         */
        void reset();

        /**
         * Set the simulator state, i.e., net signals, to a given state.
         * Does not influence gates/nets added to the simulation set.
         *
         * @param[in] state - The state to apply.
         */
        void set_simulation_state(const Simulation& state);

        /**
         * Get the current simulation state.
         *
         * @returns The current simulation state.
         */
        const Simulation& get_simulation_state() const;

        /**
         * Set the iteration timeout, i.e., the maximum number of events processed for a single point in time.
         * Useful to abort in case of infinite loops.
         * A value of 0 disables the timeout.
         *
         * @param[in] iterations - The iteration timeout.
         */
        void set_iteration_timeout(u64 iterations);

        /**
         * Get the current iteration timeout value.
         *
         * @returns The iteration timeout.
         */
        u64 get_simulation_timeout() const;

        /**
         * Generates the a VCD file for parts the simulated netlist.
         * 
         * @param[in] path - The path to the VCD file.
         * @param[in] start_time - Start of the timeframe to write to the file (in picoseconds).
         * @param[in] end_time - End of the timeframe to write to the file (in picoseconds).
         * @param[in] nets - Nets to include in the VCD file.
         * @returns True if the file gerneration was successful, false otherwise.
         */
        bool generate_vcd(const std::filesystem::path& path, u32 start_time, u32 end_time, std::set<const Net*> nets = {}) const;

    private:
        friend class NetlistSimulatorPlugin;
        NetlistSimulator();

        struct Clock
        {
            const Net* clock_net;
            u64 switch_time;
            bool start_at_zero;
        };

        std::unordered_set<Gate*> m_simulation_set;
        std::vector<Clock> m_clocks;

        std::vector<const Net*> m_input_nets;
        std::vector<const Net*> m_output_nets;

        bool m_needs_initialization = true;

        u64 m_current_time = 0;
        std::vector<Event> m_event_queue;
        Simulation m_simulation;
        u64 m_timeout_iterations = 10000000ul;
        u64 m_id_counter         = 0;

        struct SimulationGate
        {
            const Gate* m_gate;
            std::vector<std::string> m_input_pins;
            std::vector<const Net*> m_input_nets;
            std::unordered_map<std::string, BooleanFunction::Value> m_input_values;

            SimulationGate(const Gate* gate);
            virtual ~SimulationGate() = default;

            virtual bool simulate(const Simulation& simulation, const Event& event, std::map<std::pair<const Net*, u64>, BooleanFunction::Value>& new_events) = 0;
        };

        struct SimulationGateCombinational : public SimulationGate
        {
            std::vector<std::string> m_output_pins;
            std::vector<const Net*> m_output_nets;
            std::unordered_map<const Net*, BooleanFunction> m_functions;

            SimulationGateCombinational(const Gate* gate);

            bool simulate(const Simulation& simulation, const Event& event, std::map<std::pair<const Net*, u64>, BooleanFunction::Value>& new_events) override;
        };

        struct SimulationGateSequential : public SimulationGate
        {
            SimulationGateSequential(const Gate* gate);

            virtual bool simulate(const Simulation& simulation, const Event& event, std::map<std::pair<const Net*, u64>, BooleanFunction::Value>& new_events) = 0;
            virtual void clock(const u64 current_time, std::map<std::pair<const Net*, u64>, BooleanFunction::Value>& new_events)                              = 0;
        };

        struct SimulationGateFF : public SimulationGateSequential
        {
            BooleanFunction m_clock_func;
            BooleanFunction m_clear_func;
            BooleanFunction m_preset_func;
            BooleanFunction m_next_state_func;
            std::vector<const Net*> m_state_output_nets;
            std::vector<const Net*> m_state_inverted_output_nets;
            std::vector<const Net*> m_clock_nets;
            AsyncSetResetBehavior m_sr_behavior_out;
            AsyncSetResetBehavior m_sr_behavior_out_inverted;
            BooleanFunction::Value m_output;
            BooleanFunction::Value m_inv_output;

            SimulationGateFF(const Gate* gate);

            bool simulate(const Simulation& simulation, const Event& event, std::map<std::pair<const Net*, u64>, BooleanFunction::Value>& new_events) override;
            void clock(const u64 current_time, std::map<std::pair<const Net*, u64>, BooleanFunction::Value>& new_events) override;
        };

        struct SimulationGateRAM : public SimulationGateSequential
        {
            struct Port
            {
                const Net* clock_net;
                BooleanFunction clock_func;
                BooleanFunction enable_func;
                bool is_write;

                std::vector<std::string> address_pins;
                std::vector<std::string> data_pins;
            };

            std::vector<Port> m_ports;
            std::vector<u64> m_data;
            std::vector<size_t> m_clocked_port_indices;

            SimulationGateRAM(const Gate* gate);

            bool simulate(const Simulation& simulation, const Event& event, std::map<std::pair<const Net*, u64>, BooleanFunction::Value>& new_events) override;
            void clock(const u64 current_time, std::map<std::pair<const Net*, u64>, BooleanFunction::Value>& new_events) override;
        };

        std::unordered_map<const Net*, std::vector<std::pair<SimulationGate*, std::vector<std::string>>>> m_successors;
        std::vector<std::unique_ptr<SimulationGate>> m_sim_gates;

        void compute_input_nets();
        void compute_output_nets();
        void initialize();
        void prepare_clock_events(u64 nanoseconds);
        void process_events(u64 timeout);

        BooleanFunction::Value process_clear_preset_behavior(AsyncSetResetBehavior behavior, BooleanFunction::Value previous_output);
    };
}    // namespace hal
