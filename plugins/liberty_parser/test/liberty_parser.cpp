#include "netlist_test_utils.h"
#include "liberty_parser/liberty_parser.h"
#include "hal_core/netlist/gate_library/gate_type/gate_type_lut.h"
#include <experimental/filesystem>

namespace hal {

    class LibertyParserTest : public ::testing::Test {
    protected:
        virtual void SetUp() {
            test_utils::init_log_channels();
        }

        virtual void TearDown() {
        }
    };

    /**
     * Testing the creation of a combinatorial Gate type with one input pin, one output pin and a boolean function.
     *
     * Functions: parse
     */
    TEST_F(LibertyParserTest, check_combinatorial) {
        TEST_START
            {
                std::string path_lib = utils::get_base_directory().string() + "/bin/hal_plugins/test-files/liberty_parser/test1.lib";
                LibertyParser liberty_parser;
                std::unique_ptr<GateLibrary> gl = liberty_parser.parse(path_lib);

                ASSERT_NE(gl, nullptr);

                // Check that the name of the Gate library
                EXPECT_EQ(gl->get_name(), "TEST_GATE_LIBRARY");

                // Check that the Gate type was created
                auto gate_types = gl->get_gate_types();
                ASSERT_EQ(gate_types.size(), 1);
                auto gt_it = gate_types.find("TEST_GATE_TYPE");
                ASSERT_TRUE(gt_it != gate_types.end());
                const GateType* gt = gt_it->second;

                // Check the content of the created Gate type
                EXPECT_EQ(gt->get_base_type(), GateType::BaseType::combinatorial);
                // -- Check the pins
                EXPECT_EQ(gt->get_input_pins(), std::vector<std::string>({"VDD", "GND", "I"}));
                EXPECT_EQ(gt->get_output_pins(), std::vector<std::string>({"O"}));
                EXPECT_EQ(gt->get_power_pins(), std::unordered_set<std::string>({"VDD"}));
                EXPECT_EQ(gt->get_ground_pins(), std::unordered_set<std::string>({"GND"}));
                // -- Check the boolean functions
                ASSERT_TRUE(gt->get_boolean_functions().find("O") != gt->get_boolean_functions().end());
                EXPECT_EQ(gt->get_boolean_functions().at("O"),
                          BooleanFunction::from_string("I", std::vector<std::string>({"I"})));
                ASSERT_TRUE(
                    gt->get_boolean_functions().find("O_undefined") != gt->get_boolean_functions().end()); // x_function
                EXPECT_EQ(gt->get_boolean_functions().at("O_undefined"),
                          BooleanFunction::from_string("!I", std::vector<std::string>({"I"})));
            }
        TEST_END
    }

    /**
     * Testing the creation of a LUT Gate type
     *
     * Functions: parse
     */
    TEST_F(LibertyParserTest, check_lut) {
        TEST_START
            {
                std::string path_lib = utils::get_base_directory().string() + "/bin/hal_plugins/test-files/liberty_parser/test2.lib";
                LibertyParser liberty_parser;
                std::unique_ptr<GateLibrary> gl = liberty_parser.parse(path_lib);

                ASSERT_NE(gl, nullptr);

                // Check that the ascending LUT gate type was created
                auto gate_types = gl->get_gate_types();
                ASSERT_EQ(gate_types.size(), 2);
                auto gt_it_asc = gate_types.find("TEST_LUT_ASC");
                ASSERT_TRUE(gt_it_asc != gate_types.end());
                const GateType* gt_asc = gt_it_asc->second;
                ASSERT_EQ(gt_asc->get_base_type(), GateType::BaseType::lut);
                const GateTypeLut* gt_lut_asc = dynamic_cast<const GateTypeLut*>(gt_asc);

                // Check the content of the created gate type
                EXPECT_EQ(gt_lut_asc->get_input_pins(), std::vector<std::string>({"I0", "I1"}));
                EXPECT_EQ(gt_lut_asc->get_output_pins(), std::vector<std::string>({"O0", "O1", "O2", "O3"}));
                ASSERT_TRUE(gt_lut_asc->get_boolean_functions().find("O1") != gt_lut_asc->get_boolean_functions().end());
                EXPECT_EQ(gt_lut_asc->get_boolean_functions().at("O1"),
                          BooleanFunction::from_string("I0 ^ I1", std::vector<std::string>({"I0", "I1"})));
                ASSERT_TRUE(gt_lut_asc->get_boolean_functions().find("O3") != gt_lut_asc->get_boolean_functions().end());
                EXPECT_EQ(gt_lut_asc->get_boolean_functions().at("O3"),
                          BooleanFunction::from_string("I0 & I1", std::vector<std::string>({"I0", "I1"})));
                EXPECT_EQ(gt_lut_asc->get_lut_pins(), std::unordered_set<std::string>({"O0", "O2"}));
                EXPECT_EQ(gt_lut_asc->get_config_data_category(), "test_category");
                EXPECT_EQ(gt_lut_asc->get_config_data_identifier(), "test_identifier");
                EXPECT_EQ(gt_lut_asc->is_config_data_ascending_order(), true);

                // Check that the descending LUT gate type was created
                auto gt_it_desc = gate_types.find("TEST_LUT_DESC");
                ASSERT_TRUE(gt_it_desc != gate_types.end());
                const GateType* gt_desc = gt_it_desc->second;
                ASSERT_EQ(gt_desc->get_base_type(), GateType::BaseType::lut);
                const GateTypeLut* gt_lut_desc = dynamic_cast<const GateTypeLut*>(gt_desc);

                // Check the content of the created gate type
                EXPECT_EQ(gt_lut_desc->is_config_data_ascending_order(), false);
            }
        TEST_END
    }

