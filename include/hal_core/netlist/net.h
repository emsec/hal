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

#include "hal_core/defines.h"
#include "hal_core/netlist/data_container.h"

#include <functional>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <tuple>
#include <vector>

namespace hal
{
    /* forward declaration */
    class Netlist;
    class Gate;
    class NetlistInternalManager;
    class Endpoint;

    /**
     * Net class containing information about a net including its source and destination.
     *
     * @ingroup netlist
     */
    class NETLIST_API Net : public DataContainer
    {
    public:
        /**
         * Gets the unique id of the net.
         *
         * @returns The net's id.
         */
        u32 get_id() const;

        /**
         * Get the parent netlist of the net.
         *
         * @returns The netlist.
         */
        Netlist* get_netlist() const;

        /**
         * Get the name of the net.
         *
         * @returns The name.
         */
        std::string get_name() const;

        /**
         * Set the name of the net.
         *
         * @param[in] name - The new name.
         */
        void set_name(const std::string& name);

        /*
         *      src specific functions
         */

        /**
         * Add a source to this net.
         *
         * @param[in] gate - The source gate.
         * @param[in] pin - The input pin of the gate.
         * @returns The new endpoint or nullptr on error.
         */
        Endpoint* add_source(Gate* gate, const std::string& pin);

        /**
         * Remove a source from this net.
         *
         * @param[in] gate - The source gate.
         * @param[in] pin - The input pin of the gate.
         * @returns True on success.
         */
        bool remove_source(Gate* gate, const std::string& pin);

        /**
         * Remove a source endpoint from this net.
         * If the endpoint is a destination-endpoint this function aborts.
         *
         * @param[in] ep - The source endpoint.
         * @returns True on success.
         */
        bool remove_source(Endpoint* ep);

        /**
         * Check whether a gate is a source of this net.
         *
         * @param[in] gate - The source gate.
         * @param[in] pin - The source output pin.
         * @returns True if the gate's pin is a source of this net.
         */
        bool is_a_source(Gate* gate, const std::string& pin) const;

        /**
         * Check whether an endpoint is a source of this net.
         * If the endpoint is a destination-endpoint this function aborts.
         *
         * @param[in] ep - The endpoint.
         * @returns True if the endpoint is a source of this net.
         */
        bool is_a_source(Endpoint* ep) const;

        /**
         * Get the number of sources.<br>
         * Faster than get_sources().size().
         *
         * @returns The number of sources of this net.
         */
        u32 get_num_of_sources() const;

        /**
         * Get the vector of sources of the net.
         *
         * @param[in] filter - a filter for endpoints.
         * @returns A vector of source endpoints.
         */
        std::vector<Endpoint*> get_sources(const std::function<bool(Endpoint* ep)>& filter = nullptr) const;

        /**
         * DEPRECATED
         * Get the (first) src of the net.
         * If there was no src assigned, the gate element of the returned endpoint is a nullptr.
         * If the net is multi-driven a warning is printed.
         *
         * @returns The (first) source endpoint.
         */
        [[deprecated("Use get_sources() instead.")]]
        Endpoint* get_source() const;

        /*
         *      dst specific functions
         */

        /**
         * Add a destination to this net.
         *
         * @param[in] gate - The destination gate.
         * @param[in] pin - The input pin of the gate.
         * @returns The new endpoint or nullptr on error.
         */
        Endpoint* add_destination(Gate* gate, const std::string& pin);

        /**
         * Remove a destination from this net.
         *
         * @param[in] gate - The destination gate.
         * @param[in] pin - The input pin of the gate.
         * @returns True on success.
         */
        bool remove_destination(Gate* gate, const std::string& pin);

        /**
         * Remove a destination endpoint from this net.
         * If the endpoint is a source-endpoint this function aborts.
         *
         * @param[in] ep - The destination endpoint.
         * @returns True on success.
         */
        bool remove_destination(Endpoint* ep);

        /**
         * Check whether a gate is a destination of this net.
         *
         * @param[in] gate - The destination gate.
         * @param[in] pin - The destination output pin.
         * @returns True if the gate's pin is a destination of this net.
         */
        bool is_a_destination(Gate* gate, const std::string& pin) const;

        /**
         * Check whether an endpoint is a destination of this net.
         * If the endpoint is a source-endpoint this function aborts.
         *
         * @param[in] ep - The endpoint.
         * @returns True if the endpoint is a destination of this net.
         */
        bool is_a_destination(Endpoint* ep) const;

        /**
         * Get the number of destinations.<br>
         * Faster than get_destinations().size().
         *
         * @returns The number of destinations of this net.
         */
        u32 get_num_of_destinations() const;

        /**
         * Get the vector of destinations of the net.
         *
         * @param[in] filter - a filter for endpoints.
         * @returns A vector of destination endpoints.
         */
        std::vector<Endpoint*> get_destinations(const std::function<bool(Endpoint* ep)>& filter = nullptr) const;

        /**
         * Check whether the net is routed, i.e. it has no source or the no destinations.
         *
         * @returns True if the net is unrouted.
         */
        bool is_unrouted() const;

        /**
         * Mark this net as a global input net.
         *
         * @returns True on success.
         */
        bool mark_global_input_net();

        /**
         * Mark this net as a global output net.
         *
         * @returns True on success.
         */
        bool mark_global_output_net();

        /**
         * Unmark this net as a global input net.
         *
         * @returns True on success.
         */
        bool unmark_global_input_net();

        /**
         * Unmark this net as a global output net.
         *
         * @returns True on success.
         */
        bool unmark_global_output_net();

        /**
         * Checks whether this net is a global input net.
         *
         * @returns True if the net is a global input net.
         */
        bool is_global_input_net();

        /**
         * Checks whether this net is a global output net.
         *
         * @returns True if the net is a global output net.
         */
        bool is_global_output_net();

    private:
        friend class NetlistInternalManager;
        explicit Net(NetlistInternalManager* internal_manager, const u32 id, const std::string& name = "");

        Net(const Net&) = delete;
        Net(Net&&)      = delete;
        Net& operator=(const Net&) = delete;
        Net& operator=(Net&&) = delete;

        NetlistInternalManager* m_internal_manager;

        /* stores the id of the net */
        u32 m_id;

        /* stores the name of the net */
        std::string m_name;

        /* stores the dst gate and pin id of the dst gate */
        std::vector<std::unique_ptr<Endpoint>> m_destinations;
        std::vector<std::unique_ptr<Endpoint>> m_sources;
        std::vector<Endpoint*> m_destinations_raw;
        std::vector<Endpoint*> m_sources_raw;
    };
}    // namespace hal
