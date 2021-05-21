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

#include "hal_core/defines.h"

namespace hal
{
    /* forward declaration */
    class Gate;
    class Net;

    /**
     * An endpoint comprises the pin of a gate, the respective gate, and the connected net.
     *
     * @ingroup netlist
     */
    class Endpoint
    {
    public:
        /**
         * Check whether two endpoints are equal.
         *
         * @param[in] other - The endpoint to compare against.
         * @returns True if both endpoints are equal, false otherwise.
         */
        bool operator==(const Endpoint& other) const;

        /**
         * Check whether two endpoints are unequal.
         *
         * @param[in] other - The endpoint to compare against.
         * @returns True if both endpoints are unequal, false otherwise.
         */
        bool operator!=(const Endpoint& other) const;

        /**
         * Get the gate associated with the endpoint.
         *
         * @returns The gate.
         */
        Gate* get_gate() const;

        /**
         * Get the name of the pin associated with the endpoint.
         *
         * @returns The name of the pin.
         */
        std::string get_pin() const;

        /**
         * Get the net associated with the endpoint.
         *
         * @returns The net.
         */
        Net* get_net() const;

        /**
         * Checks whether the pin of the endpoint is a source (output) pin.
         *
         * @returns True if the endpoint is an source (output) pin, false otherwise.
         */
        bool is_source_pin() const;

        /**
         * Checks whether the pin of the endpoint is a destination (input) pin.
         *
         * @returns True if the endpoint is an destination (input) pin, false otherwise.
         */
        bool is_destination_pin() const;

    private:
        friend class NetlistInternalManager;
        Endpoint(Gate* gate, const std::string& pin, Net* net, bool is_a_destination);

        Endpoint(const Endpoint&) = delete;
        Endpoint(Endpoint&&)      = delete;
        Endpoint& operator=(const Endpoint&) = delete;
        Endpoint& operator=(Endpoint&&) = delete;

        Gate* m_gate;
        std::string m_pin;
        Net* m_net;
        bool m_is_a_destination;
    };
}    // namespace hal
