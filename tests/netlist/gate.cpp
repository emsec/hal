#include "netlist/gate_library/gate_library_manager.h"
#include "netlist/netlist.h"
#include "netlist/netlist_factory.h"
#include "netlist_test_utils.h"
#include "gtest/gtest.h"
#include <core/log.h>
#include <iostream>
#include <netlist/gate.h>
#include <netlist/net.h>
#include <netlist/module.h>
#include <netlist/gate_library/gate_type/gate_type_lut.h>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <cassert>

using namespace test_utils;

class gate_test : public ::testing::Test
{
protected:

    virtual void SetUp()
    {
        NO_COUT_BLOCK;
        gate_library_manager::load_all();
    }

    virtual void TearDown()
    {
    }

    std::string i_to_hex_string(const int i) const{
        std::stringstream ss;
        ss << std::hex << i;
        return ss.str();
    }

    // UNUSED
    std::vector<boolean_function::value> get_truth_table_from_i(const int i, unsigned bit, bool flipped = false){
        std::vector<boolean_function::value> res;
        for(int b = bit-1; b >= 0; b--){
            if(((1 << b) & i) > 0){
                res.push_back(boolean_function::value::ONE);
            }
            else{
                res.push_back(boolean_function::value::ZERO);
            }
        }
        if (flipped){
            std::reverse(res.begin(),res.end());
        }
        return res;
    }

    // Turns a hex string into a boolean_function truth table (e.g: "A9" -> {1,0,1,0,1,0,0,1})
    // If the string is to small/big, the BEGINNING of the table is filled with zero/erased before the flip.
    std::vector<boolean_function::value> get_truth_table_from_hex_string(std::string str, unsigned bit, bool flipped = false){
        std::string char_val = "0123456789ABCDEF";
        std::vector<boolean_function::value> res;
        std::transform(str.begin(), str.end(),str.begin(), ::toupper);
        for(char c : str){
            if (std::size_t val = char_val.find(c) ; val != std::string::npos){
                for (int b = 3; b >= 0; b--){
                    if (((1 << b) & val) > 0){
                        res.push_back(boolean_function::value::ONE);
                    }
                    else{
                        res.push_back(boolean_function::value::ZERO);
                    }
                }
            }
            else{ // input has non-hex characters
                return std::vector<boolean_function::value>();
            }
        }
        int size_diff = res.size() - bit;
        if (size_diff > 0){
            res.erase (res.begin(), res.begin()+size_diff);
        }
        else if (size_diff < 0){
            res.insert (res.begin(), -size_diff, boolean_function::value::ZERO);
        }
        assert(res.size() == bit);
        if (flipped){
            std::reverse(res.begin(),res.end());
        }
        return res;
    }

};

/**
 * Testing the constructor as well as the copy constructor of the gate
 *
 * Functions: constructor, copy constructor, get_id, get_name, get_type
 */
TEST_F(gate_test, check_constructor)
{
    TEST_START
        // Create a gate (id = 100) and append it to its netlist
        std::shared_ptr<netlist> nl     = create_empty_netlist();
        std::shared_ptr<gate> test_gate = nl->create_gate(MIN_GATE_ID+100, get_gate_type_by_name("AND2"), "gate_name");

        ASSERT_NE(test_gate, nullptr);
        EXPECT_EQ(test_gate->get_id(), (u32)(MIN_GATE_ID+100));
        EXPECT_EQ(test_gate->get_type()->get_name(), "AND2");
        EXPECT_EQ(test_gate->get_name(), "gate_name");
        EXPECT_EQ(test_gate->get_netlist(), nl);

    TEST_END
}

/**
 * Testing the output operator<< by testing, if the output stream contains certain
 * information, like id, name ...
 *
 * Functions: operator<<
 */
/*
TEST_F(gate_test, check_out_operator) {
    TEST_START
        std::shared_ptr<netlist> nl = create_example_netlist(0);

        {
            std::stringstream ss;
            ss << *(nl->get_gate_by_id(0));

        }


    TEST_END
}*/

/**
 * Testing the function set_name and get_name
 *
 * Functions: get_name, set_name
 */
TEST_F(gate_test, check_set_and_get_name)
{
    TEST_START
        // ########################
        // POSITIVE TESTS
        // ########################
        // Create a gate and append it to its netlist
        std::shared_ptr<netlist> nl     = create_empty_netlist();
        std::shared_ptr<gate> test_gate = nl->create_gate(MIN_GATE_ID+0, get_gate_type_by_name("AND2"), "gate_name");

        EXPECT_EQ(test_gate->get_name(), "gate_name");

        // Set a new name
        NO_COUT(test_gate->set_name("new_name"));
        EXPECT_EQ(test_gate->get_name(), "new_name");

        // Set the name to the same new name again
        NO_COUT(test_gate->set_name("new_name"));
        EXPECT_EQ(test_gate->get_name(), "new_name");

        // ########################
        // NEGATIVE TESTS
        // ########################
        // Set the name to the empty string (should do nothing)
        NO_COUT(test_gate->set_name("gate_name")); // Set it initially
        NO_COUT(test_gate->set_name("")); // Set it initially
        EXPECT_EQ(test_gate->get_name(), "gate_name");



    TEST_END
}


/**
 * Testing functions which returns the pin types. Further test for different gate types
 * are for in the tests for netlist (get_input_pin_types, ... )
 *
 * Functions: get_input_pin_types, get_output_pin_types, get_inout_pin_types
 */
