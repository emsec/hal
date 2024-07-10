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

#include "hal_core/defines.h"
#include "hal_core/netlist/boolean_function.h"
#include "hal_core/netlist/data_container.h"
#include "hal_core/netlist/endpoint.h"
#include "hal_core/netlist/event_system/event_handler.h"
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
         * Hash function for python binding.
         *
         * @return Pybind11 compatible hash.
         */
        ssize_t get_hash() const;

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
         * Get all modules that contain this gate, either directly or as parent of another module.
         * If `recursive` is set to true, indirect parent modules are also included. Otherwise, only the module containing the gate directly is returned.<br>
         * The optional filter is evaluated on every candidate such that the result only contains those matching the specified condition.
         *
         * @param[in] filter - An optional filter.
         * @param[in] recursive - Set `true` to include indirect parents as well, `false` otherwise.
         * @returns A vector of modules.
         */
        std::vector<Module*> get_modules(const std::function<bool(Module*)>& filter = nullptr, bool recursive = true) const;

        /**
         * Gets the grouping in which this gate is contained.<br>
         * If no grouping contains this gate, a nullptr is returned.
         *
         * @returns The grouping.
         */
        Grouping* get_grouping() const;

        /**
         * Get the Boolean function specified by the given name.
         * This name can for example be an output pin of the gate or any other user-defined function name.
         *
         * @param[in] name - The name.
         * @returns The Boolean function on success, an empty Boolean function otherwise.
         */
        BooleanFunction get_boolean_function(const std::string& name) const;

        /**
          * Get the Boolean function corresponding to the given output pin.
          * If `pin` is a `nullptr`, the Boolean function of the first output pin is returned.
          *
          * @param[in] pin - The pin.
          * @returns The Boolean function on success, an empty Boolean function otherwise.
         */
        BooleanFunction get_boolean_function(const GatePin* pin = nullptr) const;

        /**
         * Get a map from function name to Boolean function for all boolean functions associated with this gate.
         *
         * @param[in] only_custom_functions - Set `true` to get only Boolean functions that are local to the gate, `false` otherwise.
         * @returns A map from function name to function on success, an empty map otherwise.
         */
        std::unordered_map<std::string, BooleanFunction> get_boolean_functions(bool only_custom_functions = false) const;

        /**
          * Get the resolved Boolean function corresponding to the given output pin, i.e., a Boolean function that only depends on input pins (or nets) and no internal or output pins.
          * If fan-in nets are used to derive variable names, the variable names are generated using the `BooleanFunctionNetDecorator`.
          *
          * @param[in] pin - The output pin.
          * @param[in] use_net_variables - Set `true` to use variable names derived from fan-in nets of the gate, `false` to use input pin names instead. Defaults to `false`.
          * @returns The Boolean function on success, an error otherwise.
          */
        Result<BooleanFunction> get_resolved_boolean_function(const GatePin* pin, const bool use_net_variables = false) const;

        /**
         * Add a Boolean function with the given name to the gate.
         *
         * @param[in] name - The name.
         * @param[in] func - The function.
         */
        bool add_boolean_function(const std::string& name, const BooleanFunction& func);

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
         * Get a vector of all fan-in nets of the gate, i.e., all nets that are connected to one of the input pins.
         *
         * @returns A vector of all fan-in nets.
         */
        const std::vector<Net*>& get_fan_in_nets() const;

        /**
         * Get a vector of all fan-in nets of the gate, i.e., all nets that are connected to one of the input pins.
         * The filter is evaluated on every candidate such that the result only contains those matching the specified condition.
         *
         * @param[in] filter - Filter function to be evaluated on each net.
         * @returns A vector of all fan-in nets.
         */
        std::vector<Net*> get_fan_in_nets(const std::function<bool(Net*)>& filter) const;

        /**
         * Get the fan-in net corresponding to the input pin specified by name.
         *
         * @param[in] pin_name - The input pin name.
         * @returns The fan-in net on success, a `nullptr` otherwise.
         */
        Net* get_fan_in_net(const std::string& pin_name) const;

        /**
         * Get the fan-in net corresponding to the specified input pin.
         *
         * @param[in] pin - The input pin.
         * @returns The fan-in net on success, a nullptr otherwise.
         */
        Net* get_fan_in_net(const GatePin* pin) const;

        /**
         * Check whether the given net is a fan-in of the gate.
         * 
         * @param[in] net - The net. 
         * @returns `true` if the net is a fan-in of the gate, `false` otherwise. 
         */
        bool is_fan_in_net(const Net* net) const;

        /**
         * Get a vector of all fan-in endpoints of the gate, i.e., all endpoints associated with an input pin of the gate.
         *
         * @returns A vector of all fan-in endpoints.
         */
        const std::vector<Endpoint*>& get_fan_in_endpoints() const;

        /**
         * Get a vector of all fan-in endpoints of the gate, i.e., all endpoints associated with an input pin of the gate.
         * The filter is evaluated on every candidate such that the result only contains those matching the specified condition.
         *
         * @param[in] filter - Filter function to be evaluated on each endpoint.
         * @returns A vector of all fan-in endpoints.
         */
        std::vector<Endpoint*> get_fan_in_endpoints(const std::function<bool(Endpoint*)>& filter) const;

        /**
         * Get the fan-in endpoint corresponding to the input pin specified by name.
         *
         * @param[in] pin_name - The input pin name.
         * @returns The endpoint on success, a `nullptr` otherwise.
         */
        Endpoint* get_fan_in_endpoint(const std::string& pin_name) const;

        /**
         * Get the fan-in endpoint corresponding to the specified input pin.
         *
         * @param[in] pin - The input pin.
         * @returns The endpoint on success, a `nullptr` otherwise.
         */
        Endpoint* get_fan_in_endpoint(const GatePin* pin) const;

        /**
         * Get the fan-in endpoint connected to the specified input net.
         *
         * @param[in] net - The input net.
         * @returns The endpoint on success, a `nullptr` otherwise.
         */
        Endpoint* get_fan_in_endpoint(const Net* net) const;

        /**
         * Get a vector of all fan-out nets of the gate, i.e., all nets that are connected to one of the output pins.
         *
         * @returns A vector of all fan-out nets.
         */
        const std::vector<Net*>& get_fan_out_nets() const;

        /**
         * Get a vector of all fan-out nets of the gate, i.e., all nets that are connected to one of the output pins.
         * The filter is evaluated on every candidate such that the result only contains those matching the specified condition.
         *
         * @param[in] filter - Filter function to be evaluated on each net.
         * @returns A vector of all fan-out nets.
         */
        std::vector<Net*> get_fan_out_nets(const std::function<bool(Net*)>& filter) const;

        /**
         * Get the fan-out net corresponding to the output pin specified by name.
         *
         * @param[in] pin_name - The output pin name.
         * @returns The fan-out net on success, a `nullptr` otherwise.
         */
        Net* get_fan_out_net(const std::string& pin_name) const;

        /**
         * Get the fan-out net corresponding to the specified output pin.
         *
         * @param[in] pin - The output pin.
         * @returns The fan-out net on success, a nullptr otherwise.
         */
        Net* get_fan_out_net(const GatePin* pin) const;

        /**
         * Check whether the given net is a fan-out of the gate.
         * 
         * @param[in] net - The net. 
         * @returns `true` if the net is a fan-out of the gate, `false` otherwise. 
         */
        bool is_fan_out_net(const Net* net) const;

        /**
         * Get a vector of all fan-out endpoints of the gate, i.e., all endpoints associated with an output pin of the gate.
         *
         * @returns A vector of all fan-out endpoints.
         */
        const std::vector<Endpoint*>& get_fan_out_endpoints() const;

        /**
         * Get a vector of all fan-out endpoints of the gate, i.e., all endpoints associated with an output pin of the gate.
         * The filter is evaluated on every candidate such that the result only contains those matching the specified condition.
         *
         * @param[in] filter - Filter function to be evaluated on each endpoint.
         * @returns A vector of all fan-out endpoints.
         */
        std::vector<Endpoint*> get_fan_out_endpoints(const std::function<bool(Endpoint*)>& filter) const;

        /**
         * Get the fan-out endpoint corresponding to the output pin specified by name.
         *
         * @param[in] pin_name - The output pin name.
         * @returns The endpoint on success, a `nullptr` otherwise.
         */
        Endpoint* get_fan_out_endpoint(const std::string& pin_name) const;

        /**
         * Get the fan-out endpoint corresponding to the specified input pin.
         *
         * @param[in] pin - The output pin.
         * @returns The endpoint on success, a `nullptr` otherwise.
         */
        Endpoint* get_fan_out_endpoint(const GatePin* pin) const;

        /**
         * Get the fan-out endpoint connected to the specified output net.
         *
         * @param[in] net - The output net.
         * @returns The endpoint on success, a `nullptr` otherwise.
         */
        Endpoint* get_fan_out_endpoint(const Net* net) const;

        /**
         * Get a vector of all unique predecessor gates of the gate. 
         * The optional filter is evaluated on every candidate such that the result only contains those matching the specified condition.
         *
         * @param[in] filter - An optional filter being evaluated on the gate's input pin as well as the predecessor endpoint.
         * @returns A vector of unique predecessor gates.
         */
        std::vector<Gate*> get_unique_predecessors(const std::function<bool(const GatePin* pin, Endpoint* ep)>& filter = nullptr) const;

        /**
         * Get a vector of all direct predecessor endpoints of the gate, i.e., all predecessor endpoints that are connected to an input pin of the gate. 
         * The optional filter is evaluated on every candidate such that the result only contains those matching the specified condition.
         *
         * @param[in] filter - An optional filter being evaluated on the gate's input pin as well as the predecessor endpoint.
         * @returns A vector of predecessor endpoints.
         */
        std::vector<Endpoint*> get_predecessors(const std::function<bool(const GatePin* pin, Endpoint* ep)>& filter = nullptr) const;

        /**
         * Get a single direct predecessor endpoint that is connected to the input pin specified by name.
         * Fails if there are no or more than one predecessors.
         *
         * @param[in] pin_name - The input pin name.
         * @returns The predecessor endpoint on success, a `nullptr` otherwise.
         */
        Endpoint* get_predecessor(const std::string& pin_name) const;

        /**
         * Get a single direct predecessor endpoint that is connected to the specified input pin.
         * Fails if there are no or more than one predecessors.
         *
         * @param[in] pin - The input pin.
         * @returns The predecessor endpoint on success, a `nullptr` otherwise.
         */
        Endpoint* get_predecessor(const GatePin* pin) const;

        /**
         * Get a vector of all unique successor gates of the gate. 
         * The optional filter is evaluated on every candidate such that the result only contains those matching the specified condition.
         *
         * @param[in] filter - An optional filter being evaluated on the gate's output pin as well as the successor endpoint.
         * @returns A vector of unique successor gates.
         */
        std::vector<Gate*> get_unique_successors(const std::function<bool(const GatePin* pin, Endpoint* ep)>& filter = nullptr) const;

        /**
         * Get a vector of all direct successor endpoints of the gate, i.e., all successor endpoints that are connected to an output pin of the gate. 
         * The optional filter is evaluated on every candidate such that the result only contains those matching the specified condition.
         *
         * @param[in] filter - An optional filter being evaluated on the gate's output pin as well as the successor endpoint.
         * @returns A vector of successor endpoints.
         */
        std::vector<Endpoint*> get_successors(const std::function<bool(const GatePin* pin, Endpoint* ep)>& filter = nullptr) const;

        /**
         * Get a single direct successor endpoint that is connected to the output pin specified by name.
         * Fails if there are no or more than one successors.
         *
         * @param[in] pin_name - The output pin name.
         * @returns The successor endpoint on success, a `nullptr` otherwise.
         */
        Endpoint* get_successor(const std::string& pin_name) const;

        /**
         * Get a single direct successor endpoint that is connected to the specified output pin.
         * Fails if there are no or more than one successors.
         *
         * @param[in] pin - The output pin.
         * @returns The successor endpoint on success, a `nullptr` otherwise.
         */
        Endpoint* get_successor(const GatePin* pin) const;

        /**
         * Get the INIT data of the gate, if available. 
         * An error is returned in case the gate does not hold any INIT data.
         * 
         * @return The INIT data as a vector on success, an error message otherwise.
         */
        Result<std::vector<std::string>> get_init_data() const;

        /**
         * Set the INIT data of the gate, if available. 
         * An error is returned in case the gate does not hold any INIT data.
         * 
         * @param[in] init_data - The INIT data as a vector.
         * @returns Ok on success, an error message otherwise.
         */
        Result<std::monostate> set_init_data(const std::vector<std::string>& init_data);

    private:
        friend class NetlistInternalManager;
        Gate(NetlistInternalManager* mgr, EventHandler* event_handler, u32 id, GateType* gt, const std::string& name, i32 x, i32 y);

        Gate(const Gate&)            = delete;
        Gate(Gate&&)                 = delete;
        Gate& operator=(const Gate&) = delete;
        Gate& operator=(Gate&&)      = delete;

        BooleanFunction get_lut_function(const GatePin* pin) const;

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
