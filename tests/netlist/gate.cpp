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
        EXPECT_TRUE(vectors_have_same_content(gate_3->get_predecessors(DONT_CARE, "O"), pred));
    }
    {
        // Get predecessors for a given (non-existing) output pin type
        std::shared_ptr<gate> gate_1 = nl_2->get_gate_by_id(MIN_GATE_ID+1);
        EXPECT_TRUE(gate_1->get_predecessors(DONT_CARE, "NEx_PIN").empty());
        EXPECT_EQ(gate_1->get_predecessors(DONT_CARE, "NEx_PIN").size(), (size_t)0);
    }
    {
        // Get predecessors for a given (existing) input pin type
        std::shared_ptr<gate> gate_3 = nl_2->get_gate_by_id(MIN_GATE_ID+3);
        std::vector<endpoint> pred   = {get_endpoint(nl_2, MIN_GATE_ID+0, "O")};
        EXPECT_TRUE(vectors_have_same_content(gate_3->get_predecessors("I0"), pred));
    }
    {
        // Get predecessors for a given (existing) unconnected input pin type
        std::shared_ptr<gate> gate_0 = nl_2->get_gate_by_id(MIN_GATE_ID+0);
        std::vector<endpoint> pred   = {};
        EXPECT_TRUE(vectors_have_same_content(gate_0->get_predecessors("I0"), pred));
    }
    {
        // Get predecessors for a given (non-existing) input pin type
        std::shared_ptr<gate> gate_1 = nl_2->get_gate_by_id(MIN_GATE_ID+1);
        EXPECT_TRUE(gate_1->get_predecessors("NEx_PIN").empty());
        EXPECT_EQ(gate_1->get_predecessors("NEx_PIN").size(), (size_t)0);
    }
    {
        // Get predecessors for a given (existing) input pin type, but non existing output pin type
        std::shared_ptr<gate> gate_3 = nl_2->get_gate_by_id(MIN_GATE_ID+3);
        std::vector<endpoint> pred   = {};
        EXPECT_TRUE(vectors_have_same_content(gate_3->get_predecessors("I0","NEx_PIN"), pred));
    }
    {
        // Get predecessors for a given (existing) input pin type, but non existing gate type
        std::shared_ptr<gate> gate_3 = nl_2->get_gate_by_id(MIN_GATE_ID+3);
        std::vector<endpoint> pred   = {};
        EXPECT_TRUE(vectors_have_same_content(gate_3->get_predecessors("I0", DONT_CARE, "NEx_Gate_Type"), pred));
    }
    {
        // Get predecessors for a given (existing) gate type
        std::shared_ptr<gate> gate_0 = nl_1->get_gate_by_id(MIN_GATE_ID+0);
        std::vector<endpoint> pred   = {get_endpoint(nl_1, MIN_GATE_ID+3, "O")};
        EXPECT_TRUE(vectors_have_same_content(gate_0->get_predecessors(DONT_CARE, DONT_CARE, "INV"), pred));
        EXPECT_EQ(gate_0->get_predecessors(DONT_CARE, DONT_CARE, "INV").size(), (size_t)1);
    }
    {
        // Get predecessors for a given (non-existing) gate type
        std::shared_ptr<gate> gate_1 = nl_2->get_gate_by_id(MIN_GATE_ID+1);
        EXPECT_TRUE(gate_1->get_predecessors(DONT_CARE, DONT_CARE, "NEx_GATE").empty());
        EXPECT_EQ(gate_1->get_predecessors(DONT_CARE, DONT_CARE, "NEx_GATE").size(), (size_t)0);
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
        EXPECT_TRUE(gate_0->get_predecessors("I").empty());
        EXPECT_EQ(gate_0->get_predecessors("I").size(), (size_t)0);
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
        EXPECT_TRUE(vectors_have_same_content(gate_0->get_successors(DONT_CARE, "I0"), succ));
        EXPECT_EQ(gate_0->get_successors(DONT_CARE, "I0").size(), (size_t)2);
    }
    {
        // Get successors for a given (non-existing) intput pin type
        std::shared_ptr<gate> gate_0 = nl_2->get_gate_by_id(MIN_GATE_ID+0);
        EXPECT_TRUE(gate_0->get_successors(DONT_CARE, "NEx_PIN").empty());
        EXPECT_EQ(gate_0->get_successors(DONT_CARE, "NEx_PIN").size(), (size_t)0);
    }
    {
        // Get successors for a given (existing) output pin type
        std::shared_ptr<gate> gate_0 = nl_2->get_gate_by_id(MIN_GATE_ID+0);
        std::vector<endpoint> succ   = {get_endpoint(nl_2, MIN_GATE_ID+1, "I0"), get_endpoint(nl_2, MIN_GATE_ID+1, "I1"),
                                        get_endpoint(nl_2, MIN_GATE_ID+1, "I2"), get_endpoint(nl_2, MIN_GATE_ID+3, "I0")};
        EXPECT_TRUE(vectors_have_same_content(gate_0->get_successors("O"), succ));
        EXPECT_EQ(gate_0->get_successors("O").size(), (size_t)4);
    }
    {
        // Get successors for a given (existing) output pin type and an input pin type filter
        std::shared_ptr<gate> gate_0 = nl_2->get_gate_by_id(MIN_GATE_ID+0);
        std::vector<endpoint> succ   = {get_endpoint(nl_2, MIN_GATE_ID+1, "I0"), get_endpoint(nl_2, MIN_GATE_ID+3, "I0")};
        EXPECT_TRUE(vectors_have_same_content(gate_0->get_successors("O","I0"), succ));
        EXPECT_EQ(gate_0->get_successors("O","I0").size(), (size_t)2);
    }
    {
        // Get successors for a given (existing) output pin type and an gate type filter (existing gate type)
        std::shared_ptr<gate> gate_0 = nl_2->get_gate_by_id(MIN_GATE_ID+0);
        std::vector<endpoint> succ   = {get_endpoint(nl_2, MIN_GATE_ID+1, "I0"), get_endpoint(nl_2, MIN_GATE_ID+1, "I1"),
                                        get_endpoint(nl_2, MIN_GATE_ID+1, "I2"), get_endpoint(nl_2, MIN_GATE_ID+3, "I0")};
        EXPECT_TRUE(vectors_have_same_content(gate_0->get_successors("O",DONT_CARE,"AND4"), succ));
        EXPECT_EQ(gate_0->get_successors("O",DONT_CARE,"AND4").size(), (size_t)4);
    }
    {
        // Get successors for a given (existing) output pin type and an gate type filter (non existing gate type)
        std::shared_ptr<gate> gate_0 = nl_2->get_gate_by_id(MIN_GATE_ID+0);
        std::vector<endpoint> succ   = {};
        EXPECT_TRUE(vectors_have_same_content(gate_0->get_successors("O",DONT_CARE,"NEx_Gate"), succ));
        EXPECT_EQ(gate_0->get_successors("O",DONT_CARE,"NEx_Gate").size(), (size_t)0);
    }
    {
        // Get successors for a given (non-existing) output pin type
        std::shared_ptr<gate> gate_0 = nl_2->get_gate_by_id(MIN_GATE_ID+0);
        EXPECT_TRUE(gate_0->get_successors("NEx_PIN").empty());
    }
    {
        // Get successors for a given (existing) output pin type with no successors
        std::shared_ptr<gate> gate_1 = nl_2->get_gate_by_id(MIN_GATE_ID+1);
        EXPECT_TRUE(gate_1->get_successors("O").empty());
    }
    {
        // Get successors for a given (existing) gate type
        std::shared_ptr<gate> gate_0 = nl_1->get_gate_by_id(MIN_GATE_ID+0);
        std::vector<endpoint> succ   = {get_endpoint(nl_1, MIN_GATE_ID+4, "I")};
        EXPECT_TRUE(vectors_have_same_content(gate_0->get_successors(DONT_CARE, DONT_CARE, "INV"), succ));
        EXPECT_EQ(gate_0->get_successors(DONT_CARE, DONT_CARE, "INV").size(), (size_t)1);
    }
    {
        // Get successors for a given (non-existing) gate type
        std::shared_ptr<gate> gate_0 = nl_1->get_gate_by_id(MIN_GATE_ID+0);
        EXPECT_TRUE(gate_0->get_successors(DONT_CARE, DONT_CARE, "NEx_GATE").empty());
        EXPECT_EQ(gate_0->get_successors(DONT_CARE, DONT_CARE, "NEx_GATE").size(), (size_t)0);
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