TEST_F(gate_test, check_pin_types)
{
    TEST_START
    // Create a gate and append it to its netlist
    std::shared_ptr<netlist> nl     = create_empty_netlist();
    std::shared_ptr<gate> test_gate = nl->create_gate(MIN_GATE_ID+0, get_gate_type_by_name("AND2"), "gate_name");

    EXPECT_EQ(test_gate->get_input_pins(), std::vector<std::string>({"I0", "I1"}));
    EXPECT_EQ(test_gate->get_output_pins(), std::vector<std::string>({"O"}));

    TEST_END
}


/*
     *      Example netlist circuit diagram (Id in brackets). Used for get fan in and
     *      out nets.
     *
     *
     *      GND (2) =-= INV (4) =--=            ------= INV (5) =
     *                               AND2 (1) =-
     *      VCC (3) =--------------=            ------=
     *                                                  AND2 (6) =
     *                                                =
     *
     *     =                    =          =----------=        =
     *       BUF (7)           ... OR2 (8) ...          OR2 (9)
     *     =                    =          =          =        =
     */

/**
 * Testing functions which returns the fan-in nets nets by using the
 * example netlist (see above)
 *
 * Functions: get_fan_in_nets
 */
TEST_F(gate_test, check_get_fan_in_nets)
{
    TEST_START

    // Create the example
    std::shared_ptr<netlist> nl = create_example_netlist();

    // ########################
    // POSITIVE TESTS
    // ########################
    {
        // All input pins are occupied
        std::shared_ptr<gate> gate_0                 = nl->get_gate_by_id(MIN_GATE_ID+0);
        std::set<std::shared_ptr<net>> fan_in_nets_0 = {nl->get_net_by_id(MIN_NET_ID+30), nl->get_net_by_id(MIN_NET_ID+20)};
        EXPECT_EQ(gate_0->get_fan_in_nets(), fan_in_nets_0);
    }
    {
        // Not all input pins are occupied
        std::shared_ptr<gate> gate_5                 = nl->get_gate_by_id(MIN_GATE_ID+5);
        std::set<std::shared_ptr<net>> fan_in_nets_5 = {nl->get_net_by_id(MIN_NET_ID+045)};
        EXPECT_EQ(gate_5->get_fan_in_nets(), fan_in_nets_5);
    }
    {
        // No input pins are occupied
        std::shared_ptr<gate> gate_6                 = nl->get_gate_by_id(MIN_GATE_ID+6);
        std::set<std::shared_ptr<net>> fan_in_nets_6 = {};
        EXPECT_EQ(gate_6->get_fan_in_nets(), fan_in_nets_6);
    }
    {
        // No input-pins exist
        std::shared_ptr<gate> gate_1                 = nl->get_gate_by_id(MIN_GATE_ID+1);
        std::set<std::shared_ptr<net>> fan_in_nets_1 = {};
        EXPECT_EQ(gate_1->get_fan_in_nets(), fan_in_nets_1);
    }
    TEST_END
}

/**
 * Testing functions which returns the fan-out nets nets by using the
 * example netlist (see above)
 *
 * Functions: get_fan_out_nets
 */
TEST_F(gate_test, check_get_fan_out_nets)
{
    TEST_START

    // Create the example
    std::shared_ptr<netlist> nl = create_example_netlist(0);

    // ########################
    // POSITIVE TESTS
    // ########################
    {
        // All output pins are occupied
        std::shared_ptr<gate> gate_0                  = nl->get_gate_by_id(MIN_GATE_ID+0);
        std::set<std::shared_ptr<net>> fan_out_nets_0 = {nl->get_net_by_id(MIN_NET_ID+045)};
        EXPECT_EQ(gate_0->get_fan_out_nets(), fan_out_nets_0);
    }
    {
        // Not all output pins are occupied
        std::shared_ptr<gate> gate_7                  = nl->get_gate_by_id(MIN_GATE_ID+7);
        std::set<std::shared_ptr<net>> fan_out_nets_7 = {nl->get_net_by_id(MIN_NET_ID+78)};
        EXPECT_EQ(gate_7->get_fan_out_nets(), fan_out_nets_7);
    }
    {
        // No output pins are occupied
        std::shared_ptr<gate> gate_8                  = nl->get_gate_by_id(MIN_GATE_ID+8);
        std::set<std::shared_ptr<net>> fan_out_nets_8 = {};
        EXPECT_EQ(gate_8->get_fan_out_nets(), fan_out_nets_8);
    }
    {
        // No output pin exist
        std::shared_ptr<gate> gate_6                  = nl->get_gate_by_id(MIN_GATE_ID+6);
        std::set<std::shared_ptr<net>> fan_out_nets_6 = {};
        EXPECT_EQ(gate_6->get_fan_out_nets(), fan_out_nets_6);
    }
    TEST_END
}

/**
 * Testing functions which returns the fan-in net, connected to a specific pin-type,
 * by using the example netlist (see above)
 *
 * Functions: get_fan_in_net
 */
