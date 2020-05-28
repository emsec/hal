
#ifndef HAL_NETLIST_TEST_UTILS_H
#define HAL_NETLIST_TEST_UTILS_H

#include <core/utils.h>
#include "test_def.h"
#include <fstream>
#include "netlist/netlist.h"
#include "netlist/gate_library/gate_library.h"
#include "netlist/gate_library/gate_library_manager.h"
#include "netlist/gate.h"
#include "netlist/net.h"
#include "netlist/module.h"
#include "netlist/endpoint.h"

namespace fs = hal::fs;

namespace test_utils
{

    /*********************************************************
     *                      Constants                        *
     *********************************************************/

    // Minimum and invali ids for netlists, gates, nets and modules
    const u32 INVALID_GATE_ID = 0;
    const u32 INVALID_NET_ID = 0;
    const u32 INVALID_MODULE_ID = 0;

    const u32 MIN_MODULE_ID = 2;
    const u32 MIN_GATE_ID = 1;
    const u32 MIN_NET_ID = 1;
    const u32 MIN_NETLIST_ID = 1;
    const u32 TOP_MODULE_ID = 1;

    // Name for accessing our example gate library
    static const std::string g_lib_name = "EXAMPLE_GATE_LIBRARY";
    //static const std::string g_lib_name = "example_library";
    // Name for accessing the custom gate library after the call of 'create_temp_gate_lib()'
    const std::string temp_lib_name = "TEMP_GATE_LIBRARY";


    /*********************************************************
     *                      Functions                        *
     *********************************************************/

    /**
     * Creates an empty netlist.
     *
     * @param[in] id - Id of the netlist
     * @returns An empty netlist
     */
    std::shared_ptr<netlist> create_empty_netlist(const int id = -1);

    /**
     * Creates an endpoint object from a passed gate and pin type
     *
     * @param[in] g - gate
     * @param[in] pin_type - pin type
     * @returns the endpoint object
     */
    endpoint get_endpoint(const std::shared_ptr<gate> g, const std::string pin_type);

    /**
     * Creating an endpoint object by passing the netlist, the id of the gate and the pin type.
     * if there is no gate with the passed id, it returns (nullptr, "")
     *
     * @param[in] nl - netlist
     * @param[in] gate_id - id of the gate
     * @param[in] pin_type - pin type
     * @returns the endpoint object
     */
    endpoint get_endpoint(const std::shared_ptr<netlist> nl, const int gate_id, const std::string pin_type);

    /**
     * Checks if an endpoint is empty (i.e. (nullptr, ""))
     *
     * @param[in] ep - endpoint
     * @return true, if the endpoint is the empty endpoint
     */
    bool is_empty(const endpoint ep);

    /**
     * Get a gate type by its name
     *
     * @param name - the name of the gate_type
     * @param gate_library_name - the name of the gate library, the gate_type can be found. If empty, the example gate library (g_lib_name) is taken.
     * @return the gate_type pointer if found. If no gate type matches, return nullptr
     */
    std::shared_ptr<const gate_type> get_gate_type_by_name(std::string name, std::string gate_library_name = "");

    /**
     * Given a vector of endpoints. Returns the first endpoint that has a certain pin type
     *
     * @param[in] dsts - vector of endpoints
     * @param[in] pin_type - pin type
     * @returns the first endpoint of a certain pin type. (nullptr, "") if no endpoint matches.
     */
    endpoint get_dst_by_pin_type(const std::vector<endpoint> dsts, const std::string pin_type);

