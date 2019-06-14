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
#ifndef __HAL_MODULE_H__
#define __HAL_MODULE_H__

#include "def.h"

#include "netlist/gate_library/gate_library.h"
#include "netlist/netlist_constants.h"

#include "netlist/data_container.h"

#include <map>
#include <memory>
#include <set>
#include <string>
#include <tuple>
#include <type_traits>

/** forward declaration */
class netlist;
class netlist_internal_manager;
class net;
class gate;

/**
 * /class module
 *
 * @ingroup module
 */
class NETLIST_API module : public data_container, public std::enable_shared_from_this<module>
{
    friend class netlist_internal_manager;
    friend class netlist;
public:
    /**
     * Get the module's id.
     *
     * @returns The module's id.
     */
    u32 get_id() const;

    /**
     * Get the module's name.
     *
     * @returns The module's name.
     */
    std::string get_name() const;

    /**
     * Set the module's name.
     *
     * @params[in] name - The new name.
     */
    void set_name(const std::string& name);

    /**
     * Get the parent of this module.<br>
     * This returns nullptr for the top module.
     *
     * @returns The parent module.
     */
    std::shared_ptr<module> get_parent_module() const;

    /**
     * Sets a new parent for this module.<br>
     * If the new parent is a submodule of this module, the new parent is added as a direct submodule to the old parent first.
     *
     * @param[in] new_parent - the new parent module
     * @returns True if the parent was changed
     */
    bool set_parent_module(const std::shared_ptr<module>& new_parent);

    /**
     * Get all direct submodules of this module.<br>
     * If \p recursive is true, all indirect submodules are also included.
     *
     * @param[in] name_filter - Filter for the name
     * @param[in] recursive - Look into submodules as well
     * @returns The set of submodules
     */
    std::set<std::shared_ptr<module>> get_submodules(const std::string& name_filter = DONT_CARE, bool recursive = false) const;

    /**
     * Get the netlist this module is associated with.
     *
     * @returns The netlist.
     */
    std::shared_ptr<netlist> get_netlist() const;

    /**
     * Get the input nets to this module.<br>
     * A module input net is either a global input to the netlist or has a source outside of the module.
     *
     * @param[in] name_filter - Filter for the name
     * @returns The set of module input nets.
     */
    std::set<std::shared_ptr<net>> get_input_nets(const std::string& name_filter = DONT_CARE) const;

    /**
     * Get the output nets of this module.<br>
     * A module output net is either a global output of the netlist or has a destination outside of the module.
     *
     * @param[in] name_filter - Filter for the name
     * @returns The set of module output nets.
     */
    std::set<std::shared_ptr<net>> get_output_nets(const std::string& name_filter = DONT_CARE) const;

    /*
     * ################################################################
     *      gate functions
     * ################################################################
     */

    /**
     * Moves a gate into this module.<br>
     * The gate is removed from its previous module in the process.
     *
     * @param[in] gate - The gate to move.
     * @returns True on success.
     */
    bool assign_gate(std::shared_ptr<gate> gate);

    /**
     * Removes a gate from the module.<br>
     * It is automatically moved to the netlist's top module.
     *
     * @param[in] gate - Pointer to the gate pointer.
     * @returns True on success.
     */
    bool remove_gate(std::shared_ptr<gate> gate);

    /**
     * Checks whether a gate is in the module.<br>
     * If \p recursive is true, all submodules are searched as well.
     *
     * @param[in] gate - The gate to check.
     * @param[in] recursive - Look into submodules too
     * @returns True if the gate is in module
     */
    bool contains_gate(const std::shared_ptr<gate> gate, bool recursive = false) const;

    /**
     * Get a gate specified by id.<br>
     * If \p recursive is true, all submodules are searched as well.
     *
     * @param[in] id - The gate's id.
     * @param[in] recursive - Look into submodules too
     * @returns The gate or a nullptr.
     */
    std::shared_ptr<gate> get_gate_by_id(const u32 id, bool recursive = false) const;

    /**
     * Get all gates of the module. <br>
     * You can filter the set before output with the optional parameters.<br>
     * If \p recursive is true, all submodules are searched as well.
     *
     * @param[in] gate_type_filter - Filter for the gate type
     * @param[in] name_filter - Filter for the name
     * @param[in] recursive - Look into submodules too
     * @return A set of gates.
     */
    std::set<std::shared_ptr<gate>> get_gates(const std::string& gate_type_filter = DONT_CARE, const std::string& name_filter = DONT_CARE, bool recursive = false) const;

    /*
     * ################################################################
     *      net functions
     * ################################################################
     */

    /**
     * Moves a net into this module.<br>
     * The net is removed from its previous module in the process.
     *
     * @param[in] net - The net to move.
     * @returns True on success.
     */
    bool assign_net(std::shared_ptr<net> net);

    /**
     * Removes a net from the module.<br>
     * It is automatically moved to the netlist's top module.
     *
     * @param[in] net - Pointer to the net pointer.
     * @returns True on success.
     */
    bool remove_net(std::shared_ptr<net> net);

    /**
     * Checks whether a net is registered in the module.<br>
     * If \p recursive is true, all submodules are searched as well.
     *
     * @param[in] net - The net to check.
     * @param[in] recursive - Look into submodules too
     * @returns True if the net is in module
     */
    bool contains_net(const std::shared_ptr<net> net, bool recursive = false) const;

    /**
     * Get a net specified by id.<br>
     * If \p recursive is true, all submodules are searched as well.
     *
     * @param[in] id - The net's id.
     * @param[in] recursive - Look into submodules too
     * @returns The net or a nullptr.
     */
    std::shared_ptr<net> get_net_by_id(const u32 id, bool recursive = false) const;

    /**
     * Get all nets of the module. <br>
     * You can filter the set before output with the optional parameter.<br>
     * If \p recursive is true, all submodules are searched as well.
     *
     * @param[in] name_filter - Filter for the name
     * @param[in] recursive - Look into submodules too
     * @return A set of nets.
     */
    std::set<std::shared_ptr<net>> get_nets(const std::string& name_filter = DONT_CARE, bool recursive = false) const;

private:
    module(u32 id, std::shared_ptr<module> parent, const std::string& name, netlist_internal_manager* internal_manager);

    module(const module&) = delete;               //disable copy-constructor
    module& operator=(const module&) = delete;    //disable copy-assignment

    std::string m_name;

    netlist_internal_manager* m_internal_manager;
    u32 m_id;

    std::shared_ptr<module> m_parent;
    std::map<u32, std::shared_ptr<module>> m_submodules_map;
    std::set<std::shared_ptr<module>> m_submodules_set;

    /** stores gates sorted by id*/
    std::map<u32, std::shared_ptr<gate>> m_gates_map;
    std::set<std::shared_ptr<gate>> m_gates_set;

    /** stores nets storted by id  */
    std::map<u32, std::shared_ptr<net>> m_nets_map;
    std::set<std::shared_ptr<net>> m_nets_set;
};

#endif /* __HAL_MODULE_H__ */