TEST_F(gate_test, check_get_fan_in_net)
{
    TEST_START

    // Create the example
    std::shared_ptr<netlist> nl = create_example_netlist();

    // ########################
    // POSITIVE TESTS
    // ########################
    {
        // Get an existing net at an existing pin-type
        std::shared_ptr<gate> gate_0 = nl->get_gate_by_id(MIN_GATE_ID+0);
        EXPECT_EQ(gate_0->get_fan_in_net("I0"), nl->get_net_by_id(MIN_NET_ID+30));
        EXPECT_EQ(gate_0->get_fan_in_net("I1"), nl->get_net_by_id(MIN_NET_ID+20));
    }
    {
        // Get the net of a pin where no net is connected
        std::shared_ptr<gate> gate_5 = nl->get_gate_by_id(MIN_GATE_ID+5);
        EXPECT_EQ(gate_5->get_fan_in_net("I1"), nullptr);
    }
    {
        // Get the net of a non existing pin
        std::shared_ptr<gate> gate_0 = nl->get_gate_by_id(MIN_GATE_ID+0);
        EXPECT_EQ(gate_0->get_fan_in_net("NEx_PIN"), nullptr);
    }
    {
        // Get the net of a non existing pin-type of a gate where no input pin exist
        std::shared_ptr<gate> gate_1 = nl->get_gate_by_id(MIN_GATE_ID+1);
        EXPECT_EQ(gate_1->get_fan_in_net("NEx_PIN"), nullptr);
    }
    {
        // Pass an empty string
        std::shared_ptr<gate> gate_0 = nl->get_gate_by_id(MIN_GATE_ID+0);
        EXPECT_EQ(gate_0->get_fan_in_net(""), nullptr);
    }
    TEST_END
}

/**
 * Testing functions which returns the fan-out net, connected to a specific pin-type,
 * by using the example netlist (see above)
 *
 * Functions: get_fan_out_net
 */
TEST_F(gate_test, check_get_fan_out_net)
{
    TEST_START

    // Create the example
    std::shared_ptr<netlist> nl = create_example_netlist();

    // ########################
    // POSITIVE TESTS
    // ########################
    {
        // Get an existing net at an existing pin-type
        std::shared_ptr<gate> gate_0 = nl->get_gate_by_id(MIN_GATE_ID+0);
        EXPECT_EQ(gate_0->get_fan_out_net("O"), nl->get_net_by_id(MIN_NET_ID+045));
    }
    {
        // Get the net of a pin where no net is connected
        std::shared_ptr<gate> gate_4 = nl->get_gate_by_id(MIN_GATE_ID+4);
        EXPECT_EQ(gate_4->get_fan_out_net("O"), nullptr);
    }
    {
        // Get the net of a non existing pin
        std::shared_ptr<gate> gate_0 = nl->get_gate_by_id(MIN_GATE_ID+0);
        EXPECT_EQ(gate_0->get_fan_out_net("NEx_PIN"), nullptr);
    }
    {
        // Get the net of a non existing pin-type of a gate where no output pin exist
        std::shared_ptr<gate> gate_6 = nl->get_gate_by_id(MIN_GATE_ID+6);
        EXPECT_EQ(gate_6->get_fan_out_net("NEx_PIN"), nullptr);
    }
    {
        // Pass an empty string
        std::shared_ptr<gate> gate_0 = nl->get_gate_by_id(MIN_GATE_ID+0);
        EXPECT_EQ(gate_0->get_fan_out_net(""), nullptr);
    }
    TEST_END
}

/**
 * Testing the get_predecessors function by using the example netlists (see above)
 *
 * Functions: get_predecessors
 */
