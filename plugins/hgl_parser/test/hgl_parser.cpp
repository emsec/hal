#include "hgl_parser/hgl_parser.h"

#include "hal_core/netlist/gate_library/gate_type_component/ff_component.h"
#include "hal_core/netlist/gate_library/gate_type_component/init_component.h"
#include "hal_core/netlist/gate_library/gate_type_component/latch_component.h"
#include "hal_core/netlist/gate_library/gate_type_component/lut_component.h"
#include "hal_core/netlist/gate_library/gate_type_component/state_component.h"
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
            auto res = parser.parse(path_lib);
            ASSERT_TRUE(res.is_ok());
            std::unique_ptr<GateLibrary> gl = res.get();

            // parsing succeeds
            ASSERT_NE(gl, nullptr);

            // gate library name
            EXPECT_EQ(gl->get_name(), "example_library");

            // gate types
            std::unordered_map<std::string, GateType*> gate_types = gl->get_gate_types();
            ASSERT_EQ(gate_types.size(), 8);

            // combinational gate type
            {
                GateType* gt = gl->get_gate_type_by_name("gt_combinational");
                ASSERT_NE(gt, nullptr);

                // general stuff
                EXPECT_EQ(gt->get_id(), 1);
                EXPECT_EQ(gt->get_name(), "gt_combinational");
                EXPECT_EQ(gt->get_properties(), std::set<GateTypeProperty>({GateTypeProperty::combinational}));

                // pins
                EXPECT_EQ(gt->get_input_pin_names(), std::vector<std::string>({"VDD", "GND", "A", "B"}));
                EXPECT_EQ(gt->get_output_pin_names(), std::vector<std::string>({"B", "O"}));

                // pin types
                EXPECT_EQ(gt->get_pin_by_name("VDD").get()->get_type(), PinType::power);
                EXPECT_EQ(gt->get_pin_by_name("GND").get()->get_type(), PinType::ground);
                EXPECT_EQ(gt->get_pin_by_name("A").get()->get_type(), PinType::none);
                EXPECT_EQ(gt->get_pin_by_name("B").get()->get_type(), PinType::none);
                EXPECT_EQ(gt->get_pin_by_name("O").get()->get_type(), PinType::none);

                // pin groups
                EXPECT_TRUE(gt->get_pin_groups().empty());

                // Boolean functions
                std::unordered_map<std::string, BooleanFunction> functions = gt->get_boolean_functions();
                ASSERT_FALSE(functions.empty());
                EXPECT_NE(functions.find("O"), functions.end());
                EXPECT_EQ(functions.at("O"), BooleanFunction::from_string("A & B").get());
                EXPECT_NE(functions.find("O_undefined"), functions.end());
                EXPECT_EQ(functions.at("O_undefined"), BooleanFunction::from_string("!A & B").get());
                EXPECT_NE(functions.find("O_tristate"), functions.end());
                EXPECT_EQ(functions.at("O_tristate"), BooleanFunction::from_string("!A & !B").get());

                // Components
                EXPECT_TRUE(gt->get_components().empty());
            }

            // combinational group gate type
            {
                GateType* gt = gl->get_gate_type_by_name("gt_group");
                ASSERT_NE(gt, nullptr);

                // general stuff
                EXPECT_EQ(gt->get_id(), 2);
                EXPECT_EQ(gt->get_name(), "gt_group");
                EXPECT_EQ(gt->get_properties(), std::set<GateTypeProperty>({GateTypeProperty::combinational}));

                // pins
                EXPECT_EQ(gt->get_input_pin_names(), std::vector<std::string>({"VDD", "GND", "A(0)", "A(1)", "B(0)", "B(1)"}));
                EXPECT_EQ(gt->get_output_pin_names(), std::vector<std::string>({"B(0)", "B(1)", "C(0)", "C(1)", "O"}));

                // pin types
                EXPECT_EQ(gt->get_pin_by_name("VDD").get()->get_type(), PinType::power);
                EXPECT_EQ(gt->get_pin_by_name("GND").get()->get_type(), PinType::ground);
                auto p_a0 = gt->get_pin_by_name("A(0)").get();
                EXPECT_EQ(p_a0->get_type(), PinType::none);
                auto p_a1 = gt->get_pin_by_name("A(1)").get();
                EXPECT_EQ(p_a1->get_type(), PinType::none);
                auto p_b0 = gt->get_pin_by_name("B(0)").get();
                EXPECT_EQ(p_b0->get_type(), PinType::none);
                auto p_b1 = gt->get_pin_by_name("B(1)").get();
                EXPECT_EQ(p_b1->get_type(), PinType::none);
                auto p_c0 = gt->get_pin_by_name("C(0)").get();
                EXPECT_EQ(p_c0->get_type(), PinType::none);
                auto p_c1 = gt->get_pin_by_name("C(1)").get();
                EXPECT_EQ(p_c1->get_type(), PinType::none);
                EXPECT_EQ(gt->get_pin_by_name("O").get()->get_type(), PinType::none);

                // pin groups
                std::unordered_map<std::string, std::vector<std::pair<u32, std::string>>> expected_groups(
                    {{"A", {{0, "A(0)"}, {1, "A(1)"}}}, {"B", {{0, "B(0)"}, {1, "B(1)"}}}, {"C", {{0, "C(0)"}, {1, "C(1)"}}}});
                EXPECT_EQ(gt->get_pin_groups(), expected_groups);
                auto pin_groups = gt->get_pin_groups();
                ASSERT_EQ(pin_groups.size(), 3);
                {
                    auto pg_a = pin_groups.at(0);
                    EXPECT_EQ(pg_a->get_name(), "A");
                    EXPECT_EQ(pg_a->get_pins(), std::vector<GatePin*>({p_a0, p_a1}));
                    EXPECT_EQ(pg_a->get_direction(), PinDirection::input);
                    EXPECT_EQ(pg_a->get_type(), PinType::none);
                    EXPECT_EQ(pg_a->get_index(p_a0).get(), 0);
                    EXPECT_EQ(pg_a->get_index(p_a1).get(), 1);
                    EXPECT_EQ(pg_a->is_ascending(), true);
                    EXPECT_EQ(pg_a->get_start_index(), 0);
                }
                {
                    auto pg_b = pin_groups.at(1);
                    EXPECT_EQ(pg_b->get_name(), "B");
                    EXPECT_EQ(pg_b->get_pins(), std::vector<GatePin*>({p_b0, p_b1}));
                    EXPECT_EQ(pg_b->get_direction(), PinDirection::inout);
                    EXPECT_EQ(pg_b->get_type(), PinType::none);
                    EXPECT_EQ(pg_b->get_index(p_b0).get(), 0);
                    EXPECT_EQ(pg_b->get_index(p_b1).get(), 1);
                    EXPECT_EQ(pg_b->is_ascending(), true);
                    EXPECT_EQ(pg_b->get_start_index(), 0);
                }
                {
                    auto pg_c = pin_groups.at(2);
                    EXPECT_EQ(pg_c->get_name(), "C");
                    EXPECT_EQ(pg_c->get_pins(), std::vector<GatePin*>({p_c0, p_c1}));
                    EXPECT_EQ(pg_c->get_direction(), PinDirection::output);
                    EXPECT_EQ(pg_c->get_type(), PinType::none);
                    EXPECT_EQ(pg_c->get_index(p_c0).get(), 0);
                    EXPECT_EQ(pg_c->get_index(p_c1).get(), 1);
                    EXPECT_EQ(pg_c->is_ascending(), true);
                    EXPECT_EQ(pg_c->get_start_index(), 0);
                }

                // Boolean functions
                std::unordered_map<std::string, BooleanFunction> functions = gt->get_boolean_functions();
                ASSERT_FALSE(functions.empty());
                ASSERT_NE(functions.find("O"), functions.end());
                ASSERT_EQ(functions.at("O"), BooleanFunction::from_string("A(1) & B(0)").get());
                ASSERT_NE(functions.find("O_undefined"), functions.end());
                ASSERT_EQ(functions.at("O_undefined"), BooleanFunction::from_string("!A(0) & B(0)").get());
                ASSERT_NE(functions.find("O_tristate"), functions.end());
                ASSERT_EQ(functions.at("O_tristate"), BooleanFunction::from_string("!A(1) & !B(1)").get());

                // Components
                EXPECT_TRUE(gt->get_components().empty());
            }

            // ascending LUT gate type
            {
                GateType* gt = gl->get_gate_type_by_name("gt_lut_asc");
                ASSERT_NE(gt, nullptr);

                // general stuff
                EXPECT_EQ(gt->get_id(), 3);
                EXPECT_EQ(gt->get_name(), "gt_lut_asc");
                EXPECT_EQ(gt->get_properties(), std::set<GateTypeProperty>({GateTypeProperty::lut, GateTypeProperty::combinational}));

                // pins
                EXPECT_EQ(gt->get_input_pin_names(), std::vector<std::string>({"I1", "I2"}));
                EXPECT_EQ(gt->get_output_pin_names(), std::vector<std::string>({"O"}));

                // pin types
                EXPECT_EQ(gt->get_pin_by_name("I1").get()->get_type(), PinType::none);
                EXPECT_EQ(gt->get_pin_by_name("I2").get()->get_type(), PinType::none);
                EXPECT_EQ(gt->get_pin_by_name("O").get()->get_type(), PinType::lut);

                // pin groups
                EXPECT_TRUE(gt->get_pin_groups().empty());

                // Boolean functions
                std::unordered_map<std::string, BooleanFunction> functions = gt->get_boolean_functions();
                ASSERT_TRUE(functions.empty());

                // Components
                ASSERT_EQ(gt->get_components().size(), 2);
                const LUTComponent* lut_component =
                    gt->get_component_as<LUTComponent>([](const GateTypeComponent* component) { return component->get_type() == GateTypeComponent::ComponentType::lut; });
                const InitComponent* init_component =
                    gt->get_component_as<InitComponent>([](const GateTypeComponent* component) { return component->get_type() == GateTypeComponent::ComponentType::init; });
                ASSERT_NE(lut_component, nullptr);
                ASSERT_NE(init_component, nullptr);

                EXPECT_EQ(init_component->get_init_category(), "generic");
                EXPECT_EQ(init_component->get_init_identifiers(), std::vector<std::string>({"INIT"}));
                EXPECT_EQ(lut_component->is_init_ascending(), true);
            }

            // descending LUT gate type
            {
                GateType* gt = gl->get_gate_type_by_name("gt_lut_desc");
                ASSERT_NE(gt, nullptr);

                // general stuff
                EXPECT_EQ(gt->get_id(), 4);
                EXPECT_EQ(gt->get_name(), "gt_lut_desc");
                EXPECT_EQ(gt->get_properties(), std::set<GateTypeProperty>({GateTypeProperty::lut, GateTypeProperty::combinational}));

                // pins
                EXPECT_EQ(gt->get_input_pin_names(), std::vector<std::string>({"I1", "I2"}));
                EXPECT_EQ(gt->get_output_pin_names(), std::vector<std::string>({"O"}));

                // pin types
                EXPECT_EQ(gt->get_pin_by_name("I1").get()->get_type(), PinType::none);
                EXPECT_EQ(gt->get_pin_by_name("I2").get()->get_type(), PinType::none);
                EXPECT_EQ(gt->get_pin_by_name("O").get()->get_type(), PinType::lut);

                // pin groups
                EXPECT_TRUE(gt->get_pin_groups().empty());

                // Boolean functions
                std::unordered_map<std::string, BooleanFunction> functions = gt->get_boolean_functions();
                ASSERT_TRUE(functions.empty());

                // Components
                ASSERT_EQ(gt->get_components().size(), 2);
                const LUTComponent* lut_component =
                    gt->get_component_as<LUTComponent>([](const GateTypeComponent* component) { return component->get_type() == GateTypeComponent::ComponentType::lut; });
                const InitComponent* init_component =
                    gt->get_component_as<InitComponent>([](const GateTypeComponent* component) { return component->get_type() == GateTypeComponent::ComponentType::init; });
                ASSERT_NE(lut_component, nullptr);
                ASSERT_NE(init_component, nullptr);

                EXPECT_EQ(lut_component->is_init_ascending(), false);
                EXPECT_EQ(init_component->get_init_category(), "generic");
                EXPECT_EQ(init_component->get_init_identifiers(), std::vector<std::string>({"INIT"}));
            }

            // FF gate type without initialization
            {
                GateType* gt = gl->get_gate_type_by_name("gt_ff");
                ASSERT_NE(gt, nullptr);

                // general stuff
                EXPECT_EQ(gt->get_id(), 5);
                EXPECT_EQ(gt->get_name(), "gt_ff");
                EXPECT_EQ(gt->get_properties(), std::set<GateTypeProperty>({GateTypeProperty::ff, GateTypeProperty::sequential}));

                // pins
                EXPECT_EQ(gt->get_input_pin_names(), std::vector<std::string>({"CLK", "D", "EN", "R", "S"}));
                EXPECT_EQ(gt->get_output_pin_names(), std::vector<std::string>({"Q", "QN"}));

                // pin types
                EXPECT_EQ(gt->get_pin_by_name("CLK").get()->get_type(), PinType::clock);
                EXPECT_EQ(gt->get_pin_by_name("D").get()->get_type(), PinType::data);
                EXPECT_EQ(gt->get_pin_by_name("EN").get()->get_type(), PinType::enable);
                EXPECT_EQ(gt->get_pin_by_name("R").get()->get_type(), PinType::reset);
                EXPECT_EQ(gt->get_pin_by_name("S").get()->get_type(), PinType::set);
                EXPECT_EQ(gt->get_pin_by_name("Q").get()->get_type(), PinType::state);
                EXPECT_EQ(gt->get_pin_by_name("QN").get()->get_type(), PinType::neg_state);

                // pin groups
                EXPECT_TRUE(gt->get_pin_groups().empty());

                // Boolean functions
                EXPECT_TRUE(gt->get_boolean_functions().empty());

                // Components
                ASSERT_EQ(gt->get_components().size(), 2);
                const FFComponent* ff_component = gt->get_component_as<FFComponent>([](const GateTypeComponent* c) { return FFComponent::is_class_of(c); });
                ASSERT_NE(ff_component, nullptr);
                const StateComponent* state_component = gt->get_component_as<StateComponent>([](const GateTypeComponent* c) { return StateComponent::is_class_of(c); });
                ASSERT_NE(state_component, nullptr);

                EXPECT_EQ(ff_component->get_async_set_reset_behavior(), std::pair(AsyncSetResetBehavior::L, AsyncSetResetBehavior::H));
                EXPECT_EQ(ff_component->get_next_state_function(), BooleanFunction::from_string("D").get());
                EXPECT_EQ(ff_component->get_clock_function(), BooleanFunction::from_string("CLK & EN").get());
                EXPECT_EQ(ff_component->get_async_reset_function(), BooleanFunction::from_string("R").get());
                EXPECT_EQ(ff_component->get_async_set_function(), BooleanFunction::from_string("S").get());
                EXPECT_EQ(state_component->get_state_identifier(), "IQ");
                EXPECT_EQ(state_component->get_neg_state_identifier(), "IQN");
            }

            // FF gate type with initialization
            {
                GateType* gt = gl->get_gate_type_by_name("gt_ff_init");
                ASSERT_NE(gt, nullptr);

                // general stuff
                EXPECT_EQ(gt->get_id(), 6);
                EXPECT_EQ(gt->get_name(), "gt_ff_init");
                EXPECT_EQ(gt->get_properties(), std::set<GateTypeProperty>({GateTypeProperty::ff, GateTypeProperty::sequential}));

                // pins
                EXPECT_EQ(gt->get_input_pin_names(), std::vector<std::string>({"CLK", "D", "EN", "R", "S"}));
                EXPECT_EQ(gt->get_output_pin_names(), std::vector<std::string>({"Q", "QN"}));

                // pin types
                EXPECT_EQ(gt->get_pin_by_name("CLK").get()->get_type(), PinType::clock);
                EXPECT_EQ(gt->get_pin_by_name("D").get()->get_type(), PinType::data);
                EXPECT_EQ(gt->get_pin_by_name("EN").get()->get_type(), PinType::enable);
                EXPECT_EQ(gt->get_pin_by_name("R").get()->get_type(), PinType::reset);
                EXPECT_EQ(gt->get_pin_by_name("S").get()->get_type(), PinType::set);
                EXPECT_EQ(gt->get_pin_by_name("Q").get()->get_type(), PinType::state);
                EXPECT_EQ(gt->get_pin_by_name("QN").get()->get_type(), PinType::neg_state);

                // pin groups
                EXPECT_TRUE(gt->get_pin_groups().empty());

                // Boolean functions
                EXPECT_TRUE(gt->get_boolean_functions().empty());

                // Components
                ASSERT_EQ(gt->get_components().size(), 3);
                const FFComponent* ff_component = gt->get_component_as<FFComponent>([](const GateTypeComponent* c) { return FFComponent::is_class_of(c); });
                ASSERT_NE(ff_component, nullptr);
                const StateComponent* state_component = gt->get_component_as<StateComponent>([](const GateTypeComponent* c) { return StateComponent::is_class_of(c); });
                ASSERT_NE(state_component, nullptr);
                const InitComponent* init_component = gt->get_component_as<InitComponent>([](const GateTypeComponent* c) { return InitComponent::is_class_of(c); });
                ASSERT_NE(init_component, nullptr);

                EXPECT_EQ(ff_component->get_async_set_reset_behavior(), std::pair(AsyncSetResetBehavior::L, AsyncSetResetBehavior::H));
                EXPECT_EQ(ff_component->get_next_state_function(), BooleanFunction::from_string("D").get());
                EXPECT_EQ(ff_component->get_clock_function(), BooleanFunction::from_string("CLK & EN").get());
                EXPECT_EQ(ff_component->get_async_reset_function(), BooleanFunction::from_string("R").get());
                EXPECT_EQ(ff_component->get_async_set_function(), BooleanFunction::from_string("S").get());
                EXPECT_EQ(state_component->get_state_identifier(), "IQ");
                EXPECT_EQ(state_component->get_neg_state_identifier(), "IQN");
                EXPECT_EQ(init_component->get_init_category(), "generic");
                EXPECT_EQ(init_component->get_init_identifiers(), std::vector<std::string>({"INIT"}));
            }

            // Latch gate type
            {
                GateType* gt = gl->get_gate_type_by_name("gt_latch");
                ASSERT_NE(gt, nullptr);

                // general stuff
                EXPECT_EQ(gt->get_id(), 7);
                EXPECT_EQ(gt->get_name(), "gt_latch");
                EXPECT_EQ(gt->get_properties(), std::set<GateTypeProperty>({GateTypeProperty::latch, GateTypeProperty::sequential}));

                // pins
                EXPECT_EQ(gt->get_input_pin_names(), std::vector<std::string>({"D", "EN", "R", "S"}));
                EXPECT_EQ(gt->get_output_pin_names(), std::vector<std::string>({"Q", "QN"}));

                // pin types
                EXPECT_EQ(gt->get_pin_by_name("D").get()->get_type(), PinType::data);
                EXPECT_EQ(gt->get_pin_by_name("EN").get()->get_type(), PinType::enable);
                EXPECT_EQ(gt->get_pin_by_name("R").get()->get_type(), PinType::reset);
                EXPECT_EQ(gt->get_pin_by_name("S").get()->get_type(), PinType::set);
                EXPECT_EQ(gt->get_pin_by_name("Q").get()->get_type(), PinType::state);
                EXPECT_EQ(gt->get_pin_by_name("QN").get()->get_type(), PinType::neg_state);

                // pin groups
                EXPECT_TRUE(gt->get_pin_groups().empty());

                // Boolean functions
                EXPECT_TRUE(gt->get_boolean_functions().empty());

                // Components
                ASSERT_EQ(gt->get_components().size(), 2);
                const LatchComponent* latch_component = gt->get_component_as<LatchComponent>([](const GateTypeComponent* c) { return LatchComponent::is_class_of(c); });
                ASSERT_NE(latch_component, nullptr);
                const StateComponent* state_component = gt->get_component_as<StateComponent>([](const GateTypeComponent* c) { return StateComponent::is_class_of(c); });
                ASSERT_NE(state_component, nullptr);

                EXPECT_EQ(latch_component->get_async_set_reset_behavior(), std::pair(AsyncSetResetBehavior::N, AsyncSetResetBehavior::T));
                EXPECT_EQ(latch_component->get_data_in_function(), BooleanFunction::Var("D"));
                EXPECT_EQ(latch_component->get_enable_function(), BooleanFunction::Var("EN"));
                EXPECT_EQ(latch_component->get_async_reset_function(), BooleanFunction::Var("R"));
                EXPECT_EQ(latch_component->get_async_set_function(), BooleanFunction::Var("S"));
                EXPECT_EQ(state_component->get_state_identifier(), "IQ");
                EXPECT_EQ(state_component->get_neg_state_identifier(), "IQN");
            }

            // RAM gate type
            {
                GateType* gt = gl->get_gate_type_by_name("gt_ram");
                ASSERT_NE(gt, nullptr);

                // general stuff
                EXPECT_EQ(gt->get_id(), 8);
                EXPECT_EQ(gt->get_name(), "gt_ram");
                EXPECT_EQ(gt->get_properties(), std::set<GateTypeProperty>({GateTypeProperty::ram}));

                // pins
                EXPECT_EQ(gt->get_input_pin_names(), std::vector<std::string>({"DI(0)", "DI(1)", "DI(2)", "A(0)", "A(1)", "A(2)"}));
                EXPECT_EQ(gt->get_output_pin_names(), std::vector<std::string>({"DO(0)", "DO(1)", "DO(2)"}));

                // pin types
                EXPECT_EQ(gt->get_pin_by_name("DI(0)").get()->get_type(), PinType::data);
                EXPECT_EQ(gt->get_pin_by_name("DI(1)").get()->get_type(), PinType::data);
                EXPECT_EQ(gt->get_pin_by_name("DI(2)").get()->get_type(), PinType::data);
                EXPECT_EQ(gt->get_pin_by_name("A(0)").get()->get_type(), PinType::address);
                EXPECT_EQ(gt->get_pin_by_name("A(1)").get()->get_type(), PinType::address);
                EXPECT_EQ(gt->get_pin_by_name("A(2)").get()->get_type(), PinType::address);
                EXPECT_EQ(gt->get_pin_by_name("DO(0)").get()->get_type(), PinType::data);
                EXPECT_EQ(gt->get_pin_by_name("DO(1)").get()->get_type(), PinType::data);
                EXPECT_EQ(gt->get_pin_by_name("DO(2)").get()->get_type(), PinType::data);

                // pin groups
                std::unordered_map<std::string, std::vector<std::pair<u32, std::string>>> expected_groups(
                    {{"DI", {{0, "DI(0)"}, {1, "DI(1)"}, {2, "DI(2)"}}}, {"A", {{0, "A(0)"}, {1, "A(1)"}, {2, "A(2)"}}}, {"DO", {{0, "DO(0)"}, {1, "DO(1)"}, {2, "DO(2)"}}}});
                EXPECT_EQ(gt->get_pin_groups(), expected_groups);

                // Boolean functions
                std::unordered_map<std::string, BooleanFunction> functions = gt->get_boolean_functions();
                EXPECT_TRUE(functions.empty());

                // Components
                EXPECT_TRUE(gt->get_components().empty());
            }
        }
        TEST_END
    }
}    //namespace hal
