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
#include "hal_core/netlist/event_handler.h"
#include "hal_core/netlist/gate_library/gate_library.h"

#include <functional>
#include <memory>
#include <string>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>

namespace hal
{
    /** forward declaration */
    class NetlistInternalManager;
    class Net;
    class Gate;
    class Module;
    class Grouping;
    class Endpoint;

    /**
     * Netlist class containing information about the netlist including its gates, modules, nets, and groupings as well as the underlying gate library.
     *
     * @ingroup netlist
     */
    class NETLIST_API Netlist
    {
        friend class NetlistInternalManager;

    public:
        /**
         * Construct a new netlist for the specified gate library.<br>
         * Warning: Use the netlist_factory to create instances!
         *
         * @param[in] gate_library - The gate library.
         */
        explicit Netlist(const GateLibrary* gate_library);

        ~Netlist();

        /**
         * Check whether two netlists are equal.
         * Does not check netlist IDs.
         *
         * @param[in] other - The netlist to compare against.
         * @returns True if both netlists are equal, false otherwise.
         */
        bool operator==(const Netlist& other) const;

        /**
         * Check whether two netlists are unequal.
         * Does not check netlist IDs.
         *
         * @param[in] other - The netlist to compare against.
         * @returns True if both netlists are unequal, false otherwise.
         */
        bool operator!=(const Netlist& other) const;

        /**
         * Get the ID of the netlist.<br>
         * If not explicitly set, the ID defaults to 0.
         *
         * @returns The ID of the netlist.
         */
        u32 get_id() const;

        /**
         * Set the ID of the netlist to the specified value.
         *
         * @param[in] id - The new ID of the netlist.
         */
        void set_id(const u32 id);

        /**
         * Get the path to the input file.
         *
         * @returns The path to the input file.
         */
        std::filesystem::path get_input_filename() const;

        /**
         * Set the path to the input file.
         *
         * @param[in] path - The path to the input file.
         */
        void set_input_filename(const std::filesystem::path& path);

        /**
         * Get the name of the design.
         *
         * @returns The name of the design.
         */
        std::string get_design_name() const;

        /**
         * Set the name of the design.
         *
         * @param[in] name - The new name of the design.
         */
        void set_design_name(const std::string& name);

        /**
         * Get the name of the target device.
         *
         * @return The name of the target device.
         */
        std::string get_device_name() const;

        /**
         * Set the name of the target device.
         *
         * @param[in] name - The name of the target device.
         */
        void set_device_name(const std::string& name);

        /**
         * Get the gate library associated with the netlist.
         *
         * @returns The gate library.
         */
        const GateLibrary* get_gate_library() const;

        /**
         * Clear all internal caches of the netlist.<br>
         * In a typical application, calling this function is not required.
         */
        void clear_caches();

        /*
         * ################################################################
         *      gate functions
         * ################################################################
         */

        /**
         * Get a spare gate ID.<br>
         * The value of 0 is reserved and represents an invalid ID.
         *
         * @returns The gate ID.
         */
        u32 get_unique_gate_id();

        /**
         * Create a new gate and add it to the netlist.
         *
         * @param[in] gate_id - The unique ID of the gate.
         * @param[in] gate_type - The gate type.
         * @param[in] name - The name of the gate.
         * @param[in] x - The x-coordinate of the gate.
         * @param[in] y - The y-coordinate of the gate.
         * @returns The new gate on success, nullptr otherwise.
         */
        Gate* create_gate(const u32 gate_id, GateType* gate_type, const std::string& name = "", i32 x = -1, i32 y = -1);

        /**
         * Create a new gate and add it to the netlist.<br>
         * The ID of the gate is set automatically.
         *
         * @param[in] gate_type - The gate type.
         * @param[in] name - The name of the gate.
         * @param[in] x - The x-coordinate of the gate.
         * @param[in] y - The y-coordinate of the gate.
         * @returns The new gate on success, nullptr otherwise.
         */
        Gate* create_gate(GateType* gate_type, const std::string& name = "", i32 x = -1, i32 y = -1);

        /**
         * Remove a gate from the netlist.
         *
         * @param[in] gate - The gate.
         * @returns True on success, false otherwise.
         */
        bool delete_gate(Gate* gate);

