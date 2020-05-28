#include "netlist_test_utils.h"
#include "gtest/gtest.h"
#include <core/log.h>
#include <iostream>
#include <netlist/gate_library/gate_library.h>
#include <netlist/gate_library/gate_type/gate_type.h>
#include <netlist/gate_library/gate_type/gate_type_lut.h>
#include <netlist/gate_library/gate_type/gate_type_sequential.h>

using namespace test_utils;

/*
 * Testing the gate_library class, as well as the different gate_types (gate_type, gate_type_lut, gate_type_sequential)
 */

class gate_library_test : public ::testing::Test
{
protected:

    virtual void SetUp()
    {
    }

    virtual void TearDown()
    {
    }
};

// ========= gate_type tests ===========

/**
 * Testing the constructor and the access to the gates name and base_type (combinatorial for gate_type class)
 *
 * Functions: <constructor>, get_name, get_base_type, to_string, operator<<
 */
TEST_F(gate_library_test, check_constructor)
{
    TEST_START
        gate_type gt("gt_name");
        EXPECT_EQ(gt.get_name(), "gt_name");
        EXPECT_EQ(gt.to_string(), "gt_name");
        std::stringstream ss;
        ss << gt;
        EXPECT_EQ(ss.str(), "gt_name");
        EXPECT_EQ(gt.get_base_type(), gate_type::base_type::combinatorial);
    TEST_END
}

/**
 * Testing the addition of input and output pins and the access to them
 *
 * Functions: add_input_pin, add_input_pins, get_input_pins, add_output_pin, add_output_pins, get_output_pins
 */
TEST_F(gate_library_test, check_pin_management)
{
    TEST_START
        {
            // Add some input nets
            gate_type gt("gt_name");
            gt.add_input_pin("IN_0"); // Single
            gt.add_input_pins(std::vector<std::string>({"IN_1", "IN_2"})); // Multiple
            EXPECT_EQ(gt.get_input_pins(), std::vector<std::string>({"IN_0", "IN_1", "IN_2"}));
        }
        {
            // Add some output nets
            gate_type gt("gt_name");
            gt.add_output_pin("OUT_0"); // Single
            gt.add_output_pins(std::vector<std::string>({"OUT_1", "OUT_2"})); // Multiple
            EXPECT_EQ(gt.get_output_pins(), std::vector<std::string>({"OUT_0", "OUT_1", "OUT_2"}));
        }
        // NEGATIVE TESTS
        {
            // Add the same input/ouput pin twice (NOTE: Should not work?)
            gate_type gt("gt_name");

            gt.add_input_pin("IN_PIN");
            gt.add_input_pin("IN_PIN");
            //EXPECT_EQ(gt.get_input_pins().size(), 1); // ISSUE: fails

            gt.add_output_pin("OUT_PIN");
            gt.add_output_pin("OUT_PIN");
            //EXPECT_EQ(gt.get_output_pins().size(), 1); // ISSUE: fails
        }
    TEST_END
}

/**
 * Testing the assignment of boolean functions to the pins. Also, boolean functions for non existing pin names
 * can be created, since they are used to store group attributes like next_state, clocked_on, etc.
 *
 * Functions: add_boolean_function, get_boolean_functions
 */
TEST_F(gate_library_test, check_boolean_function_assignment)
{
    TEST_START
        {
            // Add a boolean function for an output pin
            gate_type gt("gt_name");

            gt.add_input_pins(std::vector<std::string>({"IN_0", "IN_1"}));
            gt.add_output_pin("OUT");

            boolean_function bf_out = boolean_function::from_string("IN_0 ^ IN_1");
            gt.add_boolean_function("OUT", bf_out);
            std::unordered_map<std::string, boolean_function> gt_bf_map = gt.get_boolean_functions();
            EXPECT_EQ(gt_bf_map.size(), 1);
            ASSERT_FALSE(gt_bf_map.find("OUT") == gt_bf_map.end());
            EXPECT_EQ(gt_bf_map.at("OUT"), bf_out);
        }
        {
            // Add a boolean function to a non-existing pin type.
            gate_type gt("gt_name");

            gt.add_input_pins(std::vector<std::string>({"IN_0", "IN_1"}));
            gt.add_output_pin("OUT");

            boolean_function bf_out = boolean_function::from_string("IN_0 ^ IN_1");
            gt.add_boolean_function("non_existing_pin", bf_out);
            std::unordered_map<std::string, boolean_function> gt_bf_map = gt.get_boolean_functions();

            EXPECT_EQ(gt_bf_map.size(), 1);
            ASSERT_FALSE(gt_bf_map.find("non_existing_pin") == gt_bf_map.end());
            EXPECT_EQ(gt_bf_map.at("non_existing_pin"), bf_out);
        }
    TEST_END
}

