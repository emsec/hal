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
#include <list>
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
         * TODO test
         * Get all direct parent of this module.<br>
         * If \p recursive is set to true, all indirect parents are also included.<br>
         * A filter can be applied to the result to only get parents matching the specified condition.
         *
         * @param[in] filter - Filter to be applied to the modules.
         * @param[in] recursive - True to include indirect parents as well, false otherwise.
         * @returns A vector of parent modules.
         */
        std::vector<Module*> get_parent_modules(const std::function<bool(Module*)>& filter = nullptr, bool recursive = false) const;

        /**
         * Set a new parent for this module.<br>
         * If the new parent is a submodule of this module, the new parent is added as a direct submodule to the old parent first.
         *
         * @param[in] new_parent - The new parent module.
         * @returns True if the parent was changed, false otherwise.
         */
        bool set_parent_module(Module* new_parent);

        /**
         * TODO test
         * Check if the module is a parent of the specified module.
         * 
         * @param[in] module - The module.
         * @param[in] recursive - True to check recursively, false otherwise.
         * @returns True if the module is a parent of the specified module, false otherwise.
         */
        bool is_parent_module_of(Module* module, bool recursive = true) const;

        /**
         * Get all direct submodules of this module.<br>
         * If \p recursive is set to true, all indirect submodules are also included.<br>
         * A filter can be applied to the result to only get submodules matching the specified condition.
         *
         * @param[in] filter - Filter to be applied to the modules.
         * @param[in] recursive - True to include indirect submodules as well, false otherwise.
         * @returns A vector of submodules.
         */
        std::vector<Module*> get_submodules(const std::function<bool(Module*)>& filter = nullptr, bool recursive = false) const;

        /**
         * TODO test
         * Check if the module is a submodule of the specified module.
         * 
         * @param[in] module - The module.
         * @param[in] recursive - True to check recursively, false otherwise.
         * @returns True if the module is a submodule of the specified module, false otherwise.
         */
        bool is_submodule_of(Module* module, bool recursive = true) const;

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

        /* 
         * ################################################################
         *      net functions
         * ################################################################
         */

        /**
         * TODO test
         * Check whether a net is contained in the module.<br>
         * If \p recursive is set to true, nets in submodules are considered as well.
         *
         * @param[in] net - The net to check for.
         * @param[in] recursive - True to also consider nets in submodules, false otherwise.
         * @returns True if the net is contained in the module, false otherwise.
         */
        bool contains_net(Net* net, bool recursive = false) const;

        /**
         * TODO test
         * Get all nets that have at least one source or one destination within the module.<br>
         * A filter can be applied to the result to only get nets matching the specified condition.<br>
         * If \p recursive is true, nets in submodules are considered as well.
         *
         * @param[in] filter - Filter to be applied to the nets.
         * @param[in] recursive - True to also consider nets in submodules, false otherwise.
         * @returns A vector of nets.
         */
        std::vector<Net*> get_nets(const std::function<bool(Net*)>& filter = nullptr, bool recursive = false) const;

        /**
         * Get all nets that are either a global input to the netlist or have at least one source outside of the module.
         *
         * @returns A vector of input nets.
         */
        std::vector<Net*> get_input_nets() const;

        /**
         * Get all nets that are either a global output to the netlist or have at least one destination outside of the module.
         *
         * @returns A vector of output nets.
         */
        std::vector<Net*> get_output_nets() const;

        /**
         * Get all nets that have at least one source and one destination within the module, including its submodules. The result may contain nets that are also regarded as input or output nets.
         *
         * @returns A vector of internal nets.
         */
        std::vector<Net*> get_internal_nets() const;

        /** 
         * TODO test
         * Check whether the given net is an input of the module, i.e., whether the net is a global input to the netlist or has at least one source outside of the module.
         * 
         * @param[in] net - The net.
         * @returns True if the net is an input net, false otherwise.
         */
        bool is_input_net(Net* net) const;

        /** 
         * TODO test
         * Check whether the given net is an output of the module, i.e., whether the net is a global output to the netlist or has at least one destination outside of the module.
         * 
         * @param[in] net - The net.
         * @returns True if the net is an ouput net, false otherwise.
         */
        bool is_output_net(Net* net) const;

        /** 
         * TODO test
         * Check whether the given net is an internal net of the module, i.e. whether the net has at least one source and one destination within the module.
         * 
         * @param[in] net - The net.
         * @returns True if the net is an internal net, false otherwise.
         */
        bool is_internal_net(Net* net) const;

        /* 
         * ################################################################
         *      port functions
         * ################################################################
         */

        /**
         * A module port is a named entry or exit point of a module.
         * It comprises one or more pins, each of them being connected to a net.
         * A port always has a direction and may additionally feature a type.
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
             * Get the module the port is assigned to.
             * 
             * @returns The module.
             */
            Module* get_module() const;

            /**
             * Get the name of the port.
             * 
             * @returns The name of the port.
             */
            const std::string& get_name() const;

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
             * Get the ordered pins of the port.
             * 
             * @returns An ordered vector of pins.
             */
            std::vector<std::string> get_pins() const;

            /**
             * Get the ordered nets of the port.
             * 
             * @returns An ordered vector of nets.
             */
            std::vector<Net*> get_nets() const;

            /**
             * Get the pin through which the specified net runs.
             * 
             * @param[in] net - The net.
             * @returns The pin through which the net runs.
             */
            std::string get_pin(Net* net) const;

            /**
             * Get the net that runs through the specified pin.
             * 
             * @param[in] pin_name - The name of the pin.
             * @returns The net that runs through the pin.
             */
            Net* get_net(const std::string& pin_name) const;

            /**
             * Get the ordered pins and the nets that pass through them.
             * 
             * @returns An ordered vector of pairs of pins and nets.
             */
            std::vector<std::pair<std::string, Net*>> get_pins_and_nets() const;

            /**
             * Check whether the port is a multi-bit port, i.e., contains more than one pin.
             * 
             * @returns True if the port is a multi-bit port, false otherwise.
             */
            bool is_multi_bit() const;

            /**
             * Check whether the port contains the specified pin.
             * 
             * @param[in] pin_name - The name of the pin.
             * @returns True if the port contains the pin, false otherwise.
             */
            bool contains_pin(const std::string& pin_name) const;

            /**
             * Check whether the port contains the specified net.
             * 
             * @param[in] net - The net.
             * @returns True if the port contains the net, false otherwise.
             */
            bool contains_net(Net* net) const;

            /**
             * Assign a new position to the pin specified by its name.
             * All pins beyond the specified position will be shifted backwards.
             * 
             * @param[in] pin_name - The name of the pin.
             * @param[in] new_index - The new position that the pin should be assigned to.
             * @returns True on success, false otherwise.
             */
            bool move_pin(const std::string& pin_name, u32 new_index);

            /**
             * Assign a new position to the pin specified by its net.
             * All pins beyond the specified position will be shifted backwards.
             * 
             * @param[in] net - The net.
             * @param[in] new_index - The new position that the pin should be assigned to.
             * @returns True on success, false otherwise.
             */
            bool move_pin(Net* net, u32 new_index);

        private:
            friend class Module;

            Module* m_module;
            std::string m_name;
            std::list<std::pair<std::string, Net*>> m_pins;
            PinDirection m_direction;
            PinType m_type = PinType::none;

            std::unordered_map<std::string, Net*> m_pin_to_net;
            std::unordered_map<Net*, std::string> m_net_to_pin;

            Port(const Port&) = delete;
            Port(Module* module, const std::string& name, Net* net, PinDirection direction, PinType type = PinType::none);
            Port(Module* module, const std::string& name, const std::vector<std::pair<std::string, Net*>>& pins_and_nets, PinDirection direction, PinType type = PinType::none);
        };

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
         * Get all ports of the module.
         * The optional filter is evaluated on every port such that the result only contains ports matching the specified condition.
         * 
         * @param[in] filter - Filter function to be evaluated on each port.
         * @returns A vector of ports.
         */
        std::vector<Port*> get_ports(const std::function<bool(Port*)>& filter = nullptr) const;

        /**
         * Get the port specified by the given name.
         * 
         * @param[in] port_name - The name of the port.
         * @returns The port on success, a nullptr otherwise.
         */
        Port* get_port(const std::string& port_name) const;

        /**
         * Get the port that contains the specified net.
         * 
         * @param[in] net - The net.
         * @returns The port on success, a nullptr otherwise.
         */
        Port* get_port(Net* net) const;

        /**
         * Get the port that contains the specified pin.
         * 
         * @param[in] pin_name - The name of the pin.
         * @returns The port on success, a nullptr otherwise.
         */
        Port* get_port_by_pin_name(const std::string& pin_name) const;

        /**
         * Set the name of the given port.
         * For single-bit ports, the pin name is updated as well.
         * 
         * @param[in] port - The port.
         * @param[in] new_name - The name to be assigned to the port.
         * @returns True on success, false otherwise.
         */
        bool set_port_name(Port* port, const std::string& new_name);

        /**
         * Set the type of the given port.
         * 
         * @param[in] port - The port.
         * @param[in] new_type - The type to be assigned to the port.
         * @returns True on success, false otherwise.
         */
        bool set_port_type(Port* port, PinType new_type);

        /**
         * Set the name of a pin within a port.
         * 
         * @param[in] port - The port that contains the pin.
         * @param[in] old_name - The old name of the pin.
         * @param[in] new_name - The new name of the pin.
         * @returns True on success, false otherwise.
         */
        bool set_port_pin_name(Port* port, const std::string& old_name, const std::string& new_name);

        /**
         * Set the name of a pin within a port.
         * 
         * @param[in] port - The port that contains the pin.
         * @param[in] net - The net that passes through the pin.
         * @param[in] new_name - The new name of the pin.
         * @returns True on success, false otherwise.
         */
        bool set_port_pin_name(Port* port, Net* net, const std::string& new_name);

        /**
         * Merge multiple existing ports into a single multi-bit port.
         * 
         * @param[in] name - The name of the new port.
         * @param[in] ports_to_merge - The ports to be merged in the order in which they should be assigned to the new port.
         * @returns The port on success and a nullptr otherwise.
         */
        Port* create_multi_bit_port(const std::string& name, const std::vector<Port*>& ports_to_merge);

        /**
         * Split a multi-bit port into multiple single-bit ports.
         * 
         * @param[in] port - The port to be split.
         * @returns True on success, false otherwise.
         */
        bool delete_multi_bit_port(Port* port);

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
         * TODO test
         * Assign a vector of gates to the module.<br>
         * The gates are removed from their previous module in the process.
         *
         * @param[in] gates - The gates to assign.
         * @returns True on success, false otherwise.
         */
        bool assign_gates(const std::vector<Gate*>& gates);

        /**
         * Remove a gate from the module.<br>
         * Automatically moves the gate to the top module of the netlist.
         *
         * @param[in] gate - The gate to remove.
         * @returns True on success, false otherwise.
         */
        bool remove_gate(Gate* gate);

        /**
         * TODO test
         * Remove a vector of gates from the module.<br>
         * Automatically moves the gates to the top module of the netlist.
         *
         * @param[in] gates - The gates to remove.
         * @returns True on success, false otherwise.
         */
        bool remove_gates(const std::vector<Gate*>& gates);

        /**
         * Check whether a gate is contained in the module.<br>
         * If \p recursive is true, gates in submodules are considered as well.
         *
         * @param[in] gate - The gate to check for.
         * @param[in] recursive - True to also consider gates in submodules, false otherwise.
         * @returns True if the gate is contained in the module, false otherwise.
         */
        bool contains_gate(Gate* gate, bool recursive = false) const;

        /**
         * Get a gate specified by the given ID.<br>
         * If \p recursive is true, gates in submodules are considered as well.
         *
         * @param[in] id - The unique ID of the gate.
         * @param[in] recursive - True to also consider gates in submodules, false otherwise.
         * @returns The gate if found, a nullptr otherwise.
         */
        Gate* get_gate_by_id(const u32 id, bool recursive = false) const;

        /**
         * Get all gates contained within the module.<br>
         * A filter can be applied to the result to only get gates matching the specified condition.<br>
         * If \p recursive is true, gates in submodules are considered as well.
         *
         * @param[in] filter - Filter to be applied to the gates.
         * @param[in] recursive - True to also consider gates in submodules, false otherwise.
         * @return A vector of gates.
         */
        std::vector<Gate*> get_gates(const std::function<bool(Gate*)>& filter = nullptr, bool recursive = false) const;

    private:
        friend class NetlistInternalManager;
        Module(NetlistInternalManager* internal_manager, EventHandler* event_handler, u32 id, Module* parent, const std::string& name);

        Module(const Module&) = delete;               //disable copy-constructor
        Module& operator=(const Module&) = delete;    //disable copy-assignment

        void check_net(Net* net, bool recursive = false);
        Port* assign_port_net(Net* net, PinDirection direction);
        bool remove_port_net(Net* net);

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
        u32 m_next_input_index  = 0;
        u32 m_next_inout_index  = 0;
        u32 m_next_output_index = 0;
        std::vector<std::unique_ptr<Port>> m_ports;
        std::list<Port*> m_ports_raw;
        std::map<std::string, Port*> m_port_names_map;
        std::map<std::string, Port*> m_pin_names_map;

        /* stores gates sorted by id */
        std::unordered_map<u32, Gate*> m_gates_map;
        std::vector<Gate*> m_gates;

        std::set<Net*> m_nets;
        std::set<Net*> m_input_nets;
        std::set<Net*> m_output_nets;
        std::set<Net*> m_internal_nets;

        EventHandler* m_event_handler;
    };
}    // namespace hal
