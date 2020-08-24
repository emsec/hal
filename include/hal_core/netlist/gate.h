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

#include "hal_core/defines.h"
#include "hal_core/netlist/boolean_function.h"
#include "hal_core/netlist/data_container.h"
#include "hal_core/netlist/endpoint.h"
#include "hal_core/netlist/gate_library/gate_type/gate_type.h"

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
    class Endpoint;
    class NetlistInternalManager;

    /**
     * Gate class containing information about a gate including its location, functions, and module.
     *
     * @ingroup netlist
     */
    class NETLIST_API Gate : public DataContainer
    {
        friend class NetlistInternalManager;

    public:
        /**
         * Gets the unique id of the gate.
         *
         * @returns The gate's id.
         */
        u32 get_id() const;

        /**
         * Gets the parent netlist of the gate.
         *
         * @returns The netlist.
         */
        Netlist* get_netlist() const;

        /**
         * Gets the gate's name.
         *
         * @returns The name.
         */
        std::string get_name() const;

        /**
         * Sets the gate's name.
         *
         * @param[in] name - The new name.
         */
        void set_name(const std::string& name);

        /**
         * Gets the type of the gate.
         *
         * @returns The gate's type.
         */
        const GateType* get_type() const;

        /**
         * Checks whether the gate's location in the layout is available.
         *
         * @returns True if valid location data is available, false otherwise.
         */
        bool has_location() const;

        /**
         * Sets the physical location x-coordinate of the gate in the layout.
         * Only positive values are valid, negative values will be regarded as no location assigned.
         *
         * @param[in] x - The gate's x-coordinate.
         */
        void set_location_x(float x);

        /**
         * Gets the physical location x-coordinate of the gate in the layout.
         *
         * @returns The gate's x-coordinate.
         */
        float get_location_x() const;

        /**
         * Sets the physical location y-coordinate of the gate in the layout.
         * Only positive values are valid, negative values will be regarded as no location assigned.
         *
         * @param[in] y - The gate's y-coordinate.
         */
        void set_location_y(float y);

        /**
         * Gets the physical location y-coordinate of the gate in the layout.
         *
         * @returns The gate's y-coordinate.
         */
        float get_location_y() const;

        /**
         * Sets the physical location of the gate in the layout.
         *
         * @param[in] location - A pair <x-coordinate, y-coordinate>.
         */
        void set_location(const std::pair<float, float>& location);

        /**
         * Gets the physical location of the gate in the layout.
         *
         * @returns A pair <x-coordinate, y-coordinate>.
         */
        std::pair<float, float> get_location() const;

        /**
         * Gets the module in which this gate is contained.
         *
         * @returns The module.
         */
        Module* get_module() const;

        /**
         * Get the boolean function associated with a specific name.
         * This name can for example be an output pin of the gate or a defined functionality like "reset".
         * If name is empty, the function of the first output pin is returned.
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
        bool is_vcc_gate();

        /**
         * Checks whether this gate is a global gnd gate.
         *
         * @returns True if the gate is a global gnd gate.
         */
        bool is_gnd_gate();

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
         * Get a set of all fan-in nets of the gate, i.e. all nets that are connected to one of the input pins.
         *
         * @returns A set of all connected input nets.
         */
        std::vector<Net*> get_fan_in_nets() const;

        /**
         * Get a all fan-in endpoint of the gate, i.e. all nets that are connected to one of the input pins + the respective pin.
         *
         * @returns A set of all connected input endpoints.
         */
        std::vector<Endpoint> get_fan_in_endpoints() const;

        /**
         * Get the fan-in net which is connected to a specific input pin.
         *
         * @param[in] pin_type - The input pin type.
         * @returns The connected input net.
         */
        Net* get_fan_in_net(const std::string& pin_type) const;

        /**
         * Get a set of all fan-out nets of the gate, i.e. all nets that are connected to one of the output pins.
         *
         * @returns A set of all connected output nets.
         */
        std::vector<Net*> get_fan_out_nets() const;

        /**
         * Get a all fan-out endpoint of the gate, i.e. all nets that are connected to one of the output pins + the respective pin.
         *
         * @returns A set of all connected output endpoints.
         */
        std::vector<Endpoint> get_fan_out_endpoints() const;

        /**
         * Get the fan-out net which is connected to a specific output pin.
         *
         * @param[in] pin_type - The output pin type.
         * @returns The connected output net.
         */
        Net* get_fan_out_net(const std::string& pin_type) const;

        /**
         * Get a set of all unique predecessor gates of the gate.
         * A filter can be supplied which filters out all potential values that return false.
         *
         * @param[in] filter - a function to filter the output. Leave empty for no filtering.
         * @returns A vector of unique predecessor gates.
         */
        std::vector<Gate*> get_unique_predecessors(const std::function<bool(const std::string& starting_pin, const Endpoint& ep)>& filter = nullptr) const;

        /**
         * Get a vector of all direct predecessor endpoints of the gate.
         * A filter can be supplied which filters out all potential values that return false.
         *
         * @param[in] filter - a function to filter the output. Leave empty for no filtering.
         * @returns A vector of predecessor endpoints.
         */
        std::vector<Endpoint> get_predecessors(const std::function<bool(const std::string& starting_pin, const Endpoint& ep)>& filter = nullptr) const;

        /**
         * Get the direct predecessor endpoint of the gate connected to a specific input pin.
         *
         * @param[in] which_pin - the pin of this gate to get the predecessor from.
         * @returns The predecessor endpoint.
         */
        Endpoint get_predecessor(const std::string& which_pin) const;

        /**
         * Get a set of all unique successor gates of the gate.
         * A filter can be supplied which filters out all potential values that return false.
         *
         * @param[in] filter - a function to filter the output. Leave empty for no filtering.
         * @returns A vector of unique successor gates.
         */
        std::vector<Gate*> get_unique_successors(const std::function<bool(const std::string& starting_pin, const Endpoint& ep)>& filter = nullptr) const;

        /**
         * Get a vector of all direct successor endpoints of the gate.
         * A filter can be supplied which filters out all potential values that return false.
         *
         * @param[in] filter - a function to filter the output. Leave empty for no filtering.
         * @returns A vector of successor endpoints.
         */
        std::vector<Endpoint> get_successors(const std::function<bool(const std::string& starting_pin, const Endpoint& ep)>& filter = nullptr) const;

    private:
        Gate(NetlistInternalManager* mgr, u32 id, const GateType* gt, const std::string& name, float x, float y);

        Gate(const Gate&) = delete;               //disable copy-constructor
        Gate& operator=(const Gate&) = delete;    //disable copy-assignment

        BooleanFunction get_lut_function(const std::string& pin) const;

        /* pointer to corresponding netlist parent */
        NetlistInternalManager* m_internal_manager;

        /* id of the gate */
        u32 m_id;

        /* name of the gate */
        std::string m_name;

        /* type of the gate */
        const GateType* m_type;

        /* location */
        float m_x;
        float m_y;

        /* owning module */
        Module* m_module;

        /* connected nets */
        std::vector<Endpoint> m_in_endpoints;
        std::vector<Endpoint> m_out_endpoints;
        std::vector<Net*> m_in_nets;
        std::vector<Net*> m_out_nets;

        /* dedicated functions */
        std::map<std::string, BooleanFunction> m_functions;
    };
}    // namespace hal