/**
 * Testing the equal_to operator
 *
 * Functions: operator==, operator!=
 */
TEST_F(gate_library_test, check_equal_to_operator)
{
    TEST_START
        {
            // Create two equal gate_types and compare them
            gate_type gt_0("gt_name");
            gate_type gt_1("gt_name");
            // -- add pins
            gt_0.add_input_pins(std::vector<std::string>({"IN_0", "IN_1"}));
            gt_1.add_input_pins(std::vector<std::string>({"IN_0", "IN_1"}));
            gt_0.add_output_pins(std::vector<std::string>({"OUT_0", "OUT_1"}));
            gt_1.add_output_pins(std::vector<std::string>({"OUT_0", "OUT_1"}));
            // -- add a boolean function
            boolean_function bf_out = boolean_function::from_string("NO_PIN_A ^ NO_PIN_B");
            gt_0.add_boolean_function("OUT_0", bf_out);
            gt_1.add_boolean_function("OUT_0", bf_out);
            // -- comparison
            EXPECT_TRUE(gt_0 == gt_1);
            EXPECT_FALSE(gt_0 != gt_1);
        }
        {
            // Compare the two gate types while they differ in one property (base type, pins, functions)
            gate_type gt_0("gt_name");
            gate_type gt_1("gt_name");
            // -- add pins
            gt_0.add_input_pins(std::vector<std::string>({"IN_0", "IN_1"}));
            EXPECT_FALSE(gt_0 == gt_1);
            gt_1.add_input_pins(std::vector<std::string>({"IN_0", "IN_1"}));

            gt_0.add_output_pins(std::vector<std::string>({"OUT_0", "OUT_1"}));
            EXPECT_FALSE(gt_0 == gt_1);
            gt_1.add_output_pins(std::vector<std::string>({"OUT_0", "OUT_1"}));

            // -- add a boolean function
            boolean_function bf_out = boolean_function::from_string("NO_PIN_A ^ NO_PIN_B");
            gt_0.add_boolean_function("OUT_0", bf_out);
            EXPECT_FALSE(gt_0 == gt_1);
            gt_1.add_boolean_function("OUT_0", bf_out);
        }
    TEST_END
}

// ======== gate_type_lut tests ========

/**
 * Testing the addition of output_from_init_string pins
 *
 * Functions: add_output_from_init_string_pin, get_output_from_init_string_pin
 */
TEST_F(gate_library_test, check_output_from_init_string_pin)
{
    TEST_START
        {
            // Add and get some output_from_init_string pins
            gate_type_lut gtl("gtl_name");
            gtl.add_output_pins(std::vector<std::string>({"O0","OFIS_0","OFIS_1"}));
            gtl.add_output_from_init_string_pin("OFIS_0");
            gtl.add_output_from_init_string_pin("OFIS_1");
            EXPECT_EQ(gtl.get_output_from_init_string_pins(), std::unordered_set<std::string>({"OFIS_0","OFIS_1"}));
        }
        {
            // Add and get some output_from_init_string pins (NOTE: Output-Pin doesn't have to exist)
            gate_type_lut gtl("gtl_name");
            gtl.add_output_pins(std::vector<std::string>({"O0","OFIS_0"}));
            gtl.add_output_from_init_string_pin("OFIS_0");
            gtl.add_output_from_init_string_pin("OFIS_1"); // <- not added yet
            EXPECT_EQ(gtl.get_output_from_init_string_pins(), std::unordered_set<std::string>({"OFIS_0","OFIS_1"}));
            // EXPECT_EQ(gtl.get_output_pins(), std::vector<std::string>({"O0", "OFIS_0", "OFIS_1"})); // ISSUE: OFSI_1 is not added to output pin vector
        }
    TEST_END
}

/**
 * Testing the access on config data fields (config_data_category, config_data_identifier, config_data_ascending_order)
 * as well as the acess on the base type (should be lut)
 *
 * Functions: get_base_type, get_config_data_category, set_config_data_category, get_config_data_identifier,
 *            set_config_data_identifier, set_config_data_ascending_order, is_config_data_ascending_order
 */
