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
#include "hal_core/netlist/data_container.h"
#include "hal_core/netlist/event_system/event_handler.h"

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
    class Grouping;
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
         * Check whether two nets are equal.
         *
         * @param[in] other - The net to compare against.
         * @returns True if both nets are equal, false otherwise.
         */
        bool operator==(const Net& other) const;

        /**
         * Check whether two nets are unequal.
         *
         * @param[in] other - The net to compare against.
         * @returns True if both nets are unequal, false otherwise.
         */
        bool operator!=(const Net& other) const;

        /**
         * Hash function for python binding
         *
         * @return Pybind11 compatible hash
         */
        ssize_t get_hash() const;

        /**
         * Get the unique id of the net.
         *
         * @returns The unique id.
         */
        u32 get_id() const;

        /**
         * Get the netlist this net is associated with.
         *
         * @returns The netlist.
         */
        Netlist* get_netlist() const;

        /**
         * Get the name of the net.
         *
         * @returns The name.
         */
        const std::string& get_name() const;

        /**
         * Set the name of the net.
         *
         * @param[in] name - The new name.
         */
        void set_name(const std::string& name);

        /**
         * Get the grouping in which this net is contained. <br>
         * If no grouping contains this gate, a nullptr is returned.
         *
         * @returns The grouping.
         */
        Grouping* get_grouping() const;

        /*
         *      src specific functions
         */

        /**
         * TODO pybind, test
         * Add a source endpoint to the net.
         *
         * @param[in] gate - The source gate.
         * @param[in] pin - The output pin of the source gate.
         * @returns The endpoint on success, an error otherwise.
         */
        Result<Endpoint*> add_source(Gate* gate, GatePin* pin);

        /**
         * TODO pybind, test
         * Remove a source endpoint from the net.
         *
         * @param[in] gate - The source gate.
         * @param[in] pin - The output pin of the source gate.
         * @returns Ok on success, an error otherwise.
         */
        Result<std::monostate> remove_source(Gate* gate, const GatePin* pin);

        /**
         * TODO pybind, test
         * Remove a source endpoint from the net. <br>
         *
         * @param[in] ep - The source endpoint.
         * @returns Ok on success, an error otherwise.
         */
        Result<std::monostate> remove_source(Endpoint* ep);

        /**
         * TODO pybind, test
         * Check whether a tuple of a gate and an output pin is a source endpoint of the net.
         *
         * @param[in] gate - The source gate.
         * @param[in] pin - The source output pin.
         * @returns `true` if it is a source of the net, `false` otherwise.
         */
        bool is_a_source(Gate* gate, const GatePin* pin) const;

        /**
         * TODO pybind 
         * Check whether an endpoint is a source of the net.
         *
         * @param[in] ep - The endpoint.
         * @returns `true` if is is a source of the net, `false` otherwise.
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
         * Get a list of sources of the net.<br>
         * A filter can be supplied which filters out all potential values that return false.
         *
         * @param[in] filter - A filter for endpoints. Leave empty for no filtering.
         * @returns A vector of source endpoints.
         */
        std::vector<Endpoint*> get_sources(const std::function<bool(Endpoint* ep)>& filter = nullptr) const;

        /*
         *      dst specific functions
         */

        /**
         * TODO pybind, test
         * Add a destination endpoint to the net.
         *
         * @param[in] gate - The destination gate.
         * @param[in] pin - The input pin of the destination gate.
         * @returns The endpoint on success, an error otherwise.
         */
        Result<Endpoint*> add_destination(Gate* gate, GatePin* pin);

        /**
         * TODO pybind, test
         * Remove a destination endpoint from the net.
         *
         * @param[in] gate - The destination gate.
         * @param[in] pin - The input pin of the destination gate.
         * @returns Ok on success, an error otherwise.
         */
        Result<std::monostate> remove_destination(Gate* gate, const GatePin* pin);

        /**
         * TODO pybind, test
         * Remove a destination endpoint from the net. <br>
         *
         * @param[in] ep - The destination endpoint.
         * @returns Ok on success, an error otherwise.
         */
        Result<std::monostate> remove_destination(Endpoint* ep);

        /**
         * TODO pybind, test
         * Check whether a tuple of a gate and an input pin is a destination endpoint of the net.
         *
         * @param[in] gate - The destination gate.
         * @param[in] pin - The destination input pin.
         * @returns `true` if it is a destination of the net, `false` otherwise.
         */
        bool is_a_destination(Gate* gate, const GatePin* pin) const;

        /**
         * TODO pybind 
         * Check whether an endpoint is a destination of the net.
         *
         * @param[in] ep - The endpoint.
         * @returns `true` if is is a destination of the net, `false` otherwise.
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
         * Get a list of destinations of the net. <br>
         * A filter can be supplied which filters out all potential values that return false.
         *
         * @param[in] filter - A filter for endpoints. Leave empty for no filtering.
         * @returns A vector of destination-endpoints.
         */
        std::vector<Endpoint*> get_destinations(const std::function<bool(Endpoint* ep)>& filter = nullptr) const;

        /**
         * Check whether the net is unrouted, i.e., it has no source or no destination.
         *
         * @returns True if the net is unrouted, false otherwise.
         */
        bool is_unrouted() const;

        /**
         * Check whether the net is connected to GND.
         *
         * @returns True if the net is connected to GND, false otherwise.
         */
        bool is_gnd_net() const;

        /**
         * Check whether the net is connected to VCC.
         *
         * @returns True if the net is connected to VCC, false otherwise.
         */
        bool is_vcc_net() const;

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
        bool is_global_input_net() const;

        /**
         * Checks whether this net is a global output net.
         *
         * @returns True if the net is a global output net.
         */
        bool is_global_output_net() const;

    private:
        friend class NetlistInternalManager;
        explicit Net(NetlistInternalManager* internal_manager, EventHandler* event_handler, const u32 id, const std::string& name = "");

        Net(const Net&) = delete;
        Net(Net&&)      = delete;
        Net& operator=(const Net&) = delete;
        Net& operator=(Net&&) = delete;

        NetlistInternalManager* m_internal_manager;

        /* stores the id of the net */
        u32 m_id;

        /* stores the name of the net */
        std::string m_name;

        /* grouping */
        Grouping* m_grouping = nullptr;

        /* stores the dst gate and pin id of the dst gate */
        std::vector<std::unique_ptr<Endpoint>> m_destinations;
        std::vector<std::unique_ptr<Endpoint>> m_sources;
        std::vector<Endpoint*> m_destinations_raw;
        std::vector<Endpoint*> m_sources_raw;

        EventHandler* m_event_handler;
    };
}    // namespace hal
