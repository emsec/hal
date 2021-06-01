#include "hal_core/netlist/netlist.h"
#include "hal_core/netlist/netlist_factory.h"
#include "netlist_test_utils.h"

#include "gtest/gtest.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/module.h"
#include "hal_core/netlist/gate_library/gate_type.h"
#include "hal_core/netlist/event_handler.h"

#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cassert>


namespace hal
{
    using test_utils::MIN_GATE_ID;
    using test_utils::MIN_MODULE_ID;
    using test_utils::MIN_NET_ID;

    class GateTest : public ::testing::Test
    {
    protected:
        virtual void SetUp()
        {
            test_utils::init_log_channels();
        }

        virtual void TearDown()
        {
        }

        std::string i_to_hex_string(const int i, const int len = -1) const
        {
            std::stringstream ss;

            if (len >= 0)
            {
                ss << std::hex << std::setfill('0') << std::setw(len) << i;
            }
            else
            {
                ss << std::hex << i;
            }

            return ss.str();
        }

        // UNUSED
        std::vector<BooleanFunction::Value> get_truth_table_from_i(const int i, unsigned bit, bool flipped = false)
        {
            std::vector<BooleanFunction::Value> res;
            for (int b = bit - 1; b >= 0; b--)
            {
                if (((1 << b) & i) > 0)
                {
                    res.push_back(BooleanFunction::ONE);
                }
                else
                {
                    res.push_back(BooleanFunction::ZERO);
                }
            }
            if (flipped)
            {
                std::reverse(res.begin(), res.end());
            }
            return res;
        }

        // Turns a hex string into a BooleanFunction truth table (e.g: "A9" -> {1,0,1,0,1,0,0,1})
        // If the string is to small/big, the BEGINNING of the table is filled with zero/erased before the flip.
        std::vector<BooleanFunction::Value> get_truth_table_from_hex_string(std::string str, unsigned bit, bool flipped = false)
        {
            std::string char_val = "0123456789ABCDEF";
            std::vector<BooleanFunction::Value> res;
            std::transform(str.begin(), str.end(), str.begin(), ::toupper);
            for (char c : str)
            {
                if (std::size_t val = char_val.find(c); val != std::string::npos)
                {
                    for (int b = 3; b >= 0; b--)
                    {
                        if (((1 << b) & val) > 0)
                        {
                            res.push_back(BooleanFunction::ONE);
                        }
                        else
                        {
                            res.push_back(BooleanFunction::ZERO);
                        }
                    }
                }
                else
                {    // input has non-hex characters
                    return std::vector<BooleanFunction::Value>();
                }
            }
            int size_diff = res.size() - bit;
            if (size_diff > 0)
            {
                res.erase(res.begin(), res.begin() + size_diff);
            }
            else if (size_diff < 0)
            {
                res.insert(res.begin(), -size_diff, BooleanFunction::ZERO);
            }
            assert(res.size() == bit);
            if (flipped)
            {
                std::reverse(res.begin(), res.end());
            }
            return res;
        }

        // Get the minimized version of a truth truth table by its hex value, such that unnecessary variables are eliminated
        std::vector<BooleanFunction::Value> get_min_truth_table_from_hex_string(std::string str, unsigned bit, bool flipped = false)
        {
            return test_utils::minimize_truth_table(get_truth_table_from_hex_string(str, bit, flipped));
        }
    };

    /**
     * Testing the constructor as well as the copy constructor of the Gate
     *
     * Functions: constructor, copy constructor, get_id, get_name, get_type
     */
    TEST_F(GateTest, check_constructor)
    {
        TEST_START
        {
            // Create a Gate (id = 100) and append it to its netlist
            auto nl        = test_utils::create_empty_netlist();
            auto test_gate = nl->create_gate(100, nl->get_gate_library()->get_gate_type_by_name("AND2"), "gate_name");

            ASSERT_NE(test_gate, nullptr);
            EXPECT_EQ(test_gate->get_id(), 100);
            EXPECT_EQ(test_gate->get_type()->get_name(), "AND2");
            EXPECT_EQ(test_gate->get_name(), "gate_name");
            EXPECT_EQ(test_gate->get_netlist(), nl.get());
        }
        TEST_END
    }