TEST_F(gate_test, check_get_predecessors)
{
    TEST_START
    // Create the examples
    std::shared_ptr<netlist> nl_1 = create_example_netlist();
    std::shared_ptr<netlist> nl_2 = create_example_netlist_2();

    // ########################
    // POSITIVE TESTS
    // ########################
    {
        // Get predecessors for a gate with multiple predecessors (some of them are the same gate)
        std::shared_ptr<gate> gate_1 = nl_2->get_gate_by_id(MIN_GATE_ID+1);
        std::vector<endpoint> pred   = {get_endpoint(nl_2, MIN_GATE_ID+0, "O"), get_endpoint(nl_2, MIN_GATE_ID+0, "O"),
                                        get_endpoint(nl_2, MIN_GATE_ID+0, "O"), get_endpoint(nl_2, MIN_GATE_ID+2, "O")};
        EXPECT_TRUE(vectors_have_same_content(gate_1->get_predecessors(), pred));
        EXPECT_EQ(gate_1->get_predecessors().size(), (size_t)4);
    }
    {
        // Get predecessors for a gate with no predecessors
        std::shared_ptr<gate> gate_0 = nl_2->get_gate_by_id(MIN_GATE_ID+0);
        EXPECT_TRUE(gate_0->get_predecessors().empty());
    }
    {
        // Get predecessors for a given (existing) output pin type
        std::shared_ptr<gate> gate_3 = nl_2->get_gate_by_id(MIN_GATE_ID+3);
        std::vector<endpoint> pred   = {get_endpoint(nl_2, MIN_GATE_ID+0, "O")};
        EXPECT_TRUE(vectors_have_same_content(gate_3->get_predecessors(endpoint_pin_filter("O")), pred));
    }
    {
        // Get predecessors for a given (non-existing) output pin type
        std::shared_ptr<gate> gate_1 = nl_2->get_gate_by_id(MIN_GATE_ID+1);
        EXPECT_TRUE(gate_1->get_predecessors(endpoint_pin_filter("NEx_PIN")).empty());
        EXPECT_EQ(gate_1->get_predecessors(endpoint_pin_filter("NEx_PIN")).size(), (size_t)0);
    }
    {
        // Get predecessors for a given (existing) input pin type
        std::shared_ptr<gate> gate_3 = nl_2->get_gate_by_id(MIN_GATE_ID+3);
        std::vector<endpoint> pred   = {get_endpoint(nl_2, MIN_GATE_ID+0, "O")};
        EXPECT_TRUE(vectors_have_same_content(gate_3->get_predecessors(starting_pin_filter("I0")), pred));
    }
    {
        // Get predecessors for a given (existing) unconnected input pin type
        std::shared_ptr<gate> gate_0 = nl_2->get_gate_by_id(MIN_GATE_ID+0);
        std::vector<endpoint> pred   = {};
        EXPECT_TRUE(vectors_have_same_content(gate_0->get_predecessors(starting_pin_filter("I0")), pred));
    }
    {
        // Get predecessors for a given (non-existing) input pin type
        std::shared_ptr<gate> gate_1 = nl_2->get_gate_by_id(MIN_GATE_ID+1);
        EXPECT_TRUE(gate_1->get_predecessors(starting_pin_filter("NEx_PIN")).empty());
        EXPECT_EQ(gate_1->get_predecessors(starting_pin_filter("NEx_PIN")).size(), (size_t)0);
    }
    {
        // Get predecessors for a given (existing) gate type
        std::shared_ptr<gate> gate_0 = nl_1->get_gate_by_id(MIN_GATE_ID+0);
        std::vector<endpoint> pred   = {get_endpoint(nl_1, MIN_GATE_ID+3, "O")};
        EXPECT_TRUE(vectors_have_same_content(gate_0->get_predecessors(type_filter("INV")), pred));
        EXPECT_EQ(gate_0->get_predecessors(type_filter("INV")).size(), (size_t)1);
    }
    {
        // Get predecessors for a given (non-existing) gate type
        std::shared_ptr<gate> gate_1 = nl_2->get_gate_by_id(MIN_GATE_ID+1);
        EXPECT_TRUE(gate_1->get_predecessors(type_filter("NEx_GATE")).empty());
        EXPECT_EQ(gate_1->get_predecessors(type_filter("NEx_GATE")).size(), (size_t)0);
    }
    // ########################
    // NEGATIVE TESTS
    // ########################
    std::shared_ptr<netlist> nl_neg = create_example_netlist_negative();
    {
        // Get predecessors for a gate with unconnected nets
        std::shared_ptr<gate> gate_0 = nl_neg->get_gate_by_id(MIN_GATE_ID+0);
        EXPECT_TRUE(gate_0->get_predecessors().empty());
        EXPECT_EQ(gate_0->get_predecessors().size(), (size_t)0);
    }
    {
        // Get predecessors for a gate with unconnected nets and a set input pin type
        std::shared_ptr<gate> gate_0 = nl_neg->get_gate_by_id(MIN_GATE_ID+0);
        EXPECT_TRUE(gate_0->get_predecessors(starting_pin_filter("I")).empty());
        EXPECT_EQ(gate_0->get_predecessors(starting_pin_filter("I")).size(), (size_t)0);
    }
    TEST_END
}

/**
 * Testing the get_successors function by using the example netlists (see above).
 *
 * Functions: get_successors
 */
TEST_F(gate_test, check_get_successors)
{
    TEST_START
    // Create the examples
    std::shared_ptr<netlist> nl_1 = create_example_netlist();
    std::shared_ptr<netlist> nl_2 = create_example_netlist_2();

    // ########################
    // POSITIVE TESTS
    // ########################
    {
        // Get successors for a gate with multiple successors (some of them are the same gate)
        std::shared_ptr<gate> gate_0 = nl_2->get_gate_by_id(MIN_GATE_ID+0);
        std::vector<endpoint> succ   = {get_endpoint(nl_2, MIN_GATE_ID+1, "I0"), get_endpoint(nl_2, MIN_GATE_ID+1, "I1"),
                                        get_endpoint(nl_2, MIN_GATE_ID+1, "I2"), get_endpoint(nl_2, MIN_GATE_ID+3, "I0")};
        EXPECT_TRUE(vectors_have_same_content(gate_0->get_successors(), succ));
        EXPECT_EQ(gate_0->get_successors().size(), (size_t)4);
    }
    {
        // Get successors for a gate no successors
        std::shared_ptr<gate> gate_1 = nl_2->get_gate_by_id(MIN_GATE_ID+1);
        EXPECT_TRUE(gate_1->get_successors().empty());
    }
    {
        // Get successors for a given (existing) input pin type
        std::shared_ptr<gate> gate_0 = nl_2->get_gate_by_id(MIN_GATE_ID+0);
        std::vector<endpoint> succ   = {get_endpoint(nl_2, MIN_GATE_ID+1, "I0"), get_endpoint(nl_2, MIN_GATE_ID+3, "I0")};
        EXPECT_TRUE(vectors_have_same_content(gate_0->get_successors(endpoint_pin_filter("I0")), succ));
        EXPECT_EQ(gate_0->get_successors(endpoint_pin_filter("I0")).size(), (size_t)2);
    }
    {
        // Get successors for a given (non-existing) intput pin type
        std::shared_ptr<gate> gate_0 = nl_2->get_gate_by_id(MIN_GATE_ID+0);
        EXPECT_TRUE(gate_0->get_successors(endpoint_pin_filter("NEx_PIN")).empty());
        EXPECT_EQ(gate_0->get_successors(endpoint_pin_filter("NEx_PIN")).size(), (size_t)0);
    }
    {
        // Get successors for a given (existing) output pin type
        std::shared_ptr<gate> gate_0 = nl_2->get_gate_by_id(MIN_GATE_ID+0);
        std::vector<endpoint> succ   = {get_endpoint(nl_2, MIN_GATE_ID+1, "I0"), get_endpoint(nl_2, MIN_GATE_ID+1, "I1"),
                                        get_endpoint(nl_2, MIN_GATE_ID+1, "I2"), get_endpoint(nl_2, MIN_GATE_ID+3, "I0")};
        EXPECT_TRUE(vectors_have_same_content(gate_0->get_successors(starting_pin_filter("O")), succ));
        EXPECT_EQ(gate_0->get_successors(starting_pin_filter("O")).size(), (size_t)4);
    }
    {
        // Get successors for a given (non-existing) output pin type
        std::shared_ptr<gate> gate_0 = nl_2->get_gate_by_id(MIN_GATE_ID+0);
        EXPECT_TRUE(gate_0->get_successors(starting_pin_filter("NEx_PIN")).empty());
    }
    {
        // Get successors for a given (existing) output pin type with no successors
        std::shared_ptr<gate> gate_1 = nl_2->get_gate_by_id(MIN_GATE_ID+1);
        EXPECT_TRUE(gate_1->get_successors(starting_pin_filter("O")).empty());
    }
    {
        // Get successors for a given (existing) gate type
        std::shared_ptr<gate> gate_0 = nl_1->get_gate_by_id(MIN_GATE_ID+0);
        std::vector<endpoint> succ   = {get_endpoint(nl_1, MIN_GATE_ID+4, "I")};
        EXPECT_TRUE(vectors_have_same_content(gate_0->get_successors(type_filter("INV")), succ));
        EXPECT_EQ(gate_0->get_successors(type_filter("INV")).size(), (size_t)1);
    }
    {
        // Get successors for a given (non-existing) gate type
        std::shared_ptr<gate> gate_0 = nl_1->get_gate_by_id(MIN_GATE_ID+0);
        EXPECT_TRUE(gate_0->get_successors(type_filter("NEx_GATE")).empty());
        EXPECT_EQ(gate_0->get_successors(type_filter("NEx_GATE")).size(), (size_t)0);
    }
    // ########################
    // NEGATIVE TESTS
    // ########################
    std::shared_ptr<netlist> nl_neg = create_example_netlist_negative();
    {
        // Get successors for a gate with unconnected nets
        std::shared_ptr<gate> gate_0 = nl_neg->get_gate_by_id(MIN_GATE_ID+0);
        EXPECT_TRUE(gate_0->get_successors().empty());
    }
    TEST_END
}