        /**
         * Check whether the gate is registered in the netlist.
         *
         * @param[in] gate - The gate to check.
         * @returns True if the gate is in the netlist, false otherwise.
         */
        bool is_gate_in_netlist(Gate* gate) const;

        /**
         * Get the gate specified by the given ID.
         *
         * @param[in] gate_id - The unique ID of the gate.
         * @returns The gate on success, nullptr otherwise.
         */
        Gate* get_gate_by_id(const u32 gate_id) const;

        /**
         * Get all gates contained within the netlist.
         *
         * @return A vector of gates.
         */
        const std::vector<Gate*>& get_gates() const;

        /**
         * Get all gates contained within the netlist.<br>
         * The filter is evaluated on every gate such that the result only contains gates matching the specified condition.
         *
         * @param[in] filter - Filter function to be evaluated on each gate.
         * @return A vector of gates.
         */
        std::vector<Gate*> get_gates(const std::function<bool(Gate*)>& filter) const;

        /**
         * Mark a gate as a global VCC gate.
         *
         * @param[in] gate - The gate.
         * @returns True on success, false otherwise.
         */
        bool mark_vcc_gate(Gate* gate);

        /**
         * Mark a gate as a global GND gate.
         *
         * @param[in] gate - The gate.
         * @returns True on success, false otherwise.
         */
        bool mark_gnd_gate(Gate* gate);

        /**
         * Unmark a global VCC gate.
         *
         * @param[in] gate - The gate.
         * @returns True on success, false otherwise.
         */
        bool unmark_vcc_gate(Gate* gate);

        /**
         * Unmark a global GND gate.
         *
         * @param[in] gate - The gate.
         * @returns True on success, false otherwise.
         */
        bool unmark_gnd_gate(Gate* gate);

        /**
         * Check whether a gate is a global VCC gate.
         *
         * @param[in] gate - The gate to check.
         * @returns True if the gate is a global VCC gate, false otherwise.
         */
        bool is_vcc_gate(const Gate* gate) const;

        /**
         * Check whether a gate is a global GND gate.
         *
         * @param[in] gate - The gate to check.
         * @returns True if the gate is a global GND gate, false otherwise.
         */
        bool is_gnd_gate(const Gate* gate) const;

        /**
         * Get all global VCC gates.
         *
         * @returns A vector of gates.
         */
        const std::vector<Gate*>& get_vcc_gates() const;

        /**
         * Get all global GND gates.
         *
         * @returns A vector of gates.
         */
        const std::vector<Gate*>& get_gnd_gates() const;

        /*
         * ################################################################
         *      net functions
         * ################################################################
         */

        /**
         * Get a spare net ID.<br>
         * The value of 0 is reserved and represents an invalid ID.
         *
         * @returns The net ID.
         */
        u32 get_unique_net_id();

        /**
         * Create a new net and add it to the netlist.
         *
         * @param[in] net_id - The unique ID of the net.
         * @param[in] name - The name of the net.
         * @returns The new net on success, nullptr otherwise.
         */
        Net* create_net(const u32 net_id, const std::string& name = "");

        /**
         * Create a new net and add it to the netlist.<br>
         * The ID of the net is set automatically.
         *
         * @param[in] name - The name of the net.
         * @returns The new net on success, nullptr otherwise.
         */
        Net* create_net(const std::string& name = "");

        /**
         * Remove a net from the netlist.
         *
         * @param[in] net - The net.
         * @returns True on success, false otherwise.
         */
        bool delete_net(Net* net);

        /**
         * Check whether the net is registered in the netlist.
         *
         * @param[in] net - The net to check.
         * @returns True if the net is in the netlist, false otherwise.
         */
        bool is_net_in_netlist(Net* net) const;

        /**
         * Get the net specified by the given ID.
         *
         * @param[in] net_id - The unique ID of the net.
         * @returns The net on success, nullptr otherwise.
         */
        Net* get_net_by_id(u32 net_id) const;

        /**
         * Get all nets contained within the netlist.
         *
         * @return A vector of nets.
         */
        const std::vector<Net*>& get_nets() const;