    /**
     * Test operators for equality and inequality.
     * 
     * Functions: operator==, operator!=
     */
    TEST_F(GateTest, check_operators)
    {
        TEST_START
        {
            std::unique_ptr<Netlist> nl_1 = test_utils::create_empty_netlist();
            ASSERT_NE(nl_1, nullptr);
            std::unique_ptr<Netlist> nl_2 = test_utils::create_empty_netlist();
            ASSERT_NE(nl_2, nullptr);
            const GateLibrary* gl = nl_1->get_gate_library();
            ASSERT_NE(gl, nullptr);
            ASSERT_EQ(gl, nl_2->get_gate_library());

            // standard stuff
            Gate* nl1_g1 = nl_1->create_gate(1, gl->get_gate_type_by_name("AND2"), "gate_1");
            ASSERT_NE(nl1_g1, nullptr);
            Gate* nl2_g1 = nl_2->create_gate(1, gl->get_gate_type_by_name("AND2"), "gate_1");
            ASSERT_NE(nl2_g1, nullptr);
            Gate* nl1_g2 = nl_1->create_gate(2, gl->get_gate_type_by_name("AND2"), "gate_1");
            ASSERT_NE(nl1_g2, nullptr);
            Gate* nl2_g2 = nl_2->create_gate(2, gl->get_gate_type_by_name("AND2"), "gate_2");
            ASSERT_NE(nl2_g2, nullptr);
            Gate* nl1_g3 = nl_1->create_gate(3, gl->get_gate_type_by_name("AND2"), "gate_3");
            ASSERT_NE(nl1_g3, nullptr);
            Gate* nl2_g3 = nl_2->create_gate(3, gl->get_gate_type_by_name("OR2"), "gate_3");
            ASSERT_NE(nl2_g3, nullptr);

            // GND/VCC gates
            Gate* nl1_g4 = nl_1->create_gate(4, gl->get_gate_type_by_name("GND"), "gnd");
            ASSERT_NE(nl1_g4, nullptr);
            nl1_g4->mark_gnd_gate();
            Gate* nl2_g4 = nl_2->create_gate(4, gl->get_gate_type_by_name("GND"), "gnd");
            ASSERT_NE(nl2_g4, nullptr);
            Gate* nl1_g5 = nl_1->create_gate(5, gl->get_gate_type_by_name("VCC"), "vcc");
            ASSERT_NE(nl1_g5, nullptr);
            nl1_g5->mark_vcc_gate();
            Gate* nl2_g5 = nl_2->create_gate(5, gl->get_gate_type_by_name("VCC"), "vcc");
            ASSERT_NE(nl2_g5, nullptr);

            // locations
            Gate* nl1_g6 = nl_1->create_gate(6, gl->get_gate_type_by_name("AND2"), "gate_6", 20, 30);
            ASSERT_NE(nl1_g6, nullptr);
            Gate* nl2_g6 = nl_2->create_gate(6, gl->get_gate_type_by_name("AND2"), "gate_6", 20, 30);
            ASSERT_NE(nl2_g6, nullptr);
            Gate* nl1_g7 = nl_1->create_gate(7, gl->get_gate_type_by_name("AND2"), "gate_7", 20, 30);
            ASSERT_NE(nl1_g7, nullptr);
            Gate* nl2_g7 = nl_2->create_gate(7, gl->get_gate_type_by_name("AND2"), "gate_7", 10, 30);
            ASSERT_NE(nl2_g7, nullptr);
            Gate* nl1_g8 = nl_1->create_gate(8, gl->get_gate_type_by_name("AND2"), "gate_8", 20, 30);
            ASSERT_NE(nl1_g8, nullptr);
            Gate* nl2_g8 = nl_2->create_gate(8, gl->get_gate_type_by_name("AND2"), "gate_8", 20, 40);
            ASSERT_NE(nl2_g8, nullptr);

            // data
            Gate* nl1_g9 = nl_1->create_gate(9, gl->get_gate_type_by_name("AND2"), "gate_9");
            ASSERT_NE(nl1_g9, nullptr);
            nl1_g9->set_data("some", "random", "string", "data");
            Gate* nl2_g9 = nl_2->create_gate(9, gl->get_gate_type_by_name("AND2"), "gate_9");
            ASSERT_NE(nl2_g9, nullptr);
            nl2_g9->set_data("some", "random", "string", "data");
            Gate* nl1_g10 = nl_1->create_gate(10, gl->get_gate_type_by_name("AND2"), "gate_10");
            ASSERT_NE(nl1_g10, nullptr);
            nl1_g10->set_data("some", "random", "string", "data");
            Gate* nl2_g10 = nl_2->create_gate(10, gl->get_gate_type_by_name("AND2"), "gate_10");
            ASSERT_NE(nl2_g10, nullptr);
            Gate* nl1_g11 = nl_1->create_gate(11, gl->get_gate_type_by_name("AND2"), "gate_11");
            ASSERT_NE(nl1_g11, nullptr);
            nl1_g11->set_data("some", "random", "string", "data");
            Gate* nl2_g11 = nl_2->create_gate(11, gl->get_gate_type_by_name("AND2"), "gate_11");
            ASSERT_NE(nl2_g11, nullptr);
            nl1_g11->set_data("some", "other", "string", "data");

            // Boolean functions
            Gate* nl1_g12 = nl_1->create_gate(12, gl->get_gate_type_by_name("AND2"), "gate_12");
            ASSERT_NE(nl1_g12, nullptr);
            nl1_g12->add_boolean_function("test", BooleanFunction::from_string("A & B"));
            Gate* nl2_g12 = nl_2->create_gate(12, gl->get_gate_type_by_name("AND2"), "gate_12");
            ASSERT_NE(nl2_g12, nullptr);
            nl2_g12->add_boolean_function("test", BooleanFunction::from_string("A & B"));
            Gate* nl1_g13 = nl_1->create_gate(13, gl->get_gate_type_by_name("AND2"), "gate_13");
            ASSERT_NE(nl1_g13, nullptr);
            nl1_g13->add_boolean_function("test", BooleanFunction::from_string("A & B"));
            Gate* nl2_g13 = nl_2->create_gate(13, gl->get_gate_type_by_name("AND2"), "gate_13");
            ASSERT_NE(nl2_g13, nullptr);
            Gate* nl1_g14 = nl_1->create_gate(14, gl->get_gate_type_by_name("AND2"), "gate_14");
            ASSERT_NE(nl1_g14, nullptr);
            nl1_g14->add_boolean_function("test", BooleanFunction::from_string("A & B"));
            Gate* nl2_g14 = nl_2->create_gate(14, gl->get_gate_type_by_name("AND2"), "gate_14");
            ASSERT_NE(nl2_g14, nullptr);
            nl2_g14->add_boolean_function("test", BooleanFunction::from_string("A | B"));

            EXPECT_TRUE(*nl1_g1 == *nl1_g1);        // identical gate pointer
            EXPECT_TRUE(*nl2_g1 == *nl2_g1); 
            EXPECT_TRUE(*nl1_g1 == *nl2_g1);        // identical gates, but different netlists
            EXPECT_TRUE(*nl2_g1 == *nl1_g1);
            EXPECT_FALSE(*nl1_g1 == *nl1_g2);       // different IDs
            EXPECT_FALSE(*nl2_g1 == *nl2_g2);
            EXPECT_FALSE(*nl1_g2 == *nl2_g2);       // different names
            EXPECT_FALSE(*nl2_g2 == *nl1_g2);
            EXPECT_FALSE(*nl1_g3 == *nl2_g3);       // different gate types
            EXPECT_FALSE(*nl2_g3 == *nl1_g3);
            EXPECT_FALSE(*nl1_g4 == *nl2_g4);       // one not marked as GND gate
            EXPECT_FALSE(*nl2_g4 == *nl1_g4);
            EXPECT_FALSE(*nl1_g5 == *nl2_g5);       // one not marked as VCC gate
            EXPECT_FALSE(*nl2_g5 == *nl1_g5);
            EXPECT_TRUE(*nl1_g6 == *nl2_g6);        // identical locations
            EXPECT_TRUE(*nl2_g6 == *nl1_g6);
            EXPECT_FALSE(*nl1_g7 == *nl2_g7);       // different x location
            EXPECT_FALSE(*nl2_g7 == *nl1_g7); 
            EXPECT_FALSE(*nl1_g8 == *nl2_g8);       // different y location
            EXPECT_FALSE(*nl2_g8 == *nl1_g8);
            EXPECT_TRUE(*nl1_g9 == *nl2_g9);        // identical data
            EXPECT_TRUE(*nl2_g9 == *nl1_g9);
            EXPECT_FALSE(*nl1_g10 == *nl2_g10);     // data missing
            EXPECT_FALSE(*nl2_g10 == *nl1_g10);
            EXPECT_FALSE(*nl1_g11 == *nl2_g11);     // different data
            EXPECT_FALSE(*nl2_g11 == *nl1_g11);
            EXPECT_TRUE(*nl1_g12 == *nl2_g12);      // identical Boolean function
            EXPECT_TRUE(*nl2_g12 == *nl1_g12);
            EXPECT_FALSE(*nl1_g13 == *nl2_g13);     // Boolean function missing
            EXPECT_FALSE(*nl2_g13 == *nl1_g13);
            EXPECT_FALSE(*nl1_g14 == *nl2_g14);     // different Boolean functions
            EXPECT_FALSE(*nl2_g14 == *nl1_g14);

            EXPECT_FALSE(*nl1_g1 != *nl1_g1);       // identical gate pointer
            EXPECT_FALSE(*nl2_g1 != *nl2_g1); 
            EXPECT_FALSE(*nl1_g1 != *nl2_g1);       // identical gates, but different netlists
            EXPECT_FALSE(*nl2_g1 != *nl1_g1);
            EXPECT_TRUE(*nl1_g1 != *nl1_g2);        // different IDs
            EXPECT_TRUE(*nl2_g1 != *nl2_g2);
            EXPECT_TRUE(*nl1_g2 != *nl2_g2);        // different names
            EXPECT_TRUE(*nl2_g2 != *nl1_g2);
            EXPECT_TRUE(*nl1_g3 != *nl2_g3);        // different gate types
            EXPECT_TRUE(*nl2_g3 != *nl1_g3);
            EXPECT_TRUE(*nl1_g4 != *nl2_g4);        // one not marked as GND gate
            EXPECT_TRUE(*nl2_g4 != *nl1_g4);
            EXPECT_TRUE(*nl1_g5 != *nl2_g5);        // one not marked as VCC gate
            EXPECT_TRUE(*nl2_g5 != *nl1_g5);
            EXPECT_FALSE(*nl1_g6 != *nl2_g6);       // identical locations
            EXPECT_FALSE(*nl2_g6 != *nl1_g6);
            EXPECT_TRUE(*nl1_g7 != *nl2_g7);        // different x location
            EXPECT_TRUE(*nl2_g7 != *nl1_g7); 
            EXPECT_TRUE(*nl1_g8 != *nl2_g8);        // different y location
            EXPECT_TRUE(*nl2_g8 != *nl1_g8);
            EXPECT_FALSE(*nl1_g9 != *nl2_g9);       // identical data
            EXPECT_FALSE(*nl2_g9 != *nl1_g9);
            EXPECT_TRUE(*nl1_g10 != *nl2_g10);      // data missing
            EXPECT_TRUE(*nl2_g10 != *nl1_g10);
            EXPECT_TRUE(*nl1_g11 != *nl2_g11);      // different data
            EXPECT_TRUE(*nl2_g11 != *nl1_g11);
            EXPECT_FALSE(*nl1_g12 != *nl2_g12);     // identical Boolean function
            EXPECT_FALSE(*nl2_g12 != *nl1_g12);
            EXPECT_TRUE(*nl1_g13 != *nl2_g13);      // Boolean function missing
            EXPECT_TRUE(*nl2_g13 != *nl1_g13);
            EXPECT_TRUE(*nl1_g14 != *nl2_g14);      // different Boolean functions
            EXPECT_TRUE(*nl2_g14 != *nl1_g14);
        }
        TEST_END
    }

