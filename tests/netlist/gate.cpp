#include "hal_core/netlist/gate_library/gate_library_manager.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/netlist/netlist_factory.h"
#include "netlist_test_utils.h"
#include "gtest/gtest.h"
#include <iostream>
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/module.h"
#include "hal_core/netlist/gate_library/gate_type/gate_type_lut.h"
#include <iomanip>
#include <algorithm>
#include <cassert>

#include "hal_core/netlist/event_system/gate_event_handler.h"

namespace hal {

    using test_utils::MIN_GATE_ID;
    using test_utils::MIN_NET_ID;
    using test_utils::MIN_MODULE_ID;

    class GateTest : public ::testing::Test {
    protected:

        virtual void SetUp() {
            test_utils::init_log_channels();
        }

        virtual void TearDown() {
        }

        std::string i_to_hex_string(const int i, const int len = -1) const {
            std::stringstream ss;

            if (len >= 0) {
                ss << std::hex << std::setfill('0') << std::setw(len) << i;
            } else {
                ss << std::hex << i;
            }

            return ss.str();
        }

        // UNUSED
        std::vector<BooleanFunction::Value> get_truth_table_from_i(const int i, unsigned bit, bool flipped = false) {
            std::vector<BooleanFunction::Value> res;
            for (int b = bit - 1; b >= 0; b--) {
                if (((1 << b) & i) > 0) {
                    res.push_back(BooleanFunction::ONE);
                } else {
                    res.push_back(BooleanFunction::ZERO);
                }
            }
            if (flipped) {
                std::reverse(res.begin(), res.end());
            }
            return res;
        }

        // Turns a hex string into a BooleanFunction truth table (e.g: "A9" -> {1,0,1,0,1,0,0,1})
        // If the string is to small/big, the BEGINNING of the table is filled with zero/erased before the flip.
        std::vector<BooleanFunction::Value> get_truth_table_from_hex_string(std::string str,
                                                                            unsigned bit,
                                                                            bool flipped = false) {
            std::string char_val = "0123456789ABCDEF";
            std::vector<BooleanFunction::Value> res;
            std::transform(str.begin(), str.end(), str.begin(), ::toupper);
            for (char c : str) {
                if (std::size_t val = char_val.find(c); val != std::string::npos) {
                    for (int b = 3; b >= 0; b--) {
                        if (((1 << b) & val) > 0) {
                            res.push_back(BooleanFunction::ONE);
                        } else {
                            res.push_back(BooleanFunction::ZERO);
                        }
                    }
                } else { // input has non-hex characters
                    return std::vector<BooleanFunction::Value>();
                }
            }
            int size_diff = res.size() - bit;
            if (size_diff > 0) {
                res.erase(res.begin(), res.begin() + size_diff);
            } else if (size_diff < 0) {
                res.insert(res.begin(), -size_diff, BooleanFunction::ZERO);
            }
            assert(res.size() == bit);
            if (flipped) {
                std::reverse(res.begin(), res.end());
            }
            return res;
        }

        // Get the minimized version of a truth truth table by its hex value, such that unnecessary variables are eliminated
        std::vector<BooleanFunction::Value> get_min_truth_table_from_hex_string(std::string str,
                                                                                unsigned bit,
                                                                                bool flipped = false) {
            return test_utils::minimize_truth_table(get_truth_table_from_hex_string(str, bit, flipped));
        }

    };

    /**
     * Testing the constructor as well as the copy constructor of the Gate
     *
     * Functions: constructor, copy constructor, get_id, get_name, get_type
     */
    TEST_F(GateTest, check_constructor) {
        TEST_START
            // Create a Gate (id = 100) and append it to its netlist
            auto nl = test_utils::create_empty_netlist();
            auto test_gate = nl->create_gate(MIN_GATE_ID + 100, test_utils::get_gate_type_by_name("gate_2_to_1"), "gate_name");

            ASSERT_NE(test_gate, nullptr);
            EXPECT_EQ(test_gate->get_id(), (u32) (MIN_GATE_ID + 100));
            EXPECT_EQ(test_gate->get_type()->get_name(), "gate_2_to_1");
            EXPECT_EQ(test_gate->get_name(), "gate_name");
            EXPECT_EQ(test_gate->get_netlist(), nl.get());

        TEST_END
    }

