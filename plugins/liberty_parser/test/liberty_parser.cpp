#include "netlist_test_utils.h"
#include "liberty_parser/liberty_parser.h"
#include "hal_core/netlist/gate_library/gate_type.h"
#include "hal_core/netlist/gate_library/gate_type_component/ff_component.h"
#include "hal_core/netlist/gate_library/gate_type_component/latch_component.h"
#include "hal_core/netlist/gate_library/gate_type_component/state_component.h"
#include <filesystem>

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
                auto gl_res = liberty_parser.parse(path_lib);
                ASSERT_TRUE(gl_res.is_ok());
                std::unique_ptr<GateLibrary> gl = gl_res.get();

                ASSERT_NE(gl, nullptr);

                // Check that the name of the Gate library
                EXPECT_EQ(gl->get_name(), "TEST_GATE_LIBRARY");

                // Check that the Gate type was created
                auto gate_types = gl->get_gate_types();
                ASSERT_EQ(gate_types.size(), 1);
                auto gt_it = gate_types.find("TEST_GATE_TYPE");
                ASSERT_TRUE(gt_it != gate_types.end());
                GateType* gt = gt_it->second;
                EXPECT_EQ(gt->get_properties(), std::set<GateTypeProperty>({GateTypeProperty::combinational}));

                // Check the pins
                const auto pins = gt->get_pins();
                ASSERT_EQ(pins.size(), 4);
                {
                    const auto* vdd_pin = pins.at(0);
                    EXPECT_EQ(vdd_pin->get_id(), 1);
                    EXPECT_EQ(vdd_pin->get_name(), "VDD");
                    EXPECT_EQ(vdd_pin->get_direction(), PinDirection::input);
                    EXPECT_EQ(vdd_pin->get_type(), PinType::power);
                }
                {
                    const auto* gnd_pin = pins.at(1);
                    EXPECT_EQ(gnd_pin->get_id(), 2);
                    EXPECT_EQ(gnd_pin->get_name(), "GND");
                    EXPECT_EQ(gnd_pin->get_direction(), PinDirection::input);
                    EXPECT_EQ(gnd_pin->get_type(), PinType::ground);
                }
                {
                    const auto* i_pin = pins.at(2);
                    EXPECT_EQ(i_pin->get_id(), 3);
                    EXPECT_EQ(i_pin->get_name(), "I");
                    EXPECT_EQ(i_pin->get_direction(), PinDirection::input);
                    EXPECT_EQ(i_pin->get_type(), PinType::none);
                }
                {
                    const auto* o_pin = pins.at(3);
                    EXPECT_EQ(o_pin->get_id(), 4);
                    EXPECT_EQ(o_pin->get_name(), "O");
                    EXPECT_EQ(o_pin->get_direction(), PinDirection::output);
                    EXPECT_EQ(o_pin->get_type(), PinType::none);
                }

                // Check the Boolean functions
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
                auto gl_res = liberty_parser.parse(path_lib);
                ASSERT_TRUE(gl_res.is_ok());
                std::unique_ptr<GateLibrary> gl = gl_res.get();

                ASSERT_NE(gl, nullptr);

                // Check that the Gate type was created
                auto gate_types = gl->get_gate_types();
                ASSERT_EQ(gate_types.size(), 1);
                auto gt_it = gate_types.find("TEST_FF");
                ASSERT_TRUE(gt_it != gate_types.end());
                GateType* gt = gt_it->second;
                EXPECT_EQ(gt->get_properties(), std::set<GateTypeProperty>({GateTypeProperty::sequential, GateTypeProperty::ff}));

                // Check the pins
                const auto pins = gt->get_pins();
                ASSERT_EQ(pins.size(), 8);
                {
                    const auto* clk_pin = pins.at(0);
                    EXPECT_EQ(clk_pin->get_id(), 1);
                    EXPECT_EQ(clk_pin->get_name(), "CLK");
                    EXPECT_EQ(clk_pin->get_direction(), PinDirection::input);
                    EXPECT_EQ(clk_pin->get_type(), PinType::clock);
                }
                {
                    const auto* ce_pin = pins.at(1);
                    EXPECT_EQ(ce_pin->get_id(), 2);
                    EXPECT_EQ(ce_pin->get_name(), "CE");
                    EXPECT_EQ(ce_pin->get_direction(), PinDirection::input);
                }
                {
                    const auto* d_pin = pins.at(2);
                    EXPECT_EQ(d_pin->get_id(), 3);
                    EXPECT_EQ(d_pin->get_name(), "D");
                    EXPECT_EQ(d_pin->get_direction(), PinDirection::input);
                }
                {
                    const auto* r_pin = pins.at(3);
                    EXPECT_EQ(r_pin->get_id(), 4);
                    EXPECT_EQ(r_pin->get_name(), "R");
                    EXPECT_EQ(r_pin->get_direction(), PinDirection::input);
                }
                {
                    const auto* s_pin = pins.at(4);
                    EXPECT_EQ(s_pin->get_id(), 5);
                    EXPECT_EQ(s_pin->get_name(), "S");
                    EXPECT_EQ(s_pin->get_direction(), PinDirection::input);
                }
                {
                    const auto* q_pin = pins.at(5);
                    EXPECT_EQ(q_pin->get_id(), 6);
                    EXPECT_EQ(q_pin->get_name(), "Q");
                    EXPECT_EQ(q_pin->get_direction(), PinDirection::output);
                    EXPECT_EQ(q_pin->get_type(), PinType::state);
                }
                {
                    const auto* qn_pin = pins.at(6);
                    EXPECT_EQ(qn_pin->get_id(), 7);
                    EXPECT_EQ(qn_pin->get_name(), "QN");
                    EXPECT_EQ(qn_pin->get_direction(), PinDirection::output);
                    EXPECT_EQ(qn_pin->get_type(), PinType::neg_state);
                }
                {
                    const auto* o_pin = pins.at(7);
                    EXPECT_EQ(o_pin->get_id(), 8);
                    EXPECT_EQ(o_pin->get_name(), "O");
                    EXPECT_EQ(o_pin->get_direction(), PinDirection::output);
                }

                // Check the Boolean functions
                ASSERT_TRUE(gt->get_boolean_functions().find("Q") != gt->get_boolean_functions().end());
                EXPECT_EQ(gt->get_boolean_functions().at("Q"), BooleanFunction::from_string("IQ").get());
                ASSERT_TRUE(gt->get_boolean_functions().find("QN") != gt->get_boolean_functions().end());
                EXPECT_EQ(gt->get_boolean_functions().at("QN"), BooleanFunction::from_string("IQN").get());
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
                auto gl_res = liberty_parser.parse(path_lib);
                ASSERT_TRUE(gl_res.is_ok());
                std::unique_ptr<GateLibrary> gl = gl_res.get();

                ASSERT_NE(gl, nullptr);

                // Check that the Gate type was created
                auto gate_types = gl->get_gate_types();
                ASSERT_EQ(gate_types.size(), 1);
                auto gt_it = gate_types.find("TEST_LATCH");
                ASSERT_TRUE(gt_it != gate_types.end());
                GateType* gt = gt_it->second;
                EXPECT_EQ(gt->get_properties(), std::set<GateTypeProperty>({GateTypeProperty::sequential, GateTypeProperty::latch}));

                // Check the pins
                const auto pins = gt->get_pins();
                ASSERT_EQ(pins.size(), 7);
                {
                    const auto* g_pin = pins.at(0);
                    EXPECT_EQ(g_pin->get_id(), 1);
                    EXPECT_EQ(g_pin->get_name(), "G");
                    EXPECT_EQ(g_pin->get_direction(), PinDirection::input);
                }
                {
                    const auto* d_pin = pins.at(1);
                    EXPECT_EQ(d_pin->get_id(), 2);
                    EXPECT_EQ(d_pin->get_name(), "D");
                    EXPECT_EQ(d_pin->get_direction(), PinDirection::input);
                }
                {
                    const auto* s_pin = pins.at(2);
                    EXPECT_EQ(s_pin->get_id(), 3);
                    EXPECT_EQ(s_pin->get_name(), "S");
                    EXPECT_EQ(s_pin->get_direction(), PinDirection::input);
                }
                {
                    const auto* r_pin = pins.at(3);
                    EXPECT_EQ(r_pin->get_id(), 4);
                    EXPECT_EQ(r_pin->get_name(), "R");
                    EXPECT_EQ(r_pin->get_direction(), PinDirection::input);
                }
                {
                    const auto* q_pin = pins.at(4);
                    EXPECT_EQ(q_pin->get_id(), 5);
                    EXPECT_EQ(q_pin->get_name(), "Q");
                    EXPECT_EQ(q_pin->get_direction(), PinDirection::output);
                    EXPECT_EQ(q_pin->get_type(), PinType::state);
                }
                {
                    const auto* qn_pin = pins.at(5);
                    EXPECT_EQ(qn_pin->get_id(), 6);
                    EXPECT_EQ(qn_pin->get_name(), "QN");
                    EXPECT_EQ(qn_pin->get_direction(), PinDirection::output);
                    EXPECT_EQ(qn_pin->get_type(), PinType::neg_state);
                }
                {
                    const auto* o_pin = pins.at(6);
                    EXPECT_EQ(o_pin->get_id(), 7);
                    EXPECT_EQ(o_pin->get_name(), "O");
                    EXPECT_EQ(o_pin->get_direction(), PinDirection::output);
                }

                // Check the Boolean functions
                ASSERT_TRUE(gt->get_boolean_functions().find("Q") != gt->get_boolean_functions().end());
                EXPECT_EQ(gt->get_boolean_functions().at("Q"), BooleanFunction::from_string("IQ").get());
                ASSERT_TRUE(gt->get_boolean_functions().find("QN") != gt->get_boolean_functions().end());
                EXPECT_EQ(gt->get_boolean_functions().at("QN"), BooleanFunction::from_string("IQN").get());
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
                auto gl_res = liberty_parser.parse(path_lib);
                ASSERT_TRUE(gl_res.is_ok());
                std::unique_ptr<GateLibrary> gl = gl_res.get();

                ASSERT_NE(gl, nullptr);
                // Check that the Gate type was created
                auto gate_types = gl->get_gate_types();
                ASSERT_EQ(gate_types.size(), 1);
                auto gt_it = gate_types.find("TEST_GATE_TYPE");
                ASSERT_TRUE(gt_it != gate_types.end());
                GateType* gt = gt_it->second;

                // Check that only the pins outside the comments are created
                const auto pins = gt->get_pins();
                ASSERT_EQ(pins.size(), 5);
                {
                    const auto* o0_pin = pins.at(0);
                    EXPECT_EQ(o0_pin->get_id(), 1);
                    EXPECT_EQ(o0_pin->get_name(), "I");
                    EXPECT_EQ(o0_pin->get_direction(), PinDirection::input);
                }
                {
                    const auto* o0_pin = pins.at(1);
                    EXPECT_EQ(o0_pin->get_id(), 2);
                    EXPECT_EQ(o0_pin->get_name(), "O0");
                    EXPECT_EQ(o0_pin->get_direction(), PinDirection::output);
                }
                {
                    const auto* o1_pin = pins.at(2);
                    EXPECT_EQ(o1_pin->get_id(), 3);
                    EXPECT_EQ(o1_pin->get_name(), "O1");
                    EXPECT_EQ(o1_pin->get_direction(), PinDirection::output);
                }
                {
                    const auto* o2_pin = pins.at(3);
                    EXPECT_EQ(o2_pin->get_id(), 4);
                    EXPECT_EQ(o2_pin->get_name(), "O2");
                    EXPECT_EQ(o2_pin->get_direction(), PinDirection::output);
                }
                {
                    const auto* g_pin = pins.at(4);
                    EXPECT_EQ(g_pin->get_id(), 5);
                    EXPECT_EQ(g_pin->get_name(), "O3");
                    EXPECT_EQ(g_pin->get_direction(), PinDirection::output);
                }
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
                auto gl_res = liberty_parser.parse(path_lib);
                ASSERT_TRUE(gl_res.is_error());
            }
            {
                // For a ff, use an undefined clear_preset_var1 behaviour (not in {L,H,N,T,X})
                NO_COUT_TEST_BLOCK;
                std::string path_lib = utils::get_base_directory().string() + "/bin/hal_plugins/test-files/liberty_parser/invalid_test1.lib";
                LibertyParser liberty_parser;
                auto gl_res = liberty_parser.parse(path_lib);
                ASSERT_TRUE(gl_res.is_error());
            }
            {
                // For a ff, use an undefined clear_preset_var2 behaviour (not in {L,H,N,T,X})
                NO_COUT_TEST_BLOCK;
                std::string path_lib = utils::get_base_directory().string() + "/bin/hal_plugins/test-files/liberty_parser/invalid_test2.lib";
                LibertyParser liberty_parser;
                auto gl_res = liberty_parser.parse(path_lib);
                ASSERT_TRUE(gl_res.is_error());
            }
            {
                // For a latch, use an undefined clear_preset_var1 behaviour (not in {L,H,N,T,X})
                NO_COUT_TEST_BLOCK;
                std::string path_lib = utils::get_base_directory().string() + "/bin/hal_plugins/test-files/liberty_parser/invalid_test3.lib";
                LibertyParser liberty_parser;
                auto gl_res = liberty_parser.parse(path_lib);
                ASSERT_TRUE(gl_res.is_error());
            }
            {
                // For a latch, use an undefined clear_preset_var1 behaviour (not in {L,H,N,T,X})
                NO_COUT_TEST_BLOCK;
                std::string path_lib = utils::get_base_directory().string() + "/bin/hal_plugins/test-files/liberty_parser/invalid_test4.lib";
                LibertyParser liberty_parser;
                auto gl_res = liberty_parser.parse(path_lib);
                ASSERT_TRUE(gl_res.is_error());
            }
            /*
            {
                // Use a pin with an unknown direction (not in {input, output}) as an input pin
                NO_COUT_TEST_BLOCK;
                std::string path_lib = utils::get_base_directory().string() + "/bin/hal_plugins/test-files/liberty_parser/invalid_test6.lib";
                LibertyParser liberty_parser;
                auto gl_res = liberty_parser.parse(path_lib);
                ASSERT_TRUE(gl_res.is_error());
            }
            */
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
                auto gl_res = liberty_parser.parse(path_lib);
                ASSERT_TRUE(gl_res.is_ok());
                std::unique_ptr<GateLibrary> gl = gl_res.get();

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
                auto gl_res = liberty_parser.parse(path_lib);
                ASSERT_TRUE(gl_res.is_error());

            }
            {
                // Define a Gate type twice
                NO_COUT_TEST_BLOCK;
                std::string path_lib = utils::get_base_directory().string() + "/bin/hal_plugins/test-files/liberty_parser/invalid_test10.lib";
                LibertyParser liberty_parser;
                auto gl_res = liberty_parser.parse(path_lib);
                ASSERT_TRUE(gl_res.is_error());
            }
            {
                // Pin got no direction
                NO_COUT_TEST_BLOCK;
                std::string path_lib = utils::get_base_directory().string() + "/bin/hal_plugins/test-files/liberty_parser/invalid_test11.lib";
                LibertyParser liberty_parser;
                auto gl_res = liberty_parser.parse(path_lib);
                ASSERT_TRUE(gl_res.is_error());
            }
            {
                // Test the usage of complex attributes
                std::string path_lib = utils::get_base_directory().string() + "/bin/hal_plugins/test-files/liberty_parser/invalid_test12.lib";
                LibertyParser liberty_parser;
                auto gl_res = liberty_parser.parse(path_lib);
                ASSERT_TRUE(gl_res.is_ok());
                std::unique_ptr<GateLibrary> gl = gl_res.get();

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
                auto gl_res = liberty_parser.parse(path_lib);
                ASSERT_TRUE(gl_res.is_error());
            }
        TEST_END
    }

} //namespace hal