/**
 * Testing the get_unique_predecessors and get_unique_successors function
 *
 * Functions: get_unique_predecessors, get_unique_successors
 */
TEST_F(gate_test, check_get_unique_predecessors_and_successors)
{
    TEST_START
    // Create the examples
    std::shared_ptr<netlist> nl_2 = create_example_netlist_2();

    // ########################
    // POSITIVE TESTS
    // ########################
    {
        // Get the unique predecessors
        std::shared_ptr<gate> gate_1 = nl_2->get_gate_by_id(MIN_GATE_ID+1);
        std::set<endpoint> pred      = {get_endpoint(nl_2, MIN_GATE_ID+0, "O"), get_endpoint(nl_2, MIN_GATE_ID+0, "O"),
                                        get_endpoint(nl_2, MIN_GATE_ID+2, "O")};
        EXPECT_TRUE((gate_1->get_unique_predecessors() == pred));
    }
    {
        // Get the unique successors
        std::shared_ptr<gate> gate_0 = nl_2->get_gate_by_id(MIN_GATE_ID+0);
        std::set<endpoint> succ      = {get_endpoint(nl_2, MIN_GATE_ID+1, "I0"), get_endpoint(nl_2, MIN_GATE_ID+1, "I1"), get_endpoint(nl_2, MIN_GATE_ID+1, "I2"), get_endpoint(nl_2, MIN_GATE_ID+3, "I0")};
        EXPECT_TRUE((gate_0->get_unique_successors() == succ));
    }
    TEST_END
}

/**
 * Testing the get_predecessor function
 *
 * Functions: get_predecessor
 */
TEST_F(gate_test, check_get_predecessor)
{
    TEST_START
    // Create the examples
    std::shared_ptr<netlist> nl_2 = create_example_netlist_2();

    // ########################
    // POSITIVE TESTS
    // ########################
        {
            // Get predecessor for a given (existing) input pin type
            std::shared_ptr<gate> gate_3 = nl_2->get_gate_by_id(MIN_GATE_ID+3);
            endpoint pred   = get_endpoint(nl_2, MIN_GATE_ID+0, "O");
            EXPECT_TRUE(gate_3->get_predecessor("I0") == pred);
        }
        {
            // Get predecessor for a given (existing) input pin type with no predecessors
            std::shared_ptr<gate> gate_0 = nl_2->get_gate_by_id(MIN_GATE_ID+0);
            endpoint pred   = {nullptr, ""};
            EXPECT_TRUE(gate_0->get_predecessor("I0") == pred);
        }

    TEST_END
}

/**
 * Testing the handling of global gnd/vcc gates
 *
 * Functions: mark_global_vcc_gate, mark_global_gnd_gate,
 *            unmark_global_vcc_gate, unmark_global_gnd_gate,
 *            is_global_vcc_gate, is_global_gnd_gate
 */