    /**
     * Testing the creation of flip-flops
     *
     * Functions: parse
     */
    TEST_F(LibertyParserTest, check_flip_flop) {
        TEST_START
            {
                std::string path_lib = utils::get_base_directory().string() + "/bin/hal_plugins/test-files/liberty_parser/test3.lib";
                LibertyParser liberty_parser;
                std::unique_ptr<GateLibrary> gl = liberty_parser.parse(path_lib);

                ASSERT_NE(gl, nullptr);

                // Check that the Gate type was created
                auto gate_types = gl->get_gate_types();
                ASSERT_EQ(gate_types.size(), 1);
                auto gt_it = gate_types.find("TEST_FF");
                ASSERT_TRUE(gt_it != gate_types.end());
                const GateType* gt = gt_it->second;
                ASSERT_EQ(gt->get_base_type(), GateType::BaseType::ff);
                const GateTypeSequential* gt_ff = dynamic_cast<const GateTypeSequential*>(gt);

                // Check the content of the created Gate type
                EXPECT_EQ(gt_ff->get_input_pins(), std::vector<std::string>({"CLK", "CE", "D", "R", "S"}));
                EXPECT_EQ(gt_ff->get_output_pins(), std::vector<std::string>({"Q", "QN", "O"}));
                EXPECT_EQ(gt_ff->get_clock_pins(), std::unordered_set<std::string>({"CLK"}));
                ASSERT_TRUE(gt_ff->get_boolean_functions().find("O") != gt_ff->get_boolean_functions().end());
                EXPECT_EQ(gt_ff->get_boolean_functions().at("O"),
                          BooleanFunction::from_string("S & R & D", std::vector<std::string>({"S", "R", "D"})));
                // -- Check the boolean functions of the ff group that are parsed (currently only next_state, clock_on(clock), preset(set) and clear(reset) are parsed )
                ASSERT_TRUE(gt_ff->get_boolean_functions().find("next_state") != gt_ff->get_boolean_functions().end());
                EXPECT_EQ(gt_ff->get_boolean_functions().at("next_state"),
                          BooleanFunction::from_string("D", std::vector<std::string>({"D"})));
                ASSERT_TRUE(gt_ff->get_boolean_functions().find("clock") != gt_ff->get_boolean_functions().end());
                EXPECT_EQ(gt_ff->get_boolean_functions().at("clock"),
                          BooleanFunction::from_string("CLK", std::vector<std::string>({"CLK"})));
                ASSERT_TRUE(gt_ff->get_boolean_functions().find("preset") != gt_ff->get_boolean_functions().end());
                EXPECT_EQ(gt_ff->get_boolean_functions().at("preset"),
                          BooleanFunction::from_string("S", std::vector<std::string>({"S"})));
                ASSERT_TRUE(gt_ff->get_boolean_functions().find("clear") != gt_ff->get_boolean_functions().end());
                EXPECT_EQ(gt_ff->get_boolean_functions().at("clear"),
                          BooleanFunction::from_string("R", std::vector<std::string>({"R"})));
                // -- Check the output pins
                EXPECT_EQ(gt_ff->get_state_pins(), std::unordered_set<std::string>({"Q"}));
                EXPECT_EQ(gt_ff->get_negated_state_pins(), std::unordered_set<std::string>({"QN"}));
                // -- Check the set-reset behaviour
                EXPECT_EQ(gt_ff->get_clear_preset_behavior(),
                          std::make_pair(GateTypeSequential::ClearPresetBehavior::L,
                                         GateTypeSequential::ClearPresetBehavior::H));
            }
        TEST_END
    }

