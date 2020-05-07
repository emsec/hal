#include "netlist_test_utils.h"
#include "netlist/gate_library/gate_library_manager.h"
#include "netlist/gate_library/gate_library_parser/gate_library_parser_liberty.h"
#include "netlist/gate_library/gate_type/gate_type.h"
#include "netlist/gate_library/gate_type/gate_type_sequential.h"
#include "netlist/gate_library/gate_type/gate_type_lut.h"
#include "netlist/netlist.h"
#include "gtest/gtest.h"
#include <iostream>
#include <sstream>
#include <experimental/filesystem>


using namespace test_utils;

class gate_library_parser_liberty_test : public ::testing::Test
{
protected:
    virtual void SetUp()
    {
    }

    virtual void TearDown()
    {
    }
};

/**
 * Testing the creation of a combinatorial gate type with one input pin, one output pin and a boolean function.
 *
 * Functions: parse
 */
TEST_F(gate_library_parser_liberty_test, check_combinatorial)
{
    TEST_START
        {
            std::stringstream input("library (TEST_GATE_LIBRARY) {\n"
                                    "    date : \"April 3, 2020\"\n" // <- will be ignored
                                    "    revision : 2015.03; \n"     // <- will be ignored
                                    "    define(cell);\n"
                                    "    cell(TEST_GATE_TYPE) {\n"
                                    "        area : 3; \n"  // <- will be ignored
                                    "        pin(I) {\n"
                                    "            direction: input;\n"
                                    "            capacitance : 1;\n"
                                    "        }\n"
                                    "        pin(O) {\n"
                                    "            direction: output;\n"
                                    "            function: \"I\";\n"
                                    "            x_function: \"!I\";\n"
                                    "        }\n"
                                    "    }\n"
                                    "}");
            gate_library_parser_liberty liberty_parser(input);
            std::shared_ptr<gate_library> gl = liberty_parser.parse();

            ASSERT_NE(gl, nullptr);

            // Check that the name of the gate library
            EXPECT_EQ(gl->get_name(), "TEST_GATE_LIBRARY");

            // Check that the gate type was created
            ASSERT_EQ(gl->get_gate_types().size(), 1);
            auto gt_it = gl->get_gate_types().find("TEST_GATE_TYPE");
            ASSERT_TRUE(gt_it != gl->get_gate_types().end());
            std::shared_ptr<const gate_type> gt = gt_it->second;

            // Check the content of the created gate type
            EXPECT_EQ(gt->get_base_type(), gate_type::base_type::combinatorial);
            // -- Check the pins
            EXPECT_EQ(gt->get_input_pins(), std::vector<std::string>({"I"}));
            EXPECT_EQ(gt->get_output_pins(), std::vector<std::string>({"O"}));
            // -- Check the boolean functions
            ASSERT_TRUE(gt->get_boolean_functions().find("O") != gt->get_boolean_functions().end());
            EXPECT_EQ(gt->get_boolean_functions().at("O"), boolean_function::from_string("I", std::vector<std::string>({"I"})));
            ASSERT_TRUE(gt->get_boolean_functions().find("O_undefined") != gt->get_boolean_functions().end()); // x_function
            EXPECT_EQ(gt->get_boolean_functions().at("O_undefined"), boolean_function::from_string("!I", std::vector<std::string>({"I"})));
        }
    TEST_END
}

/**
 * Testing the creation of a LUT gate type
 *
 * Functions: parse
 */