TEST_F(gate_library_test, check_config_data)
{
    TEST_START
        gate_type_lut gtl("gtl_name");
        {
            // Base Type
            EXPECT_EQ(gtl.get_base_type(), gate_type::base_type::lut);
        }
        {
            // Config Data Category
            gtl.set_config_data_category("category");
            EXPECT_EQ(gtl.get_config_data_category(), "category");
        }
        {
            // Identifiers
            gtl.set_config_data_identifier("identifier");
            EXPECT_EQ(gtl.get_config_data_identifier(), "identifier");
        }
        {
            // Ascending Order
            EXPECT_TRUE(gtl.is_config_data_ascending_order()); // Set true by constructor
            gtl.set_config_data_ascending_order(false);
            EXPECT_FALSE(gtl.is_config_data_ascending_order());
            gtl.set_config_data_ascending_order(true);
            EXPECT_TRUE(gtl.is_config_data_ascending_order());
        }
    TEST_END
}

/**
 * Testing the equal-to operator.
 *
 * Functions: operator==, (do_compare)
 */
TEST_F(gate_library_test, check_lut_comparison)
{
    TEST_START
        {
            // Create two equal gate_types and compare them
            gate_type_lut gtl_0("gtl_name");
            gate_type_lut gtl_1("gtl_name");
            gtl_0.add_output_pin("OFIS_PIN");
            gtl_1.add_output_pin("OFIS_PIN");
            // -- set config_data_category
            gtl_0.set_config_data_category("category");
            gtl_1.set_config_data_category("category");
            // -- set config_data_identifier
            gtl_0.set_config_data_identifier("identifier");
            gtl_1.set_config_data_identifier("identifier");
            // -- set ascending order
            gtl_0.set_config_data_ascending_order(false);
            gtl_1.set_config_data_ascending_order(false);
            // -- add output pin from init string pin
            gtl_0.add_output_from_init_string_pin("OFIS_PIN");
            gtl_1.add_output_from_init_string_pin("OFIS_PIN");

            EXPECT_TRUE(gtl_0 == gtl_1);
        }
        {
            // Compare two gate_type_luts while they differ in one property (category, identifier,
            // ascending order, output from init string pins (?) )
            gate_type_lut gtl_0("gtl_name");
            gate_type_lut gtl_1("gtl_name");
            // -- set config_data_category
            gtl_0.set_config_data_category("category");
            EXPECT_FALSE(gtl_0 == gtl_1);
            gtl_1.set_config_data_category("category");
            // -- set config_data_identifier
            gtl_0.set_config_data_identifier("identifier");
            EXPECT_FALSE(gtl_0 == gtl_1);
            gtl_1.set_config_data_identifier("identifier");
            // -- set ascending order
            gtl_0.set_config_data_ascending_order(false);
            EXPECT_FALSE(gtl_0 == gtl_1);
            gtl_1.set_config_data_ascending_order(false);
            // -- add output pin from init string pin
            gtl_0.add_output_from_init_string_pin("OFIS_PINs");
            // EXPECT_FALSE(gtl_0 == gtl_1); // ISSUE: outputs from init string are not considered in comparison
            gtl_1.add_output_from_init_string_pin("OFIS_PINs");
        }
        {
            // Compare two gate_type. One is a gate_type, the other one a gate_type_lut
            gate_type_lut gt_0("gt_name");
            gate_type gt_1("gt_name");

            EXPECT_FALSE(gt_0 == gt_1);
            EXPECT_FALSE(gt_1 == gt_0);
        }

    TEST_END
}


// ===== gate_type_sequential tests ====

/**
 * Testing the addition of state_output pins and inverted_state_output_pins
 *
 * Functions:
 */