    // NOTE: Using create_test_gate is messy. It should not exist. Will be removed someday...
    /**
     * Create a test gate with 3 input pins
     *
     * @param nl - the netlist, the test gate is created in
     * @param[in] id - id of the gate
     * @returns an already created AND3 gate
     */
    std::shared_ptr<gate> create_test_gate(std::shared_ptr<netlist> nl, const u32 id);
    /**
     * Checks if two vectors have the same content regardless of their order. Shouldn't be used for
     * large vectors, since it isn't really efficient.
     *
     * @param[in] vec_1 - first vector
     * @param[in] vec_2 - second vector
     * @returns TRUE if both vectors have the same content. FALSE otherwise
     */
    template <typename T>
    bool vectors_have_same_content(const std::vector<T> vector_1, const std::vector<T> vector_2)
    {
        std::vector<T> vec_1(vector_1);
        std::vector<T> vec_2(vector_2);

        if (vec_1.size() != vec_2.size())
            return false;

        // Each element of vec_1 must be found in vec_2
        while (vec_1.size() > 0)
        {
            auto it_1       = vec_1.begin();
            bool found_elem = false;
            for (auto it_2 = vec_2.begin(); it_2 != vec_2.end(); it_2++)
            {
                if (*it_1 == *it_2)
                {
                    found_elem = true;
                    vec_2.erase(it_2);
                    break;
                }
            }
            if (!found_elem)
            {
                return false;
            }
            vec_1.erase(it_1);
        }

        return true;
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
     * Get the pointer of a net, which name contains a certain substring. There must be only one name with this subname.
     *
     * @param nl - netlist
     * @param subname - substring of the net name
     * @returns the net pointer if there is exactly one net with the subname. Returns nullptr otherwise.
     */
    std::shared_ptr<net> get_net_by_subname(std::shared_ptr<netlist> nl, const std::string subname);

    /**
     * Get the pointer of a gate, which name contains a certain substring. There must be only one name with this subname.
     *
     * @param nl - netlist
     * @param subname - substring of the net name
     * @returns the gate pointer if there is exactly one gate with the subname. Returns nullptr otherwise.
     */
    std::shared_ptr<gate> get_gate_by_subname(std::shared_ptr<netlist> nl, const std::string subname);

    /**
     * Creates a custom gate library that contains certain gate types that aren't supported by the example gate library.
     * It mainly supports gate types with input and output pin vectors of dimension 1 up to 3. It contains gates of the
     * form 'GATE_<input pins per dimension>^<input dimentsion>_IN_<output pins per dimension>^<output dimentsion>_IN'
     * E.g. 'GATE_4^1_IN_1^0' whould have for input pins (I(0)-I(3)) and only one output pin (O)
     * After the library is created, it can be accessed via the name in 'temp_lib_name'
     *
     * IMPORTANT: This function creates a file in a common gate library directory. Don't forget to remove it via
     * a call of remove_temp_gate_lib()
     */
    void create_temp_gate_lib();

    /**
     * Removes the file created by the function 'create_temp_gate_lib()'. If the file doesn't exist it does nothing.
     */
    void remove_temp_gate_lib();


    // ===== Example Netlists =====

    /*
    *      example_netlist
    *
    *
    *      GND (1) =-= INV (3) =--=             .------= INV (4) =
    *                                 AND2 (0) =-
    *      VCC (2) =--------------=             '------=
    *                                                     AND2 (5) =
    *                                                  =
    *
    *     =                       =           =----------=           =
    *       BUF (6)              ... OR2 (7)             ... OR2 (8)
    *     =                       =           =          =           =
    */
    /**
     * Creates the netlist shown in the diagram above. Sets a concrete id if passed.
     *
     * @param[in] id - id of the netlist
     * @returns the created netlist object
     */
    std::shared_ptr<netlist> create_example_netlist(const int id = -1);


    /*
     *      example netlist II
     *
     *    =             .-------=          =
     *    =             +-------=          =
     *    = AND4 (0) =--+-------= AND4 (1) =
     *    =             |    .--=          =
     *                  |    |
     *    =             |    |
     *    =             |    |
     *    = AND4 (2) =--~----'
     *    =             '-------=
     *                          =
     *                          =  AND4 (3) =
     *                          =
     */
    /**
     * Creates the netlist shown in the diagram above. Sets a concrete id if passed.
     *
     * @param[in] id - id of the netlist
     * @returns the created netlist object
     */
    std::shared_ptr<netlist> create_example_netlist_2(const int id = -1);


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
    std::shared_ptr<netlist> create_example_netlist_negative(const int id = -1);


    /*
      *      Example netlist circuit diagram (Id in brackets). Used for get fan in and
      *      out nets.
      *
      *
      *      GND (1) =-= INV (3) =--=             .------=  INV (4)  =---
      *                                 AND2 (0) =-
      *      VCC (2) =--------------=             '------=
      *                                                     AND2 (5) =---
      *                                                  =
      *
      *                           =                .-----=
      *                              OR2 (6)  =----'        OR2 (7)  =---
      *                           =                      =
      */
    // Creates a simple netlist shown in the diagram above. The nets that have a GND/VCC gate as a source are named '0'/'1'
    /**
     * Creates the netlist shown in the diagram above.
     * The nets which are connected to a GND/VCC gate are named '0'/'1' (necessary for some parser/writer tests).
     * Sets a concrete id if passed.
     *
     * @param[in] id - id of the netlist
     * @returns the created netlist object
     */
    std::shared_ptr<netlist> create_example_parse_netlist(int id = -1);


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
     * @param n0[in] - net
     * @param n1[in] - other net
     * @param ignore_id - if the ids should be ignored in comparison
     * @param ignore_name - if the names should be ignored in comparison
     * @returns TRUE if n0 and n1 are equal under the considered conditions. FALSE otherwise.
     */
    bool nets_are_equal(const std::shared_ptr<net> n0, const std::shared_ptr<net> n1, const bool ignore_id = false, const bool ignore_name = false);

    /**
     * Checks if two gates are equal regardless if they are in the same netlist (they doesn't share a pointer).
     * Two gates are considered equal iff:
     * id is equal AND name is equal AND the gate type is equal AND the stored data is equal AND both or neither are VCC/GND gates
     *
     * @param g0[in] - gate
     * @param g1[in] - other gate
     * @param ignore_id - if the ids should be ignored in comparison
     * @param ignore_name - if the names should be ignored in comparison
     * @return
     */
    bool gates_are_equal(const std::shared_ptr<gate> g0, const std::shared_ptr<gate> g1, const bool ignore_id = false, const bool ignore_name = false);

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
    bool modules_are_equal(const std::shared_ptr<module> m_0, const std::shared_ptr<module> m_1, const bool ignore_id = false, const bool ignore_name = false);

    /**
     * Checks if two netlist are equal regardless if they are the same object.
     * Two netlists are considered equal iff:
     * id is equal AND gate library is equal AND their gates are equal according the function 'gates_are_equal' AND
     * their nets are equal according the function 'nets_are_equal' AND
     * their modules are equal according the function 'modules_are_equal' AND
     * they have the same global GND/VCC gates AND they have the same input/output/inout nets
     *
     *
     * @param nl_0[in] - nl
     * @param nl_1[in] - other nl
     * @param ignore_id - if the ids should be ignored in comparison (in this case the module-,gate-,net names must be unique)
     * @returns TRUE if nl_0 and nl_1 are equal under the considered conditions. FALSE otherwise.
     */
    bool netlists_are_equal(const std::shared_ptr<netlist> nl_0, const std::shared_ptr<netlist> nl_1, const bool ignore_id = false);


    // ===== Filter Factory Functions (used in module::get_gates, netlist::get_nets, moduleget_submodules, gate::get_sucessors, gate::get_predecessors) =====

    // +++ Module Filter +++

    /**
     * Filter returns true for modules with the name 'name'
     *
     * @param name - the name of the modules the filter is searching for
     * @return the std::function object of the filter function
     */
    std::function<bool(const std::shared_ptr<module>&)> module_name_filter(const std::string& name);

    // +++ Gate Filter +++

    /**
     * Filter only returns true, if the gate is of type 'type' AND has the name 'name'
     *
     * @param type - the type of the gates the filter is searching for
     * @param name - the name of the gates the filter is searching for
     * @return the std::function object of the filter function
     */
    std::function<bool(const std::shared_ptr<gate>&)> gate_filter(const std::string& type, const std::string& name);

    /**
     * Filter returns true for gates with the name 'name'
     *
     * @param name - the name of the gates the filter is searching for
     * @return the std::function object of the filter function
     */
    std::function<bool(const std::shared_ptr<gate>&)> gate_name_filter(const std::string& name);

    /**
     * Filter returns true for gates of type 'type'
     *
     * @param type - the type of the gates the filter is searching for
     * @return the std::function object of the filter function
     */
    std::function<bool(const std::shared_ptr<gate>&)> gate_type_filter(const std::string& type);

    // +++ Net Filter +++

    /**
     * Filter returns true for nets with the name 'name'
     *
     * @param name - the name of the nets the filter is searching for
     * @return the std::function object of the filter function
     */
    std::function<bool(const std::shared_ptr<net>&)> net_name_filter(const std::string& name);

    // +++ Endpoint Filter +++

    /**
     * Filter returns true, if the type of the gate, the endpoint is connected to, is of type 'type'
     *
     * @param type - the type of the gates the filter is searching for
     * @return the std::function object of the filter function
     */
    std::function<bool(const endpoint&)> endpoint_type_filter(const std::string& type);

    /**
     * Filter returns true, for all connected endpoint (of adjacent gates) of type 'pin'
     *
     * @param type - the type of the endpoints the filter is searching for
     * @return the std::function object of the filter function
     */
    std::function<bool(const std::string&, const endpoint&)> endpoint_pin_filter(const std::string& pin);

    /**
     * Filter returns true for all endpoints, that are connected to the pin of pintype 'pin' of the calling gate
     *
     * @param pin - the pin of the gate, calling the get_predecessors/sucesseors function
     * @return the std::function object of the filter function
     */
    std::function<bool(const std::string&, const endpoint&)> starting_pin_filter(const std::string& pin);

    /**
     * Filter returns true for all endpoints of adjacent gates of gate type 'type'
     *
     * @param type - the type of adjacent gates, the filter is searching for
     * @return the std::function object of the filter function
     */
    std::function<bool(const std::string&, const endpoint&)> type_filter(const std::string& type);


}    // namespace test_utils


#endif // HAL_NETLIST_TEST_UTILS_H