TEST_F(gate_library_parser_liberty_test, check_lut)
{
    TEST_START
        {
            // Create a LUT gate type with two input pins and four output pins
            // O0 and O2 generate their output by an initializer string.
            // O1 and O3 are given normal boolean functions.
            std::stringstream input("library (TEST_GATE_LIBRARY) {\n"
                                    "    define(cell);\n"
                                    "    cell(TEST_LUT) {\n"
                                    "        lut (\"lut_out\") {\n"
                                    "            data_category     : \"test_category\";\n"
                                    "            data_identifier   : \"test_identifier\";\n"
                                    "            direction         : \"ascending\";\n"
                                    "        }\n"
                                    "        pin(I0) {\n"
                                    "            direction: input;\n"
                                    "        }\n"
                                    "        pin(I1) {\n"
                                    "            direction: input;\n"
                                    "        }\n"
                                    "        pin(O0) {\n"
                                    "            direction: output;\n"
                                    "            function: \"lut_out\";\n"
                                    "        }\n"
                                    "        pin(O1) {\n"
                                    "            direction: output;\n"
                                    "            function: \"I0 ^ I1\";\n"
                                    "        }\n"
                                    "        pin(O2) {\n"
                                    "            direction: output;\n"
                                    "            function: \"lut_out\";\n"
                                    "        }\n"
                                    "        pin(O3) {\n"
                                    "            direction: output;\n"
                                    "            function: \"I0 & I1\";\n"
                                    "        }\n"
                                    "    }"
                                    "}");
            gate_library_parser_liberty liberty_parser(input);
            std::shared_ptr<gate_library> gl = liberty_parser.parse();

            ASSERT_NE(gl, nullptr);

            // Check that the gate type was created
            ASSERT_EQ(gl->get_gate_types().size(), 1);
            auto gt_it = gl->get_gate_types().find("TEST_LUT");
            ASSERT_TRUE(gt_it != gl->get_gate_types().end());
            std::shared_ptr<const gate_type> gt = gt_it->second;
            ASSERT_EQ(gt->get_base_type(), gate_type::base_type::lut);
            std::shared_ptr<const gate_type_lut> gt_lut = std::dynamic_pointer_cast<const gate_type_lut>(gt);

            // Check the content of the created gate type
            EXPECT_EQ(gt_lut->get_input_pins(), std::vector<std::string>({"I0", "I1"}));
            EXPECT_EQ(gt_lut->get_output_pins(), std::vector<std::string>({"O0", "O1", "O2", "O3"}));
            ASSERT_TRUE(gt_lut->get_boolean_functions().find("O1") != gt_lut->get_boolean_functions().end());
            EXPECT_EQ(gt_lut->get_boolean_functions().at("O1"), boolean_function::from_string("I0 ^ I1", std::vector<std::string>({"I0","I1"})));
            ASSERT_TRUE(gt_lut->get_boolean_functions().find("O3") != gt_lut->get_boolean_functions().end());
            EXPECT_EQ(gt_lut->get_boolean_functions().at("O3"), boolean_function::from_string("I0 & I1", std::vector<std::string>({"I0", "I1"})));
            // -- LUT specific
            EXPECT_EQ(gt_lut->get_output_from_init_string_pins(), std::unordered_set<std::string>({"O0", "O2"}));
            EXPECT_EQ(gt_lut->get_config_data_category(), "test_category");
            EXPECT_EQ(gt_lut->get_config_data_identifier(), "test_identifier");
            EXPECT_EQ(gt_lut->is_config_data_ascending_order(), true);

        }
        {
            // Create a simple LUT gate type with an descending bit order
            std::stringstream input("library (TEST_GATE_LIBRARY) {\n"
                                    "    define(cell);\n"
                                    "    cell(TEST_LUT) {\n"
                                    "        lut (\"lut_out\") {\n"
                                    "            data_category     : \"test_category\";\n"
                                    "            data_identifier   : \"test_identifier\";\n"
                                    "            direction         : \"descending\";\n"
                                    "        }\n"
                                    "        pin(I) {\n"
                                    "            direction: input;\n"
                                    "        }\n"
                                    "        pin(O) {\n"
                                    "            direction: output;\n"
                                    "            function: \"lut_out\";\n"
                                    "        }\n"
                                    "    }"
                                    "}");
            gate_library_parser_liberty liberty_parser(input);
            std::shared_ptr<gate_library> gl = liberty_parser.parse();

            ASSERT_NE(gl, nullptr);

            // Check that the gate type was created
            ASSERT_EQ(gl->get_gate_types().size(), 1);
            auto gt_it = gl->get_gate_types().find("TEST_LUT");
            ASSERT_TRUE(gt_it != gl->get_gate_types().end());
            std::shared_ptr<const gate_type> gt = gt_it->second;
            ASSERT_EQ(gt->get_base_type(), gate_type::base_type::lut);
            std::shared_ptr<const gate_type_lut> gt_lut = std::dynamic_pointer_cast<const gate_type_lut>(gt);

            // Check the content of the created gate type
            EXPECT_EQ(gt_lut->is_config_data_ascending_order(), false);

        }
    TEST_END
}


/**
 * Testing the creation of flip-flops
 *
 * Functions: parse
 */
