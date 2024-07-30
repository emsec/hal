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

#include <vector>
#include <unordered_set>
#include <unordered_map>
#include "hal_core/defines.h"
#include "hal_core/netlist/boolean_function.h"
#include "hal_core/netlist/pins/module_pin.h"
#include "hal_core/netlist/pins/gate_pin.h"
#include "hal_core/netlist/pins/pin_group.h"

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

        struct NetGroup
        {
            bool is_input;
            const Gate* gate;
            PinGroup<ModulePin>* module_pin_group;
            PinGroup<GatePin>* gate_pin_group;
            NetGroup() : is_input(true), gate(nullptr), module_pin_group(nullptr), gate_pin_group(nullptr) {;}
            std::vector<const Net*> get_nets() const;
            std::string get_name() const;
        };

    private:
        std::unordered_set<const Gate*> mSimulationSet;
        std::vector<Clock> m_clocks;
        std::vector<NetGroup> m_netgroups;

        std::unordered_set<const Net*> m_input_nets;
        std::vector<const Net*> m_output_nets;
        std::vector<const Net*> m_partial_nets;

        bool mNoClockUsed;

        void compute_input_nets();
        void compute_output_nets();
        void compute_partial_nets();

    public:
        SimulationInput() : mNoClockUsed(false) {;}

        /**
         * Checks whether essential data for simulation has been provided (gates, clock, input_nets)
         * @return true if essential data is present, false otherwise
         */
        bool is_ready() const;

        /**
         * Checks whether gates have been selected for simulation
         * @return true if at least one gate is in simulation set, false otherwise
         */
        bool has_gates() const;

        /**
         * Checks whether a gate is part of the simulation set
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
         * Prepare simulation where no net is defined as clock input
         */
        void set_no_clock_used();

        /**
         * Check wether no clock signal will be automatically generated as simulation input. Either there
         * is no clock present or the clock signal will be passed as normal waveform input.
         */
        bool is_no_clock_used() const;

        /**
         * Tests whether net is an input net
         * @param[in] n pointer to net
         * @return true if net is input net, false otherwise
         */
        bool is_input_net(const Net* n) const;

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

        /**
         * Clear all internal container (gate set, input nets ...)
         */
        void clear();

        /**
         * Get all nets that are considered inputs, i.e., not driven by a gate in the simulation set or global inputs.
         *
         * @returns The input nets.
         */
        const std::unordered_set<const Net*>& get_input_nets() const;

        /**
         * Get all output nets of gates in the simulation set that have a destination outside of the set or that are global outputs.
         *
         * @returns The output nets.
         */
        const std::vector<const Net*>& get_output_nets() const;

        /**
         * Get all nets from the partial netlist.
         *
         * @return The nets from partial netlist
         */
        const std::vector<const Net*>& get_partial_netlist_nets() const;

        /**
         * Dump content of simulation input instance to file or stderr if no filename given
         * @param filename name of file to be created
         */
        void dump(std::string filename = std::string()) const;

        /**
         * Group one-bit signals to multi-bit bus according to pingroups.
         * Pingroups are only evaluated if all connected nets are part of the simulation.
         * Module pingroups have priority before gate pingroups.
         */
        void compute_net_groups();

        /**
         * Get groups computed in method above.
         * @return Vector of groups.
         */
        const std::vector<NetGroup>& get_net_groups() const { return m_netgroups; }
    };
}
