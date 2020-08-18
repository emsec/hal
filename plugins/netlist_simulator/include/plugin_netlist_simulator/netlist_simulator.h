#pragma once

#include "netlist/gate.h"
#include "netlist/gate_library/gate_type/gate_type_sequential.h"
#include "netlist/net.h"
#include "plugin_netlist_simulator/simulation.h"

#include <map>
#include <unordered_set>

namespace hal
{
    class NetlistSimulator
    {
    public:
        /*
        * Add gates to the simulation set.
        * Only elements in the simulation set are considered during simulation.
        */
        void add_gates(const std::vector<Gate*>& gates);

        /*
        * Add a clock signal.
        * Specify clock speed in hertz.
        */
        void add_clock_hertz(Net* clock_net, u64 hertz, bool start_at_zero = true);

        /*
        * Add a clock signal.
        * Specify clock speed in picoseconds period.
        * A period of 10 means 5 high, 5 low.
        */
        void add_clock_period(Net* clock_net, u64 period, bool start_at_zero = true);

        /*
        * Get all gates that are in the simulation set.
        */
        std::vector<Gate*> get_gates() const;

        /*
        * Get all nets that are considered inputs, i.e., not driven by a gate in the simulation set or global inputs
        */
        std::vector<Net*> get_input_nets() const;

        /*
        * Get all output nets of gates in the simulation set that have a destination outside of the set or that are global outputs.
        */
        std::vector<Net*> get_output_nets() const;

        /*
        * Set the signal for a specific wire.
        * Can be used to control input signals between simulation cycles.
        */
        void set_input(Net* net, SignalValue value);

        /*
        * Load the initial values for all sequential elements into the current state.
        * For example, a Flip Flop may be initialized with HIGH output in FPGAs.
        */
        void load_initial_values();

        /*
        * Simulate for a specific period, advancing the internal state.
        * Use 'set_input' to control specific signals.
        */
        void simulate(u64 picoseconds);

        /*
        * Reset the simulator state, i.e., treat all signals as unknown.
        * Does not remove gates/nets from the simulation set.
        */
        void reset();

        /*
        * Set the simulator state, i.e., net signals, to a given state.
        * Does not influence gates/nets added to the simulation set.
        */
        void set_state(const Simulation& state);

        /*
        * Get the current net signal values.
        */
        Simulation get_current_state() const;

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
            bool is_flip_flop;
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

        std::unordered_map<Net*, std::vector<SimulationGate*>> m_successors;
        std::vector<std::unique_ptr<SimulationGate>> m_sim_gates;

        bool simulate(SimulationGate* gate, Event& event, std::map<std::pair<Net*, u64>, SignalValue>& new_events);
        void simulate_ff(SimulationGateFF* gate, std::map<std::pair<Net*, u64>, SignalValue>& new_events);
        void compute_input_nets();
        void compute_output_nets();
        void initialize();
        void prepare_clock_events(u64 nanoseconds);
        void process_events(u64 timeout);

        std::map<std::string, BooleanFunction::value> gather_input_values(SimulationGate* gate);
        SignalValue process_set_reset_behavior(GateTypeSequential::SetResetBehavior behavior, SignalValue previous_output);
    };
}    // namespace hal