TEST_F(gate_library_parser_liberty_test, check_flip_flop)
{
    TEST_START
        {
            // Create a flip-flop gate type with two input pins and four output pins
            std::stringstream input("library (TEST_GATE_LIBRARY) {\n"
                                    "    define(cell);\n"
                                    "    cell(TEST_FF) {\n"
                                    "        ff (\"IQ\" , \"IQN\") {\n"
                                    "            next_state          : \"D\";\n"
                                    "            clocked_on          : \"CLK\";\n"
                                    "            clocked_on_also     : \"CLK\";\n"
                                    "            preset              : \"S\";\n"
                                    "            clear               : \"R\";\n"
                                    "            clear_preset_var1   : L;\n"
                                    "            clear_preset_var2   : H;\n"
                                    "            direction           : ascending;\n"
                                    "        }\n"
                                    "        pin(CLK) {\n"
                                    "            direction: input;\n"
                                    "            clock: true;\n"
                                    "        }\n"
                                    "        pin(CE) {\n"
                                    "            direction: input;\n"
                                    "        }\n"
                                    "        pin(D) {\n"
                                    "            direction: input;\n"
                                    "        }\n"
                                    "        pin(R) {\n"
                                    "            direction: input;\n"
                                    "        }\n"
                                    "        pin(S) {\n"
                                    "            direction: input;\n"
                                    "        }\n"
                                    "        pin(Q) {\n"
                                    "            direction: output;\n"
                                    "            function: \"IQ\";\n"
                                    "        }\n"
                                    "        pin(QN) {\n"
                                    "            direction: output;\n"
                                    "            function: \"IQN\";\n"
                                    "        }\n"
                                    "        pin(O) {\n"
                                    "            direction: output;\n"
                                    "            function: \"S & R & D\";\n"
                                    "        }\n"
                                    "    }"
                                    "}");
            gate_library_parser_liberty liberty_parser(input);
            std::shared_ptr<gate_library> gl = liberty_parser.parse();

            ASSERT_NE(gl, nullptr);

            // Check that the gate type was created
            ASSERT_EQ(gl->get_gate_types().size(), 1);
            auto gt_it = gl->get_gate_types().find("TEST_FF");
            ASSERT_TRUE(gt_it != gl->get_gate_types().end());
            std::shared_ptr<const gate_type> gt = gt_it->second;
            ASSERT_EQ(gt->get_base_type(), gate_type::base_type::ff);
            std::shared_ptr<const gate_type_sequential> gt_ff = std::dynamic_pointer_cast<const gate_type_sequential>(gt);

            // Check the content of the created gate type
            EXPECT_EQ(gt_ff->get_input_pins(), std::vector<std::string>({"CLK", "CE", "D", "R", "S"}));
            EXPECT_EQ(gt_ff->get_output_pins(), std::vector<std::string>({"Q", "QN", "O"}));
            ASSERT_TRUE(gt_ff->get_boolean_functions().find("O") != gt_ff->get_boolean_functions().end());
            EXPECT_EQ(gt_ff->get_boolean_functions().at("O"), boolean_function::from_string("S & R & D", std::vector<std::string>({"S", "R", "D"})));
            // -- Check the boolean functions of the ff group that are parsed (currently only next_state, clock_on(clock), preset(set) and clear(reset) are parsed )
            ASSERT_TRUE(gt_ff->get_boolean_functions().find("next_state") != gt_ff->get_boolean_functions().end());
            EXPECT_EQ(gt_ff->get_boolean_functions().at("next_state"), boolean_function::from_string("D", std::vector<std::string>({"D"})));
            ASSERT_TRUE(gt_ff->get_boolean_functions().find("clock") != gt_ff->get_boolean_functions().end());
            EXPECT_EQ(gt_ff->get_boolean_functions().at("clock"), boolean_function::from_string("CLK", std::vector<std::string>({"CLK"})));
            ASSERT_TRUE(gt_ff->get_boolean_functions().find("set") != gt_ff->get_boolean_functions().end());
            EXPECT_EQ(gt_ff->get_boolean_functions().at("set"), boolean_function::from_string("S", std::vector<std::string>({"S"})));
            ASSERT_TRUE(gt_ff->get_boolean_functions().find("reset") != gt_ff->get_boolean_functions().end());
            EXPECT_EQ(gt_ff->get_boolean_functions().at("reset"), boolean_function::from_string("R", std::vector<std::string>({"R"})));
            // -- Check the output pins
            EXPECT_EQ(gt_ff->get_state_output_pins(), std::unordered_set<std::string>({"Q"}));
            EXPECT_EQ(gt_ff->get_inverted_state_output_pins(), std::unordered_set<std::string>({"QN"}));
            // -- Check the set-reset behaviour
            EXPECT_EQ(gt_ff->get_set_reset_behavior(), std::make_pair(gate_type_sequential::set_reset_behavior::L, gate_type_sequential::set_reset_behavior::H));
        }
    TEST_END
}

/**
 * Testing the creation of a latches
 *
 * Functions: parse
 */
