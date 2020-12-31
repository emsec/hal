#include "hgl_writer/hgl_writer.h"

#include "hgl_parser/hgl_parser.h"
#include "netlist_test_utils.h"

namespace hal
{
    namespace
    {
        static std::unique_ptr<GateLibrary> create_gate_library()
        {
            std::unique_ptr<GateLibrary> gl = std::make_unique<GateLibrary>(utils::get_base_directory().string() + "/bin/hal_plugins/test-files/hgl_writer/test.hgl", "example_library");

            {
                GateType* gt = gl->create_gate_type("gt_combinational", GateType::BaseType::combinational);

                gt->add_input_pins({"VDD", "GND", "A", "B"});
                gt->add_output_pins({"B", "O"});

                gt->assign_pin_type("VDD", GateType::PinType::power);
                gt->assign_pin_type("GND", GateType::PinType::ground);

                gt->add_boolean_function("O", BooleanFunction::from_string("A & B"));
                gt->add_boolean_function("O_undefined", BooleanFunction::from_string("!A & B"));
                gt->add_boolean_function("O_tristate", BooleanFunction::from_string("!A & !B"));
            }

            {
                GateType* gt = gl->create_gate_type("gt_group", GateType::BaseType::combinational);

                gt->add_input_pins({"VDD", "GND", "A(0)", "A(1)", "B(0)", "B(1)"});
                gt->add_output_pins({"B(0)", "B(1)", "C(0)", "C(1)", "O"});

                gt->assign_input_pin_group("A", {{0, "A(0)"}, {1, "A(1)"}});
                gt->assign_input_pin_group("B", {{0, "B(0)"}, {1, "B(1)"}});
                gt->assign_output_pin_group("B", {{0, "B(0)"}, {1, "B(1)"}});
                gt->assign_output_pin_group("C", {{0, "C(0)"}, {1, "C(1)"}});

                gt->assign_pin_type("VDD", GateType::PinType::power);
                gt->assign_pin_type("GND", GateType::PinType::ground);

                gt->add_boolean_function("O", BooleanFunction::from_string("A(1) & B(0)", {"A(1)", "B(0)"}));
                gt->add_boolean_function("O_undefined", BooleanFunction::from_string("!A(0) & B(0)", {"A(0)", "B(0)"}));
                gt->add_boolean_function("O_tristate", BooleanFunction::from_string("!A(1) & !B(1)", {"A(1)", "B(1)"}));
            }

            {
                GateType* gt = gl->create_gate_type("gt_lut_asc", GateType::BaseType::lut);

                gt->add_input_pins({"I1", "I2"});
                gt->add_output_pins({"O"});

                gt->assign_pin_type("O", GateType::PinType::lut);

                gt->set_config_data_category("generic");
                gt->set_config_data_identifier("INIT");
                gt->set_lut_init_ascending(true);
            }

            {
                GateType* gt = gl->create_gate_type("gt_lut_desc", GateType::BaseType::lut);

                gt->add_input_pins({"I1", "I2"});
                gt->add_output_pins({"O"});

                gt->assign_pin_type("O", GateType::PinType::lut);

                gt->set_config_data_category("generic");
                gt->set_config_data_identifier("INIT");
                gt->set_lut_init_ascending(false);
            }

            {
                GateType* gt = gl->create_gate_type("gt_ff", GateType::BaseType::ff);

                gt->add_input_pins({"CLK", "D", "EN", "R", "S"});
                gt->add_output_pins({"Q", "QN"});

                gt->assign_pin_type("CLK", GateType::PinType::clock);
                gt->assign_pin_type("D", GateType::PinType::data);
                gt->assign_pin_type("EN", GateType::PinType::enable);
                gt->assign_pin_type("R", GateType::PinType::reset);
                gt->assign_pin_type("S", GateType::PinType::set);
                gt->assign_pin_type("Q", GateType::PinType::state);
                gt->assign_pin_type("QN", GateType::PinType::neg_state);

                gt->add_boolean_function("next_state", BooleanFunction::from_string("D"));
                gt->add_boolean_function("clock", BooleanFunction::from_string("CLK & EN"));
                gt->add_boolean_function("clear", BooleanFunction::from_string("R"));
                gt->add_boolean_function("preset", BooleanFunction::from_string("S"));

                gt->set_clear_preset_behavior(GateType::ClearPresetBehavior::L, GateType::ClearPresetBehavior::H);
                gt->set_config_data_category("generic");
                gt->set_config_data_identifier("INIT");
            }

            {
                GateType* gt = gl->create_gate_type("gt_latch", GateType::BaseType::latch);

                gt->add_input_pins({"D", "EN", "R", "S"});
                gt->add_output_pins({"Q", "QN"});

                gt->assign_pin_type("D", GateType::PinType::data);
                gt->assign_pin_type("EN", GateType::PinType::enable);
                gt->assign_pin_type("R", GateType::PinType::reset);
                gt->assign_pin_type("S", GateType::PinType::set);
                gt->assign_pin_type("Q", GateType::PinType::state);
                gt->assign_pin_type("QN", GateType::PinType::neg_state);

                gt->add_boolean_function("data", BooleanFunction::from_string("D"));
                gt->add_boolean_function("enable", BooleanFunction::from_string("EN"));
                gt->add_boolean_function("clear", BooleanFunction::from_string("R"));
                gt->add_boolean_function("preset", BooleanFunction::from_string("S"));

                gt->set_clear_preset_behavior(GateType::ClearPresetBehavior::L, GateType::ClearPresetBehavior::H);
            }

            {
                GateType* gt = gl->create_gate_type("gt_ram", GateType::BaseType::ram);

                gt->add_input_pins({"DI(0)", "DI(1)", "DI(2)", "A(0)", "A(1)", "A(2)"});
                gt->add_output_pins({"DO(0)", "DO(1)", "DO(2)"});

                gt->assign_input_pin_group("DI", {{0, "DI(0)"}, {1, "DI(1)"}, {2, "DI(2)"}});
                gt->assign_input_pin_group("A", {{0, "A(0)"}, {1, "A(1)"}, {2, "A(2)"}});
                gt->assign_output_pin_group("DO", {{0, "DO(0)"}, {1, "DO(1)"}, {2, "DO(2)"}});

                gt->assign_pin_type("DI(0)", GateType::PinType::data);
                gt->assign_pin_type("DI(1)", GateType::PinType::data);
                gt->assign_pin_type("DI(2)", GateType::PinType::data);
                gt->assign_pin_type("A(0)", GateType::PinType::address);
                gt->assign_pin_type("A(1)", GateType::PinType::address);
                gt->assign_pin_type("A(2)", GateType::PinType::address);
                gt->assign_pin_type("DO(0)", GateType::PinType::data);
                gt->assign_pin_type("DO(1)", GateType::PinType::data);
                gt->assign_pin_type("DO(2)", GateType::PinType::data);
            }

            return gl;
        }
    }    // namespace

