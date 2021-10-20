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
#include "hal_core/netlist/gate_library/enums/pin_direction.h"
#include "hal_core/netlist/gate_library/enums/pin_type.h"
#include "hal_core/netlist/gate_library/gate_library.h"
#include "hal_core/utilities/enums.h"

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>

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
         * @param[in] other - The module to compare against.
         * @returns True if both modules are equal, false otherwise.
         */
        bool operator==(const Module& other) const;

        /**
         * Check whether two modules are unequal.
         * Does not check for parent module.
         *
         * @param[in] other - The module to compare against.
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
         * Mark all internal caches as dirty. Caches are primarily used for the nets connected to the gates of a module.
         * 
         * @param[in] is_dirty - True to mark caches as dirty, false otherwise.
         */
        void set_cache_dirty(bool is_dirty = true);

        /*
         * ################################################################
         *      port functions
         * ################################################################
         */

        /**
         * The port of a module is a named entry or exit point where a net crosses the module boundary.
         * A port always has a direction and may additionally feature a type and be part of a port group.
         * 
         * @ingroup module
         */
        class Port
        {
        public:
            /**
             * Check whether two ports are equal.
             * 
             * @param[in] other - The port to compare against.
             * @returns True if both ports are equal, false otherwise.
             */
            bool operator==(const Port& other) const;

            /**
             * Check whether two ports are unequal.
             * 
             * @param[in] other - The port to compare against.
             * @returns True if both ports are unequal, false otherwise.
             */
            bool operator!=(const Port& other) const;

            /**
             * Get the name of the port.
             * 
             * @returns The name of the port.
             */
            const std::string& get_name() const;

            /**
             * Get the net passing through the port.
             * 
             * @returns The net passing through the port.
             */
            Net* get_net() const;

            /**
             * Get the direction of the port.
             * 
             * @returns The direction of the port.
             */
            PinDirection get_direction() const;

            /**
             * Get the type of the port.
             * 
             * @returns The type of the port.
             */
            PinType get_type() const;

            /**
             * Get the name of the group that the port is part of.
             * Returns an empty string if the port is not part of a group.
             * 
             * @returns The name of the port group.
             */
            const std::string& get_group_name() const;

            /**
             * Get the index of the port within its port group.
             * Returns 0 if the port is not part of a group. 
             * Make sure to check the group name to determine whether the port is actually part of a group.
             * 
             * @returns The index of the port within the port group.
             */
            u32 get_group_index() const;

        private:
            friend Module;

            std::string m_name;
            Net* m_net;
            PinDirection m_direction;
            PinType m_type           = PinType::none;
            std::string m_group_name = "";
            u32 m_group_index        = 0;

            Port(const std::string& name, Net* net);
        };

        /**
         * \deprecated
         * DEPRECATED <br>
         * Set the name of the port corresponding to the specified input net.
         *
         * @param[in] input_net - The input net.
         * @param[in] port_name - The input port name.
         * @returns True on success, false otherwise.
         */
        [[deprecated("Will be removed in a future version. Use change_port_name() instead.")]] bool set_input_port_name(Net* input_net, const std::string& port_name);

        /**
         * \deprecated
         * DEPRECATED <br>
         * Get the name of the port corresponding to the specified input net.
         *
         * @param[in] input_net - The input net.
         * @returns The input port name.
         */
        [[deprecated("Will be removed in a future version. Use get_port_by_net() instead.")]] std::string get_input_port_name(Net* input_net) const;

        /**
         * \deprecated
         * DEPRECATED <br>
         * Get the input net of the port corresponding to the specified port name.
         *
         * @param[in] port_name - The input port name.
         * @returns The input net or a nullptr.
         */
        [[deprecated("Will be removed in a future version. Use get_port_by_name() instead.")]] Net* get_input_port_net(const std::string& port_name) const;

        /**
         * \deprecated
         * DEPRECATED <br>
         * Get the mapping of all input nets to their corresponding port names.
         *
         * @returns The map from input net to port name.
         */
        [[deprecated("Will be removed in a future version. Use get_ports() instead.")]] std::map<Net*, std::string> get_input_port_names() const;

        /**
         * \deprecated
         * DEPRECATED <br>
         * Set the name of the port corresponding to the specified output net.
         *
         * @param[in] output_net - The output net.
         * @param[in] port_name - The output port name.
         * @returns True on success, false otherwise.
         */
        [[deprecated("Will be removed in a future version. Use change_port_name() instead.")]] bool set_output_port_name(Net* output_net, const std::string& port_name);

        /**
         * \deprecated
         * DEPRECATED <br>
         * Get the name of the port corresponding to the specified output net.
         *
         * @param[in] output_net - The output net.
         * @returns The output port name.
         */
        [[deprecated("Will be removed in a future version. Use get_port_by_net() instead.")]] std::string get_output_port_name(Net* output_net) const;

        /**
         * \deprecated
         * DEPRECATED <br>
         * Get the output net of the port corresponding to the specified port name.
         *
         * @param[in] port_name - The output port name.
         * @returns The output net or a nullptr.
         */
        [[deprecated("Will be removed in a future version. Use get_port_by_name() instead.")]] Net* get_output_port_net(const std::string& port_name) const;

        /**
         * \deprecated
         * DEPRECATED <br>
         * Get the mapping of all output nets to their corresponding port names.
         *
         * @returns The map from output net to port name.
         */
        [[deprecated("Will be removed in a future version. Use get_ports() instead.")]] std::map<Net*, std::string> get_output_port_names() const;

        /**
         * \deprecated
         * DEPRECATED <br>
         * Set the next free input port ID to the given value.
         * 
         * @param[in] id - The next input port ID. 
         */
        [[deprecated("Will be removed in a future version.")]] void set_next_input_port_id(u32 id);

        /**
         * \deprecated
         * DEPRECATED <br>
         * Set the next free inout port ID to the given value.
         * 
         * @param[in] id - The next inout port ID. 
         */
        [[deprecated("Will be removed in a future version.")]] void set_next_inout_port_id(u32 id);

        /**
         * \deprecated
         * DEPRECATED <br>
         * Set the next free output port ID to the given value.
         * 
         * @param[in] id - The next output port ID. 
         */
        [[deprecated("Will be removed in a future version.")]] void set_next_output_port_id(u32 id);

        /**
         * \deprecated
         * DEPRECATED <br>
         * Get the next free input port ID.
         * 
         * @returns The next input port ID.
         */
        [[deprecated("Will be removed in a future version.")]] u32 get_next_input_port_id() const;

        /**
         * \deprecated
         * DEPRECATED <br>
         * Get the next free inout port ID.
         * 
         * @returns The next inout port ID.
         */
        [[deprecated("Will be removed in a future version.")]] u32 get_next_inout_port_id() const;

        /**
         * \deprecated
         * DEPRECATED <br>
         * Get the next free output port ID.
         * 
         * @returns The next output port ID.
         */
        [[deprecated("Will be removed in a future version.")]] u32 get_next_output_port_id() const;

        /**
         * Change the name of an existing port.
         * 
         * @param[in] port - The port.
         * @param[in] port_name - The name of the port.
         * @return True on success, false otherwise.
         */
        bool change_port_name(Port* port, const std::string& port_name);

        /**
         * Change the type of an existing port.
         * 
         * @param[in] port - The port.
         * @param[in] port_type - The type of the port.
         * @return True on success, false otherwise.
         */
        bool change_port_type(Port* port, PinType port_type);

        /**
         * Get all ports of the module.
         * 
         * @returns A vector of ports.
         */
        const std::vector<Port*>& get_ports() const;

        /**
         * Get all ports of the module.
         * The filter is evaluated on every port such that the result only contains ports matching the specified condition.
         * 
         * @param[in] filter - Filter function to be evaluated on each port.
         * @returns A vector of ports.
         */
        std::vector<Port*> get_ports(const std::function<bool(Port*)>& filter) const;

        /**
         * Get a port by the net that is passing through it.
         * 
         * @param[in] port_net - The net that passes through the port.
         * @returns The port on success, a nullptr otherwise.
         */
        Port* get_port_by_net(Net* port_net) const;

        /**
         * Get a port by its name.
         * 
         * @param[in] port_name - The name of the port.
         * @returns The port on success, a nullptr otherwise.
         */
        Port* get_port_by_name(const std::string& port_name) const;

        /**
         * Create a new port group and assign existing ports to it.
         * 
         * @param[in] group_name - The name of the port group to be created.
         * @param[in] port_indices - A vector of pairs comprising a port indices as well as the ports themselves.
         * @returns True on success, false otherweise.
         */
        bool assign_port_group(const std::string& group_name, const std::vector<std::pair<u32, Port*>>& port_indices);

        /**
         * TODO Test
         * Delete the given port group such that its pins do not belong to any group anymore.
         * 
         * @param[in] group_name - The name of the port group.
         * @returns True on success, false otherwise.
         */
        bool delete_port_group(const std::string& group_name);

        // TODO remove port from group

        /**
         * Get all port groups of the module.
         * 
         * @returns A map from port group name to a vector of ports.
         */
        const std::unordered_map<std::string, std::vector<Port*>>& get_port_groups() const;

        /**
         * Get all ports belonging to a given port group in order.
         * 
         * @param[in] group_name - The name of the port group.
         * @returns A vector of ports belonging to the specified port group.
         */
        std::vector<Port*> get_ports_of_group(const std::string& group_name) const;

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

        /**
         * Create a new port and assign it to the current module instance. Does not perform sanity checks.
         * 
         * @param[in] port_name - Name of the port.
         * @param[in] port_net - Net passing through the port.
         * @param[in] direction - Direction of the port.
         * @param[in] type - Type of the port.
         * @returns The created port.
         */
        Port* create_port(const std::string& port_name, Net* port_net, PinDirection direction, PinType type = PinType::none) const;

        /**
         * Remove a port from the current module instance. Does not perform sanity checks.
         * 
         * @param[in] port - The port to be removed.
         */
        void remove_port(Port* port) const;

        /**
         * Determine the direction of a port using the net passing through the port.
         * 
         * @param[in] net - The net passing through the port.
         * @returns The direction of the port or PinDirection::none if the net is not an input or output of the module.
         */
        PinDirection determine_port_direction(Net* net) const;

        /**
         * Update the ports of the module by analyzing its input and output nets.
         */
        void update_ports() const;

        std::string m_name;
        std::string m_type;

        NetlistInternalManager* m_internal_manager;
        u32 m_id;

        /* grouping */
        Grouping* m_grouping = nullptr;

        Module* m_parent;
        std::unordered_map<u32, Module*> m_submodules_map;
        std::vector<Module*> m_submodules;

        // ports
        mutable bool m_ports_dirty;
        mutable u32 m_next_input_index  = 0;
        mutable u32 m_next_inout_index  = 0;
        mutable u32 m_next_output_index = 0;
        mutable std::vector<std::unique_ptr<Port>> m_ports;
        mutable std::set<Net*> m_port_nets;
        mutable std::vector<Port*> m_ports_raw;
        mutable std::unordered_map<Net*, Port*> m_net_to_port;
        mutable std::unordered_map<std::string, Port*> m_name_to_port;
        mutable std::unordered_map<std::string, std::vector<Port*>> m_port_groups;

        /* stores gates sorted by id */
        std::unordered_map<u32, Gate*> m_gates_map;
        std::vector<Gate*> m_gates;

        mutable bool m_nets_dirty = true;
        mutable std::vector<Net*> m_nets;
        mutable bool m_input_nets_dirty = true;
        mutable std::vector<Net*> m_input_nets;
        mutable bool m_output_nets_dirty = true;
        mutable std::vector<Net*> m_output_nets;
        mutable bool m_internal_nets_dirty = true;
        mutable std::vector<Net*> m_internal_nets;

        EventHandler* m_event_handler;
    };
}    // namespace hal
