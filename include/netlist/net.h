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
#include "netlist/netlist_constants.h"

#include <map>
#include <memory>
#include <set>
#include <string>
#include <tuple>
#include <vector>

/* forward declaration */
class netlist;
class gate;
class netlist_internal_manager;

/**
 * @ingroup netlist
 */
class NETLIST_API net : public data_container, public std::enable_shared_from_this<net>
{
    friend class netlist_internal_manager;

public:
    /**
     * Overloaded operator
     *
     * @param[in] os - Output stream
     * @param[in] n - Graph net reference
     * @returns A standard output stream.
     */
    friend std::ostream& operator<<(std::ostream& os, const net& n);

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

    /**
     *      src specific functions
     */

    /**
     * Set the source of this net to a gate's output pin.
     *
     * @param[in] gate - The source gate.
     * @param[in] pin_type - The pin of the source gate.
     * @returns True on success.
     **/
    bool set_src(std::shared_ptr<gate> const gate, const std::string& pin_type);

    /**
     * Set the source of this net to a gate's output pin.
     *
     * @param[in] src - The source endpoint.
     * @returns True on success.
     **/
    bool set_src(endpoint src);

    /**
     * Remove the source of the net.
     *
     * @returns True on success.
     */
    bool remove_src();

    /**
     * Get the src of the net specified by type.<br>
     * If the specifications don't match the actual source, the gate element of the returned endpoint is a nullptr.
     *
     * @param[in] gate_type - The desired source gate type.
     * @returns The source endpoint.
     */
    endpoint get_src(const std::string& gate_type = DONT_CARE) const;

    /**
     * Get the src of the net specified by type.<br>
     * If the specifications don't match the actual source, the gate element of the returned endpoint is a nullptr.
     *
     * @param[in] gate_type - The desired source gate type.
     * @returns The source endpoint.
     */
    endpoint get_src_by_type(const std::string& gate_type) const;

    /**
     *      dst specific functions
     */

    /**
     * Add a destination to this net.
     *
     * @param[in] gate - The destination gate.
     * @param[in] pin_type - The input pin of the gate.
     * @returns True on success.
     **/
    bool add_dst(std::shared_ptr<gate> const gate, const std::string& pin_type);

    /**
     * Add a destination to this net.
     *
     * @param[in] dst - The destination endpoint.
     * @returns True on success.
     **/
    bool add_dst(endpoint dst);

    /**
     * Remove a destination from this net.
     *
     * @param[in] gate - The destination gate.
     * @param[in] pin_type - The input pin of the gate.
     * @returns True on success.
     **/
    bool remove_dst(std::shared_ptr<gate> const gate, const std::string& pin_type);

    /**
     * Remove a destination from this net.
     *
     * @param[in] dst - The destination endpoint.
     * @returns True on success.
     **/
    bool remove_dst(endpoint dst);

    /**
     * Check whether a gate's input pin is a destination of this net.
     *
     * @param[in] gate - The destination gate.
     * @param[in] pin_type - The input pin of the gate. DONT_CARE if the pin does not matter.
     * @returns True if the input's pin is a destination.
     **/
    bool is_a_dst(std::shared_ptr<gate> const gate, const std::string& pin_type = DONT_CARE) const;

    /**
     * Check whether a gate's input pin is a destination of this net.
     *
     * @param[in] ep - The input endpoint.
     * @returns True if the input's pin is a destination.
     **/
    bool is_a_dst(endpoint ep) const;

    /**
     * Get the number of destinations.<br>
     * Faster than get_dsts().size().
     *
     * @returns The number of destinations of this net.
     */
    u32 get_num_of_dsts() const;

    /**
     * Get the vector of destinations of the net specified by type.
     *
     * @param[in] gate_type - The desired destination gate type.
     * @returns A vector of destination endpoints.
     */
    std::vector<endpoint> get_dsts(const std::string& gate_type = DONT_CARE) const;

    /**
     * Get the vector of destinations of the net specified by type.
     *
     * @param[in] gate_type - The desired destination gate type.
     * @returns A vector of tdestination endpoints.
     */
    std::vector<endpoint> get_dsts_by_type(const std::string& gate_type) const;

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

    /** stores the id of the net */
    u32 m_id;

    /** stores the name of the net */
    std::string m_name;

    /** stores the src gate and pin id of src gate*/
    endpoint m_src;

    /** stores the dst gate and pin id of the dst gate*/
    std::vector<endpoint> m_dsts;
};