    /**
     * Testing the function set_name and get_name
     *
     * Functions: get_name, set_name
     */
    TEST_F(GateTest, check_set_and_get_name)
    {
        TEST_START
        // ########################
        // POSITIVE TESTS
        // ########################
        // Create a Gate and append it to its netlist
        auto nl         = test_utils::create_empty_netlist();
        Gate* test_gate = nl->create_gate(nl->get_gate_library()->get_gate_type_by_name("AND2"), "gate_name");

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
        NO_COUT(test_gate->set_name("gate_name"));    // Set it initially
        NO_COUT(test_gate->set_name(""));             // Set it initially
        EXPECT_EQ(test_gate->get_name(), "gate_name");

        TEST_END
    }

    /**
     * Testing functions which returns the pin types. Further test for different Gate types
     * are for in the tests for netlist (get_input_pin_types, ... )
     *
     * Functions: get_input_pin_types, get_output_pin_types, get_inout_pin_types
     */
    TEST_F(GateTest, check_pin_types)
    {
        TEST_START
        // Create a Gate and append it to its netlist
        auto nl         = test_utils::create_empty_netlist();
        Gate* test_gate = nl->create_gate(nl->get_gate_library()->get_gate_type_by_name("AND2"), "gate_name");

        EXPECT_EQ(test_gate->get_input_pins(), std::vector<std::string>({"I0", "I1"}));
        EXPECT_EQ(test_gate->get_output_pins(), std::vector<std::string>({"O"}));

        TEST_END
    }

    /**
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
    TEST_F(GateTest, check_get_fan_in_nets)
    {
        TEST_START

        // Create the example
        auto nl = test_utils::create_example_netlist();

        // ########################
        // POSITIVE TESTS
        // ########################
        {
            // All input pins are occupied
            Gate* gate_0                    = nl->get_gate_by_id(MIN_GATE_ID + 0);
            std::vector<Net*> fan_in_nets_0 = {nl->get_net_by_id(MIN_NET_ID + 30), nl->get_net_by_id(MIN_NET_ID + 20)};
            EXPECT_EQ(gate_0->get_fan_in_nets(), fan_in_nets_0);
        }
        {
            // Not all input pins are occupied
            Gate* gate_5                    = nl->get_gate_by_id(MIN_GATE_ID + 5);
            std::vector<Net*> fan_in_nets_5 = {nl->get_net_by_id(MIN_NET_ID + 045)};
            EXPECT_EQ(gate_5->get_fan_in_nets(), fan_in_nets_5);
        }
        {
            // No input pins are occupied
            Gate* gate_6                    = nl->get_gate_by_id(MIN_GATE_ID + 6);
            std::vector<Net*> fan_in_nets_6 = {};
            EXPECT_EQ(gate_6->get_fan_in_nets(), fan_in_nets_6);
        }
        {
            // No input-pins exist
            Gate* gate_1                    = nl->get_gate_by_id(MIN_GATE_ID + 1);
            std::vector<Net*> fan_in_nets_1 = {};
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
    TEST_F(GateTest, check_get_fan_out_nets)
    {
        TEST_START

        // Create the example
        auto nl = test_utils::create_example_netlist(0);

        // ########################
        // POSITIVE TESTS
        // ########################
        {
            // All output pins are occupied
            Gate* gate_0                     = nl->get_gate_by_id(MIN_GATE_ID + 0);
            std::vector<Net*> fan_out_nets_0 = {nl->get_net_by_id(MIN_NET_ID + 045)};
            EXPECT_EQ(gate_0->get_fan_out_nets(), fan_out_nets_0);
        }
        {
            // Not all output pins are occupied
            Gate* gate_7                     = nl->get_gate_by_id(MIN_GATE_ID + 7);
            std::vector<Net*> fan_out_nets_7 = {nl->get_net_by_id(MIN_NET_ID + 78)};
            EXPECT_EQ(gate_7->get_fan_out_nets(), fan_out_nets_7);
        }
        {
            // No output pins are occupied
            Gate* gate_8                     = nl->get_gate_by_id(MIN_GATE_ID + 8);
            std::vector<Net*> fan_out_nets_8 = {};
            EXPECT_EQ(gate_8->get_fan_out_nets(), fan_out_nets_8);
        }
        {
            // No output pin exist
            Gate* gate_6                     = nl->get_gate_by_id(MIN_GATE_ID + 6);
            std::vector<Net*> fan_out_nets_6 = {};
            EXPECT_EQ(gate_6->get_fan_out_nets(), fan_out_nets_6);
        }
        TEST_END
    }

    /**
     * Testing functions which returns the fan-in Net, connected to a specific pin-type,
     * by using the example netlist (see above)
     *
     * Functions: get_fan_in_net
     */
    TEST_F(GateTest, check_get_fan_in_net)
    {
        TEST_START

        // Create the example
        auto nl = test_utils::create_example_netlist();

        // ########################
        // POSITIVE TESTS
        // ########################
        {
            // Get an existing Net at an existing pin-type
            Gate* gate_0 = nl->get_gate_by_id(MIN_GATE_ID + 0);
            EXPECT_EQ(gate_0->get_fan_in_net("I0"), nl->get_net_by_id(MIN_NET_ID + 30));
            EXPECT_EQ(gate_0->get_fan_in_net("I1"), nl->get_net_by_id(MIN_NET_ID + 20));
        }
        {
            // Get the Net of a pin where no Net is connected
            Gate* gate_5 = nl->get_gate_by_id(MIN_GATE_ID + 5);
            EXPECT_EQ(gate_5->get_fan_in_net("I1"), nullptr);
        }
        {
            // Get the Net of a non existing pin
            Gate* gate_0 = nl->get_gate_by_id(MIN_GATE_ID + 0);
            EXPECT_EQ(gate_0->get_fan_in_net("NEx_PIN"), nullptr);
        }
        {
            // Get the Net of a non existing pin-type of a Gate where no input pin exist
            Gate* gate_1 = nl->get_gate_by_id(MIN_GATE_ID + 1);
            EXPECT_EQ(gate_1->get_fan_in_net("NEx_PIN"), nullptr);
        }
        {
            // Pass an empty string
            Gate* gate_0 = nl->get_gate_by_id(MIN_GATE_ID + 0);
            EXPECT_EQ(gate_0->get_fan_in_net(""), nullptr);
        }
        TEST_END
    }

