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

#include "netlist/gate_library/gate_library.h"
#include "netlist/netlist_constants.h"

#include <memory>
#include <string>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>

/** forward declaration */
class netlist_internal_manager;
class net;
class gate;
class module;
struct endpoint;

/**
 * Netlist class containing information about the netlist including its gates, modules, and nets, as well as the underlying gate library.
 *
 * @ingroup netlist
 */
class NETLIST_API netlist : public std::enable_shared_from_this<netlist>
{
    friend class netlist_internal_manager;

public:
    /**
     * Constructs a new netlist for a specific gate library.<br>
     * Use the netlist_factory to create instances!
     *
     * @param[in] library - Shared pointer to gate library.
     */
    explicit netlist(std::shared_ptr<gate_library> library);

    ~netlist();

    /**
     * Return shared_ptr for the current netlist object
     *
     * @returns The shared ptr to netlist
     */
    std::shared_ptr<netlist> get_shared();

    /**
     * Get the netlist id.<br>
     * If not changed via set_id(), the id is zero.
     *
     * @returns The netlist's id.
     */
    u32 get_id() const;

    /**
     * Sets the netlist id to a new value.
     *
     * @param[in] id - The new netlist id.
     */
    void set_id(const u32 id);

    /**
     * Get the file name of the input design.
     *
     * @returns The input file's name.
     */
    hal::path get_input_filename() const;

    /**
     * Set the file name of the input design.
     *
     * @param[in] input_filename - File name of the design.
     */
    void set_input_filename(const hal::path& input_filename);

    /**
     * Get the design name,
     *
     * @returns The design name.
     */
    std::string get_design_name() const;

    /**
     * Set the design name
     *
     * @param[in] design_name - New design name.
     */
    void set_design_name(const std::string& design_name);

    /**
     * Get the name of the hardware device if specified.
     *
     * @return The target device name.
     */
    std::string get_device_name() const;

    /**
     * Set the name of the target hardware device.
     *
     * @param[in] device_name - Name of hardware device
     */
    void set_device_name(const std::string& device_name);

    /**
     * Get the gate library associated with the netlist.
     *
     * @returns A pointer to the gate library.
     */
    std::shared_ptr<gate_library> get_gate_library() const;

    /*
     * ################################################################
     *      module functions
     * ################################################################
     */

    /**
     * Gets an unoccupied module id.<br>
     * The value 0 is reserved and represents an invalid id.
     *
     * @returns An unoccupied unique id.
     */
    u32 get_unique_module_id();

    /**
     * Creates and adds a new module to the netlist.<br>
     * It is identifiable via its unique ID.
     *
     * @param[in] id - The unique ID != 0 for the new module.
     * @param[in] name - A name for the module.
     * @param[in] parent - The parent module.
     * @returns The new module on success, nullptr on error.
     */
    std::shared_ptr<module> create_module(const u32 id, const std::string& name, std::shared_ptr<module> parent);

    /**
     * Creates and adds a new module to the netlist.<br>
     * It is identifiable via its unique ID which is automatically set to the next free ID.
     *
     * @param[in] name - A name for the module.
     * @param[in] parent - The parent module.
     * @returns The new module on success, nullptr on error.
     */
    std::shared_ptr<module> create_module(const std::string& name, std::shared_ptr<module> parent);

    /**
     * Removes and a module from the netlist.
     *
     * @param[in] module - module to remove.
     * @returns True on success.
     */
    bool delete_module(const std::shared_ptr<module> module);

    /**
     * Get the top module of the netlist.
     *
     * @returns The top module.
     */
    std::shared_ptr<module> get_top_module();

    /**
     * Get a module by its ID. <br>
     * It does not matter at which depth the module is located.
     *
     * @param[in] id - the id of the desired module
     * @returns The desired module.
     */
    std::shared_ptr<module> get_module_by_id(u32 id) const;

    /**
     * Get a set of all modules of the netlist including the top module.
     *
     * @returns The modules of the netlist.
     */
    std::set<std::shared_ptr<module>> get_modules() const;

    /**
     * Checks whether a module is registered in the netlist.
     *
     * @param[in] module - The module to check.
     * @returns True if the module is in netlist
     */
    bool is_module_in_netlist(const std::shared_ptr<module> module) const;

    /*
     * ################################################################
     *      gate functions
     * ################################################################
     */

