#pragma once

#include "hal_core/netlist/netlist_internal_manager.h"

#include <functional>
#include <unordered_map>
#include <vector>

namespace hal
{
    class Gate;
    class Net;
    class Module;

    /**
     * A grouping is an unstructured collection of gates, nets, and modules that do not need to be connected in any way.<br>
     * It is designed to act as a container to temporarily store related entities during netlist exploration.<br>
     * In contrast to a module, it does not allow for hierarchization.<br>
     * Each gate, net, or module within the netlist may only be assigned to a single grouping.
     *
     * @ingroup netlist
     */
    class NETLIST_API Grouping
    {
    public:
        /**
         * Get the unique ID of the grouping.
         *
         * @returns The unique id.
         */
        u32 get_id();

        /**
         * Set the name of the grouping.
         *
         * @params[in] name - The new name.
         */
        void set_name(std::string name);

        /**
         * Get the name of the grouping.
         *
         * @returns The name.
         */
        std::string get_name();

        /**
         * Get the netlist this grouping is associated with.
         *
         * @returns The netlist.
         */
        Netlist* get_netlist() const;

        /*
         * ################################################################
         *      gate functions
         * ################################################################
         */

        /**
         * Assigns a gate to the grouping.<br>
         * Fails if the gate is already contained within another grouping.
         *
         * @param[in] gate - The gate to assign.
         * @returns True on success, false otherwise.
         */
        bool assign_gate(Gate* gate);

        /**
         * Assigns a gate to the grouping by ID.<br>
         * Fails if the gate is already contained within another grouping.
         *
         * @param[in] gate_id - The ID of the gate to assign.
         * @returns True on success, false otherwise.
         */
        bool assign_gate_by_id(const u32 gate_id);

        /**
         * Get all gates contained within the grouping. <br>
         * A filter can be applied to the result to only get gates matching the specified condition.
         *
         * @param[in] filter - Filter to be applied to the gates.
         * @returns A vector of gates.
         */
        std::vector<Gate*> get_gates(const std::function<bool(Gate*)>& filter = nullptr);

        /**
         * Get the IDs of all gates contained within the grouping. <br>
         * A filter can be applied to the result to only get gate IDs for gates matching the specified condition.
         *
         * @param[in] filter - Filter to be applied to the gates.
         * @returns A vector of gate IDs.
         */
        std::vector<u32> get_gate_ids(const std::function<bool(Gate*)>& filter = nullptr);

        /**
         * Removes a gate from the grouping.<br>
         * Fails if the gate is not contained within the grouping.
         *
         * @param[in] gate - The gate to remove.
         * @returns True on success, false otherwise.
         */
        bool remove_gate(Gate* gate);

        /**
         * Removes a gate from the grouping by ID.<br>
         * Fails if the gate is not contained within the grouping.
         *
         * @param[in] gate_id - The ID of the gate to remove.
         * @returns True on success, false otherwise.
         */
        bool remove_gate_by_id(const u32 gate_id);

        /**
         * Checks whether a gate is in the grouping.<br>
         *
         * @param[in] gate - The gate to check for.
         * @returns True on success, false otherwise.
         */
        bool contains_gate(Gate* gate);

        /**
         * Checks whether a gate is in the grouping by ID.<br>
         *
         * @param[in] gate_id - The ID of the gate to check for.
         * @returns True on success, false otherwise.
         */
        bool contains_gate_by_id(const u32 gate_id);

        /*
         * ################################################################
         *      net functions
         * ################################################################
         */

        /**
         * Assigns a net to the grouping.<br>
         * Fails if the net is already contained within another grouping.
         *
         * @param[in] net - The net to assign.
         * @returns True on success, false otherwise.
         */
        bool assign_net(Net* net);

        /**
         * Assigns a net to the grouping by ID.<br>
         * Fails if the net is already contained within another grouping.
         *
         * @param[in] net_id - The ID of the net to assign.
         * @returns True on success, false otherwise.
         */
        bool assign_net_by_id(const u32 net_id);

        /**
         * Get all nets contained within the grouping. <br>
         * A filter can be applied to the result to only get nets matching the specified condition.
         *
         * @param[in] filter - Filter to be applied to the nets.
         * @returns A vector of nets.
         */
        std::vector<Net*> get_nets(const std::function<bool(Net*)>& filter = nullptr);

