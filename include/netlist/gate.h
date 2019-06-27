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

#include "pragma_once.h"
#ifndef __HAL_GATE_H__
#define __HAL_GATE_H__

#include "def.h"

#include "netlist/data_container.h"
#include "netlist/endpoint.h"
#include "netlist/netlist_constants.h"

#include <map>
#include <memory>
#include <set>
#include <vector>

/* forward declaration */
class netlist;
class net;
class module;
struct endpoint;

/**
 *  Graph gate data structure for hardware netlists
 *
 * @ingroup netlist
 */
class NETLIST_API gate : public data_container, public std::enable_shared_from_this<gate>
{
    friend class netlist_internal_manager;

public:
    /**
     * Gets the unique id of the gate.
     *
     * @returns The gate's id.
     */
    u32 get_id() const;

    /**
     * Gets the parent netlist of the gate.
     *
     * @returns The netlist.
     */
    std::shared_ptr<netlist> get_netlist() const;

    /**
     * Gets the gate's name.
     *
     * @returns The name.
     */
    std::string get_name() const;

    /**
     * Sets the gate's name.
     *
     * @param[in] name - The new name.
     */
    void set_name(const std::string& name);

    /**
     * Gets the type of the gate.
     *
     * @returns The gate's type.
     */
    std::string get_type() const;

    /**
     * Gets the module this gate is contained in.
     *
     * @returns The owning module.
     */
    std::shared_ptr<module> get_module() const;

    /**
     * Mark this gate as a global vcc gate.
     *
     * @returns True on success.
     */
    bool mark_global_vcc_gate();

    /**
     * Mark this gate as a global gnd gate.
     *
     * @returns True on success.
     */
    bool mark_global_gnd_gate();

    /**
     * Unmark this gate as a global vcc gate.
     *
     * @returns True on success.
     */
    bool unmark_global_vcc_gate();

    /**
     * Unmark this gate as a global gnd gate.
     *
     * @returns True on success.
     */
    bool unmark_global_gnd_gate();

    /**
     * Checks whether this gate is a global vcc gate.
     *
     * @returns True if the gate is a global vcc gate.
     */
    bool is_global_vcc_gate() const;

    /**
     * Checks whether this gate is a global gnd gate.
     *
     * @returns True if the gate is a global gnd gate.
     */
    bool is_global_gnd_gate() const;

    /*
     *      pin specific functions
     */

    /**
     * Get all input pin types of the gate.
     *
     * @returns A vector of input pin types.
     */
    std::vector<std::string> get_input_pin_types() const;

    /**
     * Get all output pin types of the gate.
     *
     * @returns A vector of output pin types.
     */
    std::vector<std::string> get_output_pin_types() const;

    /**
     * Get all inout pin types of the gate.
     *
     * @returns A vector of inout pin types.
     */
    std::vector<std::string> get_inout_pin_types() const;

    /**
     * Get all fan-in nets, i.e. all nets that are connected to one of the input pins.
     *
     * @returns A set of all connected input nets.
     */
    std::set<std::shared_ptr<net>> get_fan_in_nets() const;

    /**
     * Get the fan-in net which is connected to a specific input pin.
     *
     * @param[in] pin_type - The input pin type.
     * @returns The connected input net.
     */
    std::shared_ptr<net> get_fan_in_net(const std::string& pin_type) const;

    /**
     * Get all fan-out nets, i.e. all nets that are connected to one of the output pins.
     *
     * @returns A set of all connected output nets.
     */
    std::set<std::shared_ptr<net>> get_fan_out_nets() const;

    /**
     * Get the fan-out net which is connected to a specific output pin.
     *
     * @param[in] pin_type - The output pin type.
     * @returns The connected output net.
     */
    std::shared_ptr<net> get_fan_out_net(const std::string& pin_type) const;

    /**
     * Get all unique predecessors of a gate filterable by the gate's input pin and a specific gate type.
     *
     * @param[in] this_pin_type_filter - The filter for the input pin type of the this gate. DONT_CARE for no filtering.
     * @param[in] pred_pin_type_filter - The filter for the output pin type of the predecessor gate. DONT_CARE for no filtering.
     * @param[in] gate_type_filter - The filter for target gate types. DONT_CARE for no filtering.
     * @returns A set of unique predecessor endpoints.
     */
    std::set<endpoint>
        get_unique_predecessors(const std::string& this_pin_type_filter = DONT_CARE, const std::string& pred_pin_type_filter = DONT_CARE, const std::string& gate_type_filter = DONT_CARE) const;