TEST_F(gate_library_test, check_state_output_pin)
{
    TEST_START
        {
            // Add and get some state_output pins (NOTE: Output-Pin doesn't have to exist)
            gate_type_sequential gts("gts_name", gate_type::base_type::ff);
            gts.add_output_pins(std::vector<std::string>({"OFIS_0","OFIS_1"}));
            gts.add_state_output_pin("OFIS_0");
            gts.add_state_output_pin("OFIS_1");
            EXPECT_EQ(gts.get_state_output_pins(), std::unordered_set<std::string>({"OFIS_0","OFIS_1"}));
        }
        {
            // Add and get some inverted_state_output pins (NOTE: Output-Pin doesn't have to exist)
            gate_type_sequential gts("gts_name", gate_type::base_type::ff);
            gts.add_output_pins(std::vector<std::string>({"OFIS_0","OFIS_1"}));
            gts.add_inverted_state_output_pin("OFIS_0");
            gts.add_inverted_state_output_pin("OFIS_1");
            EXPECT_EQ(gts.get_inverted_state_output_pins(), std::unordered_set<std::string>({"OFIS_0","OFIS_1"}));
            //EXPECT_EQ(gts.get_output_pins(), std::vector<std::string>({"OFIS_0","OFIS_1"}));
        }
        // NOTE: can be booth (state_output and inverted_state_output)?
    TEST_END
}

/**
 * Testing the access on init data fields (init_data_category, init_data_category)
 * as well as the access on the base type and the set-reset behavior
 *
 * Functions: get_base_type, get_config_data_category, set_config_data_category, get_config_data_identifier,
 *            set_config_data_identifier, set_config_data_ascending_order, is_config_data_ascending_order
 */
TEST_F(gate_library_test, check_init_data)
{
    TEST_START
        gate_type_sequential gts("gtl_name", gate_type::base_type::ff);
        {
            // Base Type
            EXPECT_EQ(gts.get_base_type(), gate_type::base_type::ff);
        }
        {
            // Init Data Category
            gts.set_init_data_category("category");
            EXPECT_EQ(gts.get_init_data_category(), "category");
        }
        {
            // Identifiers
            gts.set_init_data_identifier("identifier");
            EXPECT_EQ(gts.get_init_data_identifier(), "identifier");
        }
        {
            // Set-Reset Behavior
            gts.set_set_reset_behavior(gate_type_sequential::set_reset_behavior::L, gate_type_sequential::set_reset_behavior::H);
            EXPECT_EQ(gts.get_set_reset_behavior(),
                      std::make_pair(gate_type_sequential::set_reset_behavior::L, gate_type_sequential::set_reset_behavior::H));
        }
        {
            // Get uninitialized set-reset behavior (ISSUE: fails)
            gate_type_sequential gts_2("gtl_name", gate_type::base_type::ff);
            //EXPECT_EQ(gts_2.get_set_reset_behavior(),
                      //std::make_pair(gate_type_sequential::set_reset_behavior::U, gate_type_sequential::set_reset_behavior::U));
        }
    TEST_END
}

/**
 * Testing the equal-to operator.
 *
 * Functions: operator==, (do_compare)
 */
