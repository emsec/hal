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
#include "hal_core/netlist/event_handler.h"
#include "hal_core/netlist/gate_library/gate_library.h"

#include <functional>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <tuple>
#include <type_traits>

namespace hal
{
    /** forward declaration */
    class Netlist;
    class NetlistInternalManager;
    class Net;
    class Grouping;
    class Gate;

    /**
     * A module is a container for gates and their associated nets that enables hierarchization within the netlist.<br>
     * Each gate can only be in one module at a time. Nets are only loosely associated with modules.
     *
     * @ingroup module
     */
    class NETLIST_API Module : public DataContainer
    {
    public:
        /**
         * Check whether two modules are equal.
         * Does not check for parent module.
         *
         * @param[in] other - The module to compare again.
         * @returns True if both modules are equal, false otherwise.
         */
        bool operator==(const Module& other) const;

        /**
         * Check whether two modules are unequal.
         * Does not check for parent module.
         *
         * @param[in] other - The module to compare again.
         * @returns True if both modules are unequal, false otherwise.
         */
        bool operator!=(const Module& other) const;

        /**
         * Get the unique ID of the module.
         *
         * @returns The unique id.
         */
        u32 get_id() const;

        /**
         * Get the name of the module.
         *
         * @returns The name.
         */
        std::string get_name() const;

        /**
         * Set the name of the module.
         *
         * @param[in] name - The new name.
         */
        void set_name(const std::string& name);

        /**
         * Get the type of the module.
         *
         * @returns The type.
         */
        std::string get_type() const;

        /**
         * Set the type of the module.
         *
         * @param[in] type - The new type.
         */
        void set_type(const std::string& type);

        /**
         * Get the grouping in which this module is contained.
         *
         * @returns The grouping.
         */
        Grouping* get_grouping() const;

        /**
         * Get submodule depth of module
         * @returns 0=top_level module, 1=submodule from top_level, 2=submodule from submodule, ...
         */
        int get_submodule_depth() const;

        /**
         * Get the parent module of this module.<br>
         * For the top module, a \p nullptr is returned.
         *
         * @returns The parent module.
         */
        Module* get_parent_module() const;

        /**
         * Set a new parent for this module.<br>
         * If the new parent is a submodule of this module, the new parent is added as a direct submodule to the old parent first.
         *
         * @param[in] new_parent - The new parent module.
         * @returns True if the parent was changed, false otherwise.
         */
        bool set_parent_module(Module* new_parent);

        /**
         * Get all direct submodules of this module.<br>
         * If \p recursive is set to true, all indirect submodules are also included.<br>
         * A filter can be applied to the result to only get submodules matching the specified condition.
         *
         * @param[in] filter - Filter to be applied to the modules.
         * @param[in] recursive - True to include indirect submodules as well.
         * @returns The vector of submodules.
         */
        std::vector<Module*> get_submodules(const std::function<bool(Module*)>& filter = nullptr, bool recursive = false) const;

        /**
         * Checks whether another module is a submodule of this module.<br>
         * If \p recursive is set to true, all indirect submodules are also included.
         *
         * @param[in] other - Other module to check for.
         * @param[in] recursive - True to include indirect submodules as well.
         * @returns True if the other module is a submodule, false otherwise.
         */
        bool contains_module(Module* other, bool recursive = false) const;

        /**
         * Returns true only if the module is the top module of the netlist.
         * 
         * @returns True if the module is the top module, false otherwise.
         */
        bool is_top_module() const;

        /**
         * Get the netlist this module is associated with.
         *
         * @returns The netlist.
         */
        Netlist* get_netlist() const;

        /**
         * TODO test
         * Get all nets that have at least one source or one destination within the module. Includes nets that are input and/or output to any of the submodules.
         *
         * @returns A sorted vector of nets.
         */
        const std::vector<Net*>& get_nets() const;

        /**
         * Get all nets that are either a global input to the netlist or have at least one source outside of the module.
         *
         * @returns A sorted vector of input nets.
         */
        const std::vector<Net*>& get_input_nets() const;

        /**
         * Get all nets that are either a global output to the netlist or have at least one destination outside of the module.
         *
         * @returns A sorted vector of output nets.
         */
        const std::vector<Net*>& get_output_nets() const;

        /**
         * Get all nets that have at least one source and one destination within the module, including its submodules. The result may contain nets that are also regarded as input or output nets.
         *
         * @returns A sorted vector of internal nets.
         */
        const std::vector<Net*>& get_internal_nets() const;

        /**
         * Set the name of the port corresponding to the specified input net.
         *
         * @param[in] input_net - The input net.
         * @param[in] port_name - The input port name.
         */
        void set_input_port_name(Net* input_net, const std::string& port_name);

        /**
         * Get the name of the port corresponding to the specified input net.
         *
         * @param[in] input_net - The input net.
         * @returns The input port name.
         */
        std::string get_input_port_name(Net* input_net) const;

        /**
         * Get the input net of the port corresponding to the specified port name.
         *
         * @param[in] port_name - The input port name.
         * @returns The input net or a nullptr.
         */
        Net* get_input_port_net(const std::string& port_name) const;

        /**
         * Get the mapping of all input nets to their corresponding port names.
         *
         * @returns The map from input net to port name.
         */
        const std::map<Net*, std::string>& get_input_port_names() const;