    /**
     * Testing the creation of a latches
     *
     * Functions: parse
     */
    TEST_F(LibertyParserTest, check_latch) {
        TEST_START
            {
                std::string path_lib = utils::get_base_directory().string() + "/bin/hal_plugins/test-files/liberty_parser/test4.lib";
                LibertyParser liberty_parser;
                std::unique_ptr<GateLibrary> gl = liberty_parser.parse(path_lib);

                ASSERT_NE(gl, nullptr);

                // Check that the Gate type was created
                auto gate_types = gl->get_gate_types();
                ASSERT_EQ(gate_types.size(), 1);
                auto gt_it = gate_types.find("TEST_LATCH");
                ASSERT_TRUE(gt_it != gate_types.end());
                const GateType* gt = gt_it->second;
                ASSERT_EQ(gt->get_base_type(), GateType::BaseType::latch);
                const GateTypeSequential* gt_latch = dynamic_cast<const GateTypeSequential*>(gt);

                // Check the content of the created Gate type
                EXPECT_EQ(gt_latch->get_input_pins(), std::vector<std::string>({"G", "D", "S", "R"}));
                EXPECT_EQ(gt_latch->get_output_pins(), std::vector<std::string>({"Q", "QN", "O"}));
                ASSERT_TRUE(gt_latch->get_boolean_functions().find("O") != gt_latch->get_boolean_functions().end());
                EXPECT_EQ(gt_latch->get_boolean_functions().at("O"),
                          BooleanFunction::from_string("S & R & D", std::vector<std::string>({"S", "R", "D"})));
                // -- Check the boolean functions of the latch group that are parsed (currently only enable, data_in, preset(set) and clear(reset) are parsed)
                ASSERT_TRUE(
                    gt_latch->get_boolean_functions().find("enable") != gt_latch->get_boolean_functions().end());
                EXPECT_EQ(gt_latch->get_boolean_functions().at("enable"),
                          BooleanFunction::from_string("G", std::vector<std::string>({"G"})));
                ASSERT_TRUE(gt_latch->get_boolean_functions().find("data") != gt_latch->get_boolean_functions().end());
                EXPECT_EQ(gt_latch->get_boolean_functions().at("data"),
                          BooleanFunction::from_string("D", std::vector<std::string>({"D"})));
                ASSERT_TRUE(
                    gt_latch->get_boolean_functions().find("preset") != gt_latch->get_boolean_functions().end());
                EXPECT_EQ(gt_latch->get_boolean_functions().at("preset"),
                          BooleanFunction::from_string("S", std::vector<std::string>({"S"})));
                ASSERT_TRUE(gt_latch->get_boolean_functions().find("clear") != gt_latch->get_boolean_functions().end());
                EXPECT_EQ(gt_latch->get_boolean_functions().at("clear"),
                          BooleanFunction::from_string("R", std::vector<std::string>({"R"})));
                // -- Check the output pins
                EXPECT_EQ(gt_latch->get_state_pins(), std::unordered_set<std::string>({"Q"}));
                EXPECT_EQ(gt_latch->get_negated_state_pins(), std::unordered_set<std::string>({"QN"}));
                // -- Check the clear-preset behaviour
                EXPECT_EQ(gt_latch->get_clear_preset_behavior(),
                          std::make_pair(GateTypeSequential::ClearPresetBehavior::N,
                                         GateTypeSequential::ClearPresetBehavior::T));
            }
        TEST_END
    }