    /**
     * Testing functions which returns the fan-out Net, connected to a specific pin-type,
     * by using the example netlist (see above)
     *
     * Functions: get_fan_out_net
     */
    TEST_F(GateTest, check_get_fan_out_net)
    {
        TEST_START

        // Create the example
        auto nl = test_utils::create_example_netlist();

        // ########################
        // POSITIVE TESTS
        // ########################
        {
            // Get an existing Net at an existing pin-type
            Gate* gate_0 = nl->get_gate_by_id(MIN_GATE_ID + 0);
            EXPECT_EQ(gate_0->get_fan_out_net("O"), nl->get_net_by_id(MIN_NET_ID + 045));
        }
        {
            // Get the Net of a pin where no Net is connected
            Gate* gate_4 = nl->get_gate_by_id(MIN_GATE_ID + 4);
            EXPECT_EQ(gate_4->get_fan_out_net("O"), nullptr);
        }
        {
            // Get the Net of a non existing pin
            Gate* gate_0 = nl->get_gate_by_id(MIN_GATE_ID + 0);
            EXPECT_EQ(gate_0->get_fan_out_net("NEx_PIN"), nullptr);
        }
        {
            // Get the Net of a non existing pin-type of a Gate where no output pin exist
            Gate* gate_6 = nl->get_gate_by_id(MIN_GATE_ID + 6);
            EXPECT_EQ(gate_6->get_fan_out_net("NEx_PIN"), nullptr);
        }
        {
            // Pass an empty string
            Gate* gate_0 = nl->get_gate_by_id(MIN_GATE_ID + 0);
            EXPECT_EQ(gate_0->get_fan_out_net(""), nullptr);
        }
        TEST_END
    }

    /**
     * Testing the get_predecessors function by using the example netlists (see above)
     *
     * Functions: get_predecessors
     */
    TEST_F(GateTest, check_get_predecessors)
    {
        TEST_START
        // Create the examples
        auto nl_1 = test_utils::create_example_netlist();
        auto nl_2 = test_utils::create_example_netlist_2();

        // ########################
        // POSITIVE TESTS
        // ########################
        {
            // Get predecessors for a Gate with multiple predecessors (some of them are the same Gate)
            Gate* gate_1                = nl_2->get_gate_by_id(MIN_GATE_ID + 1);
            std::vector<Endpoint*> pred = {test_utils::get_endpoint(nl_2.get(), MIN_GATE_ID + 0, "O", false),
                                           test_utils::get_endpoint(nl_2.get(), MIN_GATE_ID + 0, "O", false),
                                           test_utils::get_endpoint(nl_2.get(), MIN_GATE_ID + 0, "O", false),
                                           test_utils::get_endpoint(nl_2.get(), MIN_GATE_ID + 2, "O", false)};
            EXPECT_TRUE(test_utils::vectors_have_same_content(gate_1->get_predecessors(), pred));
            EXPECT_EQ(gate_1->get_predecessors().size(), (size_t)4);
        }
        {
            // Get predecessors for a Gate with no predecessors
            Gate* gate_0 = nl_2->get_gate_by_id(MIN_GATE_ID + 0);
            EXPECT_TRUE(gate_0->get_predecessors().empty());
        }
        {
            // Get predecessors for a given (existing) output pin type
            Gate* gate_3                = nl_2->get_gate_by_id(MIN_GATE_ID + 3);
            std::vector<Endpoint*> pred = {test_utils::get_endpoint(nl_2.get(), MIN_GATE_ID + 0, "O", false)};
            EXPECT_TRUE(test_utils::vectors_have_same_content(gate_3->get_predecessors(test_utils::adjacent_pin_filter("O")), pred));
        }
        {
            // Get predecessors for a given (non-existing) output pin type
            Gate* gate_1 = nl_2->get_gate_by_id(MIN_GATE_ID + 1);
            EXPECT_TRUE(gate_1->get_predecessors(test_utils::adjacent_pin_filter("NEx_PIN")).empty());
            EXPECT_EQ(gate_1->get_predecessors(test_utils::adjacent_pin_filter("NEx_PIN")).size(), (size_t)0);
        }
        {
            // Get predecessors for a given (existing) input pin type
            Gate* gate_3                = nl_2->get_gate_by_id(MIN_GATE_ID + 3);
            std::vector<Endpoint*> pred = {test_utils::get_endpoint(nl_2.get(), MIN_GATE_ID + 0, "O", false)};
            EXPECT_TRUE(test_utils::vectors_have_same_content(gate_3->get_predecessors(test_utils::starting_pin_filter("I0")), pred));
        }
        {
            // Get predecessors for a given (existing) unconnected input pin type
            Gate* gate_0                = nl_2->get_gate_by_id(MIN_GATE_ID + 0);
            std::vector<Endpoint*> pred = {};
            EXPECT_TRUE(test_utils::vectors_have_same_content(gate_0->get_predecessors(test_utils::starting_pin_filter("I0")), pred));
        }
        {
            // Get predecessors for a given (non-existing) input pin type
            Gate* gate_1 = nl_2->get_gate_by_id(MIN_GATE_ID + 1);
            EXPECT_TRUE(gate_1->get_predecessors(test_utils::starting_pin_filter("NEx_PIN")).empty());
            EXPECT_EQ(gate_1->get_predecessors(test_utils::starting_pin_filter("NEx_PIN")).size(), (size_t)0);
        }
        {
            // Get predecessors for a given (existing) Gate type
            Gate* gate_0                = nl_1->get_gate_by_id(MIN_GATE_ID + 0);
            std::vector<Endpoint*> pred = {test_utils::get_endpoint(nl_1.get(), MIN_GATE_ID + 3, "O", false)};
            EXPECT_TRUE(test_utils::vectors_have_same_content(gate_0->get_predecessors(test_utils::adjacent_gate_type_filter("gate_1_to_1")), pred));
            EXPECT_EQ(gate_0->get_predecessors(test_utils::adjacent_gate_type_filter("gate_1_to_1")).size(), (size_t)1);
        }
        {
            // Get predecessors for a given (non-existing) Gate type
            Gate* gate_1 = nl_2->get_gate_by_id(MIN_GATE_ID + 1);
            EXPECT_TRUE(gate_1->get_predecessors(test_utils::adjacent_gate_type_filter("NEx_GATE")).empty());
            EXPECT_EQ(gate_1->get_predecessors(test_utils::adjacent_gate_type_filter("NEx_GATE")).size(), (size_t)0);
        }
        // ########################
        // NEGATIVE TESTS
        // ########################
        auto nl_neg = test_utils::create_example_netlist_negative();
        {
            // Get predecessors for a Gate with unconnected nets
            Gate* gate_0 = nl_neg->get_gate_by_id(MIN_GATE_ID + 0);
            EXPECT_TRUE(gate_0->get_predecessors().empty());
            EXPECT_EQ(gate_0->get_predecessors().size(), (size_t)0);
        }
        {
            // Get predecessors for a Gate with unconnected nets and a set input pin type
            Gate* gate_0 = nl_neg->get_gate_by_id(MIN_GATE_ID + 0);
            EXPECT_TRUE(gate_0->get_predecessors(test_utils::starting_pin_filter("I")).empty());
            EXPECT_EQ(gate_0->get_predecessors(test_utils::starting_pin_filter("I")).size(), (size_t)0);
        }
        TEST_END
    }