TEST_F(gate_test, check_gnd_vcc_gate_handling)
{
    TEST_START
        {
            // Mark and unmark a global vcc gate
            std::shared_ptr<netlist> nl = create_empty_netlist();
            std::shared_ptr<gate> vcc_gate = nl->create_gate(MIN_GATE_ID+0, get_gate_type_by_name("VCC"), "vcc_gate");

            vcc_gate->mark_vcc_gate();
            EXPECT_TRUE(vcc_gate->is_vcc_gate());
            EXPECT_TRUE(nl->is_vcc_gate(vcc_gate));

            vcc_gate->unmark_vcc_gate();
            EXPECT_FALSE(vcc_gate->is_vcc_gate());
            EXPECT_FALSE(nl->is_vcc_gate(vcc_gate));
        }
        {
            // Mark and unmark a global gnd gate
            std::shared_ptr<netlist> nl = create_empty_netlist();
            std::shared_ptr<gate> gnd_gate = nl->create_gate(MIN_GATE_ID+0, get_gate_type_by_name("GND"), "gnd_gate");

            gnd_gate->mark_gnd_gate();
            EXPECT_TRUE(gnd_gate->is_gnd_gate());
            EXPECT_TRUE(nl->is_gnd_gate(gnd_gate));

            gnd_gate->unmark_gnd_gate();
            EXPECT_FALSE(gnd_gate->is_gnd_gate());
            EXPECT_FALSE(nl->is_gnd_gate(gnd_gate));
        }
    TEST_END
}

/**
 * Testing the get_module function
 *
 * Functions: get_module
 */
TEST_F(gate_test, check_get_module)
{
    TEST_START
        {
            // get the module of a gate (the top_module), then add it to another module and check again
            // -- create the gate at the top_module
            std::shared_ptr<netlist> nl = create_empty_netlist();
            std::shared_ptr<gate> test_gate = nl->create_gate(MIN_GATE_ID+0, get_gate_type_by_name("INV"), "test_gate");

            EXPECT_EQ(test_gate->get_module(), nl->get_top_module());

            // -- move the gate in the test_module
            std::shared_ptr<module> test_module = nl->create_module("test_module", nl->get_top_module());
            test_module->assign_gate(test_gate);

            EXPECT_EQ(test_gate->get_module(), test_module);

            // -- delete the test_module, so the gate should be moved in the top_module again
            nl->delete_module(test_module);
            EXPECT_EQ(test_gate->get_module(), nl->get_top_module());

        }
    TEST_END
}

/**
 * Testing storage and access of gate locations
 *
 * Functions: get_location_x, get_location_y, get_location, has_location, set_location_x, set_location_y, set_location
 */
TEST_F(gate_test, check_location_storage)
{
    TEST_START
        {
            // Create a gate with a location and change it afterwards
            std::shared_ptr<netlist> nl = create_empty_netlist();
            std::shared_ptr<gate> test_gate = nl->create_gate(MIN_GATE_ID+0, get_gate_type_by_name("INV"), "test_gate", 1.11f, 2.22f);
            EXPECT_EQ(test_gate->get_location_x(), 1.11f);
            EXPECT_EQ(test_gate->get_location_y(), 2.22f);
            EXPECT_EQ(test_gate->get_location(), std::make_pair(1.11f, 2.22f));
            // -- set a new x location
            test_gate->set_location_x(3.33f);
            EXPECT_EQ(test_gate->get_location(), std::make_pair(3.33f, 2.22f));
            // -- set a new y location
            test_gate->set_location_y(4.44f);
            EXPECT_EQ(test_gate->get_location(), std::make_pair(3.33f, 4.44f));
            // -- set a new location
            test_gate->set_location(std::make_pair(5.55f, 6.66f));
            EXPECT_EQ(test_gate->get_location(), std::make_pair(5.55f, 6.66f));
            // -- set the same location again
            test_gate->set_location(std::make_pair(5.55f, 6.66f));
            EXPECT_EQ(test_gate->get_location(), std::make_pair(5.55f, 6.66f));
        }
        {
            // Test the has_location function
            std::shared_ptr<netlist> nl = create_empty_netlist();
            std::shared_ptr<gate> test_gate = nl->create_gate(MIN_GATE_ID+0, get_gate_type_by_name("INV"), "test_gate", 1.11f, 2.22f);
            // -- both coordinates are >= 0
            test_gate->set_location(std::make_pair(1.11f, 2.22f));
            EXPECT_TRUE(test_gate->has_location());
            test_gate->set_location(std::make_pair(0.0f, 0.0f));
            EXPECT_TRUE(test_gate->has_location());
            // -- not both coordinates are >= 0
            test_gate->set_location(std::make_pair(-1.11f, 2.22f));
            EXPECT_FALSE(test_gate->has_location());
            test_gate->set_location(std::make_pair(1.11f, -2.22f));
            EXPECT_FALSE(test_gate->has_location());
            test_gate->set_location(std::make_pair(-1.11f, -2.22f));
            EXPECT_FALSE(test_gate->has_location());
        }
    TEST_END
}

/* boolean function access */

/**
 * Testing the access on boolean function
 *
 * Functions: get_boolean_function, get_boolean_functions, add_boolean_function
 */