TEST_F(gate_library_parser_liberty_test, check_latch)
{
    TEST_START
        {
            // Create a flip-flop gate type with two input pins and four output pins
            std::stringstream input("library (TEST_GATE_LIBRARY) {\n"
                                    "    define(cell);\n"
                                    "    cell(TEST_LATCH) {\n"
                                    "        latch (\"IQ\" , \"IQN\") {\n"
                                    "            enable              : \"G\";\n"
                                    "            data_in             : \"D\";\n"
                                    "            preset              : \"S\";\n"
                                    "            clear               : \"R\";\n"
                                    "            clear_preset_var1   : N;\n"
                                    "            clear_preset_var2   : T;\n"
                                    "        }\n"
                                    "        pin(G) {\n"
                                    "            direction: input;\n"
                                    "        }\n"
                                    "        pin(D) {\n"
                                    "            direction: input;\n"
                                    "        }\n"
                                    "        pin(S) {\n"
                                    "            direction: input;\n"
                                    "        }\n"
                                    "        pin(R) {\n"
                                    "            direction: input;\n"
                                    "        }\n"
                                    "        pin(Q) {\n"
                                    "            direction: output;\n"
                                    "            function: \"IQ\";\n"
                                    "        }\n"
                                    "        pin(QN) {\n"
                                    "            direction: output;\n"
                                    "            function: \"IQN\";\n"
                                    "        }\n"
                                    "        pin(O) {\n"
                                    "            direction: output;\n"
                                    "            function: \"S & R & D\";\n"
                                    "        }\n"
                                    "    }"
                                    "}");
            gate_library_parser_liberty liberty_parser(input);
            std::shared_ptr<gate_library> gl = liberty_parser.parse();

            ASSERT_NE(gl, nullptr);

            // Check that the gate type was created
            ASSERT_EQ(gl->get_gate_types().size(), 1);
            auto gt_it = gl->get_gate_types().find("TEST_LATCH");
            ASSERT_TRUE(gt_it != gl->get_gate_types().end());
            std::shared_ptr<const gate_type> gt = gt_it->second;
            ASSERT_EQ(gt->get_base_type(), gate_type::base_type::latch);
            std::shared_ptr<const gate_type_sequential> gt_latch = std::dynamic_pointer_cast<const gate_type_sequential>(gt);

            // Check the content of the created gate type
            EXPECT_EQ(gt_latch->get_input_pins(), std::vector<std::string>({"G", "D", "S", "R"}));
            EXPECT_EQ(gt_latch->get_output_pins(), std::vector<std::string>({"Q", "QN", "O"}));
            ASSERT_TRUE(gt_latch->get_boolean_functions().find("O") != gt_latch->get_boolean_functions().end());
            EXPECT_EQ(gt_latch->get_boolean_functions().at("O"), boolean_function::from_string("S & R & D", std::vector<std::string>({"S", "R", "D"})));
            // -- Check the boolean functions of the latch group that are parsed (currently only enable, data_in, preset(set) and clear(reset) are parsed)
            ASSERT_TRUE(gt_latch->get_boolean_functions().find("enable") != gt_latch->get_boolean_functions().end());
            EXPECT_EQ(gt_latch->get_boolean_functions().at("enable"), boolean_function::from_string("G", std::vector<std::string>({"G"})));
            ASSERT_TRUE(gt_latch->get_boolean_functions().find("data_in") != gt_latch->get_boolean_functions().end());
            EXPECT_EQ(gt_latch->get_boolean_functions().at("data_in"), boolean_function::from_string("D", std::vector<std::string>({"D"})));
            ASSERT_TRUE(gt_latch->get_boolean_functions().find("set") != gt_latch->get_boolean_functions().end());
            EXPECT_EQ(gt_latch->get_boolean_functions().at("set"), boolean_function::from_string("S", std::vector<std::string>({"S"})));
            ASSERT_TRUE(gt_latch->get_boolean_functions().find("reset") != gt_latch->get_boolean_functions().end());
            EXPECT_EQ(gt_latch->get_boolean_functions().at("reset"), boolean_function::from_string("R", std::vector<std::string>({"R"})));
            // -- Check the output pins
            EXPECT_EQ(gt_latch->get_state_output_pins(), std::unordered_set<std::string>({"Q"}));
            EXPECT_EQ(gt_latch->get_inverted_state_output_pins(), std::unordered_set<std::string>({"QN"}));
            // -- Check the set-reset behaviour
            EXPECT_EQ(gt_latch->get_set_reset_behavior(), std::make_pair(gate_type_sequential::set_reset_behavior::N, gate_type_sequential::set_reset_behavior::T));

        }
    TEST_END
}

/**
 * Testing the usage of multiline comments (via '/ *' and '* /'  (without space))
 *
 * Functions: parse
 */
