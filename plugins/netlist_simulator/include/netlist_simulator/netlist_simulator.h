#pragma once

#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/gate_library/gate_type/gate_type_sequential.h"
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
        void add_clock_frequency(Net* clock_net, u64 frequency, bool start_at_zero = true);

        /**
         * Specify a net that carries the clock signal and set the clock period in picoseconds.
         *
         * @param[in] clock_net - The net that carries the clock signal.
         * @param[in] period - The clock period from rising edge to rising edge in picoseconds.
         * @param[in] start_at_zero - Initial clock state is 0 if true, 1 otherwise.
         */
        void add_clock_period(Net* clock_net, u64 period, bool start_at_zero = true);

        /**
         * Get all gates that are in the simulation set.
         *
         * @returns The simulation set.
         */
        std::unordered_set<Gate*> get_gates() const;

        /**
         * Get all nets that are considered inputs, i.e., not driven by a gate in the simulation set or global inputs.
         *
         * @returns The input nets.
         */
        std::vector<Net*> get_input_nets() const;

        /**
         * Get all output nets of gates in the simulation set that have a destination outside of the set or that are global outputs.
         *
         * @returns The output nets.
         */
        std::vector<Net*> get_output_nets() const;

        /**
         * Set the signal for a specific wire to control input signals between simulation cycles.
         *
         * @param[in] net - The net to set a signal value for.
         * @param[in] value - The value to set.
         */
        void set_input(Net* net, SignalValue value);

        /**
         * Load the specified initial value into the current state of all sequential elements.
         * 
         * @param[in] value - The initial value to load.
         */
        void load_initial_values(SignalValue value);

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
        Simulation get_simulation_state() const;

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
         * @returns True if the file gerneration was successful, false otherwise.
         */
        bool generate_vcd(const std::filesystem::path& path, u32 start_time, u32 end_time) const;

    private:
        friend class NetlistSimulatorPlugin;
        NetlistSimulator();

        struct Clock
        {
            Net* clock_net;
            u64 switch_time;
            bool start_at_zero;
        };

        std::unordered_set<Gate*> m_simulation_set;
        std::vector<Clock> m_clocks;

        std::vector<Net*> m_input_nets;
        std::vector<Net*> m_output_nets;

        bool m_needs_initialization = true;

        u64 m_current_time = 0;
        std::vector<Event> m_event_queue;
        Simulation m_simulation;
        u64 m_timeout_iterations = 10000000ul;
        u64 m_id_counter         = 0;

        struct SimulationGate
        {
            Gate* gate;
            std::vector<std::string> input_pins;
            std::vector<Net*> input_nets;
            std::unordered_map<std::string, BooleanFunction::Value> input_values;
            bool is_flip_flop;

            virtual ~SimulationGate() = default;
        };

        struct SimulationGateCombinational : public SimulationGate
        {
            std::vector<std::string> output_pins;
            std::vector<Net*> output_nets;
            std::unordered_map<Net*, BooleanFunction> functions;
        };

        struct SimulationGateFF : public SimulationGate
        {
            BooleanFunction clock_func;
            BooleanFunction clear_func;
            BooleanFunction preset_func;
            BooleanFunction next_state_func;
            std::vector<Net*> state_output_nets;
            std::vector<Net*> state_inverted_output_nets;
            std::vector<Net*> clock_nets;
            GateTypeSequential::SetResetBehavior sr_behavior_out;
            GateTypeSequential::SetResetBehavior sr_behavior_out_inverted;
            SignalValue output;
            SignalValue inv_output;
        };

        std::unordered_map<Net*, std::vector<std::pair<SimulationGate*, std::vector<std::string>>>> m_successors;
        std::vector<std::unique_ptr<SimulationGate>> m_sim_gates;

        bool simulate_gate(SimulationGate* gate, Event& event, std::map<std::pair<Net*, u64>, SignalValue>& new_events);
        void simulate_ff(SimulationGateFF* gate, std::map<std::pair<Net*, u64>, SignalValue>& new_events);
        void compute_input_nets();
        void compute_output_nets();
        void initialize();
        void prepare_clock_events(u64 nanoseconds);
        void process_events(u64 timeout);

        SignalValue process_set_reset_behavior(GateTypeSequential::SetResetBehavior behavior, SignalValue previous_output);
    };
}    // namespace hal