TEST_F(gate_test, check_boolean_function_access)
{
    TEST_START
        boolean_function bf_i = boolean_function::from_string("I", std::vector<std::string>({"I"}));
        boolean_function bf_i_invert = boolean_function::from_string("!I", std::vector<std::string>({"I"}));
        {
            // Access the boolean function of a gate_type
            std::shared_ptr<netlist> nl = create_empty_netlist();
            std::shared_ptr<gate> test_gate = nl->create_gate(MIN_GATE_ID+0, get_gate_type_by_name("INV"), "test_gate");
            std::unordered_map<std::string, boolean_function> bf = test_gate->get_boolean_functions();
            EXPECT_EQ(bf, (std::unordered_map<std::string, boolean_function>({{"O", bf_i_invert}})));
        }
        {
            // Add a custom boolean function
            std::shared_ptr<netlist> nl = create_empty_netlist();
            std::shared_ptr<gate> test_gate = nl->create_gate(MIN_GATE_ID+0, get_gate_type_by_name("INV"), "test_gate");
            test_gate->add_boolean_function("new_bf", bf_i);

            // -- get all boolean functions
            std::unordered_map<std::string, boolean_function> bf_all = test_gate->get_boolean_functions(false);
            EXPECT_EQ(bf_all, (std::unordered_map<std::string, boolean_function>({{"O", bf_i_invert},
                                                                                  {"new_bf", bf_i  }})));
            // -- get only custom boolean functions via get_boolean_functions
            std::unordered_map<std::string, boolean_function> bf_custom = test_gate->get_boolean_functions(true);
            EXPECT_EQ(bf_custom, (std::unordered_map<std::string, boolean_function>({{"new_bf", bf_i}})));

            // -- get boolean function of the gate type by using the function get_boolean_function
            EXPECT_EQ(test_gate->get_boolean_function("O"), bf_i_invert);

            // -- get the custom boolean function by using the function get_boolean_function
            EXPECT_EQ(test_gate->get_boolean_function("new_bf"), bf_i);

            // -- get the boolean function by calling get_boolean_function with name (The first output pin should be taken)
            EXPECT_EQ(test_gate->get_boolean_function(), bf_i_invert);
        }
        // NEGATIVE
        {
            // Get a boolean function for a name that is unknown.
            std::shared_ptr<netlist> nl = create_empty_netlist();
            std::shared_ptr<gate> test_gate = nl->create_gate(MIN_GATE_ID+0, get_gate_type_by_name("INV"), "test_gate");

            // EXPECT_EQ(test_gate->get_boolean_function("unknown_name"), boolean_function()); // ISSUE (boolean_function): empty != empty
            EXPECT_TRUE(test_gate->get_boolean_function("unknown_name").is_empty());
        }
        {
            // Call the get_boolean_function function with no parameter, for a gate with no outputs
            std::shared_ptr<gate_library> gl(new gate_library("TEST_LIB"));
            std::shared_ptr<gate_type> empty_gate_type(new gate_type("EMPTY_GATE"));
            gl->add_gate_type(empty_gate_type);

            std::shared_ptr<netlist> nl   = std::make_shared<netlist>(gl);
            std::shared_ptr<gate> empty_gate = nl->create_gate(MIN_GATE_ID+0, empty_gate_type, "empty_gate");
            EXPECT_TRUE(empty_gate->get_boolean_function().is_empty());
        }
        {
            // Call the get_boolean_function function with no parameter, for a gate with no outputs
            std::shared_ptr<gate_library> gl(new gate_library("TEST_LIB"));
            std::shared_ptr<gate_type> empty_gate_type(new gate_type("EMPTY_GATE"));
            empty_gate_type->add_output_pin("");
            gl->add_gate_type(empty_gate_type);

            std::shared_ptr<netlist> nl   = std::make_shared<netlist>(gl);
            std::shared_ptr<gate> empty_gate = nl->create_gate(MIN_GATE_ID+0, empty_gate_type, "empty_gate");
            //EXPECT_TRUE(empty_gate->get_boolean_function().is_empty()); // ISSUE: infinite recursion
        }
        {

        }
    TEST_END
}


/**
 * Testing the access on the lut function of a lut gate.
 *
 * Functions: get_boolean_function, get_boolean_functions, add_boolean_function
 */