        /**
         * Get all nets contained within the netlist.<br>
         * The filter is evaluated on every net such that the result only contains nets matching the specified condition.
         *
         * @param[in] filter - Filter function to be evaluated on each net.
         * @return A vector of nets.
         */
        std::vector<Net*> get_nets(const std::function<bool(Net*)>& filter) const;

        /**
         * Mark a net as a global input net.
         *
         * @param[in] net - The net.
         * @returns True on success, false otherwise.
         */
        bool mark_global_input_net(Net* net);

        /**
         * Mark a net as a global output net.
         *
         * @param[in] net - The net.
         * @returns True on success, false otherwise.
         */
        bool mark_global_output_net(Net* net);

        /**
         * Unmark a global input net.
         *
         * @param[in] net - The net.
         * @returns True on success, false otherwise.
         */
        bool unmark_global_input_net(Net* net);

        /**
         * Unmark a global output net.
         *
         * @param[in] net - The net.
         * @returns True on success, false otherwise.
         */
        bool unmark_global_output_net(Net* net);

        /**
         * Checks whether a net is a global input net.
         *
         * @param[in] net - The net to check.
         * @returns True if the net is a global input net, false otherwise.
         */
        bool is_global_input_net(const Net* net) const;

        /**
         * Checks whether a net is a global output net.
         *
         * @param[in] net - The net to check.
         * @returns True if the net is a global output net, false otherwise.
         */
        bool is_global_output_net(const Net* net) const;

        /**
         * Get all global input nets.
         *
         * @returns A vector of nets.
         */
        const std::vector<Net*>& get_global_input_nets() const;

        /**
         * Get all global output nets.
         *
         * @returns A vector of nets.
         */
        const std::vector<Net*>& get_global_output_nets() const;

        /*
         * ################################################################
         *      module functions
         * ################################################################
         */

        /**
         * Get a spare module ID.<br>
         * The value of 0 is reserved and represents an invalid ID.
         *
         * @returns The module ID.
         */
        u32 get_unique_module_id();

        /**
         * Create a new module and add it to the netlist.
         *
         * @param[in] module_id - The unique ID of the module.
         * @param[in] name - The name of the module.
         * @param[in] parent - The parent module.
         * @param[in] gates - Gates to assign to the new module.
         * @returns The new module on success, nullptr otherwise.
         */
        Module* create_module(const u32 module_id, const std::string& name, Module* parent, const std::vector<Gate*>& gates = {});

        /**
         * Create a new module and add it to the netlist.<br>
         * The ID of the module is set automatically.
         *
         * @param[in] name - The name of the module.
         * @param[in] parent - The parent module.
         * @param[in] gates - Gates to assign to the new module.
         * @returns The new module on success, nullptr otherwise.
         */
        Module* create_module(const std::string& name, Module* parent, const std::vector<Gate*>& gates = {});

        /**
         * Remove a module from the netlist.
         *
         * @param[in] module - The module.
         * @returns True on success, false otherwise.
         */
        bool delete_module(Module* module);

        /**
         * Check whether the module is registered in the netlist.
         *
         * @param[in] module - The module to check.
         * @returns True if the module is in the netlist, false otherwise.
         */
        bool is_module_in_netlist(Module* module) const;

        /**
         * Get the module specified by the given ID.
         *
         * @param[in] module_id - The unique ID of the module.
         * @returns The module on success, nullptr otherwise.
         */
        Module* get_module_by_id(u32 module_id) const;

        /**
         * Get all modules contained within the netlist, including the top module.
         *
         * @return A vector of modules.
         */
        const std::vector<Module*>& get_modules() const;

        /**
         * Get all modules contained within the netlist, including the top module.<br>
         * The filter is evaluated on every module such that the result only contains modules matching the specified condition.
         *
         * @param[in] filter - Filter function to be evaluated on each module.
         * @return A vector of modules.
         */
        std::vector<Module*> get_modules(const std::function<bool(Module*)>& filter) const;

        /**
         * Get the top module of the netlist.
         *
         * @returns The top module.
         */
        Module* get_top_module() const;

        /*
         * ################################################################
         *      grouping functions
         * ################################################################
         */

        /**
         * Gets a spare grouping ID.<br>
         * The value of 0 is reserved and represents an invalid ID.
         *
         * @returns The grouping ID.
         */
        u32 get_unique_grouping_id();