    /**
     * Testing the get_successors function by using the example netlists (see above).
     *
     * Functions: get_successors
     */
    TEST_F(GateTest, check_get_successors)
    {
        TEST_START
        // Create the examples
        auto nl_1 = test_utils::create_example_netlist();
        auto nl_2 = test_utils::create_example_netlist_2();

        // ########################
        // POSITIVE TESTS
        // ########################
        {
            // Get successors for a Gate with multiple successors (some of them are the same Gate)
            Gate* gate_0                = nl_2->get_gate_by_id(MIN_GATE_ID + 0);
            std::vector<Endpoint*> succ = {test_utils::get_endpoint(nl_2.get(), MIN_GATE_ID + 1, "I0", true),
                                           test_utils::get_endpoint(nl_2.get(), MIN_GATE_ID + 1, "I1", true),
                                           test_utils::get_endpoint(nl_2.get(), MIN_GATE_ID + 1, "I2", true),
                                           test_utils::get_endpoint(nl_2.get(), MIN_GATE_ID + 3, "I0", true)};
            EXPECT_TRUE(test_utils::vectors_have_same_content(gate_0->get_successors(), succ));
            EXPECT_EQ(gate_0->get_successors().size(), (size_t)4);
        }
        {
            // Get successors for a Gate no successors
            Gate* gate_1 = nl_2->get_gate_by_id(MIN_GATE_ID + 1);
            EXPECT_TRUE(gate_1->get_successors().empty());
        }
        {
            // Get successors for a given (existing) input pin type
            Gate* gate_0                = nl_2->get_gate_by_id(MIN_GATE_ID + 0);
            std::vector<Endpoint*> succ = {test_utils::get_endpoint(nl_2.get(), MIN_GATE_ID + 1, "I0", true), test_utils::get_endpoint(nl_2.get(), MIN_GATE_ID + 3, "I0", true)};
            EXPECT_TRUE(test_utils::vectors_have_same_content(gate_0->get_successors(test_utils::adjacent_pin_filter("I0")), succ));
            EXPECT_EQ(gate_0->get_successors(test_utils::adjacent_pin_filter("I0")).size(), (size_t)2);
        }
        {
            // Get successors for a given (non-existing) intput pin type
            Gate* gate_0 = nl_2->get_gate_by_id(MIN_GATE_ID + 0);
            EXPECT_TRUE(gate_0->get_successors(test_utils::adjacent_pin_filter("NEx_PIN")).empty());
            EXPECT_EQ(gate_0->get_successors(test_utils::adjacent_pin_filter("NEx_PIN")).size(), (size_t)0);
        }
        {
            // Get successors for a given (existing) output pin type
            Gate* gate_0                = nl_2->get_gate_by_id(MIN_GATE_ID + 0);
            std::vector<Endpoint*> succ = {test_utils::get_endpoint(nl_2.get(), MIN_GATE_ID + 1, "I0", true),
                                           test_utils::get_endpoint(nl_2.get(), MIN_GATE_ID + 1, "I1", true),
                                           test_utils::get_endpoint(nl_2.get(), MIN_GATE_ID + 1, "I2", true),
                                           test_utils::get_endpoint(nl_2.get(), MIN_GATE_ID + 3, "I0", true)};
            EXPECT_TRUE(test_utils::vectors_have_same_content(gate_0->get_successors(test_utils::starting_pin_filter("O")), succ));
            EXPECT_EQ(gate_0->get_successors(test_utils::starting_pin_filter("O")).size(), (size_t)4);
        }
        {
            // Get successors for a given (non-existing) output pin type
            Gate* gate_0 = nl_2->get_gate_by_id(MIN_GATE_ID + 0);
            EXPECT_TRUE(gate_0->get_successors(test_utils::starting_pin_filter("NEx_PIN")).empty());
        }
        {
            // Get successors for a given (existing) output pin type with no successors
            Gate* gate_1 = nl_2->get_gate_by_id(MIN_GATE_ID + 1);
            EXPECT_TRUE(gate_1->get_successors(test_utils::starting_pin_filter("O")).empty());
        }
        {
            // Get successors for a given (existing) Gate type
            Gate* gate_0                = nl_1->get_gate_by_id(MIN_GATE_ID + 0);
            std::vector<Endpoint*> succ = {test_utils::get_endpoint(nl_1.get(), MIN_GATE_ID + 4, "I", true)};
            EXPECT_TRUE(test_utils::vectors_have_same_content(gate_0->get_successors(test_utils::adjacent_gate_type_filter("gate_1_to_1")), succ));
            EXPECT_EQ(gate_0->get_successors(test_utils::adjacent_gate_type_filter("gate_1_to_1")).size(), (size_t)1);
        }
        {
            // Get successors for a given (non-existing) Gate type
            Gate* gate_0 = nl_1->get_gate_by_id(MIN_GATE_ID + 0);
            EXPECT_TRUE(gate_0->get_successors(test_utils::adjacent_gate_type_filter("NEx_GATE")).empty());
            EXPECT_EQ(gate_0->get_successors(test_utils::adjacent_gate_type_filter("NEx_GATE")).size(), (size_t)0);
        }
        // ########################
        // NEGATIVE TESTS
        // ########################
        auto nl_neg = test_utils::create_example_netlist_negative();
        {
            // Get successors for a Gate with unconnected nets
            Gate* gate_0 = nl_neg->get_gate_by_id(MIN_GATE_ID + 0);
            EXPECT_TRUE(gate_0->get_successors().empty());
        }
        TEST_END
    }