TEST_F(gate_library_test, check_sequential_comparison)
{
    TEST_START
        {
            // Create two equal gate_types and compare them
            gate_type_sequential gts_0("gts_name", gate_type::base_type::ff);
            gate_type_sequential gts_1("gts_name", gate_type::base_type::ff);
            gts_0.add_output_pins(std::vector<std::string>({"SO_PIN","invSO_PIN"}));
            gts_1.add_output_pins(std::vector<std::string>({"SO_PIN","invSO_PIN"}));

            // -- set init_data_category
            gts_0.set_init_data_category("category");
            gts_1.set_init_data_category("category");
            // -- set init_data_identifier
            gts_0.set_init_data_identifier("identifier");
            gts_1.set_init_data_identifier("identifier");
            // -- add state output pin
            gts_0.add_state_output_pin("SO_PIN");
            gts_1.add_state_output_pin("SO_PIN");
            // -- add inverted state output pin
            gts_0.add_inverted_state_output_pin("invSO_PIN");
            gts_1.add_inverted_state_output_pin("invSO_PIN");
            // -- set set-reset behavior
            gts_0.set_set_reset_behavior(gate_type_sequential::set_reset_behavior::H, gate_type_sequential::set_reset_behavior::N);
            gts_1.set_set_reset_behavior(gate_type_sequential::set_reset_behavior::H, gate_type_sequential::set_reset_behavior::N);

            EXPECT_TRUE(gts_0 == gts_1);
        }
        {
            // Compare two gate_type_sequential while they differ in one property (category, identifier,
            // state output pins, inverted state output pins)
            gate_type_sequential gts_0("gts_name", gate_type::base_type::ff);
            gate_type_sequential gts_1("gts_name", gate_type::base_type::ff);
            gts_0.add_output_pins(std::vector<std::string>({"SO_PIN","invSO_PIN"}));
            gts_1.add_output_pins(std::vector<std::string>({"SO_PIN","invSO_PIN"}));

            // -- set init_data_category
            gts_0.set_init_data_category("category");
            EXPECT_FALSE(gts_0 == gts_1);
            gts_1.set_init_data_category("category");
            // -- set init_data_identifier
            gts_0.set_init_data_identifier("identifier");
            EXPECT_FALSE(gts_0 == gts_1);
            gts_1.set_init_data_identifier("identifier");
            // -- add state output pin
            gts_0.add_state_output_pin("SO_PIN");
            EXPECT_FALSE(gts_0 == gts_1);
            gts_1.add_state_output_pin("SO_PIN");
            // -- add inverted state output pin
            gts_0.add_inverted_state_output_pin("invSO_PIN");
            EXPECT_FALSE(gts_0 == gts_1);
            gts_1.add_inverted_state_output_pin("invSO_PIN");
            // -- set set-reset behavior
            gts_0.set_set_reset_behavior(gate_type_sequential::set_reset_behavior::H, gate_type_sequential::set_reset_behavior::N);
            EXPECT_FALSE(gts_0 == gts_1);
        }
        {
            // Compare two gate_type. One is a gate_type, the other one a gate_type_lut
            gate_type_sequential gt_0("gt_name", gate_type::base_type::latch);
            gate_type gt_1("gt_name");

            EXPECT_FALSE(gt_0 == gt_1);
            EXPECT_FALSE(gt_1 == gt_0);
        }
    TEST_END
}

// ======== gate_library tests =========

/**
 * Testing the creation of a new gate_library and the addition of gate types and includes to it
 *
 * Functions: constructor, get_name, add_gate_type, get_gate_types, get_vcc_gate_types, get_gnd_gate_types,
 *            add_include, get_includes
 */
TEST_F(gate_library_test, check_library)
{
    TEST_START
        // Create some gate types beforehand
        // -- a simple AND gate
        std::shared_ptr<gate_type> gt_and(new gate_type("gt_and"));
        gt_and->add_input_pins(std::vector<std::string>({"I0","I1"}));
        gt_and->add_output_pins(std::vector<std::string>({"O"}));
        gt_and->add_boolean_function("O", boolean_function::from_string("I0 & I1"));
        // -- a GND gate
        std::shared_ptr<gate_type> gt_gnd(new gate_type("gt_gnd"));
        gt_gnd->add_output_pins(std::vector<std::string>({"O"}));
        gt_gnd->add_boolean_function("O", boolean_function(boolean_function::value::ZERO));
        // -- a VCC gate
        std::shared_ptr<gate_type> gt_vcc(new gate_type("gt_vcc"));
        gt_vcc->add_output_pins(std::vector<std::string>({"O"}));
        gt_vcc->add_boolean_function("O", boolean_function(boolean_function::value::ONE));

        {
            std::shared_ptr<gate_library> gl(new gate_library("gl_name"));
            // Check the name
            EXPECT_EQ(gl->get_name(), "gl_name");

            // Check the addition of gate types
            // -- add the gate types
            gl->add_gate_type(gt_and);
            gl->add_gate_type(gt_gnd);
            gl->add_gate_type(gt_vcc);
            // -- get the gate types
            EXPECT_EQ(gl->get_gate_types(),(std::map<std::string,std::shared_ptr<const gate_type>>({{"gt_and", gt_and},
                                                                                                      {"gt_gnd", gt_gnd},
                                                                                                      {"gt_vcc", gt_vcc}})));
            EXPECT_EQ(gl->get_vcc_gate_types(),(std::map<std::string,std::shared_ptr<const gate_type>>({{"gt_vcc", gt_vcc}})));
            EXPECT_EQ(gl->get_gnd_gate_types(),(std::map<std::string,std::shared_ptr<const gate_type>>({{"gt_gnd", gt_gnd}})));

            // Check the addition of includes
            gl->add_include("in.clu.de");
            gl->add_include("another.include");
            gl->add_include("last.include");
            EXPECT_EQ(gl->get_includes(), std::vector<std::string>({"in.clu.de", "another.include", "last.include"}));
        }
    TEST_END
}

