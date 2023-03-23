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

#include "hal_core/netlist/boolean_function.h"
#include "hal_core/netlist/endpoint.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/gate_library/gate_library.h"
#include "hal_core/netlist/grouping.h"
#include "hal_core/netlist/module.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/utilities/utils.h"
#include "test_def.h"

#include <fstream>
#include <math.h>
#include <stack>

namespace hal
{
    namespace test_utils
    {
        /**
         * Create an empty netlist using the default testing gate library.
         *
         * @param[in] id - ID of the netlist
         * @returns An empty netlist
         */
        std::unique_ptr<Netlist> create_empty_netlist(const u32 id = 0);

        /**
         * Create a connection between two gates.
         * 
         * @param[in] netlist - The netlist.
         * @param[in] src_gate - The source gate.
         * @param[in] src_pin - The source pin at the specified source gate.
         * @param[in] dst_gate - The destination gate.
         * @param[in] dst_pin - The destination pin at the specified destination gate.
         * @param[in] net_name - The name of the net, defaults to IDs of connected gates.
         */
        Net* connect(Netlist* netlist, Gate* src_gate, const std::string& src_pin, Gate* dst_gate, const std::string& dst_pin, const std::string& net_name = "");

        /**
         * Create a connection to a global input.
         * 
         * @param[in] netlist - The netlist.
         * @param[in] dst_gate - The destination gate.
         * @param[in] dst_pin - The destination pin at the specified destination gate.
         * @param[in] net_name - The name of the net, defaults to ID of connected gate.
         */
        Net* connect_global_in(Netlist* netlist, Gate* dst_gate, const std::string& dst_pin, const std::string& net_name = "");

        /**
         * Create a connection to a global input.
         * 
         * @param[in] netlist - The netlist.
         * @param[in] src_gate - The source gate.
         * @param[in] src_pin - The source pin at the specified source gate.
         * @param[in] net_name - The name of the net, defaults to ID of connected gate.
         */
        Net* connect_global_out(Netlist* netlist, Gate* src_gate, const std::string& src_pin, const std::string& net_name = "");

        /**
         * Remove all connections of the gate.
         * 
         * @param[in] gate - The gate.
         */
        void clear_connections(Gate* gate);

        /**
         * Remove all connections of the net.
         * 
         * @param[in] net - The net.
         */
        void clear_connections(Net* net);

        // TODO clean up everything below

        /*********************************************************
         *                      Constants                        *
         *********************************************************/

        // Minimum and invali ids for netlists, gates, nets and modules
        const u32 INVALID_GATE_ID     = 0;
        const u32 INVALID_NET_ID      = 0;
        const u32 INVALID_MODULE_ID   = 0;
        const u32 INVALID_GROUPING_ID = 0;

        const u32 MIN_MODULE_ID   = 2;
        const u32 MIN_GATE_ID     = 1;
        const u32 MIN_NET_ID      = 1;
        const u32 MIN_GROUPING_ID = 1;
        const u32 MIN_NETLIST_ID  = 1;
        const u32 TOP_MODULE_ID   = 1;

        // Name for accessing our example Gate library
        static const std::string g_lib_name = "EXAMPLE_GATE_LIBRARY.lib";
        //static const std::string g_lib_name = "example_library";
        // Name for accessing the custom Gate library after the call of 'create_temp_gate_lib()'
        const std::string temp_lib_name = "TEMP_GATE_LIBRARY.lib";

        const std::filesystem::path sandbox_directory_path = std::filesystem::path("tests/sandbox_directory");

        /*********************************************************
         *                      Functions                        *
         *********************************************************/

        /**
         * Checks if the tests that contain issues are activated. Used to mark those tests using something like:
         * if(test_utils::known_issue_tests_active()) { // Testcase with Issues ... }
         *
         * Remark: Those tests are deactivated by default. They can be also activated by using "--run_known_issue_tests"
         *         as a command line argument.
         *
         * @returns the flag set by de/activate_known_issue_tests()
         */
        bool known_issue_tests_active();

        /**
         * Activates the tests that are known for containing issues and that are most likely to fail.
         */
        void activate_known_issue_tests();

        /**
         * Deactivates the tests that are known for containing issues and that are most likely to fail.
         */
        [[maybe_unused]] void deactivate_known_issue_tests();

        /**
         * Initializes all log channels used by hal.
         */
        void init_log_channels();

