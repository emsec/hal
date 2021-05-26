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
#include "hal_core/netlist/boolean_function.h"
#include "hal_core/netlist/data_container.h"
#include "hal_core/netlist/endpoint.h"
#include "hal_core/netlist/event_handler.h"
#include "hal_core/netlist/gate_library/gate_type.h"

#include <functional>
#include <map>
#include <memory>
#include <set>
#include <unordered_map>
#include <vector>

namespace hal
{
    /* forward declaration */
    class Netlist;
    class Net;
    class Module;
    class Grouping;
    class Endpoint;
    class NetlistInternalManager;

    /**
     * Gate class containing information about a gate including its location, functions, and module.
     *
     * @ingroup netlist
     */
    class NETLIST_API Gate : public DataContainer
    {
    public:
        /**
         * Check whether two gates are equal.
         * Does not check for connected nets or containing module.
         *
         * @param[in] other - The gate to compare against.
         * @returns True if both gates are equal, false otherwise.
         */
        bool operator==(const Gate& other) const;

        /**
         * Check whether two gates are unequal.
         * Does not check for connected nets or containing module.
         *
         * @param[in] other - The gate to compare against.
         * @returns True if both gates are unequal, false otherwise.
         */
        bool operator!=(const Gate& other) const;

        /**
         * Get the unique id of the gate.
         *
         * @returns The unique id.
         */
        u32 get_id() const;

        /**
         * Get the netlist this gate is associated with.
         *
         * @returns The netlist.
         */
        Netlist* get_netlist() const;

        /**
         * Get the name of the gate.
         *
         * @returns The name.
         */
        const std::string& get_name() const;

        /**
         * Set the name of the gate.
         *
         * @param[in] name - The new name.
         */
        void set_name(const std::string& name);

        /**
         * Get the type of the gate.
         *
         * @returns The type.
         */
        GateType* get_type() const;

        /**
         * Check whether the gate's location in the layout is available.
         *
         * @returns True if valid location data is available, false otherwise.
         */
        bool has_location() const;

        /**
         * Set the physical location x-coordinate of the gate in the layout.<br>
         * Only positive values are valid, negative values will be regarded as no location assigned.
         *
         * @param[in] x - The gate's x-coordinate.
         */
        void set_location_x(i32 x);

        /**
         * Get the physical location x-coordinate of the gate in the layout.<br>
         * If no valid physical location x-coordinate is assigned, a negative value is returned.
         *
         * @returns The gate's x-coordinate.
         */
        i32 get_location_x() const;

        /**
         * Set the physical location y-coordinate of the gate in the layout.<br>
         * Only positive values are valid, negative values will be regarded as no location assigned.
         *
         * @param[in] y - The gate's y-coordinate.
         */
        void set_location_y(i32 y);

        /**
         * Get the physical location y-coordinate of the gate in the layout.<br>
         * If no valid physical location y-coordinate is assigned, a negative value is returned.
         *
         * @returns The gate's y-coordinate.
         */
        i32 get_location_y() const;

        /**
         * Set the physical location of the gate in the layout.<br>
         * Only positive coordinates are valid, negative values will be regarded as no location assigned.
         *
         * @param[in] location - A pair <x-coordinate, y-coordinate>.
         */
        void set_location(const std::pair<i32, i32>& location);

        /**
         * Get the physical location of the gate in the layout.<br>
         * If no valid physical location coordinate is assigned, a negative value is returned for the
         * respective coordinate.
         *
         * @returns A pair <x-coordinate, y-coordinate>.
         */
        std::pair<i32, i32> get_location() const;

        /**
         * Get the module which contains this gate.
         *
         * @returns The module.
         */
        Module* get_module() const;

        /**
         * Gets the grouping in which this gate is contained.<br>
         * If no grouping contains this gate, a nullptr is returned.
         *
         * @returns The grouping.
         */
        Grouping* get_grouping() const;

        /**
         * Get the boolean function associated with a specific name.
         * This name can for example be an output pin of the gate or a defined functionality like "reset".<br>
         * If name is empty, the function of the first output pin is returned.<br>
         * If there is no function for the given name, an empty function is returned.
         *
         * @param[in] name - The function name.
         * @returns The boolean function.
         */
        BooleanFunction get_boolean_function(std::string name = "") const;