TEST_F(gate_library_parser_liberty_test, check_multiline_comment)
{
    TEST_START
        {
            // The output pins O0, O1, O2, O3 should be created, C0, C1, C2, C3 shouldn't
            std::stringstream input("library (TEST_GATE_LIBRARY) {\n"
                                    "    define(cell);\n"
                                    "    cell(TEST_GATE_TYPE) {\n"
                                    "        pin(I) {\n"
                                    "            direction: input;\n"
                                    "        }\n"
                                    "        pin(O0) { direction: output; function: \"I\"; }\n"
                                    "        pin(O1) { direction: output; function: \"I\"; } /* pin(C0) { direction: output; function: \"I\"; }*/\n"
                                    "        pin(O2) { direction: output; function: \"I\"; }\n"
                                    "        /*pin(C1) { direction: output; function: \"I\"; }\n"
                                    "        pin(C2) { direction: output; function: \"I\"; }\n"
                                    "        pin(C3) { direction: output; function: \"I\"; }*/\n"
                                    "        pin(O3) { direction: output; function: \"I\"; }\n"
                                    "        }\n"
                                    "    }\n"
                                    "}");
            gate_library_parser_liberty liberty_parser(input);
            std::shared_ptr<gate_library> gl = liberty_parser.parse();

            ASSERT_NE(gl, nullptr);
            // Check that the gate type was created
            ASSERT_EQ(gl->get_gate_types().size(), 1);
            auto gt_it = gl->get_gate_types().find("TEST_GATE_TYPE");
            ASSERT_TRUE(gt_it != gl->get_gate_types().end());
            std::shared_ptr<const gate_type> gt = gt_it->second;

            // Check that only the pins outside the comments are created
            EXPECT_EQ(gt->get_input_pins(), std::vector<std::string>({"I"}));
            EXPECT_EQ(gt->get_output_pins(), std::vector<std::string>({"O0", "O1", "O2", "O3"}));
        }
    TEST_END
}

/**
 * Testing the correct handling of invalid input and other uncommon inputs
 *
 * Functions: parse
 */