        /**
         * Creating an Endpoint* object by passing the netlist, the id of the Gate and the pin name.
         * if there is no Gate with the passed id, it returns (nullptr, "")
         *
         * @param[in] nl - netlist
         * @param[in] gate_id - id of the Gate
         * @param[in] pin_name - pin type
         * @param[in] is_destination - direction of Endpoint
         * @returns the Endpoint* object
         */
        Endpoint* get_endpoint(Netlist* nl, const int gate_id, const std::string& pin_name, bool is_destination);

        /**
         * Creating an Endpoint* object by passing the Gate and the pin_name. The is_destination flag is taken from the
         * Gate library of the Gate and the netlist by the Gate.
         *
         * @param[in] gate_id - id of the Gate
         * @param[in] pin_name - pin type
         * @returns the Endpoint* object
         */
        Endpoint* get_endpoint(Gate* g, const std::string& pin_name);

        /**
         * Minimizes a truth table of a boolean function such that variables that do not matter are eliminated.
         * E.g: {0,0,1,1,1,1,0,0} becomes {0,1,1,0} (the first variable is eliminated)
         *
         * This function is used to compare optimized functions with non-optimized ones
         *
         * @param tt - the truth table to minimize
         * @returns the minimized truth table
         */
        std::vector<BooleanFunction::Value> minimize_truth_table(const std::vector<BooleanFunction::Value> tt);

        /**
         * Given a vector of endpoints. Returns the first Endpoint* that has a certain pin type
         *
         * @param[in] dsts - vector of destination endpoints
         * @param[in] pin - The pin.
         * @returns the first Endpoint* of a certain pin type. (nullptr, "") if no Endpoint* matches.
         */
        Endpoint* get_destination_by_pin(const std::vector<Endpoint*> dsts, const GatePin* pin);

        /**
         * Given a vector of endpoints. Returns the firstEndpoint* that has a certain pin type
         *
         * @param[in] dsts - vector of source endpoints
         * @param[in] pin - The pin.
         * @returns the first Endpoint* of a certain pin type. (nullptr, "") if no Endpoint* matches.
         */
        Endpoint* get_source_by_pin(const std::vector<Endpoint*> srcs, const GatePin* pin);

        /**
         * Checks if two vectors have the same content regardless of their order. Shouldn't be used for
         * large vectors, since it isn't really efficient.
         *
         * @param[in] vec_1 - first vector
         * @param[in] vec_2 - second vector
         * @returns TRUE if both vectors have the same content. FALSE otherwise
         */
        template<typename T>
        bool vectors_have_same_content(std::vector<T> vector_1, std::vector<T> vector_2)
        {
            if (vector_1.size() != vector_2.size())
            {
                return false;
            }

            std::sort(vector_1.begin(), vector_1.end());
            std::sort(vector_2.begin(), vector_2.end());

            return vector_1 == vector_2;
        }

        /**
         * Checks if a string contains a substring
         *
         * @param[in] str - string to search in
         * @param[in] sub_str - the substring searched for
         * @returns TRUE if str contains sub_str. FALSE otherwise.
         */
        bool string_contains_substring(const std::string str, const std::string sub_str);

        /**
         * Get the pointer of a Net, which name contains a certain substring. There must be only one name with this subname.
         *
         * @param nl - netlist
         * @param subname - substring of the Net name
         * @returns the Net pointer if there is exactly one Net with the subname. Returns nullptr otherwise.
         */
        Net* get_net_by_subname(Netlist* nl, const std::string subname);

        /**
         * Get the pointer of a Gate, which name contains a certain substring. There must be only one name with this subname.
         *
         * @param nl - netlist
         * @param subname - substring of the Net name
         * @returns the Gate pointer if there is exactly one Gate with the subname. Returns nullptr otherwise.
         */
        Gate* get_gate_by_subname(Netlist* nl, const std::string subname);

        // ===== File Management =====
        /**
         * Create a sandbox directory within the build folder, where temporary files can be stored. Please use the function
         * remove_sandbox_directory() at the end of the test in order to remove the directory.
         *
         * @returns the absolute path of the sandbox directory
         */
        std::filesystem::path create_sandbox_directory();

        /**
         * Remove the sandbox directory if it was created before
         */
        void remove_sandbox_directory();

