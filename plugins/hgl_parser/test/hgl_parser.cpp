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
                {
                    auto vdd_res = gt->get_pin_by_name("VDD");
                    ASSERT_TRUE(vdd_res.is_ok());
                    auto vdd_pin = vdd_res.get();
                    EXPECT_EQ(vdd_pin->get_name(), "VDD");
                    EXPECT_EQ(vdd_pin->get_direction(), PinDirection::input);
                    EXPECT_EQ(vdd_pin->get_type(), PinType::power);
                    EXPECT_NE(vdd_pin->get_group().first, nullptr);
                    EXPECT_EQ(vdd_pin->get_group().second, 0);
                }
                {
                    auto gnd_res = gt->get_pin_by_name("GND");
                    ASSERT_TRUE(gnd_res.is_ok());
                    auto gnd_pin = gnd_res.get();
                    EXPECT_EQ(gnd_pin->get_name(), "GND");
                    EXPECT_EQ(gnd_pin->get_direction(), PinDirection::input);
                    EXPECT_EQ(gnd_pin->get_type(), PinType::ground);
                    EXPECT_NE(gnd_pin->get_group().first, nullptr);
                    EXPECT_EQ(gnd_pin->get_group().second, 0);
                }
                {
                    auto a_res = gt->get_pin_by_name("A");
                    ASSERT_TRUE(a_res.is_ok());
                    auto a_pin = a_res.get();
                    EXPECT_EQ(a_pin->get_name(), "A");
                    EXPECT_EQ(a_pin->get_direction(), PinDirection::input);
                    EXPECT_EQ(a_pin->get_type(), PinType::none);
                    EXPECT_NE(a_pin->get_group().first, nullptr);
                    EXPECT_EQ(a_pin->get_group().second, 0);
                }
                {
                    auto b_res = gt->get_pin_by_name("B");
                    ASSERT_TRUE(b_res.is_ok());
                    auto b_pin = b_res.get();
                    EXPECT_EQ(b_pin->get_name(), "B");
                    EXPECT_EQ(b_pin->get_direction(), PinDirection::inout);
                    EXPECT_EQ(b_pin->get_type(), PinType::none);
                    EXPECT_NE(b_pin->get_group().first, nullptr);
                    EXPECT_EQ(b_pin->get_group().second, 0);
                }
                {
                    auto o_res = gt->get_pin_by_name("O");
                    ASSERT_TRUE(o_res.is_ok());
                    auto o_pin = o_res.get();
                    EXPECT_EQ(o_pin->get_name(), "O");
                    EXPECT_EQ(o_pin->get_direction(), PinDirection::output);
                    EXPECT_EQ(o_pin->get_type(), PinType::none);
                    EXPECT_NE(o_pin->get_group().first, nullptr);
                    EXPECT_EQ(o_pin->get_group().second, 0);
                }

                // pin groups
                EXPECT_EQ(gt->get_pin_groups().size(), 5);

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
                ASSERT_EQ(gt->get_pin_groups().size(), 6);

                {
                    auto vdd_res = gt->get_pin_by_name("VDD");
                    ASSERT_TRUE(vdd_res.is_ok());
                    auto vdd_pin = vdd_res.get();
                    EXPECT_EQ(vdd_pin->get_name(), "VDD");
                    EXPECT_EQ(vdd_pin->get_direction(), PinDirection::input);
                    EXPECT_EQ(vdd_pin->get_type(), PinType::power);
                    EXPECT_NE(vdd_pin->get_group().first, nullptr);
                    EXPECT_EQ(vdd_pin->get_group().second, 0);
                }
                {
                    auto gnd_res = gt->get_pin_by_name("GND");
                    ASSERT_TRUE(gnd_res.is_ok());
                    auto gnd_pin = gnd_res.get();
                    EXPECT_EQ(gnd_pin->get_name(), "GND");
                    EXPECT_EQ(gnd_pin->get_direction(), PinDirection::input);
                    EXPECT_EQ(gnd_pin->get_type(), PinType::ground);
                    EXPECT_NE(gnd_pin->get_group().first, nullptr);
                    EXPECT_EQ(gnd_pin->get_group().second, 0);
                }
                {
                    auto a0_res = gt->get_pin_by_name("A(0)");
                    ASSERT_TRUE(a0_res.is_ok());
                    auto a0_pin = a0_res.get();
                    EXPECT_EQ(a0_pin->get_name(), "A(0)");
                    EXPECT_EQ(a0_pin->get_direction(), PinDirection::input);
                    EXPECT_EQ(a0_pin->get_type(), PinType::none);
                    EXPECT_NE(a0_pin->get_group().first, nullptr);
                    EXPECT_EQ(a0_pin->get_group().second, 0);

                    auto a1_res = gt->get_pin_by_name("A(1)");
                    ASSERT_TRUE(a1_res.is_ok());
                    auto a1_pin = a1_res.get();
                    EXPECT_EQ(a1_pin->get_name(), "A(1)");
                    EXPECT_EQ(a1_pin->get_direction(), PinDirection::input);
                    EXPECT_EQ(a1_pin->get_type(), PinType::none);
                    EXPECT_NE(a1_pin->get_group().first, nullptr);
                    EXPECT_EQ(a1_pin->get_group().second, 1);

                    auto a_res = gt->get_pin_group_by_name("A");
                    ASSERT_TRUE(a_res.is_ok());
                    auto a_pg = a_res.get();
                    EXPECT_EQ(a_pg->get_name(), "A");
                    EXPECT_EQ(a_pg->get_direction(), PinDirection::input);
                    EXPECT_EQ(a_pg->get_type(), PinType::none);
                    EXPECT_EQ(a_pg->get_pins(), std::vector<GatePin*>({a0_pin, a1_pin}));
                    EXPECT_EQ(a_pg->get_pin_at_index(0), a0_pin);
                    EXPECT_EQ(a_pg->get_pin_at_index(1), a1_pin);
                    EXPECT_FALSE(a_pg->is_ascending());
                    EXPECT_EQ(a_pg->get_start_index(), 0);
                }
                {
                    auto b0_res = gt->get_pin_by_name("B(0)");
                    ASSERT_TRUE(b0_res.is_ok());
                    auto b0_pin = b0_res.get();
                    EXPECT_EQ(b0_pin->get_name(), "B(0)");
                    EXPECT_EQ(b0_pin->get_direction(), PinDirection::inout);
                    EXPECT_EQ(b0_pin->get_type(), PinType::none);
                    EXPECT_NE(b0_pin->get_group().first, nullptr);
                    EXPECT_EQ(b0_pin->get_group().second, 0);

                    auto b1_res = gt->get_pin_by_name("B(1)");
                    ASSERT_TRUE(b1_res.is_ok());
                    auto b1_pin = b1_res.get();
                    EXPECT_EQ(b1_pin->get_name(), "B(1)");
                    EXPECT_EQ(b1_pin->get_direction(), PinDirection::inout);
                    EXPECT_EQ(b1_pin->get_type(), PinType::none);
                    EXPECT_NE(b1_pin->get_group().first, nullptr);
                    EXPECT_EQ(b1_pin->get_group().second, 1);

                    auto b_res = gt->get_pin_group_by_name("B");
                    ASSERT_TRUE(b_res.is_ok());
                    auto b_pg = b_res.get();
                    EXPECT_EQ(b_pg->get_name(), "B");
                    EXPECT_EQ(b_pg->get_direction(), PinDirection::inout);
                    EXPECT_EQ(b_pg->get_type(), PinType::none);
                    EXPECT_EQ(b_pg->get_pins(), std::vector<GatePin*>({b0_pin, b1_pin}));
                    EXPECT_EQ(b_pg->get_pin_at_index(0), b0_pin);
                    EXPECT_EQ(b_pg->get_pin_at_index(1), b1_pin);
                    EXPECT_FALSE(b_pg->is_ascending());
                    EXPECT_EQ(b_pg->get_start_index(), 0);
                }
                {
                    auto c0_res = gt->get_pin_by_name("C(0)");
                    ASSERT_TRUE(c0_res.is_ok());
                    auto c0_pin = c0_res.get();
                    EXPECT_EQ(c0_pin->get_name(), "C(0)");
                    EXPECT_EQ(c0_pin->get_direction(), PinDirection::output);
                    EXPECT_EQ(c0_pin->get_type(), PinType::none);
                    EXPECT_NE(c0_pin->get_group().first, nullptr);
                    EXPECT_EQ(c0_pin->get_group().second, 0);

                    auto c1_res = gt->get_pin_by_name("C(1)");
                    ASSERT_TRUE(c1_res.is_ok());
                    auto c1_pin = c1_res.get();
                    EXPECT_EQ(c1_pin->get_name(), "C(1)");
                    EXPECT_EQ(c1_pin->get_direction(), PinDirection::output);
                    EXPECT_EQ(c1_pin->get_type(), PinType::none);
                    EXPECT_NE(c1_pin->get_group().first, nullptr);
                    EXPECT_EQ(c1_pin->get_group().second, 1);

                    auto c_res = gt->get_pin_group_by_name("C");
                    ASSERT_TRUE(c_res.is_ok());
                    auto c_pg = c_res.get();
                    EXPECT_EQ(c_pg->get_name(), "C");
                    EXPECT_EQ(c_pg->get_direction(), PinDirection::inout);
                    EXPECT_EQ(c_pg->get_type(), PinType::none);
                    EXPECT_EQ(c_pg->get_pins(), std::vector<GatePin*>({c0_pin, c1_pin}));
                    EXPECT_EQ(c_pg->get_pin_at_index(0), c0_pin);
                    EXPECT_EQ(c_pg->get_pin_at_index(1), c1_pin);
                    EXPECT_FALSE(c_pg->is_ascending());
                    EXPECT_EQ(c_pg->get_start_index(), 0);
                }
                {
                    auto o_res = gt->get_pin_by_name("O");
                    ASSERT_TRUE(o_res.is_ok());
                    auto o_pin = o_res.get();
                    EXPECT_EQ(o_pin->get_name(), "O");
                    EXPECT_EQ(o_pin->get_direction(), PinDirection::output);
                    EXPECT_EQ(o_pin->get_type(), PinType::none);
                    EXPECT_NE(o_pin->get_group().first, nullptr);
                    EXPECT_EQ(o_pin->get_group().second, 0);
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
                {
                    auto i1_res = gt->get_pin_by_name("I1");
                    ASSERT_TRUE(i1_res.is_ok());
                    auto i1_pin = i1_res.get();
                    EXPECT_EQ(i1_pin->get_name(), "I1");
                    EXPECT_EQ(i1_pin->get_direction(), PinDirection::input);
                    EXPECT_EQ(i1_pin->get_type(), PinType::none);
                    EXPECT_NE(i1_pin->get_group().first, nullptr);
                    EXPECT_EQ(i1_pin->get_group().second, 0);
                }
                {
                    auto i2_res = gt->get_pin_by_name("I2");
                    ASSERT_TRUE(i2_res.is_ok());
                    auto i2_pin = i2_res.get();
                    EXPECT_EQ(i2_pin->get_name(), "I2");
                    EXPECT_EQ(i2_pin->get_direction(), PinDirection::input);
                    EXPECT_EQ(i2_pin->get_type(), PinType::none);
                    EXPECT_NE(i2_pin->get_group().first, nullptr);
                    EXPECT_EQ(i2_pin->get_group().second, 0);
                }
                {
                    auto o_res = gt->get_pin_by_name("O");
                    ASSERT_TRUE(o_res.is_ok());
                    auto o_pin = o_res.get();
                    EXPECT_EQ(o_pin->get_name(), "O");
                    EXPECT_EQ(o_pin->get_direction(), PinDirection::output);
                    EXPECT_EQ(o_pin->get_type(), PinType::lut);
                    EXPECT_NE(o_pin->get_group().first, nullptr);
                    EXPECT_EQ(o_pin->get_group().second, 0);
                }

                // pin groups
                EXPECT_EQ(gt->get_pin_groups().size(), 3);

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
                {
                    auto i1_res = gt->get_pin_by_name("I1");
                    ASSERT_TRUE(i1_res.is_ok());
                    auto i1_pin = i1_res.get();
                    EXPECT_EQ(i1_pin->get_name(), "I1");
                    EXPECT_EQ(i1_pin->get_direction(), PinDirection::input);
                    EXPECT_EQ(i1_pin->get_type(), PinType::none);
                    EXPECT_NE(i1_pin->get_group().first, nullptr);
                    EXPECT_EQ(i1_pin->get_group().second, 0);
                }
                {
                    auto i2_res = gt->get_pin_by_name("I2");
                    ASSERT_TRUE(i2_res.is_ok());
                    auto i2_pin = i2_res.get();
                    EXPECT_EQ(i2_pin->get_name(), "I2");
                    EXPECT_EQ(i2_pin->get_direction(), PinDirection::input);
                    EXPECT_EQ(i2_pin->get_type(), PinType::none);
                    EXPECT_NE(i2_pin->get_group().first, nullptr);
                    EXPECT_EQ(i2_pin->get_group().second, 0);
                }
                {
                    auto o_res = gt->get_pin_by_name("O");
                    ASSERT_TRUE(o_res.is_ok());
                    auto o_pin = o_res.get();
                    EXPECT_EQ(o_pin->get_name(), "O");
                    EXPECT_EQ(o_pin->get_direction(), PinDirection::output);
                    EXPECT_EQ(o_pin->get_type(), PinType::lut);
                    EXPECT_NE(o_pin->get_group().first, nullptr);
                    EXPECT_EQ(o_pin->get_group().second, 0);
                }

                // pin groups
                EXPECT_EQ(gt->get_pin_groups().size(), 3);

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
                {
                    auto clk_res = gt->get_pin_by_name("CLK");
                    ASSERT_TRUE(clk_res.is_ok());
                    auto clk_pin = clk_res.get();
                    EXPECT_EQ(clk_pin->get_name(), "CLK");
                    EXPECT_EQ(clk_pin->get_direction(), PinDirection::input);
                    EXPECT_EQ(clk_pin->get_type(), PinType::clock);
                    EXPECT_NE(clk_pin->get_group().first, nullptr);
                    EXPECT_EQ(clk_pin->get_group().second, 0);
                }
                {
                    auto d_res = gt->get_pin_by_name("D");
                    ASSERT_TRUE(d_res.is_ok());
                    auto d_pin = d_res.get();
                    EXPECT_EQ(d_pin->get_name(), "D");
                    EXPECT_EQ(d_pin->get_direction(), PinDirection::input);
                    EXPECT_EQ(d_pin->get_type(), PinType::data);
                    EXPECT_NE(d_pin->get_group().first, nullptr);
                    EXPECT_EQ(d_pin->get_group().second, 0);
                }
                {
                    auto en_res = gt->get_pin_by_name("EN");
                    ASSERT_TRUE(en_res.is_ok());
                    auto en_pin = en_res.get();
                    EXPECT_EQ(en_pin->get_name(), "EN");
                    EXPECT_EQ(en_pin->get_direction(), PinDirection::input);
                    EXPECT_EQ(en_pin->get_type(), PinType::enable);
                    EXPECT_NE(en_pin->get_group().first, nullptr);
                    EXPECT_EQ(en_pin->get_group().second, 0);
                }
                {
                    auto r_res = gt->get_pin_by_name("R");
                    ASSERT_TRUE(r_res.is_ok());
                    auto r_pin = r_res.get();
                    EXPECT_EQ(r_pin->get_name(), "R");
                    EXPECT_EQ(r_pin->get_direction(), PinDirection::input);
                    EXPECT_EQ(r_pin->get_type(), PinType::reset);
                    EXPECT_NE(r_pin->get_group().first, nullptr);
                    EXPECT_EQ(r_pin->get_group().second, 0);
                }
                {
                    auto s_res = gt->get_pin_by_name("S");
                    ASSERT_TRUE(s_res.is_ok());
                    auto s_pin = s_res.get();
                    EXPECT_EQ(s_pin->get_name(), "S");
                    EXPECT_EQ(s_pin->get_direction(), PinDirection::input);
                    EXPECT_EQ(s_pin->get_type(), PinType::set);
                    EXPECT_NE(s_pin->get_group().first, nullptr);
                    EXPECT_EQ(s_pin->get_group().second, 0);
                }
                {
                    auto q_res = gt->get_pin_by_name("Q");
                    ASSERT_TRUE(q_res.is_ok());
                    auto q_pin = q_res.get();
                    EXPECT_EQ(q_pin->get_name(), "Q");
                    EXPECT_EQ(q_pin->get_direction(), PinDirection::output);
                    EXPECT_EQ(q_pin->get_type(), PinType::state);
                    EXPECT_NE(q_pin->get_group().first, nullptr);
                    EXPECT_EQ(q_pin->get_group().second, 0);
                }
                {
                    auto qn_res = gt->get_pin_by_name("QN");
                    ASSERT_TRUE(qn_res.is_ok());
                    auto qn_pin = qn_res.get();
                    EXPECT_EQ(qn_pin->get_name(), "QN");
                    EXPECT_EQ(qn_pin->get_direction(), PinDirection::output);
                    EXPECT_EQ(qn_pin->get_type(), PinType::neg_state);
                    EXPECT_NE(qn_pin->get_group().first, nullptr);
                    EXPECT_EQ(qn_pin->get_group().second, 0);
                }

                // pin groups
                EXPECT_EQ(gt->get_pin_groups().size(), 7);

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
                {
                    auto clk_res = gt->get_pin_by_name("CLK");
                    ASSERT_TRUE(clk_res.is_ok());
                    auto clk_pin = clk_res.get();
                    EXPECT_EQ(clk_pin->get_name(), "CLK");
                    EXPECT_EQ(clk_pin->get_direction(), PinDirection::input);
                    EXPECT_EQ(clk_pin->get_type(), PinType::clock);
                    EXPECT_NE(clk_pin->get_group().first, nullptr);
                    EXPECT_EQ(clk_pin->get_group().second, 0);
                }
                {
                    auto d_res = gt->get_pin_by_name("D");
                    ASSERT_TRUE(d_res.is_ok());
                    auto d_pin = d_res.get();
                    EXPECT_EQ(d_pin->get_name(), "D");
                    EXPECT_EQ(d_pin->get_direction(), PinDirection::input);
                    EXPECT_EQ(d_pin->get_type(), PinType::data);
                    EXPECT_NE(d_pin->get_group().first, nullptr);
                    EXPECT_EQ(d_pin->get_group().second, 0);
                }
                {
                    auto en_res = gt->get_pin_by_name("EN");
                    ASSERT_TRUE(en_res.is_ok());
                    auto en_pin = en_res.get();
                    EXPECT_EQ(en_pin->get_name(), "EN");
                    EXPECT_EQ(en_pin->get_direction(), PinDirection::input);
                    EXPECT_EQ(en_pin->get_type(), PinType::enable);
                    EXPECT_NE(en_pin->get_group().first, nullptr);
                    EXPECT_EQ(en_pin->get_group().second, 0);
                }
                {
                    auto r_res = gt->get_pin_by_name("R");
                    ASSERT_TRUE(r_res.is_ok());
                    auto r_pin = r_res.get();
                    EXPECT_EQ(r_pin->get_name(), "R");
                    EXPECT_EQ(r_pin->get_direction(), PinDirection::input);
                    EXPECT_EQ(r_pin->get_type(), PinType::reset);
                    EXPECT_NE(r_pin->get_group().first, nullptr);
                    EXPECT_EQ(r_pin->get_group().second, 0);
                }
                {
                    auto s_res = gt->get_pin_by_name("S");
                    ASSERT_TRUE(s_res.is_ok());
                    auto s_pin = s_res.get();
                    EXPECT_EQ(s_pin->get_name(), "S");
                    EXPECT_EQ(s_pin->get_direction(), PinDirection::input);
                    EXPECT_EQ(s_pin->get_type(), PinType::set);
                    EXPECT_NE(s_pin->get_group().first, nullptr);
                    EXPECT_EQ(s_pin->get_group().second, 0);
                }
                {
                    auto q_res = gt->get_pin_by_name("Q");
                    ASSERT_TRUE(q_res.is_ok());
                    auto q_pin = q_res.get();
                    EXPECT_EQ(q_pin->get_name(), "Q");
                    EXPECT_EQ(q_pin->get_direction(), PinDirection::output);
                    EXPECT_EQ(q_pin->get_type(), PinType::state);
                    EXPECT_NE(q_pin->get_group().first, nullptr);
                    EXPECT_EQ(q_pin->get_group().second, 0);
                }
                {
                    auto qn_res = gt->get_pin_by_name("QN");
                    ASSERT_TRUE(qn_res.is_ok());
                    auto qn_pin = qn_res.get();
                    EXPECT_EQ(qn_pin->get_name(), "QN");
                    EXPECT_EQ(qn_pin->get_direction(), PinDirection::output);
                    EXPECT_EQ(qn_pin->get_type(), PinType::neg_state);
                    EXPECT_NE(qn_pin->get_group().first, nullptr);
                    EXPECT_EQ(qn_pin->get_group().second, 0);
                }

                // pin groups
                EXPECT_EQ(gt->get_pin_groups().size(), 7);

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
                {
                    auto d_res = gt->get_pin_by_name("D");
                    ASSERT_TRUE(d_res.is_ok());
                    auto d_pin = d_res.get();
                    EXPECT_EQ(d_pin->get_name(), "D");
                    EXPECT_EQ(d_pin->get_direction(), PinDirection::input);
                    EXPECT_EQ(d_pin->get_type(), PinType::data);
                    EXPECT_NE(d_pin->get_group().first, nullptr);
                    EXPECT_EQ(d_pin->get_group().second, 0);
                }
                {
                    auto en_res = gt->get_pin_by_name("EN");
                    ASSERT_TRUE(en_res.is_ok());
                    auto en_pin = en_res.get();
                    EXPECT_EQ(en_pin->get_name(), "EN");
                    EXPECT_EQ(en_pin->get_direction(), PinDirection::input);
                    EXPECT_EQ(en_pin->get_type(), PinType::enable);
                    EXPECT_NE(en_pin->get_group().first, nullptr);
                    EXPECT_EQ(en_pin->get_group().second, 0);
                }
                {
                    auto r_res = gt->get_pin_by_name("R");
                    ASSERT_TRUE(r_res.is_ok());
                    auto r_pin = r_res.get();
                    EXPECT_EQ(r_pin->get_name(), "R");
                    EXPECT_EQ(r_pin->get_direction(), PinDirection::input);
                    EXPECT_EQ(r_pin->get_type(), PinType::reset);
                    EXPECT_NE(r_pin->get_group().first, nullptr);
                    EXPECT_EQ(r_pin->get_group().second, 0);
                }
                {
                    auto s_res = gt->get_pin_by_name("S");
                    ASSERT_TRUE(s_res.is_ok());
                    auto s_pin = s_res.get();
                    EXPECT_EQ(s_pin->get_name(), "S");
                    EXPECT_EQ(s_pin->get_direction(), PinDirection::input);
                    EXPECT_EQ(s_pin->get_type(), PinType::set);
                    EXPECT_NE(s_pin->get_group().first, nullptr);
                    EXPECT_EQ(s_pin->get_group().second, 0);
                }
                {
                    auto q_res = gt->get_pin_by_name("Q");
                    ASSERT_TRUE(q_res.is_ok());
                    auto q_pin = q_res.get();
                    EXPECT_EQ(q_pin->get_name(), "Q");
                    EXPECT_EQ(q_pin->get_direction(), PinDirection::output);
                    EXPECT_EQ(q_pin->get_type(), PinType::state);
                    EXPECT_NE(q_pin->get_group().first, nullptr);
                    EXPECT_EQ(q_pin->get_group().second, 0);
                }
                {
                    auto qn_res = gt->get_pin_by_name("QN");
                    ASSERT_TRUE(qn_res.is_ok());
                    auto qn_pin = qn_res.get();
                    EXPECT_EQ(qn_pin->get_name(), "QN");
                    EXPECT_EQ(qn_pin->get_direction(), PinDirection::output);
                    EXPECT_EQ(qn_pin->get_type(), PinType::neg_state);
                    EXPECT_NE(qn_pin->get_group().first, nullptr);
                    EXPECT_EQ(qn_pin->get_group().second, 0);
                }

                // pin groups
                EXPECT_EQ(gt->get_pin_groups().size(), 6);

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
                ASSERT_EQ(gt->get_pin_groups().size(), 3);

                {
                    auto di0_res = gt->get_pin_by_name("DI(0)");
                    ASSERT_TRUE(di0_res.is_ok());
                    auto di0_pin = di0_res.get();
                    EXPECT_EQ(di0_pin->get_name(), "DI(0)");
                    EXPECT_EQ(di0_pin->get_direction(), PinDirection::input);
                    EXPECT_EQ(di0_pin->get_type(), PinType::data);
                    EXPECT_NE(di0_pin->get_group().first, nullptr);
                    EXPECT_EQ(di0_pin->get_group().second, 0);

                    auto di1_res = gt->get_pin_by_name("DI(1)");
                    ASSERT_TRUE(di1_res.is_ok());
                    auto di1_pin = di1_res.get();
                    EXPECT_EQ(di1_pin->get_name(), "DI(1)");
                    EXPECT_EQ(di1_pin->get_direction(), PinDirection::input);
                    EXPECT_EQ(di1_pin->get_type(), PinType::data);
                    EXPECT_NE(di1_pin->get_group().first, nullptr);
                    EXPECT_EQ(di1_pin->get_group().second, 1);

                    auto di2_res = gt->get_pin_by_name("DI(2)");
                    ASSERT_TRUE(di2_res.is_ok());
                    auto di2_pin = di2_res.get();
                    EXPECT_EQ(di2_pin->get_name(), "DI(2)");
                    EXPECT_EQ(di2_pin->get_direction(), PinDirection::input);
                    EXPECT_EQ(di2_pin->get_type(), PinType::data);
                    EXPECT_NE(di2_pin->get_group().first, nullptr);
                    EXPECT_EQ(di2_pin->get_group().second, 2);

                    auto di_res = gt->get_pin_group_by_name("DI");
                    ASSERT_TRUE(di_res.is_ok());
                    auto di_pg = di_res.get();
                    EXPECT_EQ(di_pg->get_name(), "DI");
                    EXPECT_EQ(di_pg->get_direction(), PinDirection::input);
                    EXPECT_EQ(di_pg->get_type(), PinType::data);
                    EXPECT_EQ(di_pg->get_pins(), std::vector<GatePin*>({di0_pin, di1_pin, di2_pin}));
                    EXPECT_EQ(di_pg->get_pin_at_index(0), di0_pin);
                    EXPECT_EQ(di_pg->get_pin_at_index(1), di1_pin);
                    EXPECT_EQ(di_pg->get_pin_at_index(2), di2_pin);
                    EXPECT_FALSE(di_pg->is_ascending());
                    EXPECT_EQ(di_pg->get_start_index(), 0);
                }
                {
                    auto a0_res = gt->get_pin_by_name("A(0)");
                    ASSERT_TRUE(a0_res.is_ok());
                    auto a0_pin = a0_res.get();
                    EXPECT_EQ(a0_pin->get_name(), "A(0)");
                    EXPECT_EQ(a0_pin->get_direction(), PinDirection::input);
                    EXPECT_EQ(a0_pin->get_type(), PinType::address);
                    EXPECT_NE(a0_pin->get_group().first, nullptr);
                    EXPECT_EQ(a0_pin->get_group().second, 0);

                    auto a1_res = gt->get_pin_by_name("A(1)");
                    ASSERT_TRUE(a1_res.is_ok());
                    auto a1_pin = a1_res.get();
                    EXPECT_EQ(a1_pin->get_name(), "A(1)");
                    EXPECT_EQ(a1_pin->get_direction(), PinDirection::input);
                    EXPECT_EQ(a1_pin->get_type(), PinType::address);
                    EXPECT_NE(a1_pin->get_group().first, nullptr);
                    EXPECT_EQ(a1_pin->get_group().second, 1);

                    auto a2_res = gt->get_pin_by_name("A(2)");
                    ASSERT_TRUE(a2_res.is_ok());
                    auto a2_pin = a2_res.get();
                    EXPECT_EQ(a2_pin->get_name(), "A(2)");
                    EXPECT_EQ(a2_pin->get_direction(), PinDirection::input);
                    EXPECT_EQ(a2_pin->get_type(), PinType::address);
                    EXPECT_NE(a2_pin->get_group().first, nullptr);
                    EXPECT_EQ(a2_pin->get_group().second, 2);

                    auto a_res = gt->get_pin_group_by_name("A");
                    ASSERT_TRUE(a_res.is_ok());
                    auto a_pg = a_res.get();
                    EXPECT_EQ(a_pg->get_name(), "A");
                    EXPECT_EQ(a_pg->get_direction(), PinDirection::input);
                    EXPECT_EQ(a_pg->get_type(), PinType::address);
                    EXPECT_EQ(a_pg->get_pins(), std::vector<GatePin*>({a0_pin, a1_pin, a2_pin}));
                    EXPECT_EQ(a_pg->get_pin_at_index(0), a0_pin);
                    EXPECT_EQ(a_pg->get_pin_at_index(1), a1_pin);
                    EXPECT_EQ(a_pg->get_pin_at_index(2), a2_pin);
                    EXPECT_FALSE(a_pg->is_ascending());
                    EXPECT_EQ(a_pg->get_start_index(), 0);
                }
                {
                    auto do0_res = gt->get_pin_by_name("DO(0)");
                    ASSERT_TRUE(do0_res.is_ok());
                    auto do0_pin = do0_res.get();
                    EXPECT_EQ(do0_pin->get_name(), "DO(0)");
                    EXPECT_EQ(do0_pin->get_direction(), PinDirection::output);
                    EXPECT_EQ(do0_pin->get_type(), PinType::data);
                    EXPECT_NE(do0_pin->get_group().first, nullptr);
                    EXPECT_EQ(do0_pin->get_group().second, 0);

                    auto do1_res = gt->get_pin_by_name("DO(1)");
                    ASSERT_TRUE(do1_res.is_ok());
                    auto do1_pin = do1_res.get();
                    EXPECT_EQ(do1_pin->get_name(), "DO(1)");
                    EXPECT_EQ(do1_pin->get_direction(), PinDirection::output);
                    EXPECT_EQ(do1_pin->get_type(), PinType::data);
                    EXPECT_NE(do1_pin->get_group().first, nullptr);
                    EXPECT_EQ(do1_pin->get_group().second, 1);

                    auto do2_res = gt->get_pin_by_name("DO(2)");
                    ASSERT_TRUE(do2_res.is_ok());
                    auto do2_pin = do2_res.get();
                    EXPECT_EQ(do2_pin->get_name(), "DO(2)");
                    EXPECT_EQ(do2_pin->get_direction(), PinDirection::output);
                    EXPECT_EQ(do2_pin->get_type(), PinType::data);
                    EXPECT_NE(do2_pin->get_group().first, nullptr);
                    EXPECT_EQ(do2_pin->get_group().second, 2);

                    auto do_res = gt->get_pin_group_by_name("DO");
                    ASSERT_TRUE(do_res.is_ok());
                    auto do_pg = do_res.get();
                    EXPECT_EQ(do_pg->get_name(), "DO");
                    EXPECT_EQ(do_pg->get_direction(), PinDirection::output);
                    EXPECT_EQ(do_pg->get_type(), PinType::data);
                    EXPECT_EQ(do_pg->get_pins(), std::vector<GatePin*>({do0_pin, do1_pin, do2_pin}));
                    EXPECT_EQ(do_pg->get_pin_at_index(0), do0_pin);
                    EXPECT_EQ(do_pg->get_pin_at_index(1), do1_pin);
                    EXPECT_EQ(do_pg->get_pin_at_index(2), do2_pin);
                    EXPECT_FALSE(do_pg->is_ascending());
                    EXPECT_EQ(do_pg->get_start_index(), 0);
                }

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
