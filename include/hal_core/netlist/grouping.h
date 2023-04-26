// MIT License
//
// Copyright (c) 2019 Ruhr University Bochum, Chair for Embedded Security. All Rights reserved.
// Copyright (c) 2019 Marc Fyrbiak, Sebastian Wallat, Max Hoffmann ("ORIGINAL AUTHORS"). All rights reserved.
// Copyright (c) 2021 Max Planck Institute for Security and Privacy. All Rights reserved.
// Copyright (c) 2021 Jörn Langheinrich, Julian Speith, Nils Albartus, René Walendy, Simon Klix ("ORIGINAL AUTHORS"). All Rights reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once

#include "hal_core/netlist/event_system/event_handler.h"
#include "hal_core/netlist/netlist_internal_manager.h"
#include "hal_core/utilities/utils.h"

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
        u32 get_id() const;

        /**
         * Set the name of the grouping.
         *
         * @param[in] name - The new name.
         */
        void set_name(std::string name);

        /**
         * Get the name of the grouping.
         *
         * @returns The name.
         */
        std::string get_name() const;

        /**
         * Set the color of the grouping.
         *
         * @param[in] c - The new color.
         */
        void set_color(utils::Color c);

        /**
         * Get the color of the grouping.
         *
         * @returns The color.
         */
        utils::Color get_color() const;

        /**
         * Generates a new color distinct from previous colors. Called for each new grouping.
         *
         * @returns The next color.
         */
        utils::Color next_color();

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
         * Assign a gate to the grouping.<br>
         * Fails if the gate is already contained within another grouping.<br>
         * If \p force is set and the gate is contained in another grouping, it is removed from the previous grouping to be assigned to this one.
         *
         * @param[in] gate - The gate to assign.
         * @param[in] force - Overwrite previous assignment.
         * @returns True on success, false otherwise.
         */
        bool assign_gate(Gate* gate, bool force = false);

        /**
         * Assign a gate to the grouping by ID.<br>
         * Fails if the gate is already contained within another grouping.<br>
         * If \p force is set and the gate is contained in another grouping, it is removed from the previous grouping to be assigned to this one.
         *
         * @param[in] gate_id - The ID of the gate to assign.
         * @param[in] force - Overwrite previous assignment.
         * @returns True on success, false otherwise.
         */
        bool assign_gate_by_id(const u32 gate_id, bool force = false);

        /**
         * Get all gates contained within the grouping.
         *
         * @returns A vector of gates.
         */
        const std::vector<Gate*>& get_gates() const;

        /**
         * Get all gates contained within the grouping.
         * The filter is evaluated on every candidate such that the result only contains those matching the specified condition.
         *
         * @param[in] filter - Filter function to be evaluated on each gate.
         * @returns A vector of gates.
         */
        std::vector<Gate*> get_gates(const std::function<bool(Gate*)>& filter) const;

        /**
         * Get the IDs of all gates contained within the grouping.
         * The optional filter is evaluated on every candidate such that the result only contains those matching the specified condition.
         *
         * @param[in] filter - Filter function to be evaluated on each gate.
         * @returns A vector of gate IDs.
         */
        std::vector<u32> get_gate_ids(const std::function<bool(Gate*)>& filter = nullptr) const;

        /**
         * Remove a gate from the grouping.<br>
         * Fails if the gate is not contained within the grouping.
         *
         * @param[in] gate - The gate to remove.
         * @returns True on success, false otherwise.
         */
        bool remove_gate(Gate* gate);

        /**
         * Remove a gate from the grouping by ID.<br>
         * Fails if the gate is not contained within the grouping.
         *
         * @param[in] gate_id - The ID of the gate to remove.
         * @returns True on success, false otherwise.
         */
        bool remove_gate_by_id(const u32 gate_id);

        /**
         * Check whether a gate is in the grouping.
         *
         * @param[in] gate - The gate to check for.
         * @returns True on success, false otherwise.
         */
        bool contains_gate(Gate* gate) const;

        /**
         * Check whether a gate is in the grouping by ID.
         *
         * @param[in] gate_id - The ID of the gate to check for.
         * @returns True on success, false otherwise.
         */
        bool contains_gate_by_id(const u32 gate_id) const;

        /*
         * ################################################################
         *      net functions
         * ################################################################
         */

        /**
         * Assign a net to the grouping.<br>
         * Fails if the net is already contained within another grouping.<br>
         * If \p force is set and the net is contained in another grouping, it is removed from the previous grouping to be assigned to this one.
         *
         * @param[in] net - The net to assign.
         * @param[in] force - Overwrite previous assignment.
         * @returns True on success, false otherwise.
         */
        bool assign_net(Net* net, bool force = false);

        /**
         * Assign a net to the grouping by ID.<br>
         * Fails if the net is already contained within another grouping.<br>
         * If \p force is set and the net is contained in another grouping, it is removed from the previous grouping to be assigned to this one.
         *
         * @param[in] net_id - The ID of the net to assign.
         * @param[in] force - Overwrite previous assignment.
         * @returns True on success, false otherwise.
         */
        bool assign_net_by_id(const u32 net_id, bool force = false);

        /**
         * Get all nets contained within the grouping.
         *
         * @returns A vector of nets.
         */
        const std::vector<Net*>& get_nets() const;

        /**
         * Get all nets contained within the grouping.
         * The filter is evaluated on every candidate such that the result only contains those matching the specified condition.
         *
         * @param[in] filter - Filter function to be evaluated on each net.
         * @returns A vector of nets.
         */
        std::vector<Net*> get_nets(const std::function<bool(Net*)>& filter) const;

        /**
         * Get the IDs of all nets contained within the grouping.
         * The optional filter is evaluated on every candidate such that the result only contains those matching the specified condition.
         *
         * @param[in] filter - Filter function to be evaluated on each net.
         * @returns A vector of net IDs.
         */
        std::vector<u32> get_net_ids(const std::function<bool(Net*)>& filter = nullptr) const;

        /**
         * Remove a net from the grouping.<br>
         * Fails if the net is not contained within the grouping.
         *
         * @param[in] net - The net to remove.
         * @returns True on success, false otherwise.
         */
        bool remove_net(Net* net);

        /**
         * Remove a net from the grouping by ID.<br>
         * Fails if the net is not contained within the grouping.
         *
         * @param[in] net_id - The ID of the net to remove.
         * @returns True on success, false otherwise.
         */
        bool remove_net_by_id(const u32 net_id);

        /**
         * Check whether a net is in the grouping.<br>
         *
         * @param[in] net - The net to check for.
         * @returns True on success, false otherwise.
         */
        bool contains_net(Net* net) const;

        /**
         * Check whether a net is in the grouping by ID.<br>
         *
         * @param[in] net_id - The ID of the net to check for.
         * @returns True on success, false otherwise.
         */
        bool contains_net_by_id(const u32 net_id) const;

        /*
         * ################################################################
         *      module functions
         * ################################################################
         */

        /**
         * Assign a module to the grouping.<br>
         * Fails if the module is already contained within another grouping.<br>
         * If \p force is set and the module is contained in another grouping, it is removed from the previous grouping to be assigned to this one.
         *
         * @param[in] module - The module to assign.
         * @param[in] force - Overwrite previous assignment.
         * @returns True on success, false otherwise.
         */
        bool assign_module(Module* module, bool force = false);

        /**
         * Assign a module to the grouping by ID.<br>
         * Fails if the module is already contained within another grouping.<br>
         * If \p force is set and the module is contained in another grouping, it is removed from the previous grouping to be assigned to this one.
         *
         * @param[in] module_id - The ID of the module to assign.
         * @param[in] force - Overwrite previous assignment.
         * @returns True on success, false otherwise.
         */
        bool assign_module_by_id(const u32 module_id, bool force = false);

        /**
         * Get all modules contained within the grouping.
         *
         * @returns A vector of modules.
         */
        const std::vector<Module*>& get_modules() const;

        /**
         * Get all modules contained within the grouping.
         * The filter is evaluated on every candidate such that the result only contains those matching the specified condition.
         *
         * @param[in] filter - Filter function to be evaluated on each module.
         * @returns A vector of modules.
         */
        std::vector<Module*> get_modules(const std::function<bool(Module*)>& filter) const;

        /**
         * Get the IDs of all modules contained within the grouping.
         * The optional filter is evaluated on every candidate such that the result only contains those matching the specified condition.
         *
         * @param[in] filter - Filter function to be evaluated on each module.
         * @returns A vector of module IDs.
         */
        std::vector<u32> get_module_ids(const std::function<bool(Module*)>& filter = nullptr) const;

        /**
         * Remove a module from the grouping.<br>
         * Fails if the module is not contained within the grouping.
         *
         * @param[in] module - The module to remove.
         * @returns True on success, false otherwise.
         */
        bool remove_module(Module* module);

        /**
         * Remove a module from the grouping by ID.<br>
         * Fails if the module is not contained within the grouping.
         *
         * @param[in] module_id - The ID of the module to remove.
         * @returns True on success, false otherwise.
         */
        bool remove_module_by_id(const u32 module_id);

        /**
         * Check whether a module is in the grouping.<br>
         *
         * @param[in] module - The module to check for.
         * @returns True on success, false otherwise.
         */
        bool contains_module(Module* module) const;

        /**
         * Check whether a module is in the grouping by ID.<br>
         *
         * @param[in] module_id - The ID of the module to check for.
         * @returns True on success, false otherwise.
         */
        bool contains_module_by_id(const u32 module_id) const;

    private:
        friend class NetlistInternalManager;

        Grouping(NetlistInternalManager* internal_manager, EventHandler* event_handler, u32 id, std::string name);

        u32 m_id;
        std::string m_name;
        utils::Color m_color;
        NetlistInternalManager* m_internal_manager;

        std::vector<Gate*> m_gates;
        std::unordered_map<u32, Gate*> m_gates_map;
        std::vector<Net*> m_nets;
        std::unordered_map<u32, Net*> m_nets_map;
        std::vector<Module*> m_modules;
        std::unordered_map<u32, Module*> m_modules_map;

        EventHandler* m_event_handler;
    };
}    // namespace hal