    /**
     * Testing the function set_name and get_name
     *
     * Functions: get_name, set_name
     */
    TEST_F(GateTest, check_set_and_get_name) {
        TEST_START
            // ########################
            // POSITIVE TESTS
            // ########################
            // Create a Gate and append it to its netlist
            auto nl = test_utils::create_empty_netlist();
            Gate* test_gate =
                nl->create_gate(MIN_GATE_ID + 0, test_utils::get_gate_type_by_name("gate_2_to_1"), "gate_name");

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
     * Testing functions which returns the pin types. Further test for different Gate types
     * are for in the tests for netlist (get_input_pin_types, ... )
     *
     * Functions: get_input_pin_types, get_output_pin_types, get_inout_pin_types
     */
    TEST_F(GateTest, check_pin_types) {
        TEST_START
            // Create a Gate and append it to its netlist
            auto nl = test_utils::create_empty_netlist();
            Gate* test_gate =
                nl->create_gate(MIN_GATE_ID + 0, test_utils::get_gate_type_by_name("gate_2_to_1"), "gate_name");

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
    TEST_F(GateTest, check_get_fan_in_nets) {
        TEST_START

            // Create the example
            auto nl = test_utils::create_example_netlist();

            // ########################
            // POSITIVE TESTS
            // ########################
            {
                // All input pins are occupied
                Gate* gate_0 = nl->get_gate_by_id(MIN_GATE_ID + 0);
                std::vector<Net*>
                    fan_in_nets_0 = {nl->get_net_by_id(MIN_NET_ID + 30), nl->get_net_by_id(MIN_NET_ID + 20)};
                EXPECT_EQ(gate_0->get_fan_in_nets(), fan_in_nets_0);
            }
            {
                // Not all input pins are occupied
                Gate* gate_5 = nl->get_gate_by_id(MIN_GATE_ID + 5);
                std::vector<Net*> fan_in_nets_5 = {nl->get_net_by_id(MIN_NET_ID + 045)};
                EXPECT_EQ(gate_5->get_fan_in_nets(), fan_in_nets_5);
            }
            {
                // No input pins are occupied
                Gate* gate_6 = nl->get_gate_by_id(MIN_GATE_ID + 6);
                std::vector<Net*> fan_in_nets_6 = {};
                EXPECT_EQ(gate_6->get_fan_in_nets(), fan_in_nets_6);
            }
            {
                // No input-pins exist
                Gate* gate_1 = nl->get_gate_by_id(MIN_GATE_ID + 1);
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
    TEST_F(GateTest, check_get_fan_out_nets) {
        TEST_START

            // Create the example
            auto nl = test_utils::create_example_netlist(0);

            // ########################
            // POSITIVE TESTS
            // ########################
            {
                // All output pins are occupied
                Gate* gate_0 = nl->get_gate_by_id(MIN_GATE_ID + 0);
                std::vector<Net*> fan_out_nets_0 = {nl->get_net_by_id(MIN_NET_ID + 045)};
                EXPECT_EQ(gate_0->get_fan_out_nets(), fan_out_nets_0);
            }
            {
                // Not all output pins are occupied
                Gate* gate_7 = nl->get_gate_by_id(MIN_GATE_ID + 7);
                std::vector<Net*> fan_out_nets_7 = {nl->get_net_by_id(MIN_NET_ID + 78)};
                EXPECT_EQ(gate_7->get_fan_out_nets(), fan_out_nets_7);
            }
            {
                // No output pins are occupied
                Gate* gate_8 = nl->get_gate_by_id(MIN_GATE_ID + 8);
                std::vector<Net*> fan_out_nets_8 = {};
                EXPECT_EQ(gate_8->get_fan_out_nets(), fan_out_nets_8);
            }
            {
                // No output pin exist
                Gate* gate_6 = nl->get_gate_by_id(MIN_GATE_ID + 6);
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
    TEST_F(GateTest, check_get_fan_in_net) {
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
    TEST_F(GateTest, check_get_fan_out_net) {
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
    TEST_F(GateTest, check_get_predecessors) {
        TEST_START
            // Create the examples
            auto nl_1 = test_utils::create_example_netlist();
            auto nl_2 = test_utils::create_example_netlist_2();

            // ########################
            // POSITIVE TESTS
            // ########################
            {
                // Get predecessors for a Gate with multiple predecessors (some of them are the same Gate)
                Gate* gate_1 = nl_2->get_gate_by_id(MIN_GATE_ID + 1);
                std::vector<Endpoint*> pred = {test_utils::get_endpoint(nl_2.get(), MIN_GATE_ID + 0, "O", false),
                                              test_utils::get_endpoint(nl_2.get(), MIN_GATE_ID + 0, "O", false),
                                              test_utils::get_endpoint(nl_2.get(), MIN_GATE_ID + 0, "O", false),
                                              test_utils::get_endpoint(nl_2.get(), MIN_GATE_ID + 2, "O", false)};
                EXPECT_TRUE(test_utils::vectors_have_same_content(gate_1->get_predecessors(), pred));
                EXPECT_EQ(gate_1->get_predecessors().size(), (size_t) 4);
            }
            {
                // Get predecessors for a Gate with no predecessors
                Gate* gate_0 = nl_2->get_gate_by_id(MIN_GATE_ID + 0);
                EXPECT_TRUE(gate_0->get_predecessors().empty());
            }
            {
                // Get predecessors for a given (existing) output pin type
                Gate* gate_3 = nl_2->get_gate_by_id(MIN_GATE_ID + 3);
                std::vector<Endpoint*> pred = {test_utils::get_endpoint(nl_2.get(), MIN_GATE_ID + 0, "O", false)};
                EXPECT_TRUE(test_utils::vectors_have_same_content(gate_3
                                                                      ->get_predecessors(test_utils::adjacent_pin_filter(
                                                                          "O")), pred));
            }
            {
                // Get predecessors for a given (non-existing) output pin type
                Gate* gate_1 = nl_2->get_gate_by_id(MIN_GATE_ID + 1);
                EXPECT_TRUE(gate_1->get_predecessors(test_utils::adjacent_pin_filter("NEx_PIN")).empty());
                EXPECT_EQ(gate_1->get_predecessors(test_utils::adjacent_pin_filter("NEx_PIN")).size(), (size_t) 0);
            }
            {
                // Get predecessors for a given (existing) input pin type
                Gate* gate_3 = nl_2->get_gate_by_id(MIN_GATE_ID + 3);
                std::vector<Endpoint*> pred = {test_utils::get_endpoint(nl_2.get(), MIN_GATE_ID + 0, "O", false)};
                EXPECT_TRUE(test_utils::vectors_have_same_content(gate_3
                                                                      ->get_predecessors(test_utils::starting_pin_filter(
                                                                          "I0")), pred));
            }
            {
                // Get predecessors for a given (existing) unconnected input pin type
                Gate* gate_0 = nl_2->get_gate_by_id(MIN_GATE_ID + 0);
                std::vector<Endpoint*> pred = {};
                EXPECT_TRUE(test_utils::vectors_have_same_content(gate_0
                                                                      ->get_predecessors(test_utils::starting_pin_filter(
                                                                          "I0")), pred));
            }
            {
                // Get predecessors for a given (non-existing) input pin type
                Gate* gate_1 = nl_2->get_gate_by_id(MIN_GATE_ID + 1);
                EXPECT_TRUE(gate_1->get_predecessors(test_utils::starting_pin_filter("NEx_PIN")).empty());
                EXPECT_EQ(gate_1->get_predecessors(test_utils::starting_pin_filter("NEx_PIN")).size(), (size_t) 0);
            }
            {
                // Get predecessors for a given (existing) Gate type
                Gate* gate_0 = nl_1->get_gate_by_id(MIN_GATE_ID + 0);
                std::vector<Endpoint*> pred = {test_utils::get_endpoint(nl_1.get(), MIN_GATE_ID + 3, "O", false)};
                EXPECT_TRUE(test_utils::vectors_have_same_content(gate_0->get_predecessors(test_utils::adjacent_gate_type_filter(
                    "gate_1_to_1")), pred));
                EXPECT_EQ(gate_0->get_predecessors(test_utils::adjacent_gate_type_filter("gate_1_to_1")).size(), (size_t) 1);
            }
            {
                // Get predecessors for a given (non-existing) Gate type
                Gate* gate_1 = nl_2->get_gate_by_id(MIN_GATE_ID + 1);
                EXPECT_TRUE(gate_1->get_predecessors(test_utils::adjacent_gate_type_filter("NEx_GATE")).empty());
                EXPECT_EQ(gate_1->get_predecessors(test_utils::adjacent_gate_type_filter("NEx_GATE")).size(), (size_t) 0);
            }
            // ########################
            // NEGATIVE TESTS
            // ########################
            auto nl_neg = test_utils::create_example_netlist_negative();
            {
                // Get predecessors for a Gate with unconnected nets
                Gate* gate_0 = nl_neg->get_gate_by_id(MIN_GATE_ID + 0);
                EXPECT_TRUE(gate_0->get_predecessors().empty());
                EXPECT_EQ(gate_0->get_predecessors().size(), (size_t) 0);
            }
            {
                // Get predecessors for a Gate with unconnected nets and a set input pin type
                Gate* gate_0 = nl_neg->get_gate_by_id(MIN_GATE_ID + 0);
                EXPECT_TRUE(gate_0->get_predecessors(test_utils::starting_pin_filter("I")).empty());
                EXPECT_EQ(gate_0->get_predecessors(test_utils::starting_pin_filter("I")).size(), (size_t) 0);
            }
        TEST_END
    }

    /**
     * Testing the get_successors function by using the example netlists (see above).
     *
     * Functions: get_successors
     */
    TEST_F(GateTest, check_get_successors) {
        TEST_START
            // Create the examples
            auto nl_1 = test_utils::create_example_netlist();
            auto nl_2 = test_utils::create_example_netlist_2();

            // ########################
            // POSITIVE TESTS
            // ########################
            {
                // Get successors for a Gate with multiple successors (some of them are the same Gate)
                Gate* gate_0 = nl_2->get_gate_by_id(MIN_GATE_ID + 0);
                std::vector<Endpoint*> succ = {test_utils::get_endpoint(nl_2.get(), MIN_GATE_ID + 1, "I0", true),
                                              test_utils::get_endpoint(nl_2.get(), MIN_GATE_ID + 1, "I1", true),
                                              test_utils::get_endpoint(nl_2.get(), MIN_GATE_ID + 1, "I2", true),
                                              test_utils::get_endpoint(nl_2.get(), MIN_GATE_ID + 3, "I0", true)};
                EXPECT_TRUE(test_utils::vectors_have_same_content(gate_0->get_successors(), succ));
                EXPECT_EQ(gate_0->get_successors().size(), (size_t) 4);
            }
            {
                // Get successors for a Gate no successors
                Gate* gate_1 = nl_2->get_gate_by_id(MIN_GATE_ID + 1);
                EXPECT_TRUE(gate_1->get_successors().empty());
            }
            {
                // Get successors for a given (existing) input pin type
                Gate* gate_0 = nl_2->get_gate_by_id(MIN_GATE_ID + 0);
                std::vector<Endpoint*> succ = {test_utils::get_endpoint(nl_2.get(), MIN_GATE_ID + 1, "I0", true),
                                              test_utils::get_endpoint(nl_2.get(), MIN_GATE_ID + 3, "I0", true)};
                EXPECT_TRUE(test_utils::vectors_have_same_content(gate_0
                                                                      ->get_successors(test_utils::adjacent_pin_filter(
                                                                          "I0")), succ));
                EXPECT_EQ(gate_0->get_successors(test_utils::adjacent_pin_filter("I0")).size(), (size_t) 2);
            }
            {
                // Get successors for a given (non-existing) intput pin type
                Gate* gate_0 = nl_2->get_gate_by_id(MIN_GATE_ID + 0);
                EXPECT_TRUE(gate_0->get_successors(test_utils::adjacent_pin_filter("NEx_PIN")).empty());
                EXPECT_EQ(gate_0->get_successors(test_utils::adjacent_pin_filter("NEx_PIN")).size(), (size_t) 0);
            }
            {
                // Get successors for a given (existing) output pin type
                Gate* gate_0 = nl_2->get_gate_by_id(MIN_GATE_ID + 0);
                std::vector<Endpoint*> succ = {test_utils::get_endpoint(nl_2.get(), MIN_GATE_ID + 1, "I0", true),
                                              test_utils::get_endpoint(nl_2.get(), MIN_GATE_ID + 1, "I1", true),
                                              test_utils::get_endpoint(nl_2.get(), MIN_GATE_ID + 1, "I2", true),
                                              test_utils::get_endpoint(nl_2.get(), MIN_GATE_ID + 3, "I0", true)};
                EXPECT_TRUE(test_utils::vectors_have_same_content(gate_0
                                                                      ->get_successors(test_utils::starting_pin_filter(
                                                                          "O")), succ));
                EXPECT_EQ(gate_0->get_successors(test_utils::starting_pin_filter("O")).size(), (size_t) 4);
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
                Gate* gate_0 = nl_1->get_gate_by_id(MIN_GATE_ID + 0);
                std::vector<Endpoint*> succ = {test_utils::get_endpoint(nl_1.get(), MIN_GATE_ID + 4, "I", true)};
                EXPECT_TRUE(test_utils::vectors_have_same_content(gate_0->get_successors(test_utils::adjacent_gate_type_filter(
                    "gate_1_to_1")), succ));
                EXPECT_EQ(gate_0->get_successors(test_utils::adjacent_gate_type_filter("gate_1_to_1")).size(), (size_t) 1);
            }
            {
                // Get successors for a given (non-existing) Gate type
                Gate* gate_0 = nl_1->get_gate_by_id(MIN_GATE_ID + 0);
                EXPECT_TRUE(gate_0->get_successors(test_utils::adjacent_gate_type_filter("NEx_GATE")).empty());
                EXPECT_EQ(gate_0->get_successors(test_utils::adjacent_gate_type_filter("NEx_GATE")).size(), (size_t) 0);
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
    TEST_F(GateTest, check_get_unique_predecessors_and_successors) {
        TEST_START
            // Create the examples
            auto nl_2 = test_utils::create_example_netlist_2();

            // ########################
            // POSITIVE TESTS
            // ########################
            {
                // Get the unique predecessors
                Gate* gate_1 = nl_2->get_gate_by_id(MIN_GATE_ID + 1);
                std::vector<Gate*> pred = {nl_2->get_gate_by_id(MIN_GATE_ID + 0), nl_2->get_gate_by_id(MIN_GATE_ID + 2)};
                std::vector<Gate*> res = gate_1->get_unique_predecessors();
                EXPECT_TRUE(test_utils::vectors_have_same_content(res, pred));
            }
            {
                // Get the unique successors
                Gate* gate_0 = nl_2->get_gate_by_id(MIN_GATE_ID + 0);
                std::vector<Gate*> succ = {nl_2->get_gate_by_id(MIN_GATE_ID + 1), nl_2->get_gate_by_id(MIN_GATE_ID + 3)};
                std::vector<Gate*> res = gate_0->get_unique_successors();
                EXPECT_TRUE(test_utils::vectors_have_same_content(res, succ));
            }
        TEST_END
    }

    /**
     * Testing the get_predecessor function
     *
     * Functions: get_predecessor
     */
    TEST_F(GateTest, check_get_predecessor) {
        TEST_START
            // Create the examples
            auto nl_2 = test_utils::create_example_netlist_2();

            // ########################
            // POSITIVE TESTS
            // ########################
            {
                // Get predecessor for a given (existing) input pin type
                auto gate_3 = nl_2->get_gate_by_id(MIN_GATE_ID + 3);
                auto pred = test_utils::get_endpoint(nl_2.get(), MIN_GATE_ID + 0, "O", false);
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
    TEST_F(GateTest, check_gnd_vcc_gate_handling) {
        TEST_START
            {
                // Mark and unmark a global vcc Gate
                auto nl = test_utils::create_empty_netlist();
                Gate*
                    vcc_gate = nl->create_gate(MIN_GATE_ID + 0, test_utils::get_gate_type_by_name("vcc"), "vcc_gate");

                vcc_gate->mark_vcc_gate();
                EXPECT_TRUE(vcc_gate->is_vcc_gate());
                EXPECT_TRUE(nl->is_vcc_gate(vcc_gate));

                vcc_gate->unmark_vcc_gate();
                EXPECT_FALSE(vcc_gate->is_vcc_gate());
                EXPECT_FALSE(nl->is_vcc_gate(vcc_gate));
            }
            {
                // Mark and unmark a global gnd Gate
                auto nl = test_utils::create_empty_netlist();
                Gate*
                    gnd_gate = nl->create_gate(MIN_GATE_ID + 0, test_utils::get_gate_type_by_name("gnd"), "gnd_gate");

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
    TEST_F(GateTest, check_get_module) {
        TEST_START
            {
                // get the Module of a Gate (the top_module), then add it to another Module and check again
                // -- create the Gate at the top_module
                auto nl = test_utils::create_empty_netlist();
                Gate* test_gate =
                    nl->create_gate(MIN_GATE_ID + 0, test_utils::get_gate_type_by_name("gate_1_to_1"), "test_gate");

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
     * Testing storage and access of Gate locations
     *
     * Functions: get_location_x, get_location_y, get_location, has_location, set_location_x, set_location_y, set_location
     */
    TEST_F(GateTest, check_location_storage) {
        TEST_START
            {
                // Create a Gate with a location and change it afterwards
                auto nl = test_utils::create_empty_netlist();
                Gate* test_gate = nl->create_gate(MIN_GATE_ID + 0,
                                                                  test_utils::get_gate_type_by_name("gate_1_to_1"),
                                                                  "test_gate",
                                                                  1.11f,
                                                                  2.22f);
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
                auto nl = test_utils::create_empty_netlist();
                Gate* test_gate = nl->create_gate(MIN_GATE_ID + 0,
                                                                  test_utils::get_gate_type_by_name("gate_1_to_1"),
                                                                  "test_gate",
                                                                  1.11f,
                                                                  2.22f);
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
    TEST_F(GateTest, check_boolean_function_access) {
        TEST_START
            BooleanFunction bf_i = BooleanFunction::from_string("I", std::vector<std::string>({"I"}));
            BooleanFunction bf_i_invert = BooleanFunction::from_string("!I", std::vector<std::string>({"I"}));

            auto inv_gl_owner = std::make_unique<GateLibrary>("imaginary_path", "TEST_LIB");
            auto inv_gl = inv_gl_owner.get();
            auto inv_gate_type_owner = std::make_unique<GateType>("gate_1_to_1_inv");
            auto inv_gate_type = inv_gate_type_owner.get();
            inv_gate_type->add_input_pin("I");
            inv_gate_type->add_output_pin("O");
            inv_gate_type->add_boolean_function("O", bf_i_invert);
            inv_gl->add_gate_type(std::move(inv_gate_type_owner));

            {
                // Access the boolean function of a gate_type
                auto nl = std::make_unique<Netlist>(inv_gl);
                Gate* test_gate = nl->create_gate(MIN_GATE_ID + 0,
                                                                  test_utils::get_gate_type_by_name("gate_1_to_1_inv",
                                                                                                    inv_gl),
                                                                  "test_gate");
                std::unordered_map<std::string, BooleanFunction> bf = test_gate->get_boolean_functions();
                EXPECT_EQ(bf, (std::unordered_map<std::string, BooleanFunction>({{"O", bf_i_invert}})));
            }
            {
                // Add a custom boolean function
                auto nl = std::make_unique<Netlist>(inv_gl);
                Gate* test_gate = nl->create_gate(MIN_GATE_ID + 0, inv_gate_type, "test_gate");
                test_gate->add_boolean_function("new_bf", bf_i);

                // -- get all boolean functions
                std::unordered_map<std::string, BooleanFunction> bf_all = test_gate->get_boolean_functions(false);
                EXPECT_EQ(bf_all, (std::unordered_map<std::string, BooleanFunction>({{"O", bf_i_invert},
                                                                                     {"new_bf", bf_i}})));
                // -- get only custom boolean functions via get_boolean_functions
                std::unordered_map<std::string, BooleanFunction> bf_custom = test_gate->get_boolean_functions(true);
                EXPECT_EQ(bf_custom, (std::unordered_map<std::string, BooleanFunction>({{"new_bf", bf_i}})));

                // -- get boolean function of the Gate type by using the function get_boolean_function
                EXPECT_EQ(test_gate->get_boolean_function("O"), bf_i_invert);

                // -- get the custom boolean function by using the function get_boolean_function
                EXPECT_EQ(test_gate->get_boolean_function("new_bf"), bf_i);

                // -- get the boolean function by calling get_boolean_function with name (The first output pin should be taken)
                EXPECT_EQ(test_gate->get_boolean_function(), bf_i_invert);
            }
            // NEGATIVE
            {
                // Get a boolean function for a name that is unknown.
                auto nl = std::make_unique<Netlist>(inv_gl);
                Gate* test_gate = nl->create_gate(MIN_GATE_ID + 0,
                                                                  test_utils::get_gate_type_by_name("gate_1_to_1_inv",
                                                                                                    inv_gl),
                                                                  "test_gate");

                EXPECT_EQ(test_gate->get_boolean_function("unknown_name"), BooleanFunction());
                EXPECT_TRUE(test_gate->get_boolean_function("unknown_name").is_empty());
            }
            {
                // Call the get_boolean_function function with no parameter, for a Gate with no outputs
                GateLibrary gl("imaginary_path", "TEST_LIB");
                auto empty_gate_type_owner = std::make_unique<GateType>("EMPTY_GATE");
                auto empty_gate_type = empty_gate_type_owner.get();
                gl.add_gate_type(std::move(empty_gate_type_owner));

                Netlist nl(&gl);
                auto empty_gate = nl.create_gate(MIN_GATE_ID + 0, empty_gate_type, "empty_gate");
                EXPECT_TRUE(empty_gate->get_boolean_function().is_empty());
            }
            {
                // Call the get_boolean_function function with no parameter, for a Gate with no outputs
                GateLibrary gl("imaginary_path", "TEST_LIB");
                auto empty_gate_type_owner = std::make_unique<GateType>("EMPTY_GATE");
                auto empty_gate_type = empty_gate_type_owner.get();
                empty_gate_type->add_output_pin("");
                gl.add_gate_type(std::move(empty_gate_type_owner));

                Netlist nl(&gl);
                auto empty_gate = nl.create_gate(MIN_GATE_ID + 0, empty_gate_type, "empty_gate");
                EXPECT_TRUE(empty_gate->get_boolean_function().is_empty());
            }
            {

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
            // Create a custom GateLibrary which contains custom lut gates
            GateLibrary lib("imaginary_path", "TEST_LIB");
            auto gl = &lib;
            auto lut_owner = std::make_unique<GateTypeLut>("LUT_GATE");
            auto lut = lut_owner.get();

            std::vector<std::string> input_pins({"I0", "I1", "I2"});
            std::vector<std::string> output_pins({"O_LUT", "O_normal", "O_LUT_other"});

            lut->add_input_pins(input_pins);
            lut->add_output_pins(output_pins);
            lut->add_output_from_init_string_pin("O_LUT");
            lut->add_output_from_init_string_pin("O_LUT_other");
            lut->set_config_data_ascending_order(true);
            lut->set_config_data_identifier("data_identifier");
            lut->set_config_data_category("data_category");
            gl->add_gate_type(std::move(lut_owner));
            {
                // Get the boolean function of the lut in different ways
                // is not taken into account!
                auto nl = std::make_unique<Netlist>(gl);
                Gate* lut_gate = nl->create_gate(MIN_GATE_ID + 0, lut, "lut");

                int i = 1;
                lut_gate->set_data(lut->get_config_data_category(),
                                   lut->get_config_data_identifier(),
                                   "bit_vector",
                                   i_to_hex_string(i, 2));

                // Testing the access via the function get_boolean_function
                EXPECT_EQ(lut_gate->get_boolean_function("O_LUT").get_truth_table(input_pins),
                          get_truth_table_from_i(i, 8));
                EXPECT_EQ(lut_gate->get_boolean_function("O_LUT_other").get_truth_table(input_pins),
                          get_truth_table_from_i(i, 8));

                // Test the access via the get_boolean_functions map
                std::unordered_map<std::string, BooleanFunction> bfs = lut_gate->get_boolean_functions();
                ASSERT_TRUE(bfs.find("O_LUT") != bfs.end());
                EXPECT_EQ(bfs["O_LUT"].get_truth_table(input_pins), get_truth_table_from_i(i, 8));
                ASSERT_TRUE(bfs.find("O_LUT_other") != bfs.end());
                EXPECT_EQ(bfs["O_LUT_other"].get_truth_table(input_pins), get_truth_table_from_i(i, 8));

            }
            {
                // Access the boolean function of a lut, that is stored in ascending order
                auto nl = std::make_unique<Netlist>(gl);
                Gate* lut_gate = nl->create_gate(MIN_GATE_ID + 0, lut, "lut");
                for (int i = 0x0; i <= 0xff; i++) {
                    lut_gate->set_data(lut->get_config_data_category(),
                                       lut->get_config_data_identifier(),
                                       "bit_vector",
                                       i_to_hex_string(i));
                    BooleanFunction bf = lut_gate->get_boolean_function("O_LUT");
                    EXPECT_EQ(bf.get_truth_table(input_pins),
                              get_truth_table_from_hex_string(i_to_hex_string(i), 8, false));
                }
            }
            {
                // Access the boolean function of a lut, that is stored in descending order
                lut->set_config_data_ascending_order(false);
                auto nl = std::make_unique<Netlist>(gl);
                Gate* lut_gate = nl->create_gate(MIN_GATE_ID + 0, lut, "lut");
                for (int i = 0x0; i <= 0xff; i++) {
                    lut_gate->set_data(lut->get_config_data_category(),
                                       lut->get_config_data_identifier(),
                                       "bit_vector",
                                       i_to_hex_string(i));
                    BooleanFunction bf = lut_gate->get_boolean_function("O_LUT");
                    EXPECT_EQ(bf.get_truth_table(input_pins),
                              get_truth_table_from_hex_string(i_to_hex_string(i), 8, true));
                }
                lut->set_config_data_ascending_order(true);
            }
            {
                // Add a boolean function to a lut pin
                auto nl = std::make_unique<Netlist>(gl);
                Gate* lut_gate = nl->create_gate(MIN_GATE_ID + 0, lut, "lut");

                BooleanFunction lut_bf = BooleanFunction::from_string("I0 | I1 | I2", input_pins);
                lut_gate->add_boolean_function("O_LUT", lut_bf);
                get_truth_table_from_hex_string("EF", 8);
            }
            // NEGATIVE
            {
                // There is no hex string at the config data path (0 truth table)
                auto nl = std::make_unique<Netlist>(gl);
                Gate* lut_gate = nl->create_gate(MIN_GATE_ID + 0, lut, "lut");
                lut_gate
                    ->set_data(lut->get_config_data_category(), lut->get_config_data_identifier(), "bit_vector", "");
                EXPECT_EQ(lut_gate->get_boolean_function("O_LUT").get_truth_table(input_pins),
                          get_truth_table_from_i(0, 8));
            }
            {
                // There is no hex string at the config data path
                NO_COUT_TEST_BLOCK;
                auto nl = std::make_unique<Netlist>(gl);
                Gate* lut_gate = nl->create_gate(MIN_GATE_ID + 0, lut, "lut");

                lut_gate->set_data(lut->get_config_data_category(),
                                   lut->get_config_data_identifier(),
                                   "bit_vector",
                                   "NOHx");
                EXPECT_EQ(lut_gate->get_boolean_function("O_LUT").get_truth_table(input_pins),
                          std::vector<BooleanFunction::Value>(8, BooleanFunction::X));

            }
            {
                // Test a lut with more than 6 inputs
                NO_COUT_TEST_BLOCK;
                std::vector<std::string> new_input_pins({"I3", "I4", "I5", "I6"});
                lut->add_input_pins(new_input_pins);
                input_pins.insert(input_pins.begin(), new_input_pins.begin(), new_input_pins.end());

                auto nl = std::make_unique<Netlist>(gl);
                Gate* lut_gate = nl->create_gate(MIN_GATE_ID + 0, lut, "lut");

                std::string long_hex = "DEADBEEFC001D00DDEADC0DEDEADDA7A";

                lut_gate->set_data(lut->get_config_data_category(),
                                   lut->get_config_data_identifier(),
                                   "bit_vector",
                                   long_hex);
                EXPECT_EQ(lut_gate->get_boolean_function("O_LUT").get_truth_table(input_pins),
                          std::vector<BooleanFunction::Value>((1 << input_pins.size()), BooleanFunction::X));
            }
        TEST_END
    }

    /*************************************
     * Event System
     *************************************/

    /**
     * Testing the triggering of events.
     */
    TEST_F(GateTest, check_events) {
        TEST_START
            const u32 NO_DATA = 0xFFFFFFFF;
            {
                // Check the 'gate created' event handling
                // -- create the listener
                test_utils::EventListener<void, gate_event_handler::event, Gate*, u32> gate_created_listener;
                std::function<void(gate_event_handler::event, Gate*, u32)> cb = gate_created_listener.get_conditional_callback(
                    [=](gate_event_handler::event ev, Gate* g, u32 id){return ev == gate_event_handler::event::created;}
                );
                // -- register the callback hook
                gate_event_handler::register_callback("gate_created_cb", cb);

                // -- check if the event is triggered
                std::unique_ptr<Netlist> nl = test_utils::create_empty_netlist();
                Gate* test_gate = nl->create_gate(MIN_GATE_ID + 1, test_utils::get_gate_type_by_name("gate_1_to_1"), "gate_name");
                EXPECT_EQ(gate_created_listener.get_event_count(), 1);
                EXPECT_EQ(gate_created_listener.get_last_parameters(), std::make_tuple(gate_event_handler::event::created, test_gate, NO_DATA));

                // -- remove the callback hook
                gate_event_handler::unregister_callback("gate_created_cb");
            }
            {
                // Check the 'gate removed' event handling
                // -- create the listener
                test_utils::EventListener<void, gate_event_handler::event, Gate*, u32> gate_removed_listener;
                std::function<void(gate_event_handler::event, Gate*, u32)> cb = gate_removed_listener.get_conditional_callback(
                    [=](gate_event_handler::event ev, Gate* g, u32 id){return ev == gate_event_handler::event::removed;}
                );
                // -- register the callback hook
                gate_event_handler::register_callback("gate_removed_cb", cb);

                // -- check if the event is triggered
                std::unique_ptr<Netlist> nl = test_utils::create_empty_netlist();
                Gate* test_gate = nl->create_gate(MIN_GATE_ID + 1, test_utils::get_gate_type_by_name("gate_1_to_1"), "gate_name");
                EXPECT_EQ(gate_removed_listener.get_event_count(), 0);
                nl->delete_gate(test_gate);
                EXPECT_EQ(gate_removed_listener.get_event_count(), 1);
                EXPECT_EQ(gate_removed_listener.get_last_parameters(), std::make_tuple(gate_event_handler::event::removed, test_gate, NO_DATA));

                // -- remove the callback hook
                gate_event_handler::unregister_callback("gate_removed_cb");
            }
            {
                // Check the 'name changed' event handling
                // -- create the listener
                test_utils::EventListener<void, gate_event_handler::event, Gate*, u32> gate_name_changed_listener;
                std::function<void(gate_event_handler::event, Gate*, u32)> cb = gate_name_changed_listener.get_conditional_callback(
                    [=](gate_event_handler::event ev, Gate* g, u32 id){return ev == gate_event_handler::event::name_changed;}
                );
                // -- register the callback hook
                gate_event_handler::register_callback("gate_name_changed_cb", cb);

                // -- check if the event is triggered
                std::unique_ptr<Netlist> nl = test_utils::create_empty_netlist();
                Gate* test_gate = nl->create_gate(MIN_GATE_ID + 1, test_utils::get_gate_type_by_name("gate_1_to_1"), "gate_name");
                test_gate->set_name("gate_name"); // if the old name equals the new one, the event should not be triggered
                EXPECT_EQ(gate_name_changed_listener.get_event_count(), 0);
                gate_name_changed_listener.reset_events();
                test_gate->set_name("new_name");
                EXPECT_EQ(gate_name_changed_listener.get_event_count(), 1);
                EXPECT_EQ(gate_name_changed_listener.get_last_parameters(), std::make_tuple(gate_event_handler::event::name_changed, test_gate, NO_DATA));

                // -- remove the callback hook
                gate_event_handler::unregister_callback("gate_name_changed_cb");
            }
            {
                // Check the 'gate location changed' event handling
                // -- create the listener
                test_utils::EventListener<void, gate_event_handler::event, Gate*, u32> gate_location_changed_listener;
                std::function<void(gate_event_handler::event, Gate*, u32)> cb = gate_location_changed_listener.get_conditional_callback(
                    [=](gate_event_handler::event ev, Gate* g, u32 id){return ev == gate_event_handler::event::location_changed;}
                );
                // -- register the callback hook
                gate_event_handler::register_callback("gate_location_changed_cb", cb);

                // -- check if the event is triggered
                std::unique_ptr<Netlist> nl = test_utils::create_empty_netlist();

                Gate* test_gate = nl->create_gate(MIN_GATE_ID + 1, test_utils::get_gate_type_by_name("gate_1_to_1"), "gate_name");
                test_gate->set_location_x(1.23f);
                EXPECT_EQ(gate_location_changed_listener.get_event_count(), 1);
                EXPECT_EQ(gate_location_changed_listener.get_last_parameters(), std::make_tuple(gate_event_handler::event::location_changed, test_gate, NO_DATA));
                gate_location_changed_listener.reset_events();

                test_gate->set_location_y(4.56f);
                EXPECT_EQ(gate_location_changed_listener.get_event_count(), 1);
                EXPECT_EQ(gate_location_changed_listener.get_last_parameters(), std::make_tuple(gate_event_handler::event::location_changed, test_gate, NO_DATA));
                gate_location_changed_listener.reset_events();

                test_gate->set_location(std::make_pair(7.89f, 10.11));
                if(test_utils::known_issue_tests_active())
                    EXPECT_EQ(gate_location_changed_listener.get_event_count(), 1); // ISSUE: (?) Only one notification should be done
                EXPECT_EQ(gate_location_changed_listener.get_last_parameters(), std::make_tuple(gate_event_handler::event::location_changed, test_gate, NO_DATA));
                gate_location_changed_listener.reset_events();

                // -- remove the callback hook
                gate_event_handler::unregister_callback("gate_location_changed_cb");
            }
        TEST_END
    }
} // namespace hal
