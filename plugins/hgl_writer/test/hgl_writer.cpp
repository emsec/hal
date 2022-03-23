#include "hgl_writer/hgl_writer.h"
#include "hal_core/netlist/gate_library/gate_type_component/gate_type_component.h"
#include "hal_core/netlist/gate_library/gate_type_component/ff_component.h"
#include "hal_core/netlist/gate_library/gate_type_component/lut_component.h"
#include "hal_core/netlist/gate_library/gate_type_component/latch_component.h"
#include "hal_core/netlist/gate_library/gate_type_component/init_component.h"
#include "hgl_parser/hgl_parser.h"
#include "netlist_test_utils.h"

namespace hal
{
    namespace
    {
        static std::unique_ptr<GateLibrary> create_gate_library(const std::filesystem::path& file_path)
        {
            auto gl = std::make_unique<GateLibrary>(file_path, "example_library");

            {
                GateType* gt = gl->create_gate_type("gt_combinational", {GateTypeProperty::combinational});

                if (gt->create_pin("VDD", PinDirection::input, PinType::power).is_error())
                {
                    return nullptr;
                }
                if (gt->create_pin("GND", PinDirection::input, PinType::ground).is_error())
                {
                    return nullptr;
                }
                if (gt->create_pin("A", PinDirection::input).is_error())
                {
                    return nullptr;
                }
                if (gt->create_pin("B", PinDirection::inout).is_error())
                {
                    return nullptr;
                }
                if (gt->create_pin("VDD", PinDirection::output).is_error())
                {
                    return nullptr;
                }

                gt->add_boolean_function("O", BooleanFunction::from_string("A & B").get());
                gt->add_boolean_function("O_undefined", BooleanFunction::from_string("!A & B").get());
                gt->add_boolean_function("O_tristate", BooleanFunction::from_string("!A & !B").get());
            }

            {
                GateType* gt = gl->create_gate_type("gt_group", {GateTypeProperty::combinational});

                std::vector<GatePin*> a_pins, b_pins, c_pins;
                if (gt->create_pin("VDD", PinDirection::input, PinType::power).is_error())
                {
                    return nullptr;
                }
                if (gt->create_pin("GND", PinDirection::input, PinType::ground).is_error())
                {
                    return nullptr;
                }
                if (auto res = gt->create_pin("A(0)", PinDirection::input); res.is_error())
                {
                    return nullptr;
                }
                else 
                {
                    a_pins.push_back(res.get());
                }
                if (auto res = gt->create_pin("A(1)", PinDirection::input); res.is_error())
                {
                    return nullptr;
                }
                else 
                {
                    a_pins.push_back(res.get());
                }
                if (auto res = gt->create_pin("B(0)", PinDirection::inout); res.is_error())
                {
                    return nullptr;
                }
                else 
                {
                    b_pins.push_back(res.get());
                }
                if (auto res = gt->create_pin("B(1)", PinDirection::inout); res.is_error())
                {
                    return nullptr;
                }
                else 
                {
                    b_pins.push_back(res.get());
                }
                if (auto res = gt->create_pin("C(0)", PinDirection::output); res.is_error())
                {
                    return nullptr;
                }
                else 
                {
                    c_pins.push_back(res.get());
                }
                if (auto res = gt->create_pin("C(1)", PinDirection::output); res.is_error())
                {
                    return nullptr;
                }
                else 
                {
                    c_pins.push_back(res.get());
                }
                if (gt->create_pin("O", PinDirection::output).is_error())
                {
                    return nullptr;
                }

                if (gt->create_pin_group("A", a_pins, PinDirection::input, PinType::none, false, 0).is_error())
                {
                    return nullptr;
                }
                if (gt->create_pin_group("B", b_pins, PinDirection::inout, PinType::none, false, 5).is_error())
                {
                    return nullptr;
                }
                if (gt->create_pin_group("C", c_pins, PinDirection::output, PinType::none, true, 0).is_error())
                {
                    return nullptr;
                }

                gt->add_boolean_function("O", BooleanFunction::from_string("A(1) & B(0)").get());
                gt->add_boolean_function("O_undefined", BooleanFunction::from_string("!A(0) & B(0)").get());
                gt->add_boolean_function("O_tristate", BooleanFunction::from_string("!A(1) & !B(1)").get());
            }

            {
                GateType* gt = gl->create_gate_type("gt_lut_asc", {GateTypeProperty::lut, GateTypeProperty::combinational}, GateTypeComponent::create_lut_component(GateTypeComponent::create_init_component("generic", {"INIT"}), true));

                if (gt->create_pin("I1", PinDirection::input).is_error())
                {
                    return nullptr;
                }
                if (gt->create_pin("I2", PinDirection::inout).is_error())
                {
                    return nullptr;
                }
                if (gt->create_pin("O", PinDirection::output, PinType::lut).is_error())
                {
                    return nullptr;
                }
            }

            {
                GateType* gt = gl->create_gate_type("gt_lut_desc", {GateTypeProperty::lut, GateTypeProperty::combinational}, GateTypeComponent::create_lut_component(GateTypeComponent::create_init_component("generic", {"INIT"}), false));

                if (gt->create_pin("I1", PinDirection::input).is_error())
                {
                    return nullptr;
                }
                if (gt->create_pin("I2", PinDirection::inout).is_error())
                {
                    return nullptr;
                }
                if (gt->create_pin("O", PinDirection::output, PinType::lut).is_error())
                {
                    return nullptr;
                }
            }

            {
                GateType* gt = gl->create_gate_type("gt_ff", {GateTypeProperty::ff}, GateTypeComponent::create_ff_component(GateTypeComponent::create_state_component(nullptr, "IQ", "IQN"), BooleanFunction::Var("D"), BooleanFunction::from_string("CLK & EN").get()));
                FFComponent* ff_component = gt->get_component_as<FFComponent>([](const GateTypeComponent* component){ return component->get_type() == GateTypeComponent::ComponentType::ff; });
                assert(ff_component != nullptr);

                if (gt->create_pin("CLK", PinDirection::input, PinType::clock).is_error())
                {
                    return nullptr;
                }
                if (gt->create_pin("D", PinDirection::input, PinType::data).is_error())
                {
                    return nullptr;
                }
                if (gt->create_pin("EN", PinDirection::input, PinType::enable).is_error())
                {
                    return nullptr;
                }
                if (gt->create_pin("R", PinDirection::input, PinType::reset).is_error())
                {
                    return nullptr;
                }
                if (gt->create_pin("S", PinDirection::input, PinType::set).is_error())
                {
                    return nullptr;
                }
                if (gt->create_pin("Q", PinDirection::output, PinType::state).is_error())
                {
                    return nullptr;
                }
                if (gt->create_pin("QN", PinDirection::output, PinType::neg_state).is_error())
                {
                    return nullptr;
                }

                ff_component->set_async_reset_function(BooleanFunction::Var("R"));
                ff_component->set_async_set_function(BooleanFunction::Var("S"));
                ff_component->set_async_set_reset_behavior(AsyncSetResetBehavior::L, AsyncSetResetBehavior::H);
            }

            {
                GateType* gt = gl->create_gate_type("gt_ff_init", {GateTypeProperty::ff}, GateTypeComponent::create_ff_component(GateTypeComponent::create_state_component(GateTypeComponent::create_init_component("generic", {"INIT"}), "IQ", "IQN"), BooleanFunction::Var("D"), BooleanFunction::from_string("CLK & EN").get()));
                FFComponent* ff_component = gt->get_component_as<FFComponent>([](const GateTypeComponent* component){ return component->get_type() == GateTypeComponent::ComponentType::ff; });
                assert(ff_component != nullptr);

                if (gt->create_pin("CLK", PinDirection::input, PinType::clock).is_error())
                {
                    return nullptr;
                }
                if (gt->create_pin("D", PinDirection::input, PinType::data).is_error())
                {
                    return nullptr;
                }
                if (gt->create_pin("EN", PinDirection::input, PinType::enable).is_error())
                {
                    return nullptr;
                }
                if (gt->create_pin("R", PinDirection::input, PinType::reset).is_error())
                {
                    return nullptr;
                }
                if (gt->create_pin("S", PinDirection::input, PinType::set).is_error())
                {
                    return nullptr;
                }
                if (gt->create_pin("Q", PinDirection::output, PinType::state).is_error())
                {
                    return nullptr;
                }
                if (gt->create_pin("QN", PinDirection::output, PinType::neg_state).is_error())
                {
                    return nullptr;
                }

                ff_component->set_async_reset_function(BooleanFunction::Var("R"));
                ff_component->set_async_set_function(BooleanFunction::Var("S"));
                ff_component->set_async_set_reset_behavior(AsyncSetResetBehavior::L, AsyncSetResetBehavior::H);
            }

            {
                GateType* gt = gl->create_gate_type("gt_latch", {GateTypeProperty::latch}, GateTypeComponent::create_latch_component(GateTypeComponent::create_state_component(nullptr, "IQ", "IQN")));
                LatchComponent* latch_component = gt->get_component_as<LatchComponent>([](const GateTypeComponent* component){ return component->get_type() == GateTypeComponent::ComponentType::latch; });
                assert(latch_component != nullptr);

                if (gt->create_pin("D", PinDirection::input, PinType::data).is_error())
                {
                    return nullptr;
                }
                if (gt->create_pin("EN", PinDirection::input, PinType::enable).is_error())
                {
                    return nullptr;
                }
                if (gt->create_pin("R", PinDirection::input, PinType::reset).is_error())
                {
                    return nullptr;
                }
                if (gt->create_pin("S", PinDirection::input, PinType::set).is_error())
                {
                    return nullptr;
                }
                if (gt->create_pin("Q", PinDirection::output, PinType::state).is_error())
                {
                    return nullptr;
                }
                if (gt->create_pin("QN", PinDirection::output, PinType::neg_state).is_error())
                {
                    return nullptr;
                }

                latch_component->set_async_set_reset_behavior(AsyncSetResetBehavior::L, AsyncSetResetBehavior::H);
            }

            {
                GateType* gt = gl->create_gate_type("gt_ram", {GateTypeProperty::ram});

                std::vector<GatePin*> din_pins, addr_pins, dout_pins;
                if (auto res = gt->create_pin("DI(0)", PinDirection::input, PinType::data); res.is_error())
                {
                    return nullptr;
                }
                else 
                {
                    din_pins.push_back(res.get());
                }
                if (auto res = gt->create_pin("DI(1)", PinDirection::input, PinType::data); res.is_error())
                {
                    return nullptr;
                }
                else 
                {
                    din_pins.push_back(res.get());
                }
                if (auto res = gt->create_pin("DI(2)", PinDirection::input, PinType::data); res.is_error())
                {
                    return nullptr;
                }
                else 
                {
                    din_pins.push_back(res.get());
                }

                if (auto res = gt->create_pin("A(0)", PinDirection::input, PinType::address); res.is_error())
                {
                    return nullptr;
                }
                else 
                {
                    addr_pins.push_back(res.get());
                }
                if (auto res = gt->create_pin("A(1)", PinDirection::input, PinType::address); res.is_error())
                {
                    return nullptr;
                }
                else 
                {
                    addr_pins.push_back(res.get());
                }
                if (auto res = gt->create_pin("A(2)", PinDirection::input, PinType::address); res.is_error())
                {
                    return nullptr;
                }
                else 
                {
                    addr_pins.push_back(res.get());
                }

                if (auto res = gt->create_pin("DO(0)", PinDirection::output, PinType::data); res.is_error())
                {
                    return nullptr;
                }
                else 
                {
                    dout_pins.push_back(res.get());
                }
                if (auto res = gt->create_pin("DO(1)", PinDirection::output, PinType::data); res.is_error())
                {
                    return nullptr;
                }
                else 
                {
                    dout_pins.push_back(res.get());
                }
                if (auto res = gt->create_pin("DO(2)", PinDirection::output, PinType::data); res.is_error())
                {
                    return nullptr;
                }
                else 
                {
                    dout_pins.push_back(res.get());
                }

                if (gt->create_pin_group("DI", din_pins, PinDirection::input, PinType::data).is_error())
                {
                    return nullptr;
                }
                if (gt->create_pin_group("A", addr_pins, PinDirection::input, PinType::address).is_error())
                {
                    return nullptr;
                }
                if (gt->create_pin_group("DO", dout_pins, PinDirection::output, PinType::data).is_error())
                {
                    return nullptr;
                }
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
            test_utils::create_sandbox_directory();
        }

        virtual void TearDown()
        {
            test_utils::remove_sandbox_directory();
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

        std::string path_lib = test_utils::create_sandbox_path("test.hgl");

        std::unique_ptr<GateLibrary> gl_original = create_gate_library(path_lib);

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
            EXPECT_EQ(gt1->get_properties(), gt2->get_properties());
            EXPECT_EQ(gt1->get_input_pins(), gt2->get_input_pins());
            EXPECT_EQ(gt1->get_output_pins(), gt2->get_output_pins());
            EXPECT_EQ(gt1->get_pin_groups(), gt2->get_pin_groups());
            EXPECT_EQ(gt1->get_pin_types(), gt2->get_pin_types());
            EXPECT_EQ(gt1->get_boolean_functions(), gt2->get_boolean_functions());

            if(const LUTComponent* lut_component1 = gt1->get_component_as<LUTComponent>([](const GateTypeComponent* component){ return component->get_type() == GateTypeComponent::ComponentType::lut; }); lut_component1 != nullptr) 
            {
                const LUTComponent* lut_component2 = gt2->get_component_as<LUTComponent>([](const GateTypeComponent* component){ return component->get_type() == GateTypeComponent::ComponentType::lut; });
                ASSERT_NE(lut_component2, nullptr);
                EXPECT_EQ(lut_component1->is_init_ascending(), lut_component2->is_init_ascending());
            }

            if(const FFComponent* ff_component1 = gt1->get_component_as<FFComponent>([](const GateTypeComponent* component){ return component->get_type() == GateTypeComponent::ComponentType::ff; }); ff_component1 != nullptr) 
            {
                const FFComponent* ff_component2 = gt2->get_component_as<FFComponent>([](const GateTypeComponent* component){ return component->get_type() == GateTypeComponent::ComponentType::ff; });
                ASSERT_NE(ff_component2, nullptr);
                EXPECT_EQ(ff_component1->get_next_state_function(), ff_component2->get_next_state_function());
                EXPECT_EQ(ff_component1->get_clock_function(), ff_component2->get_clock_function());
                EXPECT_EQ(ff_component1->get_async_reset_function(), ff_component2->get_async_reset_function());
                EXPECT_EQ(ff_component1->get_async_set_function(), ff_component2->get_async_set_function());
                EXPECT_EQ(ff_component1->get_async_set_reset_behavior(), ff_component2->get_async_set_reset_behavior());
            }

            if(const LatchComponent* latch_component1 = gt1->get_component_as<LatchComponent>([](const GateTypeComponent* component){ return component->get_type() == GateTypeComponent::ComponentType::latch; }); latch_component1 != nullptr) 
            {
                const LatchComponent* latch_component2 = gt2->get_component_as<LatchComponent>([](const GateTypeComponent* component){ return component->get_type() == GateTypeComponent::ComponentType::latch; });
                ASSERT_NE(latch_component2, nullptr);
                EXPECT_EQ(latch_component1->get_data_in_function(), latch_component2->get_data_in_function());
                EXPECT_EQ(latch_component1->get_enable_function(), latch_component2->get_enable_function());
                EXPECT_EQ(latch_component1->get_async_reset_function(), latch_component2->get_async_reset_function());
                EXPECT_EQ(latch_component1->get_async_set_function(), latch_component2->get_async_set_function());
                EXPECT_EQ(latch_component1->get_async_set_reset_behavior(), latch_component2->get_async_set_reset_behavior());
            }

            if(const InitComponent* init_component1 = gt1->get_component_as<InitComponent>([](const GateTypeComponent* component){ return component->get_type() == GateTypeComponent::ComponentType::init; }); init_component1 != nullptr) 
            {
                const InitComponent* init_component2 = gt2->get_component_as<InitComponent>([](const GateTypeComponent* component){ return component->get_type() == GateTypeComponent::ComponentType::init; });
                ASSERT_NE(init_component2, nullptr);
                EXPECT_EQ(init_component1->get_init_category(), init_component2->get_init_category());
                EXPECT_EQ(init_component1->get_init_identifiers(), init_component2->get_init_identifiers());
            }
        }

        TEST_END
    }
}    //namespace hal
