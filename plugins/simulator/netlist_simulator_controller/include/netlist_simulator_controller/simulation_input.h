#pragma once

#include <vector>
#include <unordered_set>
#include <unordered_map>
#include "hal_core/defines.h"
#include "hal_core/netlist/boolean_function.h"

namespace hal {
    class Gate;
    class Net;

    class SimulationInputNetEvent : public std::unordered_map<const Net*,BooleanFunction::Value>
    {
        u64 mSimulationDuration;
    public:
        u64 get_simulation_duration() const { return mSimulationDuration; }
        void set_simulation_duration(u64 t) { mSimulationDuration = t; }
    };

    class SimulationInput {

    public:
        struct Clock
        {
            const Net* clock_net;
            u64 switch_time;
            bool start_at_zero;
            u64 period() const { return switch_time * 2; }
        };

    private:
        std::unordered_set<const Gate*> mSimulationSet;
        std::vector<Clock> m_clocks;

        std::vector<const Net*> m_input_nets;
        std::vector<const Net*> m_output_nets;

        std::vector<SimulationInputNetEvent> mSimulationInputNetEvents;

        void compute_input_nets();
        void compute_output_nets();

    public:
        SimulationInput() {;}

        /**
         * Checks wether a gate is part of the simulation set
         * @param[in] g the gate
         * @return true if part, false otherwise
         */
        bool contains_gate(const Gate* g) const;

        /**
         * Returns the gates from simulation set
         * @return reference to gate set
         */
        const std::unordered_set<const Gate*>& get_gates() const;

        /**
         * Returns the clock(s) set up for simulation
         * @return reference to clock(s)
         */
        const std::vector<Clock>& get_clocks() const { return m_clocks; }

        /**
         * Tests whether net has been selected as clock
         * @param[in] n pointer to net
         * @return true if net has been selected as clock, false otherwise
         */
        bool is_clock(const Net* n) const;

        /**
         * Adds a clock
         * @param[in] clk reference to clock instance
         */
        void add_clock(const Clock& clk);

        /**
         * Add gates to the simulation set that contains all gates that are considered during simulation.
         * This function can only be called before the simulation has been initialized.
         *
         * @param[in] gates - The gates to add.
         */
        void add_gates(const std::vector<Gate*>& gates);

        void clear();

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
         * Add single event to simulation input net event vector. This method will be used by controller to setup
         * all events to be simulated prior to call the engines setSimulationInput method.
         *
         * @param[in] ev the simulation input net event
         */
        void add_simulation_net_event(const SimulationInputNetEvent& ev) { mSimulationInputNetEvents.push_back(ev); }

        /**
         * Get all net events setup for simulation
         * @return reference to vector of simulation input net events
         */
        const std::vector<SimulationInputNetEvent>& get_simulation_net_events() const { return mSimulationInputNetEvents; }
    };
}