        /**
         * Set the next free input port ID to the given value.
         * 
         * @param[in] id - The next input port ID. 
         */
        void set_next_input_port_id(u32 id);

        /**
         * Get the next free input port ID.
         * 
         * @returns The next input port ID.
         */
        u32 get_next_input_port_id() const;

        /**
         * Set the name of the port corresponding to the specified output net.
         *
         * @param[in] output_net - The output net.
         * @param[in] port_name - The output port name.
         */
        void set_output_port_name(Net* output_net, const std::string& port_name);

        /**
         * Get the name of the port corresponding to the specified output net.
         *
         * @param[in] output_net - The output net.
         * @returns The output port name.
         */
        std::string get_output_port_name(Net* output_net) const;

        /**
         * Get the output net of the port corresponding to the specified port name.
         *
         * @param[in] port_name - The output port name.
         * @returns The output net or a nullptr.
         */
        Net* get_output_port_net(const std::string& port_name) const;

        /**
         * Get the mapping of all output nets to their corresponding port names.
         *
         * @returns The map from output net to port name.
         */
        const std::map<Net*, std::string>& get_output_port_names() const;

        /**
         * Set the next free output port ID to the given value.
         * 
         * @param[in] id - The next output port ID. 
         */
        void set_next_output_port_id(u32 id);

        /**
         * Get the next free output port ID.
         * 
         * @returns The next output port ID.
         */
        u32 get_next_output_port_id() const;

        /**
         * Mark all internal caches as dirty. Caches are primarily used for the nets connected to the gates of a module.
         * 
         * @param[in] is_dirty - True to mark caches as dirty, false otherwise.
         */
        void set_cache_dirty(bool is_dirty = true);

        /*
         * ################################################################
         *      gate functions
         * ################################################################
         */

        /**
         * Assign a gate to the module.<br>
         * The gate is removed from its previous module in the process.
         *
         * @param[in] gate - The gate to assign.
         * @returns True on success, false otherwise.
         */
        bool assign_gate(Gate* gate);

        /**
         * Remove a gate from the module.<br>
         * Automatically moves the gate to the top module of the netlist.
         *
         * @param[in] gate - The gate to remove.
         * @returns True on success, false otherwise.
         */
        bool remove_gate(Gate* gate);

        /**
         * Check whether a gate is in the module.<br>
         * If \p recursive is set to true, all submodules are searched as well.
         *
         * @param[in] gate - The gate to check for.
         * @param[in] recursive - True to also search in submodules.
         * @returns True if the gate is in the module, false otherwise.
         */
        bool contains_gate(Gate* gate, bool recursive = false) const;

        /**
         * Get a gate specified by the given ID.<br>
         * If \p recursive is true, all submodules are searched as well.
         *
         * @param[in] id - The unique ID of the gate.
         * @param[in] recursive - True to also search in submodules.
         * @returns The gate if found, a nullptr otherwise.
         */
        Gate* get_gate_by_id(const u32 id, bool recursive = false) const;

        /**
         * Get all gates contained within the module.<br>
         * A filter can be applied to the result to only get gates matching the specified condition.<br>
         * If \p recursive is true, all submodules are searched as well.
         *
         * @param[in] filter - Filter to be applied to the gates.
         * @param[in] recursive - True to also search in submodules.
         * @return The vector of all gates.
         */
        std::vector<Gate*> get_gates(const std::function<bool(Gate*)>& filter = nullptr, bool recursive = false) const;

    private:
        friend class NetlistInternalManager;
        Module(NetlistInternalManager* internal_manager, EventHandler* event_handler, u32 id, Module* parent, const std::string& name);

        Module(const Module&) = delete;               //disable copy-constructor
        Module& operator=(const Module&) = delete;    //disable copy-assignment

        std::string m_name;
        std::string m_type;

        NetlistInternalManager* m_internal_manager;
        u32 m_id;

        /* grouping */
        Grouping* m_grouping = nullptr;

        Module* m_parent;
        std::unordered_map<u32, Module*> m_submodules_map;
        std::vector<Module*> m_submodules;

        /* port names */
        mutable u32 m_next_input_port_id  = 0;
        mutable u32 m_next_output_port_id = 0;
        mutable std::set<Net*> m_named_input_nets;                        // ordering necessary, cannot be replaced with unordered_set
        mutable std::set<Net*> m_named_output_nets;                       // ordering necessary, cannot be replaced with unordered_set
        mutable std::map<Net*, std::string> m_input_net_to_port_name;     // ordering necessary, cannot be replaced with unordered_map
        mutable std::map<Net*, std::string> m_output_net_to_port_name;    // ordering necessary, cannot be replaced with unordered_map
        mutable std::unordered_set<std::string> m_input_port_names;
        mutable std::unordered_set<std::string> m_output_port_names;

        /* stores gates sorted by id */
        std::unordered_map<u32, Gate*> m_gates_map;
        std::vector<Gate*> m_gates;

        mutable bool m_nets_dirty;
        mutable std::vector<Net*> m_nets;
        mutable bool m_input_nets_dirty;
        mutable std::vector<Net*> m_input_nets;
        mutable bool m_output_nets_dirty;
        mutable std::vector<Net*> m_output_nets;
        mutable bool m_internal_nets_dirty;
        mutable std::vector<Net*> m_internal_nets;

        EventHandler* m_event_handler;
    };
}    // namespace hal