TEST_F(gate_library_parser_liberty_test, check_invalid_input)
{
    TEST_START
        {
            // Pass an empty input stream
            NO_COUT_TEST_BLOCK;
            std::stringstream input("");
            gate_library_parser_liberty liberty_parser(input);
            std::shared_ptr<gate_library> gl = liberty_parser.parse();

            EXPECT_EQ(gl, nullptr);
        }
        {
            // For a ff, use an undefined clear_preset_var1 behaviour (not in {L,H,N,T,X})
            NO_COUT_TEST_BLOCK;
            std::stringstream input("library (TEST_GATE_LIBRARY) {\n"
                                    "    define(cell);\n"
                                    "    cell(TEST_FF) {\n"
                                    "        ff (\"IQ\" , \"IQN\") {\n"
                                    "            next_state          : \"P\";\n"
                                    "            clocked_on          : \"P\";\n"
                                    "            preset              : \"P\";\n"
                                    "            clear               : \"P\";\n"
                                    "            clear_preset_var1   : Z;\n"
                                    "            clear_preset_var2   : L;\n"
                                    "        }\n"
                                    "        pin(P) {\n"
                                    "            direction: input;\n"
                                    "            clock: true;\n"
                                    "        }\n"
                                    "        pin(Q) {\n"
                                    "            direction: output;\n"
                                    "            function: \"IQ\";\n"
                                    "        }\n"
                                    "    }"
                                    "}");
            gate_library_parser_liberty liberty_parser(input);
            std::shared_ptr<gate_library> gl = liberty_parser.parse();

            EXPECT_EQ(gl, nullptr);
        }
        {
            // For a ff, use an undefined clear_preset_var2 behaviour (not in {L,H,N,T,X})
            NO_COUT_TEST_BLOCK;
            std::stringstream input("library (TEST_GATE_LIBRARY) {\n"
                                    "    define(cell);\n"
                                    "    cell(TEST_FF) {\n"
                                    "        ff (\"IQ\" , \"IQN\") {\n"
                                    "            next_state          : \"P\";\n"
                                    "            clocked_on          : \"P\";\n"
                                    "            preset              : \"P\";\n"
                                    "            clear               : \"P\";\n"
                                    "            clear_preset_var1   : L;\n"
                                    "            clear_preset_var2   : Z;\n"
                                    "        }\n"
                                    "        pin(P) {\n"
                                    "            direction: input;\n"
                                    "            clock: true;\n"
                                    "        }\n"
                                    "        pin(Q) {\n"
                                    "            direction: output;\n"
                                    "            function: \"IQ\";\n"
                                    "        }\n"
                                    "    }"
                                    "}");
            gate_library_parser_liberty liberty_parser(input);
            std::shared_ptr<gate_library> gl = liberty_parser.parse();

            EXPECT_EQ(gl, nullptr);
        }
        {
            // For a latch, use an undefined clear_preset_var1 behaviour (not in {L,H,N,T,X})
            NO_COUT_TEST_BLOCK;
            std::stringstream input("library (TEST_GATE_LIBRARY) {\n"
                                    "    define(cell);\n"
                                    "    cell(TEST_LATCH) {\n"
                                    "        latch (\"IQ\" , \"IQN\") {\n"
                                    "            enable              : \"P\";\n"
                                    "            data_in             : \"P\";\n"
                                    "            preset              : \"P\";\n"
                                    "            clear               : \"P\";\n"
                                    "            clear_preset_var1   : Z;\n"
                                    "            clear_preset_var2   : L;\n"
                                    "        }\n"
                                    "        pin(P) {\n"
                                    "            direction: input;\n"
                                    "        }\n"
                                    "        pin(Q) {\n"
                                    "            direction: output;\n"
                                    "            function: \"IQ\";\n"
                                    "        }\n"
                                    "    }"
                                    "}");
            gate_library_parser_liberty liberty_parser(input);
            std::shared_ptr<gate_library> gl = liberty_parser.parse();

            EXPECT_EQ(gl, nullptr);
        }
        {
            // For a latch, use an undefined clear_preset_var1 behaviour (not in {L,H,N,T,X})
            NO_COUT_TEST_BLOCK;
            std::stringstream input("library (TEST_GATE_LIBRARY) {\n"
                                    "    define(cell);\n"
                                    "    cell(TEST_LATCH) {\n"
                                    "        latch (\"IQ\" , \"IQN\") {\n"
                                    "            enable              : \"P\";\n"
                                    "            data_in             : \"P\";\n"
                                    "            preset              : \"P\";\n"
                                    "            clear               : \"P\";\n"
                                    "            clear_preset_var1   : L;\n"
                                    "            clear_preset_var2   : Z;\n"
                                    "        }\n"
                                    "        pin(P) {\n"
                                    "            direction: input;\n"
                                    "        }\n"
                                    "        pin(Q) {\n"
                                    "            direction: output;\n"
                                    "            function: \"IQ\";\n"
                                    "        }\n"
                                    "    }"
                                    "}");
            gate_library_parser_liberty liberty_parser(input);
            std::shared_ptr<gate_library> gl = liberty_parser.parse();

            EXPECT_EQ(gl, nullptr);
        }
        {
            // Use an undefined direction in the lut group block (not in {ascending, descending})
            NO_COUT_TEST_BLOCK;
            std::stringstream input("library (TEST_GATE_LIBRARY) {\n"
                                    "    define(cell);\n"
                                    "    cell(TEST_LUT) {\n"
                                    "        lut (\"lut_out\") {\n"
                                    "            data_category     : \"test_category\";\n"
                                    "            data_identifier   : \"test_identifier\";\n"
                                    "            direction         : \"north-east\";\n" // <-"north-east" is no valid data direction
                                    "        }\n"
                                    "        pin(I) {\n"
                                    "            direction: input;\n"
                                    "        }\n"
                                    "        pin(O) {\n"
                                    "            direction: output;\n"
                                    "            function: \"lut_out\";\n"
                                    "        }\n"
                                    "    }"
                                    "}");
            gate_library_parser_liberty liberty_parser(input);
            std::shared_ptr<gate_library> gl = liberty_parser.parse();

            EXPECT_EQ(gl, nullptr);
        }
        {
            // Use an undefined direction in the ff group block (not in {ascending, descending})
            NO_COUT_TEST_BLOCK;
            std::stringstream input("library (TEST_GATE_LIBRARY) {\n"
                                    "    define(cell);\n"
                                    "    cell(TEST_LUT) {\n"
                                    "        ff (\"IQ\" , \"IQN\") {\n"
                                    "            direction         : \"north-east\";\n" // <-"north-east" is no valid data direction
                                    "        }\n"
                                    "        pin(I) {\n"
                                    "            direction: input;\n"
                                    "        }\n"
                                    "        pin(O) {\n"
                                    "            direction: output;\n"
                                    "            function: \"I\";\n"
                                    "        }\n"
                                    "    }"
                                    "}");
            gate_library_parser_liberty liberty_parser(input);
            std::shared_ptr<gate_library> gl = liberty_parser.parse();

            EXPECT_EQ(gl, nullptr);
        }
        {
            // Use a pin with an unknown direction (not in {input, output}) as an input pin
            NO_COUT_TEST_BLOCK;
            std::stringstream input("library (TEST_GATE_LIBRARY) {\n"
                                    "    define(cell);\n"
                                    "    cell(TEST_GATE_TYPE) {\n"
                                    "        pin(I) {\n"
                                    "            direction: WAMBO;\n"
                                    "        }\n"
                                    "        pin(O) {\n"
                                    "            direction: output;\n"
                                    "            function: \"I\";\n"
                                    "        }\n"
                                    "    }\n"
                                    "}");
            gate_library_parser_liberty liberty_parser(input);
            std::shared_ptr<gate_library> gl = liberty_parser.parse();

            EXPECT_EQ(gl, nullptr);
        }
        {
            // Use an unknown variable in a boolean function
            NO_COUT_TEST_BLOCK;
            std::stringstream input("library (TEST_GATE_LIBRARY) {\n"
                                    "    define(cell);\n"
                                    "    cell(TEST_GATE_TYPE) {\n"
                                    "        pin(I) {\n"
                                    "            direction: input;\n"
                                    "        }\n"
                                    "        pin(O) {\n"
                                    "            direction: output;\n"
                                    "            function: \"I & WAMBO\";\n" // <- WAMBO is undefined
                                    "        }\n"
                                    "    }\n"
                                    "}");
            gate_library_parser_liberty liberty_parser(input);
            std::shared_ptr<gate_library> gl = liberty_parser.parse();

            EXPECT_EQ(gl, nullptr);
        }
        {
            // Use an unknown cell group (should be filtered out)
            NO_COUT_TEST_BLOCK;
            std::stringstream input("library (TEST_GATE_LIBRARY) {\n"
                                    "    define(cell);\n"
                                    "    cell(TEST_GATE_TYPE) {\n"
                                    "        biological_cell (\"A\" , \"B\") {\n" // the parser does not support biological cells ;)
                                    "            cell_type: eukaryotic; \n"
                                    "            species: dog; \n  "
                                    "            has_cell_wall: nope; \n  "
                                    "        }\n"
                                    "        pin(P) {\n"
                                    "            direction: input;\n"
                                    "        }\n"
                                    "        pin(Q) {\n"
                                    "            direction: output;\n"
                                    "            function: \"P\";\n"
                                    "        }\n"
                                    "    }"
                                    "}");
            gate_library_parser_liberty liberty_parser(input);
            std::shared_ptr<gate_library> gl = liberty_parser.parse();

            ASSERT_NE(gl, nullptr);

            ASSERT_TRUE(gl->get_gate_types().find("TEST_GATE_TYPE") != gl->get_gate_types().end());
            EXPECT_EQ(gl->get_gate_types().at("TEST_GATE_TYPE")->get_base_type(), gate_type::base_type::combinatorial);
        }
        {
            // Define a pin twice (ISSUE: Pin can be defined twice. Both are added, but only the boolean function of the second one is stored)
            //NO_COUT_TEST_BLOCK;
            std::stringstream input("library (TEST_GATE_LIBRARY) {\n"
                                    "    define(cell);\n"
                                    "    cell(TEST_GATE_TYPE) {\n"
                                    "        pin(I) {\n"
                                    "            direction: input;\n"
                                    "        }\n"
                                    "        pin(O) {\n"
                                    "            direction: output;\n"
                                    "            function: \"I\";\n"
                                    "        }\n"
                                    "        pin(O) {\n"
                                    "            direction: output;\n"
                                    "            function: \"!I\";\n"
                                    "        }\n"
                                    "    }\n"
                                    "}");
            gate_library_parser_liberty liberty_parser(input);
            std::shared_ptr<gate_library> gl = liberty_parser.parse();

            ASSERT_NE(gl, nullptr);

            ASSERT_TRUE(gl->get_gate_types().find("TEST_GATE_TYPE") != gl->get_gate_types().end());
            EXPECT_EQ(gl->get_gate_types().at("TEST_GATE_TYPE")->get_base_type(), gate_type::base_type::combinatorial);
            // EXPECT_EQ(gl->get_gate_types().at("TEST_GATE_TYPE")->get_output_pins().size(), 1); // ISSUE
        }
        {
            // Define a gate type twice (ISSUE: Gate can be defined twice, but only the first one is added with no warning)
            //NO_COUT_TEST_BLOCK;
            std::stringstream input("library (TEST_GATE_LIBRARY) {\n"
                                    "    define(cell);\n"
                                    "    cell(TEST_GATE_TYPE) {\n"
                                    "        pin(I) {\n"
                                    "            direction: input;\n"
                                    "        }\n"
                                    "        pin(O) {\n"
                                    "            direction: output;\n"
                                    "            function: \"!I\";\n"
                                    "        }\n"
                                    "    }\n"
                                    "    cell(TEST_GATE_TYPE) {\n"  // <- is ignored ISSUE?
                                    "        pin(I0) {\n"
                                    "            direction: input;\n"
                                    "        }\n"
                                    "        pin(O0) {\n"
                                    "            direction: output;\n"
                                    "            function: \"!I0\";\n"
                                    "        }\n"
                                    "    }\n"
                                    "}");
            gate_library_parser_liberty liberty_parser(input);
            std::shared_ptr<gate_library> gl = liberty_parser.parse();

            ASSERT_NE(gl, nullptr);

            ASSERT_TRUE(gl->get_gate_types().find("TEST_GATE_TYPE") != gl->get_gate_types().end());
            EXPECT_EQ(gl->get_gate_types().at("TEST_GATE_TYPE")->get_base_type(), gate_type::base_type::combinatorial);
            // EXPECT_EQ(gl->get_gate_types().at("TEST_GATE_TYPE")->get_output_pins().size(), 1); // ISSUE
        }
        {
            // Pin got no direction (should be ignored)
            NO_COUT_TEST_BLOCK;
            std::stringstream input("library (TEST_GATE_LIBRARY) {\n"
                                    "    define(cell);\n"
                                    "    cell(TEST_GATE_TYPE) {\n"
                                    "        pin(I) {\n"
                                    "            direction: input;\n"
                                    "        }\n"
                                    "        pin(I_nodir) {\n"
                                    "            capacitance : 1.0;"
                                    "        }\n"
                                    "        pin(O) {\n"
                                    "            direction: output;\n"
                                    "            function: \"I\";\n"
                                    "        }\n"
                                    "    }\n"
                                    "}");
            gate_library_parser_liberty liberty_parser(input);
            std::shared_ptr<gate_library> gl = liberty_parser.parse();

            ASSERT_NE(gl, nullptr);

            ASSERT_TRUE(gl->get_gate_types().find("TEST_GATE_TYPE") != gl->get_gate_types().end());
            EXPECT_EQ(gl->get_gate_types().at("TEST_GATE_TYPE")->get_base_type(), gate_type::base_type::combinatorial);
            EXPECT_EQ(gl->get_gate_types().at("TEST_GATE_TYPE")->get_input_pins().size(), 1);


        }
        {
            // Test the usage of compex attributes
            std::stringstream input("library (TEST_GATE_LIBRARY) {\n"
                                    "    define(cell);\n"
                                    "    cell(TEST_GATE_TYPE) {\n"
                                    "        pin(I) {\n"
                                    "            direction: input;\n"
                                    "        }\n"
                                    "        pin(O) {\n"
                                    "            complex_attr(param1, param2);\n"
                                    "            direction: output;\n"
                                    "            function: \"!I\";\n"
                                    "        }\n"
                                    "    }\n"
                                    "}");
            gate_library_parser_liberty liberty_parser(input);
            std::shared_ptr<gate_library> gl = liberty_parser.parse();

            ASSERT_NE(gl, nullptr);

            ASSERT_TRUE(gl->get_gate_types().find("TEST_GATE_TYPE") != gl->get_gate_types().end());
            EXPECT_EQ(gl->get_gate_types().at("TEST_GATE_TYPE")->get_base_type(), gate_type::base_type::combinatorial);
            EXPECT_EQ(gl->get_gate_types().at("TEST_GATE_TYPE")->get_output_pins().size(), 1);


        }
        { // ISSUE: Interprets backslash as character of attribute name ("direction\")
            // Test usage of a backslash (\) to continue a statement over multiple lines
            std::stringstream input("library (TEST_GATE_LIBRARY) {\n"
                                    "    define(cell);\n"
                                    "    cell(TEST_GATE_TYPE) {\n"
                                    "        pin(I) {\n"
                                    "            direction: input;\n"
                                    "        }\n"
                                    "        pin(O) {\n"
                                    "            complex_attr(param1, param2);\n"
                                    "            direction\\\n"
                                    "            : output;\n"
                                    "            function: \"!I\";\n"
                                    "        }\n"
                                    "    }\n"
                                    "}");
            gate_library_parser_liberty liberty_parser(input);
            std::shared_ptr<gate_library> gl = liberty_parser.parse();

            ASSERT_NE(gl, nullptr);

            ASSERT_TRUE(gl->get_gate_types().find("TEST_GATE_TYPE") != gl->get_gate_types().end());
            EXPECT_EQ(gl->get_gate_types().at("TEST_GATE_TYPE")->get_base_type(), gate_type::base_type::combinatorial);
            //EXPECT_EQ(gl->get_gate_types().at("TEST_GATE_TYPE")->get_output_pins().size(), 1); // ISSUE: fails


        }
    TEST_END
}