        /**
         * Get the IDs of all nets contained within the grouping. <br>
         * A filter can be applied to the result to only get net IDs for nets matching the specified condition.
         *
         * @param[in] filter - Filter to be applied to the nets.
         * @returns A vector of net IDs.
         */
        std::vector<u32> get_net_ids(const std::function<bool(Net*)>& filter = nullptr);

        /**
         * Removes a net from the grouping.<br>
         * Fails if the net is not contained within the grouping.
         *
         * @param[in] net - The net to remove.
         * @returns True on success, false otherwise.
         */
        bool remove_net(Net* net);

        /**
         * Removes a net from the grouping by ID.<br>
         * Fails if the net is not contained within the grouping.
         *
         * @param[in] net_id - The ID of the net to remove.
         * @returns True on success, false otherwise.
         */
        bool remove_net_by_id(const u32 net_id);

        /**
         * Checks whether a net is in the grouping.<br>
         *
         * @param[in] net - The net to check for.
         * @returns True on success, false otherwise.
         */
        bool contains_net(Net* net);

        /**
         * Checks whether a net is in the grouping by ID.<br>
         *
         * @param[in] net_id - The ID of the net to check for.
         * @returns True on success, false otherwise.
         */
        bool contains_net_by_id(const u32 net_id);

        /*
         * ################################################################
         *      module functions
         * ################################################################
         */

        /**
         * Assigns a module to the grouping.<br>
         * Fails if the module is already contained within another grouping.
         *
         * @param[in] module - The module to assign.
         * @returns True on success, false otherwise.
         */
        bool assign_module(Module* module);

        /**
         * Assigns a module to the grouping by ID.<br>
         * Fails if the module is already contained within another grouping.
         *
         * @param[in] module_id - The ID of the module to assign.
         * @returns True on success, false otherwise.
         */
        bool assign_module_by_id(const u32 module_id);

        /**
         * Get all modules contained within the grouping. <br>
         * A filter can be applied to the result to only get modules matching the specified condition.
         *
         * @param[in] filter - Filter to be applied to the modules.
         * @returns A vector of modules.
         */
        std::vector<Module*> get_modules(const std::function<bool(Module*)>& filter = nullptr);

        /**
         * Get the IDs of all modules contained within the grouping. <br>
         * A filter can be applied to the result to only get module IDs for modules matching the specified condition.
         *
         * @param[in] filter - Filter to be applied to the modules.
         * @returns A vector of module IDs.
         */
        std::vector<u32> get_module_ids(const std::function<bool(Module*)>& filter = nullptr);

        /**
         * Removes a module from the grouping.<br>
         * Fails if the module is not contained within the grouping.
         *
         * @param[in] module - The module to remove.
         * @returns True on success, false otherwise.
         */
        bool remove_module(Module* module);

        /**
         * Removes a module from the grouping by ID.<br>
         * Fails if the module is not contained within the grouping.
         *
         * @param[in] module_id - The ID of the module to remove.
         * @returns True on success, false otherwise.
         */
        bool remove_module_by_id(const u32 module_id);

        /**
         * Checks whether a module is in the grouping.<br>
         *
         * @param[in] module - The module to check for.
         * @returns True on success, false otherwise.
         */
        bool contains_module(Module* module);

        /**
         * Checks whether a module is in the grouping by ID.<br>
         *
         * @param[in] module_id - The ID of the module to check for.
         * @returns True on success, false otherwise.
         */
        bool contains_module_by_id(const u32 module_id);

    private:
        friend class NetlistInternalManager;

        Grouping(NetlistInternalManager* internal_manager, u32 id, std::string name);

        u32 m_id;
        std::string m_name;
        NetlistInternalManager* m_internal_manager;

        std::vector<Gate*> m_gates;
        std::unordered_map<u32, Gate*> m_gates_map;
        std::vector<Net*> m_nets;
        std::unordered_map<u32, Net*> m_nets_map;
        std::vector<Module*> m_modules;
        std::unordered_map<u32, Module*> m_modules_map;
    };
}    // namespace hal