    /**
     * Testing the get_unique_predecessors and get_unique_successors function
     *
     * Functions: get_unique_predecessors, get_unique_successors
     */
    TEST_F(GateTest, check_get_unique_predecessors_and_successors)
    {
        TEST_START
        // Create the examples
        auto nl_2 = test_utils::create_example_netlist_2();

        // ########################
        // POSITIVE TESTS
        // ########################
        {
            // Get the unique predecessors
            Gate* gate_1            = nl_2->get_gate_by_id(MIN_GATE_ID + 1);
            std::vector<Gate*> pred = {nl_2->get_gate_by_id(MIN_GATE_ID + 0), nl_2->get_gate_by_id(MIN_GATE_ID + 2)};
            std::vector<Gate*> res  = gate_1->get_unique_predecessors();
            EXPECT_TRUE(test_utils::vectors_have_same_content(res, pred));
        }
        {
            // Get the unique successors
            Gate* gate_0            = nl_2->get_gate_by_id(MIN_GATE_ID + 0);
            std::vector<Gate*> succ = {nl_2->get_gate_by_id(MIN_GATE_ID + 1), nl_2->get_gate_by_id(MIN_GATE_ID + 3)};
            std::vector<Gate*> res  = gate_0->get_unique_successors();
            EXPECT_TRUE(test_utils::vectors_have_same_content(res, succ));
        }
        TEST_END
    }

