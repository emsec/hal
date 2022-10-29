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

#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/gate_library/enums/async_set_reset_behavior.h"
#include "hal_core/netlist/gate_library/gate_type.h"
#include "hal_core/netlist/net.h"
#include "netlist_simulator/simulation.h"
#include "netlist_simulator_controller/simulation_engine.h"

#include <map>
#include <unordered_set>

namespace hal
{
    class SimulationInput;

    class NetlistSimulator : public SimulationEngineEventDriven
    {
        friend class NetlistSimulatorFactory;

    public:
        /**
         * Get all gates that are in the simulation set.
         *
         * @returns The simulation set.
         */
        const std::unordered_set<Gate*>& get_gates() const;

        /**
         * Set the signal for a specific wire to control input signals between simulation cycles.
         *
         * @param[in] net - The net to set a signal value for.
         * @param[in] value - The value to set.
         */
        void set_input(const Net* net, BooleanFunction::Value value);

        /**
         * Configure the sequential gates matching the (optional) user-defined filter condition with initialization data specified within the netlist.
         * Schedules the respective gates for initialization, the actual configuration is applied during initialization of the simulator.
         * This function can only be called before the simulation has started.
         * 
         * @param[in] filter - The optional filter to be applied before initialization.
         */
        void initialize_sequential_gates(const std::function<bool(const Gate*)>& filter = nullptr);

        /**
         * Configure the sequential gates matching the (optional) user-defined filter condition with the specified value.
         * Schedules the respective gates for initialization, the actual configuration is applied during initialization of the simulator.
         * This function can only be called before the simulation has started.
         * 
         * @param[in] value - The value to initialize the selected gates with.
         * @param[in] filter - The optional filter to be applied before initialization.
         */
        void initialize_sequential_gates(BooleanFunction::Value value, const std::function<bool(const Gate*)>& filter = nullptr);

        /**
         * \deprecated
         * DEPRECATED <br>
         * Load the specified initial value into the current state of all sequential elements.
         * 
         * @param[in] value - The initial value to load.
         */
        [[deprecated("Will be removed in a future version. Use initialize_sequential_gates() instead.")]] void load_initial_values(BooleanFunction::Value value);

        /**
         * \deprecated
         * DEPRECATED <br>
         * Load the initial value specified within the netlist file into the current state of all sequential elements.
         * This is especially relevant for FPGA netlists, since these may provide initial values to load on startup.
         */
        [[deprecated("Will be removed in a future version. Use initialize_sequential_gates() instead.")]] void load_initial_values_from_netlist();

        /**
         * Initialize the simulation.
         * No additional gates or clocks can be added after this point.
         */
        void initialize();

        /**
         * Simulate for a specific period, advancing the internal state.
         * Automatically initializes the simulation if 'initialize' has not yet been called.
         * Use 'set_input' to control specific signals.
         *
         * @param[in] picoseconds - The duration to simulate.
         */
        void simulate(u64 picoseconds);

        SimulationInput* get_simulation_input() const override
        {
            return mSimulationInput;
        }

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

        /**
         * Get vector of simulated events for net
         *
         * @param[in] n - The net for which events where simulated
         * @return Vector of events
         */
        std::vector<WaveEvent> get_simulation_events(u32 netId) const override;

        bool inputEvent(const SimulationInputNetEvent& netEv) override;

    private:
        friend class NetlistSimulatorPlugin;

        struct SimulationGate
        {
            const Gate* m_gate;
            std::vector<GatePin*> m_input_pins;
            std::vector<const Net*> m_input_nets;
            std::unordered_map<std::string, BooleanFunction::Value> m_input_values;

            SimulationGate(const Gate* gate);
            virtual ~SimulationGate() = default;

            virtual bool simulate(const Simulation& simulation, const WaveEvent& event, std::map<std::pair<const Net*, u64>, BooleanFunction::Value>& new_events) = 0;
        };

        struct SimulationGateCombinational : public SimulationGate
        {
            std::vector<GatePin*> m_output_pins;
            std::vector<const Net*> m_output_nets;
            std::unordered_map<const Net*, BooleanFunction> m_functions;

            SimulationGateCombinational(const Gate* gate);

            bool simulate(const Simulation& simulation, const WaveEvent& event, std::map<std::pair<const Net*, u64>, BooleanFunction::Value>& new_events) override;
        };

        struct SimulationGateSequential : public SimulationGate
        {
            SimulationGateSequential(const Gate* gate);

            virtual void initialize(std::map<const Net*, BooleanFunction::Value>& new_events, bool from_netlist, BooleanFunction::Value value)                    = 0;
            virtual bool simulate(const Simulation& simulation, const WaveEvent& event, std::map<std::pair<const Net*, u64>, BooleanFunction::Value>& new_events) = 0;
            virtual void clock(const u64 current_time, std::map<std::pair<const Net*, u64>, BooleanFunction::Value>& new_events)                                  = 0;
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

            void initialize(std::map<const Net*, BooleanFunction::Value>& new_events, bool from_netlist, BooleanFunction::Value value) override;
            bool simulate(const Simulation& simulation, const WaveEvent& event, std::map<std::pair<const Net*, u64>, BooleanFunction::Value>& new_events) override;
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

                std::vector<GatePin*> address_pins;
                std::vector<GatePin*> data_pins;
            };

            std::vector<Port> m_ports;
            std::vector<u64> m_data;
            u32 m_bit_size;
            std::vector<size_t> m_clocked_read_ports;
            std::vector<size_t> m_clocked_write_ports;

            SimulationGateRAM(const Gate* gate);

            void initialize(std::map<const Net*, BooleanFunction::Value>& new_events, bool from_netlist, BooleanFunction::Value value) override;
            bool simulate(const Simulation& simulation, const WaveEvent& event, std::map<std::pair<const Net*, u64>, BooleanFunction::Value>& new_events) override;
            void clock(const u64 current_time, std::map<std::pair<const Net*, u64>, BooleanFunction::Value>& new_events) override;
        };

        bool m_is_initialized = false;
        std::vector<std::tuple<bool, BooleanFunction::Value, const std::function<bool(const Gate*)>>> m_init_seq_gates;

        u64 m_current_time = 0;
        std::vector<WaveEvent> m_event_queue;
        Simulation m_simulation;
        u64 m_timeout_iterations = 10000000ul;
        u64 m_id_counter         = 0;

        std::unordered_map<const Net*, std::vector<std::pair<SimulationGate*, std::vector<const GatePin*>>>> m_successors;
        std::vector<std::unique_ptr<SimulationGate>> m_sim_gates;
        std::vector<SimulationGate*> m_sim_gates_raw;

        NetlistSimulator(const std::string& nam);
        void compute_input_nets();
        void compute_output_nets();
        void prepare_clock_events(u64 nanoseconds);
        void process_events(u64 timeout);

        BooleanFunction::Value process_clear_preset_behavior(AsyncSetResetBehavior behavior, BooleanFunction::Value previous_output);
    };

    class NetlistSimulatorFactory : public SimulationEngineFactory
    {
    public:
        NetlistSimulatorFactory() : SimulationEngineFactory("hal_simulator")
        {
            ;
        }
        SimulationEngine* createEngine() const override;
    };
}    // namespace hal
