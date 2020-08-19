//  MIT License
//
//  Copyright (c) 2019 Ruhr-University Bochum, Germany, Chair for Embedded Security. All Rights reserved.
//  Copyright (c) 2019 Marc Fyrbiak, Sebastian Wallat, Max Hoffmann ("ORIGINAL AUTHORS"). All rights reserved.
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

#include "def.h"

namespace hal
{
    /* forward declaration */
    class Gate;
    class Net;

    /**
     *  Endpoint data structure for (gate, pin) tuples
     *
     * @ingroup netlist
     */
    class Endpoint
    {
    public:
        /**
         * Construct an endpoint object.
         *
         * @param[in] gate - The gate of the endpoint.
         * @param[in] pin - The pin of the endpoint.
         * @param[in] net - The net of the endpoint.
         * @param[in] is_a_destination - True if the endpoint is an output pin, false if it is an input pin.
         */
        Endpoint(Gate* gate, const std::string& pin, Net* net, bool is_a_destination);
        Endpoint();
        Endpoint(const Endpoint&) = default;
        Endpoint(Endpoint&&)      = default;
        Endpoint& operator=(const Endpoint&) = default;
        Endpoint& operator=(Endpoint&&) = default;

        /**
         * Standard "less than". <br>
         * Required for searching through sets.
         *
         * @param[in] rhs - Compare target.
         * @returns True if this is less than rhs.
         */
        bool operator<(const Endpoint& rhs) const;

        /**
         * Standard "equals". <br>
         * Required for searching through sets.
         *
         * @param[in] rhs - Compare target.
         * @returns True if this is equal to rhs.
         */
        bool operator==(const Endpoint& rhs) const;

        /**
         * Standard "unequal".
         *
         * @param[in] rhs - Compare target.
         * @returns True if this is unequal to rhs.
         */
        bool operator!=(const Endpoint& rhs) const;

        /**
         * Returns the gate of the endpoint.
         *
         * @returns The gate.
         */
        Gate* get_gate() const;

        /**
         * Returns the pin of the endpoint.
         *
         * @returns The pin.
         */
        const std::string& get_pin() const;

        /**
         * Returns the net of the endpoint.
         *
         * @returns The net.
         */
        Net* get_net() const;

        /**
         * Checks whether the pin of the endpoint is a destination pin.
         *
         * @returns True, if the endpoint is an input pin.
         */
        bool is_destination_pin() const;

        /**
         * Checks whether the pin of the endpoint is a source pin.
         *
         * @returns True, if the endpoint is an output pin.
         */
        bool is_source_pin() const;

    private:
        Gate* m_gate;
        std::string m_pin;
        Net* m_net;
        bool m_is_a_destination;
    };
}    // namespace hal
