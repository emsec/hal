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
#include "netlist/data_container.h"
#include "netlist/endpoint.h"

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
    class netlist;
    class gate;
    class netlist_internal_manager;

    /**
     * Net class containing information about a net including its source and destination.
     *
     * @ingroup netlist
     */
    class NETLIST_API net : public data_container, public std::enable_shared_from_this<net>
    {
        friend class netlist_internal_manager;

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
        std::shared_ptr<netlist> get_netlist() const;

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
         * @returns True on success.
         */
        bool add_source(const std::shared_ptr<gate>& gate, const std::string& pin);

        /**
         * Add a source to this net by endpoint.
         * If the endpoint is a destination-endpoint this function aborts.
         *
         * @param[in] ep - The endpoint.
         * @returns True on success.
         */
        bool add_source(const endpoint& ep);

        /**
         * Remove a source from this net.
         *
         * @param[in] gate - The source gate.
         * @param[in] pin - The input pin of the gate.
         * @returns True on success.
         */
        bool remove_source(const std::shared_ptr<gate>& gate, const std::string& pin);

        /**
         * Remove a source from this net by endpoint.
         * If the endpoint is a destination-endpoint this function aborts.
         *
         * @param[in] ep - The endoint.
         * @returns True on success.
         */
        bool remove_source(const endpoint& ep);

        /**
         * Check whether a gate is a source of this net.
         *
         * @param[in] gate - The source gate.
         * @param[in] pin - The source output pin.
         * @returns True if the gate's pin is a source of this net.
         */
        bool is_a_source(const std::shared_ptr<gate>& gate, const std::string& pin) const;

        /**
         * Check whether an endpoint is a source of this net.
         * If the endpoint is a destination-endpoint this function aborts.
         *
         * @param[in] ep - The endpoint.
         * @returns True if the endpoint is a source of this net.
         */
        bool is_a_source(const endpoint& ep) const;

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
        std::vector<endpoint> get_sources(const std::function<bool(const endpoint& ep)>& filter = nullptr) const;

        /**
         * Get the (first) src of the net.
         * If there was no src assigned, the gate element of the returned endpoint is a nullptr.
         * If the net is multi-driven a warning is printed.
         *
         * @returns The (first) source endpoint.
         */
        endpoint get_source() const;

        /*
         *      dst specific functions
         */

        /**
         * Add a destination to this net.
         *
         * @param[in] gate - The destination gate.
         * @param[in] pin - The input pin of the gate.
         * @returns True on success.
         */
        bool add_destination(const std::shared_ptr<gate>& gate, const std::string& pin);

        /**
         * Add a destination to this net by endpoint.
         *
         * @param[in] ep - The endpoint.
         * @returns True on success.
         */
        bool add_destination(const endpoint& ep);

        /**
         * Remove a destination from this net.
         * If the endpoint is a source-endpoint this function aborts.
         *
         * @param[in] gate - The destination gate.
         * @param[in] pin - The input pin of the gate.
         * @returns True on success.
         */
        bool remove_destination(const std::shared_ptr<gate>& gate, const std::string& pin);

        /**
         * Remove a destination from this net by endpoint.
         * If the endpoint is a source-endpoint this function aborts.
         *
         * @param[in] ep - The endoint.
         * @returns True on success.
         */
        bool remove_destination(const endpoint& ep);

        /**
         * Check whether a gate is a destination of this net.
         *
         * @param[in] gate - The destination gate.
         * @param[in] pin - The destination output pin.
         * @returns True if the gate's pin is a destination of this net.
         */
        bool is_a_destination(const std::shared_ptr<gate>& gate, const std::string& pin) const;

        /**
         * Check whether an endpoint is a destination of this net.
         * If the endpoint is a source-endpoint this function aborts.
         *
         * @param[in] ep - The endpoint.
         * @returns True if the endpoint is a destination of this net.
         */
        bool is_a_destination(const endpoint& ep) const;

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
        std::vector<endpoint> get_destinations(const std::function<bool(const endpoint& ep)>& filter = nullptr) const;

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
        bool is_global_input_net() const;

        /**
         * Checks whether this net is a global output net.
         *
         * @returns True if the net is a global output net.
         */
        bool is_global_output_net() const;

    private:
        /**
         * Constructs a new net and initializes it with the parameter fields.<br>
         * The net is not automatically added to the netlist.
         *
         * @param[in] g - The parent netlist.
         * @param[in] id - A unique id.
         * @param[in] name - A name for the net.
         */
        explicit net(netlist_internal_manager* internal_manager, const u32 id, const std::string& name = "");

        net(const net&) = delete;               //disable copy-constructor
        net& operator=(const net&) = delete;    //disable copy-assignment

        netlist_internal_manager* m_internal_manager;

        /* stores the id of the net */
        u32 m_id;

        /* stores the name of the net */
        std::string m_name;

        /* stores the dst gate and pin id of the dst gate */
        std::vector<endpoint> m_destinations;
        std::vector<endpoint> m_sources;
    };
}    // namespace hal