    /**
     * Testing the get_predecessor function
     *
     * Functions: get_predecessor
     */
    TEST_F(GateTest, check_get_predecessor)
    {
        TEST_START
        // Create the examples
        auto nl_2 = test_utils::create_example_netlist_2();

        // ########################
        // POSITIVE TESTS
        // ########################
        {
            // Get predecessor for a given (existing) input pin type
            auto gate_3 = nl_2->get_gate_by_id(MIN_GATE_ID + 3);
            auto pred   = test_utils::get_endpoint(nl_2.get(), MIN_GATE_ID + 0, "O", false);
            EXPECT_TRUE(gate_3->get_predecessor("I0") == pred);
        }
        {
            // Get predecessor for a given (existing) input pin type with no predecessors
            auto gate_0 = nl_2->get_gate_by_id(MIN_GATE_ID + 0);
            EXPECT_TRUE(gate_0->get_predecessor("I0") == nullptr);
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
    TEST_F(GateTest, check_gnd_vcc_gate_handling)
    {
        TEST_START
        {// Mark and unmark a global vcc Gate
            auto nl = test_utils::create_empty_netlist();
            Gate* vcc_gate = nl->create_gate(nl->get_gate_library()->get_gate_type_by_name("VCC"), "vcc_gate");

            vcc_gate->mark_vcc_gate();
            EXPECT_TRUE(vcc_gate->is_vcc_gate());
            EXPECT_TRUE(nl->is_vcc_gate(vcc_gate));

            vcc_gate->unmark_vcc_gate();
            EXPECT_FALSE(vcc_gate->is_vcc_gate());
            EXPECT_FALSE(nl->is_vcc_gate(vcc_gate));
        }    // namespace hal
        {
            // Mark and unmark a global gnd Gate
            auto nl        = test_utils::create_empty_netlist();
            Gate* gnd_gate = nl->create_gate(nl->get_gate_library()->get_gate_type_by_name("GND"), "gnd_gate");

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
    TEST_F(GateTest, check_get_module)
    {
        TEST_START
        {// get the Module of a Gate (the top_module), then add it to another Module and check again
            // -- create the Gate at the top_module
            auto nl = test_utils::create_empty_netlist();
            Gate* test_gate = nl->create_gate(nl->get_gate_library()->get_gate_type_by_name("BUF"), "test_gate");

            EXPECT_EQ(test_gate->get_module(), nl->get_top_module());

            // -- move the Gate in the test_module
            Module* test_module = nl->create_module("test_module", nl->get_top_module());
            test_module->assign_gate(test_gate);

            EXPECT_EQ(test_gate->get_module(), test_module);

            // -- delete the test_module, so the Gate should be moved in the top_module again
            nl->delete_module(test_module);
            EXPECT_EQ(test_gate->get_module(), nl->get_top_module());
        }
        TEST_END
    }

    /**
     * Testing the get_grouping function
     *
     * Functions: get_grouping
     */
    TEST_F(GateTest, check_get_grouping)
    {
        TEST_START
        {// get the grouping of a Gate (nullptr), then add it to another grouping and check again
            auto nl = test_utils::create_empty_netlist();
            Gate* test_gate = nl->create_gate(nl->get_gate_library()->get_gate_type_by_name("BUF"), "test_gate");

            EXPECT_EQ(test_gate->get_grouping(), nullptr);

            // -- move the Gate in the test_grouping
            Grouping* test_grouping = nl->create_grouping("test_grouping");
            test_grouping->assign_gate(test_gate);

            EXPECT_EQ(test_gate->get_grouping(), test_grouping);

            // -- delete the test_grouping, so the Gate should be nullptr again
            nl->delete_grouping(test_grouping);
            EXPECT_EQ(test_gate->get_grouping(), nullptr);
        }
        TEST_END
    }

    /**
     * Testing storage and access of Gate locations
     *
     * Functions: get_location_x, get_location_y, get_location, has_location, set_location_x, set_location_y, set_location
     */
    TEST_F(GateTest, check_location_storage)
    {
        TEST_START
        {// Create a Gate with a location and change it afterwards
            auto nl = test_utils::create_empty_netlist();
            Gate* test_gate = nl->create_gate(nl->get_gate_library()->get_gate_type_by_name("BUF"), "test_gate", 1, 2);
            EXPECT_EQ(test_gate->get_location_x(), 1);
            EXPECT_EQ(test_gate->get_location_y(), 2);
            EXPECT_EQ(test_gate->get_location(), std::make_pair(1, 2));
            // -- set a new x location
            test_gate->set_location_x(3);
            EXPECT_EQ(test_gate->get_location(), std::make_pair(3, 2));
            // -- set a new y location
            test_gate->set_location_y(4);
            EXPECT_EQ(test_gate->get_location(), std::make_pair(3, 4));
            // -- set a new location
            test_gate->set_location(std::make_pair(5, 6));
            EXPECT_EQ(test_gate->get_location(), std::make_pair(5, 6));
            // -- set the same location again
            test_gate->set_location(std::make_pair(5, 6));
            EXPECT_EQ(test_gate->get_location(), std::make_pair(5, 6));
        }
        {
            // Test the has_location function
            auto nl         = test_utils::create_empty_netlist();
            Gate* test_gate = nl->create_gate(nl->get_gate_library()->get_gate_type_by_name("BUF"), "test_gate", 1, 2);
            // -- both coordinates are >= 0
            test_gate->set_location(std::make_pair(1, 2));
            EXPECT_TRUE(test_gate->has_location());
            test_gate->set_location(std::make_pair(0, 0));
            EXPECT_TRUE(test_gate->has_location());
            // -- not both coordinates are >= 0
            test_gate->set_location(std::make_pair(-1, 2));
            EXPECT_FALSE(test_gate->has_location());
            test_gate->set_location(std::make_pair(-1, -1));
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
    TEST_F(GateTest, check_boolean_function_access)
    {
        TEST_START
        {
            // Access the boolean function of a gate_type
            auto nl = test_utils::create_empty_netlist();
            Gate* test_gate = nl->create_gate(nl->get_gate_library()->get_gate_type_by_name("INV"), "test_gate");
            std::unordered_map<std::string, BooleanFunction> functions = test_gate->get_boolean_functions();
            EXPECT_EQ(functions, (std::unordered_map<std::string, BooleanFunction>({{"O", BooleanFunction::from_string("!I")}})));

            test_gate->add_boolean_function("new_bf", BooleanFunction::from_string("I"));

            functions = test_gate->get_boolean_functions();
            EXPECT_EQ(functions, (std::unordered_map<std::string, BooleanFunction>({{"O", BooleanFunction::from_string("!I")}, {"new_bf", BooleanFunction::from_string("I")}})));

            EXPECT_EQ(test_gate->get_boolean_function("O"), BooleanFunction::from_string("!I"));

            EXPECT_EQ(test_gate->get_boolean_function("new_bf"), BooleanFunction::from_string("I"));

            // should be function of first output pin
            EXPECT_EQ(test_gate->get_boolean_function(), BooleanFunction::from_string("!I"));
        }
        // NEGATIVE
        {
            // Get a boolean function for a name that is unknown.
            auto nl = test_utils::create_empty_netlist();
            Gate* test_gate = nl->create_gate(nl->get_gate_library()->get_gate_type_by_name("INV"), "test_gate");

            EXPECT_TRUE(test_gate->get_boolean_function("unknown_name").is_empty());
        }
        {
            // Call the get_boolean_function function with no parameter, for a Gate with no outputs
            auto nl = test_utils::create_empty_netlist();
            Gate* test_gate = nl->create_gate(nl->get_gate_library()->get_gate_type_by_name("RAM"), "test_gate");

            EXPECT_TRUE(test_gate->get_boolean_function().is_empty());
        }
        TEST_END
    }

    /**
     * Testing the access on the lut function of a lut Gate.
     *
     * Functions: get_boolean_function, get_boolean_functions, add_boolean_function
     */
    TEST_F(GateTest, check_lut_function) {
        TEST_START
        {
            // Get the boolean function of the lut in different ways
            // is not taken into account!
            auto nl = test_utils::create_empty_netlist();
            GateType* lut_type = nl->get_gate_library()->get_gate_type_by_name("LUT3");
            Gate* lut_gate = nl->create_gate(lut_type, "lut");

            int i = 1;
            lut_gate->set_data(lut_type->get_config_data_category(),
                            lut_type->get_config_data_identifier(),
                            "bit_vector",
                            i_to_hex_string(i, 2));

            // Testing the access via the function get_boolean_function
            EXPECT_EQ(lut_gate->get_boolean_function("O").get_truth_table(lut_type->get_input_pins()), get_truth_table_from_i(i, 8));

            // Test the access via the get_boolean_functions map
            std::unordered_map<std::string, BooleanFunction> functions = lut_gate->get_boolean_functions();
            ASSERT_TRUE(functions.find("O") != functions.end());
            EXPECT_EQ(functions["O"].get_truth_table(lut_type->get_input_pins()), get_truth_table_from_i(i, 8));
        }
        {
            // Access the boolean function of a lut, that is stored in ascending order
            auto nl = test_utils::create_empty_netlist();
            GateType* lut_type = nl->get_gate_library()->get_gate_type_by_name("LUT3");
            Gate* lut_gate = nl->create_gate(lut_type, "lut");

            for (int i = 0x0; i <= 0xff; i++) 
            {
                lut_gate->set_data(lut_type->get_config_data_category(),
                                lut_type->get_config_data_identifier(),
                                "bit_vector",
                                i_to_hex_string(i));
                EXPECT_EQ(lut_gate->get_boolean_function("O").get_truth_table(lut_type->get_input_pins()),
                        get_truth_table_from_hex_string(i_to_hex_string(i), 8, false));
            }
        }
        {
            // Access the boolean function of a lut, that is stored in descending order
            auto nl = test_utils::create_empty_netlist();
            GateType* lut_type = nl->get_gate_library()->get_gate_type_by_name("LUT3");
            Gate* lut_gate = nl->create_gate(lut_type, "lut");
            lut_type->set_lut_init_ascending(false);

            for (int i = 0x0; i <= 0xff; i++) {
                lut_gate->set_data(lut_type->get_config_data_category(),
                                lut_type->get_config_data_identifier(),
                                "bit_vector",
                                i_to_hex_string(i));
                EXPECT_EQ(lut_gate->get_boolean_function("O").get_truth_table(lut_type->get_input_pins()),
                        get_truth_table_from_hex_string(i_to_hex_string(i), 8, true));
            }

            lut_type->set_lut_init_ascending(true);
        }
        {
            // Add a boolean function to a lut pin
            auto nl = test_utils::create_empty_netlist();
            GateType* lut_type = nl->get_gate_library()->get_gate_type_by_name("LUT3");
            Gate* lut_gate = nl->create_gate(lut_type, "lut");

            BooleanFunction lut_bf = BooleanFunction::from_string("I0 & I1 & I2");
            lut_gate->add_boolean_function("O", lut_bf);
            EXPECT_EQ(lut_gate->get_boolean_functions().size(), 1);
            EXPECT_EQ(lut_gate->get_boolean_function("O"), lut_bf);
            EXPECT_EQ(lut_gate->get_data(lut_type->get_config_data_category(), lut_type->get_config_data_identifier()), std::make_tuple(std::string("bit_vector"), std::string("1")));
        }
        // NEGATIVE
        {
            // There is an empty hex string at the config data path
            auto nl = test_utils::create_empty_netlist();
            GateType* lut_type = nl->get_gate_library()->get_gate_type_by_name("LUT3");
            Gate* lut_gate = nl->create_gate(lut_type, "lut");

            lut_gate->set_data(lut_type->get_config_data_category(), lut_type->get_config_data_identifier(), "bit_vector", "");
            EXPECT_EQ(lut_gate->get_boolean_function("O").get_truth_table(lut_type->get_input_pins()), get_truth_table_from_i(0, 8));
        }
        {
            // There is invalid data at the config data path
            NO_COUT_TEST_BLOCK;
            auto nl = test_utils::create_empty_netlist();
            GateType* lut_type = nl->get_gate_library()->get_gate_type_by_name("LUT3");
            Gate* lut_gate = nl->create_gate(lut_type, "lut");

            lut_gate->set_data(lut_type->get_config_data_category(), lut_type->get_config_data_identifier(), "bit_vector", "NOHx");
            EXPECT_EQ(lut_gate->get_boolean_function("O").get_truth_table(lut_type->get_input_pins()), std::vector<BooleanFunction::Value>(8, BooleanFunction::X));

        }
        TEST_END
    }

    /*************************************
     * Event System
     *************************************/

    /**
     * Testing the triggering of events.
     */
    TEST_F(GateTest, check_events)
    {
        TEST_START
        const u32 NO_DATA = 0xFFFFFFFF;
        {
            // Check the 'gate created' event handling
            // -- create the listener
            test_utils::EventListener<void, GateEvent::event, Gate*, u32> gate_created_listener;
            std::function<void(GateEvent::event, Gate*, u32)> cb =
                gate_created_listener.get_conditional_callback([=](GateEvent::event ev, Gate* g, u32 id) { return ev == GateEvent::event::created; });

            std::unique_ptr<Netlist> nl = test_utils::create_empty_netlist();

            // -- register the callback hook
            nl->get_event_handler()->register_callback("gate_created_cb", cb);

            // -- check if the event is triggered
            Gate* test_gate             = nl->create_gate(nl->get_gate_library()->get_gate_type_by_name("BUF"), "gate_name");
            EXPECT_EQ(gate_created_listener.get_event_count(), 1);
            EXPECT_EQ(gate_created_listener.get_last_parameters(), std::make_tuple(GateEvent::event::created, test_gate, NO_DATA));

            // -- remove the callback hook
            nl->get_event_handler()->unregister_callback("gate_created_cb");
        }
        {
            // Check the 'gate removed' event handling
            // -- create the listener
            test_utils::EventListener<void, GateEvent::event, Gate*, u32> gate_removed_listener;
            std::function<void(GateEvent::event, Gate*, u32)> cb =
                gate_removed_listener.get_conditional_callback([=](GateEvent::event ev, Gate* g, u32 id) { return ev == GateEvent::event::removed; });

            std::unique_ptr<Netlist> nl = test_utils::create_empty_netlist();

            // -- register the callback hook
            nl->get_event_handler()->register_callback("gate_removed_cb", cb);

            // -- check if the event is triggered
            Gate* test_gate             = nl->create_gate(nl->get_gate_library()->get_gate_type_by_name("BUF"), "gate_name");
            EXPECT_EQ(gate_removed_listener.get_event_count(), 0);
            nl->delete_gate(test_gate);
            EXPECT_EQ(gate_removed_listener.get_event_count(), 1);
            EXPECT_EQ(gate_removed_listener.get_last_parameters(), std::make_tuple(GateEvent::event::removed, test_gate, NO_DATA));

            // -- remove the callback hook
            nl->get_event_handler()->unregister_callback("gate_removed_cb");
        }
        {
            // Check the 'name changed' event handling
            // -- create the listener
            test_utils::EventListener<void, GateEvent::event, Gate*, u32> gate_name_changed_listener;
            std::function<void(GateEvent::event, Gate*, u32)> cb =
                gate_name_changed_listener.get_conditional_callback([=](GateEvent::event ev, Gate* g, u32 id) { return ev == GateEvent::event::name_changed; });
            std::unique_ptr<Netlist> nl = test_utils::create_empty_netlist();

            // -- register the callback hook
            nl->get_event_handler()->register_callback("gate_name_changed_cb", cb);

            // -- check if the event is triggered
            Gate* test_gate             = nl->create_gate(nl->get_gate_library()->get_gate_type_by_name("BUF"), "gate_name");
            test_gate->set_name("gate_name");    // if the old name equals the new one, the event should not be triggered
            EXPECT_EQ(gate_name_changed_listener.get_event_count(), 0);
            gate_name_changed_listener.reset_events();
            test_gate->set_name("new_name");
            EXPECT_EQ(gate_name_changed_listener.get_event_count(), 1);
            EXPECT_EQ(gate_name_changed_listener.get_last_parameters(), std::make_tuple(GateEvent::event::name_changed, test_gate, NO_DATA));

            // -- remove the callback hook
            nl->get_event_handler()->unregister_callback("gate_name_changed_cb");
        }
        {
            // Check the 'gate location changed' event handling
            // -- create the listener
            test_utils::EventListener<void, GateEvent::event, Gate*, u32> gate_location_changed_listener;
            std::function<void(GateEvent::event, Gate*, u32)> cb =
                gate_location_changed_listener.get_conditional_callback([=](GateEvent::event ev, Gate* g, u32 id) { return ev == GateEvent::event::location_changed; });

            std::unique_ptr<Netlist> nl = test_utils::create_empty_netlist();

            // -- register the callback hook
            nl->get_event_handler()->register_callback("gate_location_changed_cb", cb);

            // -- check if the event is triggered
            Gate* test_gate = nl->create_gate(nl->get_gate_library()->get_gate_type_by_name("BUF"), "gate_name");
            test_gate->set_location_x(1);
            EXPECT_EQ(gate_location_changed_listener.get_event_count(), 1);
            EXPECT_EQ(gate_location_changed_listener.get_last_parameters(), std::make_tuple(GateEvent::event::location_changed, test_gate, NO_DATA));
            gate_location_changed_listener.reset_events();

            test_gate->set_location_y(5);
            EXPECT_EQ(gate_location_changed_listener.get_event_count(), 1);
            EXPECT_EQ(gate_location_changed_listener.get_last_parameters(), std::make_tuple(GateEvent::event::location_changed, test_gate, NO_DATA));
            gate_location_changed_listener.reset_events();

            test_gate->set_location(std::make_pair(8, 10));
            EXPECT_EQ(gate_location_changed_listener.get_event_count(), 2);    // throws two events
            EXPECT_EQ(gate_location_changed_listener.get_last_parameters(), std::make_tuple(GateEvent::event::location_changed, test_gate, NO_DATA));
            gate_location_changed_listener.reset_events();

            nl->get_event_handler()->unregister_callback("gate_location_changed_cb");
        }
        TEST_END
    }
}    // namespace hal