    /**
     * Gets an unoccupied gate id. <br>
     * The value 0 is reserved and represents an invalid id.
     *
     * @returns An unoccupied unique id.
     */
    u32 get_unique_gate_id();

    /**
     * Creates and adds a new gate to the netlist.<br>
     * It is identifiable via its unique ID.
     *
     * @param[in] id - The unique ID != 0 for the new gate.
     * @param[in] gt - The gate type.
     * @param[in] name - A name for the gate.
     * @param[in] x - The x-coordinate of the gate.
     * @param[in] y - The y-coordinate of the gate.
     * @returns The new gate on success, nullptr on error.
     */
    std::shared_ptr<gate> create_gate(const u32 id, std::shared_ptr<const gate_type> gt, const std::string& name = "", float x = -1, float y = -1);

    /**
     * Creates and adds a new gate to the netlist.<br>
     * It is identifiable via its unique ID which is automatically set to the next free ID.
     *
     * @param[in] gt - The gate type.
     * @param[in] name - A name for the gate.
     * @param[in] x - The x-coordinate of the gate.
     * @param[in] y - The y-coordinate of the gate.
     * @returns The new gate on success, nullptr on error.
     */
    std::shared_ptr<gate> create_gate(std::shared_ptr<const gate_type> gt, const std::string& name = "", float x = -1, float y = -1);

    /**
     * Removes a gate from the netlist.
     *
     * @param[in] gate - Pointer to the gate pointer.
     * @returns True on success.
     */
    bool delete_gate(std::shared_ptr<gate> gate);

    /**
     * Checks whether a gate is registered in the netlist.
     *
     * @param[in] gate - The gate to check.
     * @returns True if the gate is in netlist
     */
    bool is_gate_in_netlist(const std::shared_ptr<gate> gate) const;

    /**
     * Get a gate specified by id.
     *
     * @param[in] gate_id - The gate's id.
     * @returns The gate or a nullptr.
     */
    std::shared_ptr<gate> get_gate_by_id(const u32 gate_id) const;

    /**
     * Get all gates of the netlist regardless of the module they are in. <br>
     * You can filter the set before output with the optional parameters.
     *
     * @param[in] gate_type_filter - Filter for the gate type
     * @param[in] name_filter - Filter for the name
     * @return A set of gates.
     */
    std::set<std::shared_ptr<gate>> get_gates(const std::string& gate_type_filter = DONT_CARE, const std::string& name_filter = DONT_CARE) const;

    /**
     * Mark a gate as a global vcc gate.
     *
     * @param[in] gate - The gate.
     * @returns True on success.
     */
    bool mark_vcc_gate(const std::shared_ptr<gate> gate);

    /**
     * Mark a gate as a global gnd gate.
     *
     * @param[in] gate - The negate.
     * @returns True on success.
     */
    bool mark_gnd_gate(const std::shared_ptr<gate> gate);

    /**
     * Unmark a global vcc gate.
     *
     * @param[in] gate - The gate.
     * @returns True on success.
     */
    bool unmark_vcc_gate(const std::shared_ptr<gate> gate);

    /**
     * Unmark a global gate.
     *
     * @param[in] gate - The new gate.
     * @returns True on success.
     */
    bool unmark_gnd_gate(const std::shared_ptr<gate> gate);

    /**
     * Checks whether a gate is a global vcc gate.
     *
     * @param[in] gate - The gate to check.
     * @returns True if the gate is a global vcc gate.
     */
    bool is_vcc_gate(const std::shared_ptr<gate> gate) const;

    /**
     * Checks whether a gate is a global gnd gate.
     *
     * @param[in] gate - The gate to check.
     * @returns True if the gate is a global gnd gate.
     */
    bool is_gnd_gate(const std::shared_ptr<gate> gate) const;

    /**
     * Get all global vcc gates.
     *
     * @returns A set of gates.
     */
    std::set<std::shared_ptr<gate>> get_vcc_gates() const;

    /**
     * Get all global gnd gates.
     *
     * @returns A set of gates.
     */
    std::set<std::shared_ptr<gate>> get_gnd_gates() const;

    /*
     * ################################################################
     *      net functions
     * ################################################################
     */

    /**
     * Gets an unoccupied net id. <br>
     * The value 0 is reserved and represents an invalid id.
     *
     * @returns An unoccupied unique id.
     */
    u32 get_unique_net_id();

