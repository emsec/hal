#include "netlist_test_utils.h"
#include "liberty_parser/liberty_parser.h"
#include "hal_core/netlist/gate_library/gate_type.h"
#include "hal_core/netlist/gate_library/gate_type_component/ff_component.h"
#include "hal_core/netlist/gate_library/gate_type_component/latch_component.h"
#include "hal_core/netlist/gate_library/gate_type_component/state_component.h"
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
                GateType* gt = gt_it->second;

                // Check the content of the created Gate type
                EXPECT_EQ(gt->get_properties(), std::set<GateTypeProperty>({GateTypeProperty::combinational}));
                // -- Check the pins
                EXPECT_EQ(gt->get_input_pins(), std::vector<std::string>({"VDD", "GND", "I"}));
                EXPECT_EQ(gt->get_output_pins(), std::vector<std::string>({"O"}));
                EXPECT_EQ(gt->get_pins_of_type(PinType::power), std::unordered_set<std::string>({"VDD"}));
                EXPECT_EQ(gt->get_pins_of_type(PinType::ground), std::unordered_set<std::string>({"GND"}));
                // -- Check the boolean functions
                ASSERT_TRUE(gt->get_boolean_functions().find("O") != gt->get_boolean_functions().end());
                EXPECT_EQ(gt->get_boolean_functions().at("O"), BooleanFunction::Var("I"));
                ASSERT_TRUE(gt->get_boolean_functions().find("O_undefined") != gt->get_boolean_functions().end()); // x_function
                EXPECT_EQ(gt->get_boolean_functions().at("O_undefined"), BooleanFunction::from_string("!I").get());

                // Components
                ASSERT_EQ(gt->get_components().size(), 0);
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
                GateType* gt = gt_it->second;
                EXPECT_EQ(gt->get_properties(), std::set<GateTypeProperty>({GateTypeProperty::sequential, GateTypeProperty::ff}));

                // Check the content of the created Gate type
                EXPECT_EQ(gt->get_input_pins(), std::vector<std::string>({"CLK", "CE", "D", "R", "S"}));
                EXPECT_EQ(gt->get_output_pins(), std::vector<std::string>({"Q", "QN", "O"}));
                EXPECT_EQ(gt->get_pins_of_type(PinType::clock), std::unordered_set<std::string>({"CLK"}));
                EXPECT_EQ(gt->get_pins_of_type(PinType::state), std::unordered_set<std::string>({"Q"}));
                EXPECT_EQ(gt->get_pins_of_type(PinType::neg_state), std::unordered_set<std::string>({"QN"}));
                ASSERT_TRUE(gt->get_boolean_functions().find("O") != gt->get_boolean_functions().end());
                EXPECT_EQ(gt->get_boolean_functions().at("O"), BooleanFunction::from_string("S & R & D").get());
                
                // Components
                ASSERT_EQ(gt->get_components().size(), 2);
                const FFComponent* ff_component = gt->get_component_as<FFComponent>([](const GateTypeComponent* c){ return FFComponent::is_class_of(c); });
                ASSERT_NE(ff_component, nullptr);
                const StateComponent* state_component = gt->get_component_as<StateComponent>([](const GateTypeComponent* c){ return StateComponent::is_class_of(c); });
                ASSERT_NE(state_component, nullptr);

                EXPECT_EQ(ff_component->get_next_state_function(), BooleanFunction::Var("D"));
                EXPECT_EQ(ff_component->get_clock_function(), BooleanFunction::Var("CLK"));
                EXPECT_EQ(ff_component->get_async_reset_function(), BooleanFunction::Var("R"));
                EXPECT_EQ(ff_component->get_async_set_function(), BooleanFunction::Var("S"));
                EXPECT_EQ(ff_component->get_async_set_reset_behavior(), std::make_pair(AsyncSetResetBehavior::L, AsyncSetResetBehavior::H));
                EXPECT_EQ(state_component->get_state_identifier(), "IQ");
                EXPECT_EQ(state_component->get_neg_state_identifier(), "IQN");
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
                GateType* gt = gt_it->second;
                EXPECT_EQ(gt->get_properties(), std::set<GateTypeProperty>({GateTypeProperty::sequential, GateTypeProperty::latch}));

                // Check the content of the created Gate type
                EXPECT_EQ(gt->get_input_pins(), std::vector<std::string>({"G", "D", "S", "R"}));
                EXPECT_EQ(gt->get_output_pins(), std::vector<std::string>({"Q", "QN", "O"}));
                EXPECT_EQ(gt->get_pins_of_type(PinType::state), std::unordered_set<std::string>({"Q"}));
                EXPECT_EQ(gt->get_pins_of_type(PinType::neg_state), std::unordered_set<std::string>({"QN"}));
                ASSERT_TRUE(gt->get_boolean_functions().find("O") != gt->get_boolean_functions().end());
                EXPECT_EQ(gt->get_boolean_functions().at("O"), BooleanFunction::from_string("S & R & D").get());

                // Components
                ASSERT_EQ(gt->get_components().size(), 2);
                const LatchComponent* latch_component = gt->get_component_as<LatchComponent>([](const GateTypeComponent* c){ return LatchComponent::is_class_of(c); });
                ASSERT_NE(latch_component, nullptr);
                const StateComponent* state_component = gt->get_component_as<StateComponent>([](const GateTypeComponent* c){ return StateComponent::is_class_of(c); });
                ASSERT_NE(state_component, nullptr);

                EXPECT_EQ(latch_component->get_data_in_function(), BooleanFunction::Var("D"));
                EXPECT_EQ(latch_component->get_enable_function(), BooleanFunction::Var("G"));
                EXPECT_EQ(latch_component->get_async_reset_function(), BooleanFunction::Var("R"));
                EXPECT_EQ(latch_component->get_async_set_function(), BooleanFunction::Var("S"));
                EXPECT_EQ(latch_component->get_async_set_reset_behavior(), std::make_pair(AsyncSetResetBehavior::N, AsyncSetResetBehavior::T));
                EXPECT_EQ(state_component->get_state_identifier(), "IQ");
                EXPECT_EQ(state_component->get_neg_state_identifier(), "IQN");
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
                GateType* gt = gt_it->second;

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
                // Use a pin with an unknown direction (not in {input, output}) as an input pin
                NO_COUT_TEST_BLOCK;
                std::string path_lib = utils::get_base_directory().string() + "/bin/hal_plugins/test-files/liberty_parser/invalid_test6.lib";
                LibertyParser liberty_parser;
                std::unique_ptr<GateLibrary> gl = liberty_parser.parse(path_lib);

                ASSERT_EQ(gl, nullptr);
            }
            // { // NOTE: Works (is ok?)
            //     // Use an unknown variable in a boolean function
            //     NO_COUT_TEST_BLOCK;
            //     std::string path_lib = utils::get_base_directory().string() + "/bin/hal_plugins/test-files/liberty_parser/invalid_test7.lib";
            //     LibertyParser liberty_parser;
            //     std::unique_ptr<GateLibrary> gl = liberty_parser.parse(path_lib);

            //     EXPECT_EQ(gl, nullptr); // NOTE: Ok? BF is parsed anyway with Variable WAMBO
            // }
            {
                // Use an unknown cell group (should be filtered out)
                NO_COUT_TEST_BLOCK;
                std::string path_lib = utils::get_base_directory().string() + "/bin/hal_plugins/test-files/liberty_parser/invalid_test8.lib";
                LibertyParser liberty_parser;
                std::unique_ptr<GateLibrary> gl = liberty_parser.parse(path_lib);

                ASSERT_NE(gl, nullptr);

                auto gate_types = gl->get_gate_types();
                ASSERT_TRUE(gate_types.find("TEST_GATE_TYPE") != gate_types.end());
                EXPECT_EQ(gate_types.at("TEST_GATE_TYPE")->get_properties(), std::set<GateTypeProperty>({GateTypeProperty::combinational}));
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

                ASSERT_EQ(gl, nullptr);
            }
            {
                // Test the usage of complex attributes
                std::string path_lib = utils::get_base_directory().string() + "/bin/hal_plugins/test-files/liberty_parser/invalid_test12.lib";
                LibertyParser liberty_parser;
                std::unique_ptr<GateLibrary> gl = liberty_parser.parse(path_lib);

                ASSERT_NE(gl, nullptr);

                auto gate_types = gl->get_gate_types();
                ASSERT_TRUE(gate_types.find("TEST_GATE_TYPE") != gate_types.end());
                EXPECT_EQ(gate_types.at("TEST_GATE_TYPE")->get_properties(), std::set<GateTypeProperty>({GateTypeProperty::combinational}));
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