        /**
         * Creates a file path to a file in the sandbox directory. Note that this function can only called, if the sandbox
         * directory exists. (Use create_sandbox_directory() to create it)
         *
         * @param file_name - The name of the file (including its extension)
         * @returns the absolute path of the file.
         */
        std::filesystem::path create_sandbox_path(const std::string file_name);

        /**
         * Creates a file with a given content within the sandbox directory. Note that this function can only called
         * if the sandbox directory exists. (Use create_sandbox_directory() to create it)
         *
         * @param file_name - The name of the file (including its extension)
         * @param content - The content of the file
         * @returns the absolute path of the file
         */
        std::filesystem::path create_sandbox_file(std::string file_name, std::string content);

        /**
         * Creates a Gate library dedicated solely to testing. Construction of that Gate library is independent of the Gate library parser.
         */
        GateLibrary* get_testing_gate_library();

        // ===== Example Netlists =====

        /*
        *      example_netlist
        *
        *
        *      gnd (1) =--= gate_1_to_1 (3) =--=                  .------= gate_1_to_1 (4) =
        *                                        gate_2_to_1 (0) =+
        *      vcc (2) =-----------------------=                  '------=
        *                                                                  gate_2_to_1 (5) =
        *                                                                =
        *
        *     =                       =                 =----------=
        *       gate_2_to_0 (6)         gate_2_to_1 (7)            ...  gate_2_to_1 (8) =
        *     =                       =                 =          =
        */
        /**
         * Creates the netlist shown in the diagram above. Sets a concrete id if passed.
         *
         * @param[in] id - id of the netlist
         * @returns the created netlist object
         */
        std::unique_ptr<Netlist> create_example_netlist(const int id = -1);

        /*
         *      example netlist II
         *
         *    =                    .-------=                 =
         *    =                    +-------=                 =
         *    = gate_4_to_1 (0) =--+-------= gate_4_to_1 (1) =
         *    =                    |    .--=                 =
         *                         |    |
         *    =                    |    |
         *    =                    |    |
         *    = gate_4_to_1 (2) =--~----'
         *    =                    '-------=
         *                                 =
         *                                 =  gate_4_to_1 (3) =
         *                                 =
         */
        /**
         * Creates the netlist shown in the diagram above. Sets a concrete id if passed.
         *
         * @param[in] id - id of the netlist
         * @returns the created netlist object
         */
        std::unique_ptr<Netlist> create_example_netlist_2(const int id = -1);

        /*
         *      example netlist negative
         *
         *        -----= INV (1) =-----
         *
         */
        /**
         * Creates the netlist shown in the diagram above. Sets a concrete id if passed.
         *
         * @param[in] id - id of the netlist
         * @returns the created netlist object
         */
        std::unique_ptr<Netlist> create_example_netlist_negative(const int id = -1);

        // ===== Netlist Comparison Functions (mainly used to test parser and writer) =====

        /*
         * The following functions are mainly used to compare netlists that are written and parsed. Therefore
         * the pointers of the netlists objects can't be compared but their content (name, id, etc.) and
         * their connection to other objects.
         */
        /**
         * Checks if two nets are equal regardless if they are in the same netlist (they doesn't share a pointer).
         * Two nets are considered equal iff:
         * id is equal AND name is equal AND the stored data is equal AND the connected pin type is equal AND
         * the connected gates are equal according the function 'gates_are_equal'  AND booth or neither are global inputs/outputs
         *
         * @param n0[in] - Net
         * @param n1[in] - other Net
         * @param ignore_id - if the ids should be ignored in comparison
         * @param ignore_name - if the names should be ignored in comparison
         * @returns TRUE if n0 and n1 are equal under the considered conditions. FALSE otherwise.
         */
        bool nets_are_equal(Net* n0, Net* n1, const bool ignore_id = false, const bool ignore_name = false);

        /**
         * Checks if two gates are equal regardless if they are in the same netlist (they doesn't share a pointer).
         * Two gates are considered equal iff:
         * id is equal AND name is equal AND the Gate type is equal AND the stored data is equal AND both or neither are VCC/GND gates
         *
         * @param g0[in] - Gate
         * @param g1[in] - other Gate
         * @param ignore_id - if the ids should be ignored in comparison
         * @param ignore_name - if the names should be ignored in comparison
         * @return
         */
        bool gates_are_equal(Gate* g0, Gate* g1, const bool ignore_id = false, const bool ignore_name = false);