        /**
         * Create a new grouping and add it to the netlist.
         *
         * @param[in] grouping_id - The unique ID of the grouping.
         * @param[in] name - The name of the grouping.
         * @returns The new grouping on success, nullptr otherwise.
         */
        Grouping* create_grouping(const u32 grouping_id, const std::string& name = "");

        /**
         * Create a new grouping and add it to the netlist.<br>
         * The ID of the grouping is set automatically.
         *
         * @param[in] name - The name of the grouping.
         * @returns The new grouping on success, nullptr otherwise.
         */
        Grouping* create_grouping(const std::string& name = "");

        /**
         * Remove a grouping from the netlist.
         *
         * @param[in] grouping - The grouping.
         * @returns True on success, false otherwise.
         */
        bool delete_grouping(Grouping* grouping);

        /**
         * Check whether the grouping is registered in the netlist.
         *
         * @param[in] grouping - The grouping to check.
         * @returns True if the grouping is in the netlist, false otherwise.
         */
        bool is_grouping_in_netlist(Grouping* grouping) const;

        /**
         * Get the grouping specified by the given ID.
         *
         * @param[in] grouping_id - The unique ID of the grouping.
         * @returns The grouping on success, nullptr otherwise.
         */
        Grouping* get_grouping_by_id(u32 grouping_id) const;

        /**
         * Get all groupings contained within the netlist.<br>
         * A filter can be applied to the result to only get groupings matching the specified condition.
         *
         * @param[in] filter - Filter to be applied to the groupings.
         * @return A vector of groupings.
         */
        std::vector<Grouping*> get_groupings(const std::function<bool(Grouping*)>& filter = nullptr) const;

        /*
         * ################################################################
         *      getter/setter for ID tracking
         * ################################################################
         */

        /**
         * Get the gate ID following the highest currently used ID.
         * 
         * @returns The next gate ID.
         */
        u32 get_next_gate_id() const;

        /**
         * Set the gate ID following the highest currently used ID.
         * 
         * @param[in] id - The next gate ID.
         */
        void set_next_gate_id(const u32 id);

        /**
         * Get a set of all currently used gate IDs.
         * 
         * @returns All used gate IDs.
         */
        std::set<u32> get_used_gate_ids() const;

        /**
         * Set a set of all currently used gate IDs.
         * 
         * @param[in] ids - All used gate IDs.
         */
        void set_used_gate_ids(const std::set<u32> ids);

        /**
         * Get a set of all gate IDs that have previously been used but been freed ever since.
         * 
         * @returns All freed gate IDs.
         */
        std::set<u32> get_free_gate_ids() const;

        /**
         * Set a set of all gate IDs that have previously been used but been freed ever since.
         * 
         * @param[in] ids - All freed gate IDs.
         */
        void set_free_gate_ids(const std::set<u32> ids);

        /**
         * Get the net ID following the highest currently used ID.
         * 
         * @returns The next net ID.
         */
        u32 get_next_net_id() const;

        /**
         * Set the net ID following the highest currently used ID.
         * 
         * @param[in] id - The next net ID.
         */
        void set_next_net_id(const u32 id);

        /**
         * Get a set of all currently used net IDs.
         * 
         * @returns All used net IDs.
         */
        std::set<u32> get_used_net_ids() const;

        /**
         * Set a set of all currently used net IDs.
         * 
         * @param[in] ids - All used net IDs.
         */
        void set_used_net_ids(const std::set<u32> ids);

        /**
         * Get a set of all net IDs that have previously been used but been freed ever since.
         * 
         * @returns All freed net IDs.
         */
        std::set<u32> get_free_net_ids() const;

        /**
         * Set a set of all net IDs that have previously been used but been freed ever since.
         * 
         * @param[in] ids - All freed net IDs.
         */
        void set_free_net_ids(const std::set<u32> ids);

        /**
         * Get the module ID following the highest currently used ID.
         * 
         * @returns The next module ID.
         */
        u32 get_next_module_id() const;

        /**
         * Set the module ID following the highest currently used ID.
         * 
         * @param[in] id - The next module id.
         */
        void set_next_module_id(const u32 id);

