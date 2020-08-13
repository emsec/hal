#pragma once

#include "netlist/gate.h"
#include "netlist/net.h"

#include "plugin_netlist_simulator/simulation.h"

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
        * Specify clock speed in nanosecond period.
        * A period of 10ns means 5ns high, 5ns low.
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
        void simulate(u64 nanoseconds);

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
        u64 m_timeout_iterations = 100000;
        u64 m_id_counter = 0;

        std::unordered_map<Net*, std::vector<Gate*>> m_successors;

        void compute_input_nets();
        void compute_output_nets();
        void initialize();
        void prepare_clock_events(u64 nanoseconds);
        void process_events(u64 timeout);
    };
}    // namespace hal