    /**
     * Creates and adds a new net to the netlist.<br>
     * It is identifiable via its unique ID.
     *
     * @param[in] id - The unique ID != 0 for the new net.
     * @param[in] name - A name for the net.
     * @returns The new net on success, nullptr on error.
     */
    std::shared_ptr<net> create_net(const u32 id, const std::string& name = "");

    /**
     * Creates and adds a new net to the netlist.<br>
     * It is identifiable via its unique ID which is automatically set to the next free ID.
     *
     * @param[in] name - A name for the net.
     * @returns The new net on success, nullptr on error.
     */
    std::shared_ptr<net> create_net(const std::string& name = "");

    /**
     * Removes a net from the netlist.
     *
     * @param[in] n - Pointer to the net pointer.
     * @returns True on success.
     */
    bool delete_net(std::shared_ptr<net> n);

    /**
     * Checks whether a net is registered in the netlist.
     *
     * @param[in] n - The net to check.
     * @returns True if the net is in netlist
     */
    bool is_net_in_netlist(std::shared_ptr<net> const n) const;

    /**
     * Get a net specified by id.
     *
     * @param[in] id - The net's id.
     * @returns The net or a nullptr.
     */
    std::shared_ptr<net> get_net_by_id(u32 id) const;

    /**
     * Get all nets of the netlist regardless of the module they are in. <br>
     * You can filter the set before output with the optional parameter.
     *
     * @param[in] name_filter - Filter for the name
     * @return A set of nets.
     */
    std::unordered_set<std::shared_ptr<net>> get_nets(const std::string& name_filter = DONT_CARE) const;

    /**
     * Mark a net as a global input net.
     *
     * @param[in] net - The net.
     * @returns True on success.
     */
    bool mark_global_input_net(std::shared_ptr<net> const net);

    /**
     * Mark a net as a global output net.
     *
     * @param[in] net - The net.
     * @returns True on success.
     */
    bool mark_global_output_net(std::shared_ptr<net> const net);

    /**
     * Unmark a global input net.
     *
     * @param[in] net - The net.
     * @returns True on success.
     */
    bool unmark_global_input_net(std::shared_ptr<net> const net);

    /**
     * Unmark a global output net.
     *
     * @param[in] net - The net.
     * @returns True on success.
     */
    bool unmark_global_output_net(std::shared_ptr<net> const net);

    /**
     * Checks whether a net is a global input net.
     *
     * @param[in] net - The net to check.
     * @returns True if the net is a global input net.
     */
    bool is_global_input_net(std::shared_ptr<net> const net) const;

    /**
     * Checks whether a net is a global output net.
     *
     * @param[in] net - The net to check.
     * @returns True if the net is a global output net.
     */
    bool is_global_output_net(std::shared_ptr<net> const net) const;

    /**
     * Get all global input nets.
     *
     * @returns A set of nets.
     */
    std::set<std::shared_ptr<net>> get_global_input_nets() const;

    /**
     * Get all global output nets.
     *
     * @returns A set of nets.
     */
    std::set<std::shared_ptr<net>> get_global_output_nets() const;

private:
    /** stores the pointer to the netlist internal manager */
    netlist_internal_manager* m_manager;

    /** stores the gate library */
    std::shared_ptr<gate_library> m_gate_library;

    /** stores the netlist id */
    u32 m_netlist_id;

    /* stores the name of the input file */
    hal::path m_file_name;

    /* stores the name of the design */
    std::string m_design_name;

    /* stores the name of the device */
    std::string m_device_name;

    /** stores the auto generated ids for fast next id */
    u32 m_next_gate_id;
    std::set<u32> m_used_gate_ids;
    std::set<u32> m_free_gate_ids;
    u32 m_next_net_id;
    std::set<u32> m_used_net_ids;
    std::set<u32> m_free_net_ids;
    u32 m_next_module_id;
    std::set<u32> m_used_module_ids;
    std::set<u32> m_free_module_ids;

    /** stores the modules */
    std::shared_ptr<module> m_top_module;
    std::unordered_map<u32, std::shared_ptr<module>> m_modules;

    /** stores the nets */
    std::unordered_map<u32, std::shared_ptr<net>> m_nets_map;
    std::unordered_set<std::shared_ptr<net>> m_nets_set;

    /** stores the set of global gates and nets */
    std::set<std::shared_ptr<net>> m_global_input_nets;

    std::set<std::shared_ptr<net>> m_global_output_nets;

    std::set<std::shared_ptr<gate>> m_gnd_gates;

    std::set<std::shared_ptr<gate>> m_vcc_gates;
};