        /**
         * Get a map from function name to boolean function for all boolean functions associated with this gate.
         *
         * @param[in] only_custom_functions - If true, this returns only the functions which were set via add_boolean_function.
         * @returns A map from function name to function.
         */
        std::unordered_map<std::string, BooleanFunction> get_boolean_functions(bool only_custom_functions = false) const;

        /**
         * Add the boolean function with the specified name only for this gate.
         *
         * @param[in] name - The function name, usually an output port.
         * @param[in] func - The function.
         */
        void add_boolean_function(const std::string& name, const BooleanFunction& func);

        /**
         * Mark this gate as a global vcc gate.
         *
         * @returns True on success.
         */
        bool mark_vcc_gate();

        /**
         * Mark this gate as a global gnd gate.
         *
         * @returns True on success.
         */
        bool mark_gnd_gate();

        /**
         * Unmark this gate as a global vcc gate.
         *
         * @returns True on success.
         */
        bool unmark_vcc_gate();

        /**
         * Unmark this gate as a global gnd gate.
         *
         * @returns True on success.
         */
        bool unmark_gnd_gate();

        /**
         * Checks whether this gate is a global vcc gate.
         *
         * @returns True if the gate is a global vcc gate.
         */
        bool is_vcc_gate() const;

        /**
         * Checks whether this gate is a global gnd gate.
         *
         * @returns True if the gate is a global gnd gate.
         */
        bool is_gnd_gate() const;

        /*
         *      pin specific functions
         */

        /**
         * Get a list of all input pin types of the gate.
         *
         * @returns A vector of input pin types.
         */
        std::vector<std::string> get_input_pins() const;

        /**
         * Get a list of all output pin types of the gate.
         *
         * @returns A vector of output pin types.
         */
        std::vector<std::string> get_output_pins() const;

        /**
         * Get a list of all fan-in nets of the gate, i.e., all nets that are connected to one of the input pins.
         *
         * @returns A vector of all connected input nets.
         */
        std::vector<Net*> get_fan_in_nets() const;

        /**
         * Get a list of all fan-in endpoints of the gate, i.e., all connected endpoints that represent an input pin of this gate.
         *
         * @returns A vector of all connected input endpoints.
         */
        std::vector<Endpoint*> get_fan_in_endpoints() const;

        /**
         * Get the fan-in net which is connected to a specific input pin. <br>
         * If the input pin type is unknown or no net is connected, a nullptr is returned.
         *
         * @param[in] pin - The input pin type.
         * @returns The connected input net.
         */
        Net* get_fan_in_net(const std::string& pin) const;

        /**
         * Get the fan-in endpoint which represents a specific input pin. <br>
         * If the input pin type is unknown or no net is connected to the respective pin, a nullptr is returned.
         *
         * @param[in] pin - The input pin type.
         * @returns The endpoint.
         */
        Endpoint* get_fan_in_endpoint(const std::string& pin) const;

        /**
         * Get a list of all fan-out nets of the gate, i.e., all nets that are connected to one of the output pins.
         *
         * @returns A vector of all connected output nets.
         */
        std::vector<Net*> get_fan_out_nets() const;

        /**
         * Get a list of all fan-out endpoints of the gate, i.e., all connected endpoints that represent an output pin of this gate.
         *
         * @returns A vector of all connected output endpoints.
         */
        std::vector<Endpoint*> get_fan_out_endpoints() const;

        /**
         * Get the fan-out net which is connected to a specific output pin. <br>
         * If the output pin type is unknown or no net is connected, a nullptr is returned.
         *
         * @param[in] pin - The output pin type.
         * @returns The connected output net.
         */
        Net* get_fan_out_net(const std::string& pin) const;

        /**
         * Get the fan-out endpoint which represents a specific output pin. <br>
         * If the input pin type is unknown or no net is connected to the respective pin, a nullptr is returned.
         *
         * @param[in] pin - The output pin type.
         * @returns The endpoint.
         */
        Endpoint* get_fan_out_endpoint(const std::string& pin) const;