    class HGLWriterTest : public ::testing::Test
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
     * Testing writing an HGL file.
     *
     * Functions: write
     */
    TEST_F(HGLWriterTest, check_library)
    {
        TEST_START

        std::string path_lib = utils::get_base_directory().string() + "/bin/hal_plugins/test-files/hgl_writer/test.hgl";

        std::unique_ptr<GateLibrary> gl_original = create_gate_library();

        HGLWriter writer;
        ASSERT_TRUE(writer.write(gl_original.get(), path_lib));
        HGLParser parser;
        std::unique_ptr<GateLibrary> gl_written = parser.parse(path_lib);
        ASSERT_NE(gl_written, nullptr);

        EXPECT_EQ(gl_original->get_name(), gl_written->get_name());
        EXPECT_EQ(gl_original->get_path(), gl_written->get_path());

        std::unordered_map<std::string, GateType*> gt_original = gl_original->get_gate_types();
        std::unordered_map<std::string, GateType*> gt_written  = gl_written->get_gate_types();
        ASSERT_EQ(gt_original.size(), gt_written.size());

        for (auto [it1, it2] = std::tuple{gt_original.begin(), gt_written.begin()}; it1 != gt_original.end() && it2 != gt_written.end(); it1++, it2++)
        {
            GateType* gt1 = it1->second;
            GateType* gt2 = it2->second;

            EXPECT_EQ(gt1->get_id(), gt2->get_id());
            EXPECT_EQ(gt1->get_name(), gt2->get_name());
            EXPECT_EQ(gt1->get_base_type(), gt2->get_base_type());
            EXPECT_EQ(gt1->get_input_pins(), gt2->get_input_pins());
            EXPECT_EQ(gt1->get_output_pins(), gt2->get_output_pins());
            EXPECT_EQ(gt1->get_input_pin_groups(), gt2->get_input_pin_groups());
            EXPECT_EQ(gt1->get_output_pin_groups(), gt2->get_output_pin_groups());
            EXPECT_EQ(gt1->get_pin_types(), gt2->get_pin_types());
            EXPECT_EQ(gt1->get_boolean_functions(), gt2->get_boolean_functions());
            EXPECT_EQ(gt1->get_clear_preset_behavior(), gt2->get_clear_preset_behavior());
            EXPECT_EQ(gt1->get_config_data_category(), gt2->get_config_data_category());
            EXPECT_EQ(gt1->get_config_data_identifier(), gt2->get_config_data_identifier());
            EXPECT_EQ(gt1->is_lut_init_ascending(), gt2->is_lut_init_ascending());
        }

        TEST_END
    }
}    //namespace hal