        /**
         * Get a set of all currently used module IDs.
         * 
         * @returns All used module IDs.
         */
        std::set<u32> get_used_module_ids() const;

        /**
         * Set a set of all currently used module IDs.
         * 
         * @param[in] ids - All used module IDs.
         */
        void set_used_module_ids(const std::set<u32> ids);

        /**
         * Get a set of all module IDs that have previously been used but been freed ever since.
         * 
         * @returns All freed module IDs.
         */
        std::set<u32> get_free_module_ids() const;

        /**
         * Set a set of all module IDs that have previously been used but been freed ever since.
         * 
         * @param[in] ids - All freed module IDs.
         */
        void set_free_module_ids(const std::set<u32> ids);

        /**
         * Get the grouping ID following the highest currently used ID.
         * 
         * @returns The next grouping ID.
         */
        u32 get_next_grouping_id() const;

        /**
         * Set the grouping ID following the highest currently used ID.
         * 
         * @param[in] id - The next grouping ID.
         */
        void set_next_grouping_id(const u32 id);

        /**
         * Get a set of all currently used grouping IDs.
         * 
         * @returns All used grouping IDs.
         */
        std::set<u32> get_used_grouping_ids() const;

        /**
         * Set a set of all currently used grouping IDs.
         * 
         * @param[in] ids - All used grouping IDs.
         */
        void set_used_grouping_ids(const std::set<u32> ids);

        /**
         * Get a set of all grouping IDs that have previously been used but been freed ever since.
         * 
         * @returns All freed grouping ids.
         */
        std::set<u32> get_free_grouping_ids() const;

        /**
         * Get a set of all grouping IDs that have previously been used but been freed ever since.
         * 
         * @param[in] ids - All freed grouping ids.
         */
        void set_free_grouping_ids(const std::set<u32> ids);

        /**
         * Get event handler. Should only be used to register callbacks
         *
         * @return Pointer to netlist event handler
         */
        EventHandler* get_event_handler() const;

    private:
        /* stores the gate library */
        const GateLibrary* m_gate_library;

        /* stores the netlist id */
        u32 m_netlist_id;

        /* stores the name of the input file */
        std::filesystem::path m_file_name;

        /* stores the name of the design */
        std::string m_design_name;

        /* stores the name of the device */
        std::string m_device_name;

        /* stores the pointer to the netlist internal manager */
        NetlistInternalManager* m_manager;

        /* the event handler associated with the netlist */
        std::unique_ptr<EventHandler> m_event_handler;

        /* stores the auto generated ids for fast next id */
        u32 m_next_gate_id;
        std::set<u32> m_used_gate_ids;
        std::set<u32> m_free_gate_ids;
        u32 m_next_net_id;
        std::set<u32> m_used_net_ids;
        std::set<u32> m_free_net_ids;
        u32 m_next_module_id;
        std::set<u32> m_used_module_ids;
        std::set<u32> m_free_module_ids;
        u32 m_next_grouping_id;
        std::set<u32> m_used_grouping_ids;
        std::set<u32> m_free_grouping_ids;

        /* stores the modules */
        Module* m_top_module;
        std::unordered_map<u32, std::unique_ptr<Module>> m_modules_map;
        std::unordered_set<Module*> m_modules_set;
        std::vector<Module*> m_modules;

        /* stores the nets */
        std::unordered_map<u32, std::unique_ptr<Net>> m_nets_map;
        std::unordered_set<Net*> m_nets_set;
        std::vector<Net*> m_nets;

        /* stores the gates */
        std::unordered_map<u32, std::unique_ptr<Gate>> m_gates_map;
        std::unordered_set<Gate*> m_gates_set;
        std::vector<Gate*> m_gates;

        /* stores the groupings */
        std::unordered_map<u32, std::unique_ptr<Grouping>> m_groupings_map;
        std::unordered_set<Grouping*> m_groupings_set;
        std::vector<Grouping*> m_groupings;

        /* stores the set of global gates and nets */
        std::vector<Net*> m_global_input_nets;
        std::vector<Net*> m_global_output_nets;
        std::vector<Gate*> m_gnd_gates;
        std::vector<Gate*> m_vcc_gates;
    };
}    // namespace hal