        /**
         * Get a list of all unique predecessor gates of the gate. <br>
         * A filter can be supplied which filters out all potential values that return false.
         *
         * @param[in] filter - A function to filter the output, using the input pin type of the gate (1st param) and
         *                     a connected predecessor endpoint (2nd param). Leave empty for no filtering.
         * @returns A vector of unique predecessor gates.
         */
        std::vector<Gate*> get_unique_predecessors(const std::function<bool(const std::string& starting_pin, Endpoint* ep)>& filter = nullptr) const;

        /**
         * Get a list of all direct predecessor endpoints of the gate,
         * i.e. all output endpoints that are connected to an input pin of the gate. <br>
         * A filter can be supplied which filters out all potential values that return false.
         *
         * @param[in] filter - A function to filter the output, using the input pin type of the gate (1st param) and
         *                     a connected predecessor endpoint (2nd param). Leave empty for no filtering.
         * @returns A vector of predecessor endpoints.
         */
        std::vector<Endpoint*> get_predecessors(const std::function<bool(const std::string& starting_pin, Endpoint* ep)>& filter = nullptr) const;

        /**
         * Get the direct predecessor endpoint of the gate connected to a specific input pin. <br>
         * If the input pin type is unknown or there is no predecessor endpoint or there are multiple predecessor
         * endpoints, a nullptr is returned.
         *
         * @param[in] which_pin - the input pin type of this gate to get the predecessor from.
         * @returns The predecessor endpoint.
         */
        Endpoint* get_predecessor(const std::string& which_pin) const;

        /**
         * Get a list of all unique successor gates of the gate. <br>
         * A filter can be supplied which filters out all potential values that return false.
         *
         * @param[in] filter - A function to filter the output, using the output pin type of the gate (1st param) and
         *                     a connected successor endpoint (2nd param). Leave empty for no filtering.
         * @returns A vector of unique successor gates.
         */
        std::vector<Gate*> get_unique_successors(const std::function<bool(const std::string& starting_pin, Endpoint* ep)>& filter = nullptr) const;

        /**
         * Get a list of all direct successor endpoints of the gate. <br>
         * A filter can be supplied which filters out all potential values that return false.
         *
         * @param[in] filter - A function to filter the output, using the output pin type of the gate (1st param) and
         *                     a connected successor endpoint (2nd param). Leave empty for no filtering.
         * @returns A vector of successor endpoints.
         */
        std::vector<Endpoint*> get_successors(const std::function<bool(const std::string& starting_pin, Endpoint* ep)>& filter = nullptr) const;

        /**
         * Get the direct successor endpoint of the gate connected to a specific input pin. <br>
         * If the input pin type is unknown or there is no successor endpoint or there are multiple successor
         * endpoints, a nullptr is returned.
         *
         * @param[in] which_pin - The output pin type of this gate to get the successor from.
         * @returns The successor endpoint.
         */
        Endpoint* get_successor(const std::string& which_pin) const;

    private:
        friend class NetlistInternalManager;
        Gate(NetlistInternalManager* mgr, EventHandler* event_handler, u32 id, GateType* gt, const std::string& name, i32 x, i32 y);

        Gate(const Gate&) = delete;
        Gate(Gate&&)      = delete;
        Gate& operator=(const Gate&) = delete;
        Gate& operator=(Gate&&) = delete;

        BooleanFunction get_lut_function(const std::string& pin) const;

        /* pointer to corresponding netlist parent */
        NetlistInternalManager* m_internal_manager;

        /* id of the gate */
        u32 m_id;

        /* name of the gate */
        std::string m_name;

        /* type of the gate */
        GateType* m_type;

        /* location */
        i32 m_x = -1;
        i32 m_y = -1;

        /* owning module */
        Module* m_module;

        /* grouping */
        Grouping* m_grouping = nullptr;

        /* connected nets */
        std::vector<Endpoint*> m_in_endpoints;
        std::vector<Endpoint*> m_out_endpoints;
        std::vector<Net*> m_in_nets;
        std::vector<Net*> m_out_nets;

        /* dedicated functions */
        std::unordered_map<std::string, BooleanFunction> m_functions;

        EventHandler* m_event_handler;
    };
}    // namespace hal
