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
#include "hal_core/netlist/pins/module_pin.h"
#include "hal_core/netlist/pins/pin_group.h"
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
         * Hash function for python binding
         *
         * @return Pybind11 compatible hash
         */
        ssize_t get_hash() const;

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
         * Get all direct parent of this module.<br>
         * If \p recursive is set to true, all indirect parents are also included.<br>
         * A filter can be applied to the result to only get parents matching the specified condition.
         *
         * @param[in] filter - Filter to be applied to the modules.
         * @param[in] recursive - True to include indirect parents as well, false otherwise.
         * @returns A vector of parent modules.
         */
        std::vector<Module*> get_parent_modules(const std::function<bool(Module*)>& filter = nullptr, bool recursive = true) const;

        /**
         * Set a new parent for this module.<br>
         * If the new parent is a submodule of this module, the new parent is added as a direct submodule to the old parent first.
         *
         * @param[in] new_parent - The new parent module.
         * @returns True if the parent was changed, false otherwise.
         */
        bool set_parent_module(Module* new_parent);

        /**
         * Check if the module is a parent of the specified module.
         * 
         * @param[in] module - The module.
         * @param[in] recursive - True to check recursively, false otherwise.
         * @returns True if the module is a parent of the specified module, false otherwise.
         */
        bool is_parent_module_of(const Module* module, bool recursive = false) const;

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
         * Check if the module is a submodule of the specified module.
         * 
         * @param[in] module - The module.
         * @param[in] recursive - True to check recursively, false otherwise.
         * @returns True if the module is a submodule of the specified module, false otherwise.
         */
        bool is_submodule_of(const Module* module, bool recursive = false) const;

        /**
         * Checks whether another module is a submodule of this module.<br>
         * If \p recursive is set to true, all indirect submodules are also included.
         *
         * @param[in] other - Other module to check for.
         * @param[in] recursive - True to include indirect submodules as well.
         * @returns True if the other module is a submodule, false otherwise.
         */
        bool contains_module(const Module* other, bool recursive = false) const;

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
         * Iterates over all nets connected to at least one gate of the module to update the nets, internal nets, input nets, and output nets of the module.
         * Has no effect on module pins. 
         * \warning{\b WARNING: can only be used when automatic net checks have been disabled using `Netlist::enable_automatic_net_checks`.}
         * 
         * @returns `true` on success, `false` otherwise.
         */
        void update_nets();

        /**
         * Check whether a net is contained in the module.<br>
         * If \p recursive is set to true, nets in submodules are considered as well.
         *
         * @param[in] net - The net to check for.
         * @param[in] recursive - True to also consider nets in submodules, false otherwise.
         * @returns True if the net is contained in the module, false otherwise.
         */
        bool contains_net(Net* net, bool recursive = false) const;

        /**
         * TODO add pybind
         * Get all nets that have at least one source or one destination within the module.<br>
         * A filter can be applied to the result to only get nets matching the specified condition.<br>
         * If \p recursive is true, nets in submodules are considered as well.
         *
         * @param[in] filter - Filter to be applied to the nets.
         * @param[in] recursive - True to also consider nets in submodules, false otherwise.
         * @returns An unordered set of nets.
         */
        std::unordered_set<Net*> get_nets(const std::function<bool(Net*)>& filter = nullptr, bool recursive = false) const;

        /**
         * TODO add pybind
         * Get all nets that are either a global input to the netlist or have at least one source outside of the module.
         *
         * @returns An unordered set of input nets.
         */
        const std::unordered_set<Net*>& get_input_nets() const;

        /**
         * TODO add pybind
         * Get all nets that are either a global output to the netlist or have at least one destination outside of the module.
         *
         * @returns An unordered set of output nets.
         */
        const std::unordered_set<Net*>& get_output_nets() const;

        /**
         * TODO add pybind
         * Get all nets that have at least one source and one destination within the module, including its submodules. The result may contain nets that are also regarded as input or output nets.
         *
         * @returns An unordered set of internal nets.
         */
        const std::unordered_set<Net*>& get_internal_nets() const;

        /** 
         * Check whether the given net is an input of the module, i.e., whether the net is a global input to the netlist or has at least one source outside of the module.
         * 
         * @param[in] net - The net.
         * @returns True if the net is an input net, false otherwise.
         */
        bool is_input_net(Net* net) const;

        /** 
         * Check whether the given net is an output of the module, i.e., whether the net is a global output to the netlist or has at least one destination outside of the module.
         * 
         * @param[in] net - The net.
         * @returns True if the net is an ouput net, false otherwise.
         */
        bool is_output_net(Net* net) const;

        /** 
         * Check whether the given net is an internal net of the module, i.e. whether the net has at least one source and one destination within the module.
         * 
         * @param[in] net - The net.
         * @returns True if the net is an internal net, false otherwise.
         */
        bool is_internal_net(Net* net) const;

        /* 
         * ################################################################
         *      pin functions
         * ################################################################
         */

        /**
         * Manually assign a module pin to a net.
         * Checks whether the given direction matches the actual properties of the net, i.e., checks whether the net actually is an input and/or output to the module.
         * Hence, update the module nets beforehand using `Module::update_nets`.
         * If `create_group` is set to `false`, the pin will not be added to a pin group.
         * \warning{\b WARNING: can only be used when automatic net checks have been disabled using `Netlist::enable_automatic_net_checks`.}
         * 
         * @param[in] name - The name of the pin.
         * @param[in] net - The net that the pin is being assigned to.
         * @param[in] type - The type of the pin. Defaults to `PinType::none`.
         * @param[in] create_group - Set `true` to automatically create a pin group and assign the pin, `false` otherwise.
         * @returns The module pin on success, a `nullptr` on failure.
         */
        ModulePin* assign_pin(const std::string& name, Net* net, PinType type = PinType::none, bool create_group = true);

        /**
         * Get all pins of the module.
         * The optional filter is evaluated on every pin such that the result only contains pins matching the specified condition.
         * 
         * @param[in] filter - Filter function to be evaluated on each pin.
         * @returns A vector of pins.
         */
        std::vector<ModulePin*> get_pins(const std::function<bool(ModulePin*)>& filter = nullptr) const;

        /**
         * Get all pin groups of the module.
         * The optional filter is evaluated on every pin group such that the result only contains pin groups matching the specified condition.
         * 
         * @param[in] filter - Filter function to be evaluated on each pin group.
         * @returns A vector of pin groups.
         */
        std::vector<PinGroup<ModulePin>*> get_pin_groups(const std::function<bool(PinGroup<ModulePin>*)>& filter = nullptr) const;

        /**
         * Get the pin specified by the given name.
         * 
         * @param[in] name - The name of the pin.
         * @returns The pin on success, a nullptr otherwise.
         */
        ModulePin* get_pin(const std::string& name) const;

        /**
         * Get the pin that passes through the specified net.
         * 
         * @param[in] net - The net.
         * @returns The pin on success, a nullptr otherwise.
         */
        ModulePin* get_pin(Net* net) const;

        /**
         * Get the pin group specified by the given name.
         * 
         * @param[in] name - The name of the pin group.
         * @returns The pin group on success, a nullptr otherwise.
         */
        PinGroup<ModulePin>* get_pin_group(const std::string& name) const;

        /**
         * Set the name of the given pin.
         * 
         * @param[in] pin - The pin.
         * @param[in] new_name - The name to be assigned to the pin.
         * @returns True on success, false otherwise.
         */
        bool set_pin_name(ModulePin* pin, const std::string& new_name);

        /**
         * Set the name of the given pin group.
         * 
         * @param[in] pin_group - The pin group.
         * @param[in] new_name - The name to be assigned to the pin group.
         * @returns True on success, false otherwise.
         */
        bool set_pin_group_name(PinGroup<ModulePin>* pin_group, const std::string& new_name);

        /**
         * Set the type of the given pin.
         * The pin type can only be changed this way if the pin is in a pin group of size 1.
         * The new type will also be assigned to the pin group.
         * 
         * @param[in] pin - The pin.
         * @param[in] new_type - The type to be assigned to the pin.
         * @returns True on success, false otherwise.
         */
        bool set_pin_type(ModulePin* pin, PinType new_type);

        /**
         * Set the type of the given pin group.
         * The new type will also be assigned to all pins of the group.
         * 
         * @param[in] pin_group - The pin group.
         * @param[in] new_type - The type to be assigned to the pin group.
         * @returns True on success, false otherwise.
         */
        bool set_pin_group_type(PinGroup<ModulePin>* pin_group, PinType new_type);

        /**
         * Create a new pin group with the given name.
         * All pins to be added to the pin group must have the same direction and type.
         * 
         * @param[in] name - The name of the pin group.
         * @param[in] pins - The pins to be assigned to the pin group.
         * @param[in] ascending - True for ascending pin order (from 0 to n-1), false otherwise (from n-1 to 0).
         * @param[in] start_index - The start index of the pin group.
         * @returns The pin group on success, a nullptr otherwise.
         */
        PinGroup<ModulePin>* create_pin_group(const std::string& name, const std::vector<ModulePin*> pins, bool ascending = false, u32 start_index = 0);

        /**
         * Delete the given pin group.
         * 
         * @param[in] pin_group - The pin group to be deleted.
         * @returns True on success, false otherwise.
         */
        bool delete_pin_group(PinGroup<ModulePin>* pin_group);

        /**
         * Assign a pin to a pin group.
         * Only pins with matching direction and type can be assigned to an existing pin group.
         * 
         * @param[in] pin_group - The new pin group.
         * @param[in] pin - The pin to be added.
         * @returns True on success, false otherwise.
         */
        bool assign_pin_to_group(PinGroup<ModulePin>* pin_group, ModulePin* pin);

        /**
         * Move a pin to another index within the given pin group.
         * The indices of some other pins within the group will be incremented or decremented to make room for the moved pin to be inserted at the desired position.
         * 
         * @param[in] pin_group - The pin group.
         * @param[in] pin - The pin to be moved.
         * @param[in] new_index - The index to which the pin is moved.
         * @returns True on success, false otherwise.
         */
        bool move_pin_within_group(PinGroup<ModulePin>* pin_group, ModulePin* pin, u32 new_index);

        /**
         * Remove a pin from a pin group.
         * The pin will be moved to a new group that goes by the pin's name.
         * 
         * @param[in] pin_group - The old pin group.
         * @param[in] pin - The pin to be removed.
         * @returns True on success, false otherwise.
         */
        bool remove_pin_from_group(PinGroup<ModulePin>* pin_group, ModulePin* pin);

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

        struct NetConnectivity
        {
            bool has_internal_source;
            bool has_internal_destination;
            bool has_external_source;
            bool has_external_destination;
        };

        NetConnectivity check_net_endpoints(const Net* net) const;
        void check_net(Net* net, bool recursive = false);
        ModulePin* assign_pin_net(Net* net, PinDirection direction, const std::string& name = "", PinType type = PinType::none, bool create_group = true);
        bool remove_pin_net(Net* net);

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
        // std::vector<std::unique_ptr<Port>> m_ports;
        // std::list<Port*> m_ports_raw;
        // std::map<std::string, Port*> m_port_names_map;
        // std::map<std::string, Port*> m_pin_names_map;

        // pins
        std::vector<std::unique_ptr<ModulePin>> m_pins;
        std::vector<std::unique_ptr<PinGroup<ModulePin>>> m_pin_groups;
        std::list<PinGroup<ModulePin>*> m_pin_groups_ordered;
        std::unordered_map<std::string, ModulePin*> m_pin_names_map;
        std::unordered_map<std::string, PinGroup<ModulePin>*> m_pin_group_names_map;

        /* stores gates sorted by id */
        std::unordered_map<u32, Gate*> m_gates_map;
        std::vector<Gate*> m_gates;

        std::unordered_set<Net*> m_nets;
        std::unordered_set<Net*> m_input_nets;
        std::unordered_set<Net*> m_output_nets;
        std::unordered_set<Net*> m_internal_nets;

        EventHandler* m_event_handler;
    };
}    // namespace hal
