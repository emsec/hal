#include "hgl_parser/hgl_parser.h"

#include "netlist_test_utils.h"

namespace hal
{
    class HGLParserTest : public ::testing::Test
    {
    protected:
        virtual void SetUp()
        {
            test_utils::init_log_channels();
        }

        virtual void TearDown()
        {
        }
    };

    /**
     * Testing parsing an HGL file.
     *
     * Functions: parse
     */
    TEST_F(HGLParserTest, check_library) 
    {
        TEST_START
            {
                std::string path_lib = utils::get_base_directory().string() + "/bin/hal_plugins/test-files/hgl_parser/test.hgl";
                HGLParser parser;
                std::unique_ptr<GateLibrary> gl = parser.parse(path_lib);

                // parsing succeeds
                ASSERT_NE(gl, nullptr);

                // gate library name
                EXPECT_EQ(gl->get_name(), "example_library");

                // gate types
                std::unordered_map<std::string, GateType*> gate_types = gl->get_gate_types();
                ASSERT_EQ(gate_types.size(), 7);

                // combinational gate type
                {
                    GateType* gt = gl->get_gate_type_by_name("gt_combinational");
                    ASSERT_NE(gt, nullptr);

                    // general stuff
                    EXPECT_EQ(gt->get_id(), 1);
                    EXPECT_EQ(gt->get_name(), "gt_combinational");
                    EXPECT_EQ(gt->get_base_type(), GateType::BaseType::combinational);

                    // pins
                    EXPECT_EQ(gt->get_input_pins(), std::vector<std::string>({"VDD", "GND", "A", "B"}));
                    EXPECT_EQ(gt->get_output_pins(), std::vector<std::string>({"B", "O"}));

                    // pin types
                    EXPECT_EQ(gt->get_pin_type("VDD"), GateType::PinType::power);
                    EXPECT_EQ(gt->get_pin_type("GND"), GateType::PinType::ground);
                    EXPECT_EQ(gt->get_pin_type("A"), GateType::PinType::none);
                    EXPECT_EQ(gt->get_pin_type("B"), GateType::PinType::none);
                    EXPECT_EQ(gt->get_pin_type("O"), GateType::PinType::none);

                    // pin groups
                    EXPECT_TRUE(gt->get_input_pin_groups().empty());
                    EXPECT_TRUE(gt->get_output_pin_groups().empty());

                    // Boolean functions
                    std::unordered_map<std::string, BooleanFunction> functions = gt->get_boolean_functions();
                    ASSERT_FALSE(functions.empty());
                    ASSERT_NE(functions.find("O"), functions.end());
                    ASSERT_EQ(functions.at("O"), BooleanFunction::from_string("A & B"));
                    ASSERT_NE(functions.find("O_undefined"), functions.end());
                    ASSERT_EQ(functions.at("O_undefined"), BooleanFunction::from_string("!A & B"));
                    ASSERT_NE(functions.find("O_tristate"), functions.end());
                    ASSERT_EQ(functions.at("O_tristate"), BooleanFunction::from_string("!A & !B"));

                    // clear-preset behavior
                    ASSERT_EQ(gt->get_clear_preset_behavior(), std::pair(GateType::ClearPresetBehavior::U, GateType::ClearPresetBehavior::U));

                    // config data and init string
                    ASSERT_EQ(gt->get_config_data_category(), "");
                    ASSERT_EQ(gt->get_config_data_identifier(), "");
                    ASSERT_EQ(gt->is_lut_init_ascending(), true);
                }

                // combinational group gate type
                {
                    GateType* gt = gl->get_gate_type_by_name("gt_group");
                    ASSERT_NE(gt, nullptr);

                    // general stuff
                    EXPECT_EQ(gt->get_id(), 2);
                    EXPECT_EQ(gt->get_name(), "gt_group");
                    EXPECT_EQ(gt->get_base_type(), GateType::BaseType::combinational);

                    // pins
                    EXPECT_EQ(gt->get_input_pins(), std::vector<std::string>({"VDD", "GND", "A(0)", "A(1)", "B(0)", "B(1)"}));
                    EXPECT_EQ(gt->get_output_pins(), std::vector<std::string>({"B(0)", "B(1)", "C(0)", "C(1)", "O"}));

                    // pin types
                    EXPECT_EQ(gt->get_pin_type("VDD"), GateType::PinType::power);
                    EXPECT_EQ(gt->get_pin_type("GND"), GateType::PinType::ground);
                    EXPECT_EQ(gt->get_pin_type("A(0)"), GateType::PinType::none);
                    EXPECT_EQ(gt->get_pin_type("A(1)"), GateType::PinType::none);
                    EXPECT_EQ(gt->get_pin_type("B(0)"), GateType::PinType::none);
                    EXPECT_EQ(gt->get_pin_type("B(1)"), GateType::PinType::none);
                    EXPECT_EQ(gt->get_pin_type("C(0)"), GateType::PinType::none);
                    EXPECT_EQ(gt->get_pin_type("C(1)"), GateType::PinType::none);
                    EXPECT_EQ(gt->get_pin_type("O"), GateType::PinType::none);

                    // pin groups
                    std::unordered_map<std::string, std::map<u32, std::string>> expected_in_group({{"A", {{0, "A(0)"}, {1, "A(1)"}}}, {"B", {{0, "B(0)"}, {1, "B(1)"}}}});
                    std::unordered_map<std::string, std::map<u32, std::string>> expected_out_group({{"B", {{0, "B(0)"}, {1, "B(1)"}}}, {"C", {{0, "C(0)"}, {1, "C(1)"}}}});
                    EXPECT_EQ(gt->get_input_pin_groups(), expected_in_group);
                    EXPECT_EQ(gt->get_output_pin_groups(), expected_out_group);

                    // Boolean functions
                    std::unordered_map<std::string, BooleanFunction> functions = gt->get_boolean_functions();
                    ASSERT_FALSE(functions.empty());
                    ASSERT_NE(functions.find("O"), functions.end());
                    ASSERT_EQ(functions.at("O"), BooleanFunction::from_string("A(1) & B(0)", std::vector<std::string>({"A(1)", "B(0)"})));
                    ASSERT_NE(functions.find("O_undefined"), functions.end());
                    ASSERT_EQ(functions.at("O_undefined"), BooleanFunction::from_string("!A(0) & B(0)", std::vector<std::string>({"A(0)", "B(0)"})));
                    ASSERT_NE(functions.find("O_tristate"), functions.end());
                    ASSERT_EQ(functions.at("O_tristate"), BooleanFunction::from_string("!A(1) & !B(1)", std::vector<std::string>({"A(1)", "B(1)"})));

                    // clear-preset behavior
                    ASSERT_EQ(gt->get_clear_preset_behavior(), std::pair(GateType::ClearPresetBehavior::U, GateType::ClearPresetBehavior::U));

                    // config data and init string
                    ASSERT_EQ(gt->get_config_data_category(), "");
                    ASSERT_EQ(gt->get_config_data_identifier(), "");
                    ASSERT_EQ(gt->is_lut_init_ascending(), true);
                }

                // ascending LUT gate type
                {
                    GateType* gt = gl->get_gate_type_by_name("gt_lut_asc");
                    ASSERT_NE(gt, nullptr);

                    // general stuff
                    EXPECT_EQ(gt->get_id(), 3);
                    EXPECT_EQ(gt->get_name(), "gt_lut_asc");
                    EXPECT_EQ(gt->get_base_type(), GateType::BaseType::lut);

                    // pins
                    EXPECT_EQ(gt->get_input_pins(), std::vector<std::string>({"I1", "I2"}));
                    EXPECT_EQ(gt->get_output_pins(), std::vector<std::string>({"O"}));

                    // pin types
                    EXPECT_EQ(gt->get_pin_type("I1"), GateType::PinType::none);
                    EXPECT_EQ(gt->get_pin_type("I2"), GateType::PinType::none);
                    EXPECT_EQ(gt->get_pin_type("O"), GateType::PinType::lut);

                    // pin groups
                    EXPECT_TRUE(gt->get_input_pin_groups().empty());
                    EXPECT_TRUE(gt->get_output_pin_groups().empty());

                    // Boolean functions
                    std::unordered_map<std::string, BooleanFunction> functions = gt->get_boolean_functions();
                    ASSERT_TRUE(functions.empty());

                    // clear-preset behavior
                    ASSERT_EQ(gt->get_clear_preset_behavior(), std::pair(GateType::ClearPresetBehavior::U, GateType::ClearPresetBehavior::U));

                    // config data and init string
                    ASSERT_EQ(gt->get_config_data_category(), "generic");
                    ASSERT_EQ(gt->get_config_data_identifier(), "INIT");
                    ASSERT_EQ(gt->is_lut_init_ascending(), true);
                }

                // descending LUT gate type
                {
                    GateType* gt = gl->get_gate_type_by_name("gt_lut_desc");
                    ASSERT_NE(gt, nullptr);

                    // general stuff
                    EXPECT_EQ(gt->get_id(), 4);
                    EXPECT_EQ(gt->get_name(), "gt_lut_desc");
                    EXPECT_EQ(gt->get_base_type(), GateType::BaseType::lut);

                    // pins
                    EXPECT_EQ(gt->get_input_pins(), std::vector<std::string>({"I1", "I2"}));
                    EXPECT_EQ(gt->get_output_pins(), std::vector<std::string>({"O"}));

                    // pin types
                    EXPECT_EQ(gt->get_pin_type("I1"), GateType::PinType::none);
                    EXPECT_EQ(gt->get_pin_type("I2"), GateType::PinType::none);
                    EXPECT_EQ(gt->get_pin_type("O"), GateType::PinType::lut);

                    // pin groups
                    EXPECT_TRUE(gt->get_input_pin_groups().empty());
                    EXPECT_TRUE(gt->get_output_pin_groups().empty());

                    // Boolean functions
                    std::unordered_map<std::string, BooleanFunction> functions = gt->get_boolean_functions();
                    ASSERT_TRUE(functions.empty());

                    // clear-preset behavior
                    ASSERT_EQ(gt->get_clear_preset_behavior(), std::pair(GateType::ClearPresetBehavior::U, GateType::ClearPresetBehavior::U));

                    // config data and init string
                    ASSERT_EQ(gt->get_config_data_category(), "generic");
                    ASSERT_EQ(gt->get_config_data_identifier(), "INIT");
                    ASSERT_EQ(gt->is_lut_init_ascending(), false);
                }

                // FF gate type
                {
                    GateType* gt = gl->get_gate_type_by_name("gt_ff");
                    ASSERT_NE(gt, nullptr);

                    // general stuff
                    EXPECT_EQ(gt->get_id(), 5);
                    EXPECT_EQ(gt->get_name(), "gt_ff");
                    EXPECT_EQ(gt->get_base_type(), GateType::BaseType::ff);

                    // pins
                    EXPECT_EQ(gt->get_input_pins(), std::vector<std::string>({"CLK", "D", "EN", "R", "S"}));
                    EXPECT_EQ(gt->get_output_pins(), std::vector<std::string>({"Q", "QN"}));

                    // pin types
                    EXPECT_EQ(gt->get_pin_type("CLK"), GateType::PinType::clock);
                    EXPECT_EQ(gt->get_pin_type("D"), GateType::PinType::data);
                    EXPECT_EQ(gt->get_pin_type("EN"), GateType::PinType::enable);
                    EXPECT_EQ(gt->get_pin_type("R"), GateType::PinType::reset);
                    EXPECT_EQ(gt->get_pin_type("S"), GateType::PinType::set);
                    EXPECT_EQ(gt->get_pin_type("Q"), GateType::PinType::state);
                    EXPECT_EQ(gt->get_pin_type("QN"), GateType::PinType::neg_state);

                    // pin groups
                    EXPECT_TRUE(gt->get_input_pin_groups().empty());
                    EXPECT_TRUE(gt->get_output_pin_groups().empty());

                    // Boolean functions
                    std::unordered_map<std::string, BooleanFunction> functions = gt->get_boolean_functions();
                    ASSERT_FALSE(functions.empty());
                    ASSERT_NE(functions.find("next_state"), functions.end());
                    ASSERT_EQ(functions.at("next_state"), BooleanFunction::from_string("D"));
                    ASSERT_NE(functions.find("clock"), functions.end());
                    ASSERT_EQ(functions.at("clock"), BooleanFunction::from_string("CLK & EN"));
                    ASSERT_NE(functions.find("clear"), functions.end());
                    ASSERT_EQ(functions.at("clear"), BooleanFunction::from_string("R"));
                    ASSERT_NE(functions.find("preset"), functions.end());
                    ASSERT_EQ(functions.at("preset"), BooleanFunction::from_string("S"));

                    // clear-preset behavior
                    ASSERT_EQ(gt->get_clear_preset_behavior(), std::pair(GateType::ClearPresetBehavior::L, GateType::ClearPresetBehavior::H));

                    // config data and init string
                    ASSERT_EQ(gt->get_config_data_category(), "generic");
                    ASSERT_EQ(gt->get_config_data_identifier(), "INIT");
                    ASSERT_EQ(gt->is_lut_init_ascending(), true);
                }

                // Latch gate type
                {
                    GateType* gt = gl->get_gate_type_by_name("gt_latch");
                    ASSERT_NE(gt, nullptr);

                    // general stuff
                    EXPECT_EQ(gt->get_id(), 6);
                    EXPECT_EQ(gt->get_name(), "gt_latch");
                    EXPECT_EQ(gt->get_base_type(), GateType::BaseType::latch);

                    // pins
                    EXPECT_EQ(gt->get_input_pins(), std::vector<std::string>({"D", "EN", "R", "S"}));
                    EXPECT_EQ(gt->get_output_pins(), std::vector<std::string>({"Q", "QN"}));

                    // pin types
                    EXPECT_EQ(gt->get_pin_type("D"), GateType::PinType::data);
                    EXPECT_EQ(gt->get_pin_type("EN"), GateType::PinType::enable);
                    EXPECT_EQ(gt->get_pin_type("R"), GateType::PinType::reset);
                    EXPECT_EQ(gt->get_pin_type("S"), GateType::PinType::set);
                    EXPECT_EQ(gt->get_pin_type("Q"), GateType::PinType::state);
                    EXPECT_EQ(gt->get_pin_type("QN"), GateType::PinType::neg_state);

                    // pin groups
                    EXPECT_TRUE(gt->get_input_pin_groups().empty());
                    EXPECT_TRUE(gt->get_output_pin_groups().empty());

                    // Boolean functions
                    std::unordered_map<std::string, BooleanFunction> functions = gt->get_boolean_functions();
                    ASSERT_FALSE(functions.empty());
                    ASSERT_NE(functions.find("data"), functions.end());
                    ASSERT_EQ(functions.at("data"), BooleanFunction::from_string("D"));
                    ASSERT_NE(functions.find("enable"), functions.end());
                    ASSERT_EQ(functions.at("enable"), BooleanFunction::from_string("EN"));
                    ASSERT_NE(functions.find("clear"), functions.end());
                    ASSERT_EQ(functions.at("clear"), BooleanFunction::from_string("R"));
                    ASSERT_NE(functions.find("preset"), functions.end());
                    ASSERT_EQ(functions.at("preset"), BooleanFunction::from_string("S"));

                    // clear-preset behavior
                    ASSERT_EQ(gt->get_clear_preset_behavior(), std::pair(GateType::ClearPresetBehavior::N, GateType::ClearPresetBehavior::T));

                    // config data and init string
                    ASSERT_EQ(gt->get_config_data_category(), "");
                    ASSERT_EQ(gt->get_config_data_identifier(), "");
                    ASSERT_EQ(gt->is_lut_init_ascending(), true);
                }

                // RAM gate type
                {
                    GateType* gt = gl->get_gate_type_by_name("gt_ram");
                    ASSERT_NE(gt, nullptr);

                    // general stuff
                    EXPECT_EQ(gt->get_id(), 7);
                    EXPECT_EQ(gt->get_name(), "gt_ram");
                    EXPECT_EQ(gt->get_base_type(), GateType::BaseType::ram);

                    // pins
                    EXPECT_EQ(gt->get_input_pins(), std::vector<std::string>({"DI(0)", "DI(1)", "DI(2)", "A(0)", "A(1)", "A(2)"}));
                    EXPECT_EQ(gt->get_output_pins(), std::vector<std::string>({"DO(0)", "DO(1)", "DO(2)"}));

                    // pin types
                    EXPECT_EQ(gt->get_pin_type("DI(0)"), GateType::PinType::data);
                    EXPECT_EQ(gt->get_pin_type("DI(1)"), GateType::PinType::data);
                    EXPECT_EQ(gt->get_pin_type("DI(2)"), GateType::PinType::data);
                    EXPECT_EQ(gt->get_pin_type("A(0)"), GateType::PinType::address);
                    EXPECT_EQ(gt->get_pin_type("A(1)"), GateType::PinType::address);
                    EXPECT_EQ(gt->get_pin_type("A(2)"), GateType::PinType::address);
                    EXPECT_EQ(gt->get_pin_type("DO(0)"), GateType::PinType::data);
                    EXPECT_EQ(gt->get_pin_type("DO(1)"), GateType::PinType::data);
                    EXPECT_EQ(gt->get_pin_type("DO(2)"), GateType::PinType::data);

                    // pin groups
                    std::unordered_map<std::string, std::map<u32, std::string>> expected_in_group({{"DI", {{0, "DI(0)"}, {1, "DI(1)"}, {2, "DI(2)"}}}, {"A", {{0, "A(0)"}, {1, "A(1)"}, {2, "A(2)"}}}});
                    std::unordered_map<std::string, std::map<u32, std::string>> expected_out_group({{"DO", {{0, "DO(0)"}, {1, "DO(1)"}, {2, "DO(2)"}}}});
                    EXPECT_EQ(gt->get_input_pin_groups(), expected_in_group);
                    EXPECT_EQ(gt->get_output_pin_groups(), expected_out_group);

                    // Boolean functions
                    std::unordered_map<std::string, BooleanFunction> functions = gt->get_boolean_functions();
                    ASSERT_TRUE(functions.empty());

                    // clear-preset behavior
                    ASSERT_EQ(gt->get_clear_preset_behavior(), std::pair(GateType::ClearPresetBehavior::U, GateType::ClearPresetBehavior::U));

                    // config data and init string
                    ASSERT_EQ(gt->get_config_data_category(), "");
                    ASSERT_EQ(gt->get_config_data_identifier(), "");
                    ASSERT_EQ(gt->is_lut_init_ascending(), true);
                }
            }
        TEST_END
    }
}    //namespace hal