TEST_F(gate_test, check_lut_function)
{
    TEST_START
        // Create a custom gate_library which contains custom lut gates
        std::shared_ptr<gate_library> gl(new gate_library("TEST_LIB"));
        std::shared_ptr<gate_type_lut> lut(new gate_type_lut("LUT_GATE"));

        std::vector<std::string> input_pins({"I0", "I1", "I2"});
        std::vector<std::string> output_pins({"O_LUT", "O_normal", "O_LUT_other"});

        lut->add_input_pins(input_pins);
        lut->add_output_pins(output_pins);
        lut->add_output_from_init_string_pin("O_LUT");
        lut->add_output_from_init_string_pin("O_LUT_other");
        lut->set_config_data_ascending_order(true);
        lut->set_config_data_identifier("data_identifier");
        lut->set_config_data_category("data_category");
        gl->add_gate_type(lut);
        {
            // Get the boolean function of the lut in different ways
            // ISSUE: Only the first output pin is handled like the the lut output. The list from get_output_from_init_string_pins
            // is not taken into account!
            std::shared_ptr<netlist> nl   = std::make_shared<netlist>(gl);
            std::shared_ptr<gate> lut_gate = nl->create_gate(MIN_GATE_ID+0, lut, "lut");

            int i = 3;
            lut_gate->set_data(lut->get_config_data_category(), lut->get_config_data_identifier(), "bit_vector", i_to_hex_string(i));

            // Testing the access via the function get_boolean_function
            EXPECT_EQ(lut_gate->get_boolean_function("O_LUT").get_truth_table(input_pins), get_truth_table_from_i(i, 8));
            // EXPECT_EQ(lut_gate->get_boolean_function("O_LUT_other").get_truth_table(input_pins), get_truth_table_from_i(i, 8)); // ISSUE: fails

            // Test the access via the get_boolean_functions map
            std::unordered_map<std::string, boolean_function> bfs = lut_gate->get_boolean_functions();
            ASSERT_TRUE(bfs.find("O_LUT") != bfs.end());
            EXPECT_EQ(bfs["O_LUT"].get_truth_table(input_pins), get_truth_table_from_i(i, 8));
            // ASSERT_TRUE(bfs.find("O_LUT_other") != bfs.end()); // ISSUE: fails
            // EXPECT_EQ(bfs["O_LUT_other"].get_truth_table(input_pins), get_truth_table_from_i(i, 8)); // ISSUE: fails


        }
        {
            // Access the boolean function of a lut, that is stored in ascending order
            std::shared_ptr<netlist> nl   = std::make_shared<netlist>(gl);
            std::shared_ptr<gate> lut_gate = nl->create_gate(MIN_GATE_ID+0, lut, "lut");
            // ISSUE: Doesn't work for 0x0 (issue in get_lut_function) and for 0xff (issue in optimize function)
            //for (int i = 0x0; i <= 0xff; i++){
            for (int i = 0x1; i <= 0xfe; i++){
                lut_gate->set_data(lut->get_config_data_category(), lut->get_config_data_identifier(), "bit_vector", i_to_hex_string(i));
                EXPECT_EQ(lut_gate->get_boolean_function("O_LUT").get_truth_table(input_pins), get_truth_table_from_hex_string(i_to_hex_string(i), 8));
            }
        }
        {
            // Access the boolean function of a lut, that is stored in descending order
            lut->set_config_data_ascending_order(false);
            std::shared_ptr<netlist> nl   = std::make_shared<netlist>(gl);
            std::shared_ptr<gate> lut_gate = nl->create_gate(MIN_GATE_ID+0, lut, "lut");
            // ISSUE: Doesn't work for 0x0 (issue in get_lut_function) and for 0xff (issue in optimize function)
            //for (int i = 0x0; i <= 0xff; i++){
            for (int i = 0x1; i <= 0xfe; i++){
                lut_gate->set_data(lut->get_config_data_category(), lut->get_config_data_identifier(), "bit_vector", i_to_hex_string(i));
                EXPECT_EQ(lut_gate->get_boolean_function("O_LUT").get_truth_table(input_pins), get_truth_table_from_hex_string(i_to_hex_string(i), 8, true));
            }
            lut->set_config_data_ascending_order(true);
        }
        {
            // Add a boolean function to a lut pin
            std::shared_ptr<netlist> nl   = std::make_shared<netlist>(gl);
            std::shared_ptr<gate> lut_gate = nl->create_gate(MIN_GATE_ID+0, lut, "lut");

            boolean_function lut_bf = boolean_function::from_string("I0 | I1 | I2", input_pins);
            lut_gate->add_boolean_function("O_LUT", lut_bf);
            get_truth_table_from_hex_string("EF", 8);
        }
        // NEGATIVE
        {
            // There is no hex string at the config data path
            std::shared_ptr<netlist> nl   = std::make_shared<netlist>(gl);
            std::shared_ptr<gate> lut_gate = nl->create_gate(MIN_GATE_ID+0, lut, "lut");
            lut_gate->set_data(lut->get_config_data_category(), lut->get_config_data_identifier(), "bit_vector", "");
            EXPECT_EQ(lut_gate->get_boolean_function("O_LUT").get_truth_table(input_pins), get_truth_table_from_i(0, 8));
        }
        /*{ // ISSUE: Fails with stoull if content is not in Hex
            // There is no hex string at the config data path
            std::shared_ptr<netlist> nl   = std::make_shared<netlist>(gl);
            std::shared_ptr<gate> lut_gate = nl->create_gate(MIN_GATE_ID+0, lut, "lut");

            lut_gate->set_data(lut->get_config_data_category(), lut->get_config_data_identifier(), "bit_vector", "NOHx");
            EXPECT_EQ(lut_gate->get_boolean_function("O_LUT").get_truth_table(input_pins), get_truth_table_from_i(0, 8));

        }*/
        /*{ // ISSUE: Fails with stoull
            // Test a lut with more than 6 inputs
            std::vector<std::string> new_input_pins({"I3", "I4", "I5", "I6"});
            lut->add_input_pins(new_input_pins);
            input_pins.insert(input_pins.begin(), new_input_pins.begin(), new_input_pins.end());

            std::shared_ptr<netlist> nl   = std::make_shared<netlist>(gl);
            std::shared_ptr<gate> lut_gate = nl->create_gate(MIN_GATE_ID+0, lut, "lut");

            std::string long_hex = "DEADBEEFC001D00DDEADC0DEDEADDA7A";

            lut_gate->set_data(lut->get_config_data_category(), lut->get_config_data_identifier(), "bit_vector", long_hex);
            EXPECT_EQ(lut_gate->get_boolean_function("O_LUT").get_truth_table(input_pins), get_truth_table_from_hex_string(long_hex, (1 << input_pins.size())));
        }*/
        {
            // Test a lut with a to short input string
            std::vector<std::string> new_input_pins({"I3", "I4", "I5"});
            lut->add_input_pins(new_input_pins);
            input_pins.insert(input_pins.end(), new_input_pins.begin(), new_input_pins.end());

            std::shared_ptr<netlist> nl   = std::make_shared<netlist>(gl);
            std::shared_ptr<gate> lut_gate = nl->create_gate(MIN_GATE_ID+0, lut, "lut");

            std::string long_hex = "DEADBEEF";

            lut_gate->set_data(lut->get_config_data_category(), lut->get_config_data_identifier(), "bit_vector", long_hex);

            EXPECT_EQ(lut_gate->get_boolean_function("O_LUT").get_truth_table(input_pins), get_truth_table_from_hex_string(long_hex, (1 << input_pins.size())));
        }
    TEST_END
}