        /**
         * Checks if two modules are equal regardless if they are in the same netlist (they doesn't share a pointer).
         * Two modules are considered equal iff:
         * id is equal AND name is equal AND the stored data is equal AND
         * their gates are equal according the function 'gates_are_equal' AND its submodules are equal (only considering the ids)
         *
         * @param m_0[in] - module
         * @param m_1[in] - other module
         * @param ignore_id - if the ids should be ignored in comparison
         * @param ignore_name - if the names should be ignored in comparison
         * @returns TRUE if m_0 and m_1 are equal under the considered conditions. FALSE otherwise.
         */
        bool modules_are_equal(Module* m_0, Module* m_1, const bool ignore_id = false, const bool ignore_name = false);

        /**
         * Checks if two groupings are equal regardless if they are in the same netlist (they doesn't share a pointer).
         * Two groupings are considered equal iff:
         * id is equal AND name is equal AND
         * their gates, nets, and modules are equal according the function 'gates_are_equal', 'nets_are_equal', and 'modules_are_equal'
         *
         * @param g_0[in] - grouping
         * @param g_1[in] - other grouping
         * @param ignore_id - if the ids should be ignored in comparison
         * @param ignore_name - if the names should be ignored in comparison
         * @returns TRUE if g_0 and g_1 are equal under the considered conditions. FALSE otherwise.
         */
        bool groupings_are_equal(Grouping* g_0, Grouping* g_1, const bool ignore_id = false, const bool ignore_name = false);

        /**
         * Checks if two netlist are equal regardless if they are the same object.
         * Two netlists are considered equal iff:
         * id is equal AND Gate library is equal AND their gates are equal according the function 'gates_are_equal' AND
         * their nets are equal according the function 'nets_are_equal' AND
         * their modules are equal according the function 'modules_are_equal' AND
         * they have the same global GND/VCC gates AND they have the same input/output/inout nets
         *
         *
         * @param nl_0[in] - nl
         * @param nl_1[in] - other nl
         * @param ignore_id - if the ids should be ignored in comparison (in this case the module-,Gate-,Net names must be unique)
         * @returns TRUE if nl_0 and nl_1 are equal under the considered conditions. FALSE otherwise.
         */
        bool netlists_are_equal(Netlist* nl_0, Netlist* nl_1, const bool ignore_id = false);

        // ===== Filter Factory Functions (used in module::get_gates, netlist::get_nets, moduleget_submodules, Gate::get_sucessors, Gate::get_predecessors) =====

        // +++ Module Filter +++

        /**
         * Filter returns true for modules with the name 'name'
         *
         * @param name - the name of the modules the filter is searching for
         * @return the std::function object of the filter function
         */
        std::function<bool(const Module*)> module_name_filter(const std::string& name);

        // +++ Grouping Filter +++

        /**
         * Filter returns true for groupings with the name 'name'
         *
         * @param name - the name of the grouping the filter is searching for
         * @return the std::function object of the filter function
         */
        std::function<bool(const Grouping*)> grouping_name_filter(const std::string& name);

        // +++ Gate Filter +++

        /**
         * Filter only returns true, if the Gate is of type 'type' AND has the name 'name'
         *
         * @param type - the type of the gates the filter is searching for
         * @param name - the name of the gates the filter is searching for
         * @return the std::function object of the filter function
         */
        std::function<bool(const Gate*)> gate_filter(const std::string& type, const std::string& name);

        /**
         * Filter returns true for gates with the name 'name'
         *
         * @param name - the name of the gates the filter is searching for
         * @return the std::function object of the filter function
         */
        std::function<bool(const Gate*)> gate_name_filter(const std::string& name);

        /**
         * Filter returns true for gates of type 'type'
         *
         * @param type - the type of the gates the filter is searching for
         * @return the std::function object of the filter function
         */
        std::function<bool(const Gate*)> gate_type_filter(const std::string& type);

        // +++ Net Filter +++

        /**
         * Filter returns true for nets with the name 'name'
         *
         * @param name - the name of the nets the filter is searching for
         * @return the std::function object of the filter function
         */
        std::function<bool(const Net*)> net_name_filter(const std::string& name);

        // +++ Endpoint* Filter +++

        /**
         * Filter returns true, if the type of the Gate, the Endpoint* is connected to, is of type 'gate_type'
         *
         * @param gate_type - the type of the gates the filter is searching for
         * @return the std::function object of the filter function
         */
        std::function<bool(const Endpoint*)> endpoint_gate_type_filter(const std::string& gate_type);

