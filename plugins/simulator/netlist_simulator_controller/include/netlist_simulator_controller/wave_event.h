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

#include "hal_core/netlist/net.h"
#include "hal_core/netlist/boolean_function.h"

namespace hal
{
    struct WaveEvent
    {
        /**
         * The net affected by the event.
         */
        const Net* affected_net;

        /**
         * The new value caused by the event.
         */
        BooleanFunction::Value new_value;

        /**
         * The time of the event.
         */
        u64 time;

        /**
         * The unique ID of the event.
         */
        u64 id;

        /**
         * Tests whether two events are equal.
         *
         * @param[in] other - Event to compare to.
         * @returns True when both events are equal, false otherwise.
         */
        bool operator==(const WaveEvent& other) const
        {
            return affected_net == other.affected_net && new_value == other.new_value && time == other.time;
        }

        /**
         * Tests whether one event happened before the other.
         *
         * @param[in] other - Event to compare to.
         * @returns True when this event happened before the other, false otherwise.
         */
        bool operator<(const WaveEvent& other) const
        {
            if (time != other.time)
            {
                return time < other.time;
            }
            return id < other.id;
        }
    };
}    // namespace hal