    /**
     * Get all direct predecessors of a gate filterable by the gate's input pin and a specific gate type.
     *
     * @param[in] this_pin_type_filter - The filter for the input pin type of the this gate. DONT_CARE for no filtering.
     * @param[in] pred_pin_type_filter - The filter for the output pin type of the predecessor gate. DONT_CARE for no filtering.
     * @param[in] gate_type_filter - The filter for target gate types. DONT_CARE for no filtering.
     * @returns A vector of predecessor endpoints.
     */
    std::vector<endpoint>
        get_predecessors(const std::string& this_pin_type_filter = DONT_CARE, const std::string& pred_pin_type_filter = DONT_CARE, const std::string& gate_type_filter = DONT_CARE) const;

    /**
     * Get the direct predecessor of a gate connected to a specific input pin and filterable by a specific gate type.
     *
     * @param[in] this_pin_type_filter - The input pin type of the this gate. DONT_CARE for no filtering.
     * @param[in] pred_pin_type_filter - The filter for the output pin type of the predecessor gate. DONT_CARE for no filtering.
     * @param[in] gate_type_filter - The filter for target gate types. DONT_CARE for no filtering.
     * @returns The predecessor endpoint.
     */
    endpoint get_predecessor(const std::string& this_pin_type_filter, const std::string& pred_pin_type_filter = DONT_CARE, const std::string& gate_type_filter = DONT_CARE) const;

    /**
     * Get all direct unique successors of a gate filterable by the gate's output pin and a specific gate type.
     *
     * @param[in] this_pin_type_filter - The output pin type of the this gate. DONT_CARE for no filtering.
     * @param[in] suc_pin_type_filter - The filter for the input pin type of the successor gate. DONT_CARE for no filtering.
     * @param[in] gate_type_filter - The filter for target gate types. DONT_CARE for no filtering.
     * @returns A set of unique successor endpoints.
     */
    std::set<endpoint>
        get_unique_successors(const std::string& this_pin_type_filter = DONT_CARE, const std::string& suc_pin_type_filter = DONT_CARE, const std::string& gate_type_filter = DONT_CARE) const;

    /**
     * Get all direct successors of a gate filterable by the gate's output pin and a specific gate type.
     *
     * @param[in] this_pin_type_filter - The output pin type of the this gate. DONT_CARE for no filtering.
     * @param[in] suc_pin_type_filter - The filter for the input pin type of the successor gate. DONT_CARE for no filtering.
     * @param[in] gate_type_filter - The filter for target gate types. DONT_CARE for no filtering.
     * @returns A vector of successor endpoints.
     */
    std::vector<endpoint>
        get_successors(const std::string& this_pin_type_filter = DONT_CARE, const std::string& suc_pin_type_filter = DONT_CARE, const std::string& gate_type_filter = DONT_CARE) const;

private:
    /**
     * Constructs a new gate and initializes it with the parameter fields.<br>
     *
     * @param[in] g - The parent netlist.
     * @param[in] id - A unique id.
     * @param[in] gate_type - The gate type.
     * @param[in] name - A name for the gate.
     */
    gate(std::shared_ptr<netlist> const g, const u32 id, const std::string& gate_type, const std::string& name = "");

    gate(const gate&) = delete;               //disable copy-constructor
    gate& operator=(const gate&) = delete;    //disable copy-assignment

    /* pointer to corresponding netlist parent */
    std::shared_ptr<netlist> m_netlist;

    /* id of the gate */
    u32 m_id;

    /* name of the gate */
    std::string m_name;

    /* type of the gate */
    std::string m_type;

    /* owning module */
    std::shared_ptr<module> m_module;

    /* connected nets */
    std::map<std::string, std::shared_ptr<net>> m_in_nets;
    std::map<std::string, std::shared_ptr<net>> m_out_nets;
};

#endif /* __HAL_GATE_H__ */