    /**
     * Testing the usage of multiline comments (via '/ *' and '* /'  (without space))
     *
     * Functions: parse
     */
    TEST_F(LibertyParserTest, check_multiline_comment) {
        TEST_START
            {
                std::string path_lib = utils::get_base_directory().string() + "/bin/hal_plugins/test-files/liberty_parser/test5.lib";
                LibertyParser liberty_parser;
                std::unique_ptr<GateLibrary> gl = liberty_parser.parse(path_lib);

                ASSERT_NE(gl, nullptr);
                // Check that the Gate type was created
                auto gate_types = gl->get_gate_types();
                ASSERT_EQ(gate_types.size(), 1);
                auto gt_it = gate_types.find("TEST_GATE_TYPE");
                ASSERT_TRUE(gt_it != gate_types.end());
                const GateType* gt = gt_it->second;

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
    TEST_F(LibertyParserTest, check_invalid_input) {
        TEST_START
            {
                // Pass an invalid input path
                NO_COUT_TEST_BLOCK;
                std::string path_lib = utils::get_base_directory().string() + "/bin/hal_plugins/test-files/liberty_parser/test_noexist.lib";
                LibertyParser liberty_parser;
                std::unique_ptr<GateLibrary> gl = liberty_parser.parse(path_lib);

                EXPECT_EQ(gl, nullptr);
            }
            {
                // For a ff, use an undefined clear_preset_var1 behaviour (not in {L,H,N,T,X})
                NO_COUT_TEST_BLOCK;
                std::string path_lib = utils::get_base_directory().string() + "/bin/hal_plugins/test-files/liberty_parser/invalid_test1.lib";
                LibertyParser liberty_parser;
                std::unique_ptr<GateLibrary> gl = liberty_parser.parse(path_lib);

                EXPECT_EQ(gl, nullptr);
            }
            {
                // For a ff, use an undefined clear_preset_var2 behaviour (not in {L,H,N,T,X})
                NO_COUT_TEST_BLOCK;
                std::string path_lib = utils::get_base_directory().string() + "/bin/hal_plugins/test-files/liberty_parser/invalid_test2.lib";
                LibertyParser liberty_parser;
                std::unique_ptr<GateLibrary> gl = liberty_parser.parse(path_lib);

                EXPECT_EQ(gl, nullptr);
            }
            {
                // For a latch, use an undefined clear_preset_var1 behaviour (not in {L,H,N,T,X})
                NO_COUT_TEST_BLOCK;
                std::string path_lib = utils::get_base_directory().string() + "/bin/hal_plugins/test-files/liberty_parser/invalid_test3.lib";
                LibertyParser liberty_parser;
                std::unique_ptr<GateLibrary> gl = liberty_parser.parse(path_lib);

                EXPECT_EQ(gl, nullptr);
            }
            {
                // For a latch, use an undefined clear_preset_var1 behaviour (not in {L,H,N,T,X})
                NO_COUT_TEST_BLOCK;
                std::string path_lib = utils::get_base_directory().string() + "/bin/hal_plugins/test-files/liberty_parser/invalid_test4.lib";
                LibertyParser liberty_parser;
                std::unique_ptr<GateLibrary> gl = liberty_parser.parse(path_lib);

                EXPECT_EQ(gl, nullptr);
            }
            {
                // Use an undefined direction in the lut group block (not in {ascending, descending})
                NO_COUT_TEST_BLOCK;
                std::string path_lib = utils::get_base_directory().string() + "/bin/hal_plugins/test-files/liberty_parser/invalid_test5.lib";
                LibertyParser liberty_parser;
                std::unique_ptr<GateLibrary> gl = liberty_parser.parse(path_lib);

                EXPECT_EQ(gl, nullptr);
            }
            {
                // Use a pin with an unknown direction (not in {input, output}) as an input pin
                NO_COUT_TEST_BLOCK;
                std::string path_lib = utils::get_base_directory().string() + "/bin/hal_plugins/test-files/liberty_parser/invalid_test6.lib";
                LibertyParser liberty_parser;
                std::unique_ptr<GateLibrary> gl = liberty_parser.parse(path_lib);

                ASSERT_NE(gl, nullptr); // NOTE: Ok, only 'I' is not parsed
                auto g_types = gl->get_gate_types();
                ASSERT_TRUE(g_types.find("TEST_GATE_TYPE") != g_types.end());
                EXPECT_EQ(g_types["TEST_GATE_TYPE"]->get_output_pins().size(), 1);
                EXPECT_TRUE(g_types["TEST_GATE_TYPE"]->get_input_pins().empty());
            }
            /*{ // NOTE: Works (is ok?)
                // Use an unknown variable in a boolean function
                NO_COUT_TEST_BLOCK;
                std::string path_lib = utils::get_base_directory().string() + "/bin/hal_plugins/test-files/liberty_parser/invalid_test7.lib";
                LibertyParser liberty_parser;
                std::unique_ptr<GateLibrary> gl = liberty_parser.parse(path_lib);

                EXPECT_EQ(gl, nullptr); // NOTE: Ok? BF is parsed anyway with Variable WAMBO
            }*/
            {
                // Use an unknown cell group (should be filtered out)
                NO_COUT_TEST_BLOCK;
                std::string path_lib = utils::get_base_directory().string() + "/bin/hal_plugins/test-files/liberty_parser/invalid_test8.lib";
                LibertyParser liberty_parser;
                std::unique_ptr<GateLibrary> gl = liberty_parser.parse(path_lib);

                ASSERT_NE(gl, nullptr);

                auto gate_types = gl->get_gate_types();
                ASSERT_TRUE(gate_types.find("TEST_GATE_TYPE") != gate_types.end());
                EXPECT_EQ(gate_types.at("TEST_GATE_TYPE")->get_base_type(),
                          GateType::BaseType::combinatorial);
            }
            {
                // Define a pin twice
                NO_COUT_TEST_BLOCK;
                std::string path_lib = utils::get_base_directory().string() + "/bin/hal_plugins/test-files/liberty_parser/invalid_test9.lib";
                LibertyParser liberty_parser;
                std::unique_ptr<GateLibrary> gl = liberty_parser.parse(path_lib);

                EXPECT_EQ(gl, nullptr);

            }
            {
                // Define a Gate type twice
                NO_COUT_TEST_BLOCK;
                std::string path_lib = utils::get_base_directory().string() + "/bin/hal_plugins/test-files/liberty_parser/invalid_test10.lib";
                LibertyParser liberty_parser;
                std::unique_ptr<GateLibrary> gl = liberty_parser.parse(path_lib);

                EXPECT_EQ(gl, nullptr);
            }
            {
                // Pin got no direction (should be ignored)
                NO_COUT_TEST_BLOCK;
                std::string path_lib = utils::get_base_directory().string() + "/bin/hal_plugins/test-files/liberty_parser/invalid_test11.lib";
                LibertyParser liberty_parser;
                std::unique_ptr<GateLibrary> gl = liberty_parser.parse(path_lib);

                ASSERT_NE(gl, nullptr);

                auto gate_types = gl->get_gate_types();
                ASSERT_TRUE(gate_types.find("TEST_GATE_TYPE") != gate_types.end());
                EXPECT_EQ(gate_types.at("TEST_GATE_TYPE")->get_base_type(),
                          GateType::BaseType::combinatorial);
                EXPECT_EQ(gate_types.at("TEST_GATE_TYPE")->get_input_pins().size(), 1);

            }
            {
                // Test the usage of complex attributes
                std::string path_lib = utils::get_base_directory().string() + "/bin/hal_plugins/test-files/liberty_parser/invalid_test12.lib";
                LibertyParser liberty_parser;
                std::unique_ptr<GateLibrary> gl = liberty_parser.parse(path_lib);

                ASSERT_NE(gl, nullptr);

                auto gate_types = gl->get_gate_types();
                ASSERT_TRUE(gate_types.find("TEST_GATE_TYPE") != gate_types.end());
                EXPECT_EQ(gate_types.at("TEST_GATE_TYPE")->get_base_type(),
                          GateType::BaseType::combinatorial);
                EXPECT_EQ(gate_types.at("TEST_GATE_TYPE")->get_output_pins().size(), 1);

            }
            {
                // Test empty pin names
                std::string path_lib = utils::get_base_directory().string() + "/bin/hal_plugins/test-files/liberty_parser/invalid_test13.lib";
                LibertyParser liberty_parser;
                std::unique_ptr<GateLibrary> gl = liberty_parser.parse(path_lib);

                ASSERT_EQ(gl, nullptr);
            }
        TEST_END
    }

} //namespace hal