        /**
         * Filter returns true, if the type of the Gate, the Endpoint* is connected to, has the name 'name'
         *
         * @param type - the name of the gates the filter is searching for
         * @return the std::function object of the filter function
         */
        std::function<bool(const Endpoint*)> endpoint_gate_name_filter(const std::string& name);

        /**
         * Filter returns true for endpoints connected to the pin.
         *
         * @param pin - The pin.
         * @return the std::function object of the filter function
         */
        std::function<bool(const Endpoint*)> endpoint_pin_filter(const GatePin* pin);

        /**
         * Filter returns true, for all connected Endpoints (of adjacent gates) of type 'pin'
         *
         * @param pin_name - The pin name.
         * @return the std::function object of the filter function
         */
        std::function<bool(const GatePin*, const Endpoint*)> adjacent_pin_filter(const std::string& pin_name);

        /**
         * Filter returns true for all endpoints, that are connected to the pin of pintype 'pin' of the calling Gate
         *
         * @param pin_name - The pin name.
         * @return the std::function object of the filter function
         */
        std::function<bool(const GatePin*, const Endpoint*)> starting_pin_filter(const std::string& pin_name);

        /**
         * Filter returns true for all endpoints of adjacent gates of Gate type 'type'
         *
         * @param type - the type of adjacent gates, the filter is searching for
         * @return the std::function object of the filter function
         */
        std::function<bool(const GatePin*, const Endpoint*)> adjacent_gate_type_filter(const std::string& type);

        /**
         * Used to test the event system. It can create callback hooks (via get_callback/get_conditional_callback) and
         * observes if/how often they are triggered (via is_triggered/get_trigger_count)
         *
         * @tparam R - the return type of the callback function
         * @tparam P - all parameter types of the callback function
         */
        template<class R, class... P>    // R: callback return type, P: parameters
        class EventListener
        {
        private:
            std::stack<std::tuple<P...>> m_call_parameters;

            /**
             * Triggers a conditional event. Trigger count is only increased, if the condition specified by cond with
             * the given parameters (params) is fulfilled. This function is used to create conditional callback hooks.
             *
             * @param cond - a filter function that represents the conditions for given parameters
             * @param params - the actual parameters the cond is check for
             */
            void trigger_event_conditional(std::function<bool(P...)> cond, P... params)
            {
                if (cond(params...))
                {
                    m_call_parameters.push(std::tuple<P...>(params...));
                }
            }

        public:
            EventListener()  = default;
            ~EventListener() = default;

            /**
             * Get the parameter of the last event that was triggered. Note, that parameters of conditional events
             * are not stored, if the condition is not met.
             *
             * @returns a tuple of the parameters of the last triggered event
             */
            std::tuple<P...> get_last_parameters()
            {
                if (m_call_parameters.size() == 0)
                {
                    return std::tuple<P...>();
                }
                return m_call_parameters.top();
            }

            /**
             * Returns if a registered callback was called since the last reset.
             *
             * @returns true, iff at least one time, an event was triggered
             */
            bool is_triggered()
            {
                return !m_call_parameters.empty();
            }

            /**
             * Returns the amount of time, a callback triggered an event since the last reset
             *
             * @returns the event count
             */
            u32 get_event_count()
            {
                return m_call_parameters.size();
            }

            /**
             * Resets the event history.
             */
            void reset_events()
            {
                m_call_parameters = std::stack<std::tuple<P...>>();
            }

            /**
             * Removes the last event from the history
             */
            void pop_event()
            {
                if (!m_call_parameters.empty())
                    m_call_parameters.pop();
            }

            /**
             * Returns an unconditional callback hook, that only triggers an event, if the passed condition is fulfilled
             * by the parameters, the callback is called with.
             *
             * @param cond - a function that represents the condition, that must be fulfilled by the parameters.
             * @returns the conditional callback hook
             */
            std::function<R(P...)> get_conditional_callback(std::function<bool(P...)> cond)
            {
                std::function<R(P...)> f = [=](P... params) { this->trigger_event_conditional(cond, params...); };
                return f;
            }

            /**
             * Returns an unconditional callback hook, that triggers an event every time it is called (independent of the
             * passed parameters).
             *
             * @return the unconditional callback hook
             */
            std::function<R(P...)> get_callback()
            {
                return get_conditional_callback([=](P...) { return true; });
            }
        };

    }    // namespace test_utils
}    // namespace hal

//}
