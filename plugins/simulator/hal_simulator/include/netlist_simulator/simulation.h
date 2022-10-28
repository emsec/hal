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

#include "netlist_simulator_controller/wave_event.h"

#include <unordered_map>
#include <vector>

namespace hal
{
    class Net;

    class Simulation
    {
    public:
        /**
         * Get the signal value of a specific net at a specific point in time specified in picoseconds.
         *
         * @param[in] net - The net to inspect.
         * @param[in] time - The time in   .
         * @returns The net's signal value.
         */
        BooleanFunction::Value get_net_value(const Net* net, u64 time) const;

        /**
         * Adds a custom event to the simulation.
         *
         * @param[in] event - The event to add.
         */
        void add_event(const WaveEvent& event);

        /**
         * Get all events of the simulation.
         *
         * @returns A map from net to associated events for that net sorted by time.
         */
        std::unordered_map<const Net *, std::vector<WaveEvent> > get_events() const;

        /**
         * Get events of simulation for net identified by netId
         * @param[in] netId - ID of net for simulation
         * @param{out] found - true simulation events for net found, false otherwise
         * @return A vector of simulation events
         */
        std::vector<WaveEvent> get_events_by_net_id(u32 netId, bool* found=nullptr) const;

    private:
        friend class NetlistSimulator;
        std::unordered_map<const Net*, std::vector<WaveEvent>> m_events;
    };

}    // namespace hal
