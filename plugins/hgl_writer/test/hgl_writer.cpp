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

                gt->add_input_pins({"VDD", "GND", "A", "B"});
                gt->add_output_pins({"B", "O"});

                gt->assign_pin_type("VDD", PinType::power);
                gt->assign_pin_type("GND", PinType::ground);

                gt->add_boolean_function("O", BooleanFunction::from_string("A & B").get());
                gt->add_boolean_function("O_undefined", BooleanFunction::from_string("!A & B").get());
                gt->add_boolean_function("O_tristate", BooleanFunction::from_string("!A & !B").get());
            }

            {
                GateType* gt = gl->create_gate_type("gt_group", {GateTypeProperty::combinational});

                gt->add_input_pins({"VDD", "GND", "A(0)", "A(1)", "B(0)", "B(1)"});
                gt->add_output_pins({"B(0)", "B(1)", "C(0)", "C(1)", "O"});

                gt->assign_pin_group("A", {{0, "A(0)"}, {1, "A(1)"}});
                gt->assign_pin_group("B", {{0, "B(0)"}, {1, "B(1)"}});
                gt->assign_pin_group("C", {{0, "C(0)"}, {1, "C(1)"}});

                gt->assign_pin_type("VDD", PinType::power);
                gt->assign_pin_type("GND", PinType::ground);

                gt->add_boolean_function("O", BooleanFunction::from_string("A(1) & B(0)").get());
                gt->add_boolean_function("O_undefined", BooleanFunction::from_string("!A(0) & B(0)").get());
                gt->add_boolean_function("O_tristate", BooleanFunction::from_string("!A(1) & !B(1)").get());
            }

            {
                GateType* gt = gl->create_gate_type("gt_lut_asc", {GateTypeProperty::lut, GateTypeProperty::combinational}, GateTypeComponent::create_lut_component(GateTypeComponent::create_init_component("generic", {"INIT"}), true));

                gt->add_input_pins({"I1", "I2"});
                gt->add_output_pins({"O"});

                gt->assign_pin_type("O", PinType::lut);
            }

            {
                GateType* gt = gl->create_gate_type("gt_lut_desc", {GateTypeProperty::lut, GateTypeProperty::combinational}, GateTypeComponent::create_lut_component(GateTypeComponent::create_init_component("generic", {"INIT"}), false));


                gt->add_input_pins({"I1", "I2"});
                gt->add_output_pins({"O"});

                gt->assign_pin_type("O", PinType::lut);
            }

            {
                GateType* gt = gl->create_gate_type("gt_ff", {GateTypeProperty::ff}, GateTypeComponent::create_ff_component(GateTypeComponent::create_state_component(nullptr, "IQ", "IQN"), BooleanFunction::Var("D"), BooleanFunction::from_string("CLK & EN").get()));
                FFComponent* ff_component = gt->get_component_as<FFComponent>([](const GateTypeComponent* component){ return component->get_type() == GateTypeComponent::ComponentType::ff; });
                assert(ff_component != nullptr);

                gt->add_input_pins({"CLK", "D", "EN", "R", "S"});
                gt->add_output_pins({"Q", "QN"});

                gt->assign_pin_type("CLK", PinType::clock);
                gt->assign_pin_type("D", PinType::data);
                gt->assign_pin_type("EN", PinType::enable);
                gt->assign_pin_type("R", PinType::reset);
                gt->assign_pin_type("S", PinType::set);
                gt->assign_pin_type("Q", PinType::state);
                gt->assign_pin_type("QN", PinType::neg_state);

                ff_component->set_async_reset_function(BooleanFunction::Var("R"));
                ff_component->set_async_set_function(BooleanFunction::Var("S"));
                ff_component->set_async_set_reset_behavior(AsyncSetResetBehavior::L, AsyncSetResetBehavior::H);
            }

            {
                GateType* gt = gl->create_gate_type("gt_ff_init", {GateTypeProperty::ff}, GateTypeComponent::create_ff_component(GateTypeComponent::create_state_component(GateTypeComponent::create_init_component("generic", {"INIT"}), "IQ", "IQN"), BooleanFunction::Var("D"), BooleanFunction::from_string("CLK & EN").get()));
                FFComponent* ff_component = gt->get_component_as<FFComponent>([](const GateTypeComponent* component){ return component->get_type() == GateTypeComponent::ComponentType::ff; });
                assert(ff_component != nullptr);

                gt->add_input_pins({"CLK", "D", "EN", "R", "S"});
                gt->add_output_pins({"Q", "QN"});

                gt->assign_pin_type("CLK", PinType::clock);
                gt->assign_pin_type("D", PinType::data);
                gt->assign_pin_type("EN", PinType::enable);
                gt->assign_pin_type("R", PinType::reset);
                gt->assign_pin_type("S", PinType::set);
                gt->assign_pin_type("Q", PinType::state);
                gt->assign_pin_type("QN", PinType::neg_state);

                ff_component->set_async_reset_function(BooleanFunction::Var("R"));
                ff_component->set_async_set_function(BooleanFunction::Var("S"));
                ff_component->set_async_set_reset_behavior(AsyncSetResetBehavior::L, AsyncSetResetBehavior::H);
            }

            {
                GateType* gt = gl->create_gate_type("gt_latch", {GateTypeProperty::latch}, GateTypeComponent::create_latch_component(GateTypeComponent::create_state_component(nullptr, "IQ", "IQN")));
                LatchComponent* latch_component = gt->get_component_as<LatchComponent>([](const GateTypeComponent* component){ return component->get_type() == GateTypeComponent::ComponentType::latch; });
                assert(latch_component != nullptr);

                gt->add_input_pins({"D", "EN", "R", "S"});
                gt->add_output_pins({"Q", "QN"});

                gt->assign_pin_type("D", PinType::data);
                gt->assign_pin_type("EN", PinType::enable);
                gt->assign_pin_type("R", PinType::reset);
                gt->assign_pin_type("S", PinType::set);
                gt->assign_pin_type("Q", PinType::state);
                gt->assign_pin_type("QN", PinType::neg_state);

                latch_component->set_data_in_function(BooleanFunction::Var("D"));
                latch_component->set_enable_function(BooleanFunction::Var("EN"));
                latch_component->set_async_reset_function(BooleanFunction::Var("R"));
                latch_component->set_async_set_function(BooleanFunction::Var("S"));
                latch_component->set_async_set_reset_behavior(AsyncSetResetBehavior::L, AsyncSetResetBehavior::H);
            }

            {
                GateType* gt = gl->create_gate_type("gt_ram", {GateTypeProperty::ram});

                gt->add_input_pins({"DI(0)", "DI(1)", "DI(2)", "A(0)", "A(1)", "A(2)"});
                gt->add_output_pins({"DO(0)", "DO(1)", "DO(2)"});

                gt->assign_pin_group("DI", {{0, "DI(0)"}, {1, "DI(1)"}, {2, "DI(2)"}});
                gt->assign_pin_group("A", {{0, "A(0)"}, {1, "A(1)"}, {2, "A(2)"}});
                gt->assign_pin_group("DO", {{0, "DO(0)"}, {1, "DO(1)"}, {2, "DO(2)"}});

                gt->assign_pin_type("DI(0)", PinType::data);
                gt->assign_pin_type("DI(1)", PinType::data);
                gt->assign_pin_type("DI(2)", PinType::data);
                gt->assign_pin_type("A(0)", PinType::address);
                gt->assign_pin_type("A(1)", PinType::address);
                gt->assign_pin_type("A(2)", PinType::address);
                gt->assign_pin_type("DO(0)", PinType::data);
                gt->assign_pin_type("DO(1)", PinType::data);
                gt->assign_pin_type("DO(2)", PinType::data);
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
