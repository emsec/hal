#include "xilinx_toolbox/preprocessing.h"

#include "hal_core/netlist/gate_library/gate_library.h"
#include "hal_core/netlist/gate_library/gate_type.h"
#include "hal_core/netlist/gate_library/gate_type_component/gate_type_component.h"
#include "hal_core/netlist/gate_library/gate_type_component/lut_component.h"
#include "hal_core/netlist/module.h"
#include "hal_core/netlist/netlist.h"
#include "netlist_test_utils.h"

#include "gtest/gtest.h"

namespace hal
{
    class XilinxPreprocessingTest : public ::testing::Test
    {
    protected:
        std::unique_ptr<GateLibrary> m_gl;

        // Gate types stored for easy access in tests.
        GateType* m_lut6_2_type = nullptr;
        GateType* m_lut5_type   = nullptr;
        GateType* m_lut6_type   = nullptr;
        GateType* m_buf_type    = nullptr;    // generic single-input/output gate used as sink/source

        void SetUp() override
        {
            NO_COUT_BLOCK;
            test_utils::init_log_channels();

            m_gl = std::make_unique<GateLibrary>("no_path", "xilinx_test_lib");

            // LUT5: I0-I4 → O, 32-bit INIT
            m_lut5_type = m_gl->create_gate_type("LUT5", {GateTypeProperty::combinational, GateTypeProperty::c_lut}, GateTypeComponent::create_lut_component(true));
            for (int i = 0; i < 5; ++i)
                m_lut5_type->create_pin("I" + std::to_string(i), PinDirection::input);
            m_lut5_type->create_pin("O", PinDirection::output, PinType::lut);
            m_lut5_type->get_component_as<LUTComponent>([](const GateTypeComponent* c) { return LUTComponent::is_class_of(c); })->add_output_pin_config("O", "INIT", 0, 32);

            // LUT6: I0-I5 → O, 64-bit INIT
            m_lut6_type = m_gl->create_gate_type("LUT6", {GateTypeProperty::combinational, GateTypeProperty::c_lut}, GateTypeComponent::create_lut_component(true));
            for (int i = 0; i < 6; ++i)
                m_lut6_type->create_pin("I" + std::to_string(i), PinDirection::input);
            m_lut6_type->create_pin("O", PinDirection::output, PinType::lut);
            m_lut6_type->get_component_as<LUTComponent>([](const GateTypeComponent* c) { return LUTComponent::is_class_of(c); })->add_output_pin_config("O", "INIT", 0, 64);

            // LUT6_2: I0-I5 inputs; O6 = full 64-bit INIT, O5 = lower 32 bits of the same INIT
            m_lut6_2_type = m_gl->create_gate_type("LUT6_2", {GateTypeProperty::combinational, GateTypeProperty::c_lut}, GateTypeComponent::create_lut_component(true));
            for (int i = 0; i < 6; ++i)
                m_lut6_2_type->create_pin("I" + std::to_string(i), PinDirection::input);
            m_lut6_2_type->create_pin("O5", PinDirection::output, PinType::lut);
            m_lut6_2_type->create_pin("O6", PinDirection::output, PinType::lut);
            {
                auto* lc = m_lut6_2_type->get_component_as<LUTComponent>([](const GateTypeComponent* c) { return LUTComponent::is_class_of(c); });
                lc->add_output_pin_config("O6", "INIT", 0, 64);
                lc->add_output_pin_config("O5", "INIT", 0, 32);
            }

            // BUF: A → O, used as a generic sink/source in tests
            m_buf_type = m_gl->create_gate_type("BUF", {GateTypeProperty::combinational});
            m_buf_type->create_pin("A", PinDirection::input);
            m_buf_type->create_pin("O", PinDirection::output);
        }

        void TearDown() override {}

        std::unique_ptr<Netlist> make_netlist() const
        {
            return std::make_unique<Netlist>(m_gl.get());
        }

        // Create a LUT6_2 gate with all 6 inputs driven by global input nets and return the gate.
        // The output nets are created but have no destinations by default.
        Gate* make_lut6_2(Netlist* nl, const std::string& name, const std::string& init = "FEDCBA9876543210") const
        {
            Gate* g = nl->create_gate(m_lut6_2_type, name);
            for (int i = 0; i < 6; ++i)
                test_utils::connect_global_in(nl, g, "I" + std::to_string(i));
            g->set_init_string("O6", init);
            return g;
        }

        // Add a BUF sink connected to the given output pin of g; returns the net.
        Net* add_sink(Netlist* nl, Gate* g, const std::string& out_pin) const
        {
            Gate* sink = nl->create_gate(m_buf_type, g->get_name() + "_sink_" + out_pin);
            return test_utils::connect(nl, g, out_pin, sink, "A");
        }
    };

    // -------------------------------------------------------------------------
    // split_lut — single gate
    // -------------------------------------------------------------------------

    TEST_F(XilinxPreprocessingTest, check_split_lut_both_outputs)
    {
        TEST_START
        {
            auto nl  = make_netlist();
            Gate* g  = make_lut6_2(nl.get(), "g");
            Net* o5n = add_sink(nl.get(), g, "O5");
            Net* o6n = add_sink(nl.get(), g, "O6");

            auto res = xilinx_toolbox::split_lut(g);
            ASSERT_TRUE(res.is_ok());

            // Original gate deleted.
            EXPECT_EQ(nl->get_gate_by_id(g->get_id()), nullptr);

            // Two replacement gates created.
            auto lut5_gates = nl->get_gates([](const Gate* x) { return x->get_type()->get_name() == "LUT5"; });
            auto lut6_gates = nl->get_gates([](const Gate* x) { return x->get_type()->get_name() == "LUT6"; });
            ASSERT_EQ(lut5_gates.size(), 1u);
            ASSERT_EQ(lut6_gates.size(), 1u);

            Gate* lut5 = lut5_gates.front();
            Gate* lut6 = lut6_gates.front();

            // Names follow the convention.
            EXPECT_EQ(lut5->get_name(), "g_split_O5");
            EXPECT_EQ(lut6->get_name(), "g_split_O6");

            // Output nets are re-sourced correctly.
            EXPECT_EQ(o5n->get_sources().size(), 1u);
            EXPECT_EQ(o5n->get_sources().front()->get_gate(), lut5);
            EXPECT_EQ(o6n->get_sources().size(), 1u);
            EXPECT_EQ(o6n->get_sources().front()->get_gate(), lut6);
        }
        TEST_END
    }

    TEST_F(XilinxPreprocessingTest, check_split_lut_only_o6)
    {
        TEST_START
        {
            auto nl = make_netlist();
            Gate* g = make_lut6_2(nl.get(), "g");
            add_sink(nl.get(), g, "O6");
            // O5 has no destinations.

            ASSERT_TRUE(xilinx_toolbox::split_lut(g).is_ok());

            EXPECT_TRUE(nl->get_gates([](const Gate* x) { return x->get_type()->get_name() == "LUT5"; }).empty());
            EXPECT_EQ(nl->get_gates([](const Gate* x) { return x->get_type()->get_name() == "LUT6"; }).size(), 1u);
        }
        TEST_END
    }

    TEST_F(XilinxPreprocessingTest, check_split_lut_only_o5)
    {
        TEST_START
        {
            auto nl = make_netlist();
            Gate* g = make_lut6_2(nl.get(), "g");
            add_sink(nl.get(), g, "O5");
            // O6 has no destinations.

            ASSERT_TRUE(xilinx_toolbox::split_lut(g).is_ok());

            EXPECT_EQ(nl->get_gates([](const Gate* x) { return x->get_type()->get_name() == "LUT5"; }).size(), 1u);
            EXPECT_TRUE(nl->get_gates([](const Gate* x) { return x->get_type()->get_name() == "LUT6"; }).empty());
        }
        TEST_END
    }

    TEST_F(XilinxPreprocessingTest, check_split_lut_no_outputs_used)
    {
        TEST_START
        {
            auto nl  = make_netlist();
            Gate* g  = make_lut6_2(nl.get(), "g");
            u32   id = g->get_id();

            ASSERT_TRUE(xilinx_toolbox::split_lut(g).is_ok());

            // Gate still deleted even though nothing was connected.
            EXPECT_EQ(nl->get_gate_by_id(id), nullptr);
            EXPECT_TRUE(nl->get_gates([](const Gate* x) { return x->get_type()->get_name() == "LUT5" || x->get_type()->get_name() == "LUT6"; }).empty());
        }
        TEST_END
    }

    TEST_F(XilinxPreprocessingTest, check_split_lut_wrong_type)
    {
        TEST_START
        {
            auto  nl  = make_netlist();
            Gate* buf = nl->create_gate(m_buf_type, "buf");

            auto res = xilinx_toolbox::split_lut(buf);
            EXPECT_TRUE(res.is_error());
            // Gate not deleted.
            EXPECT_NE(nl->get_gate_by_id(buf->get_id()), nullptr);
        }
        TEST_END
    }

    TEST_F(XilinxPreprocessingTest, check_split_lut_init_strings)
    {
        TEST_START
        {
            auto nl = make_netlist();
            Gate* g = make_lut6_2(nl.get(), "g", "FEDCBA9876543210");
            add_sink(nl.get(), g, "O5");
            add_sink(nl.get(), g, "O6");

            ASSERT_TRUE(xilinx_toolbox::split_lut(g).is_ok());

            Gate* lut5 = nl->get_gates([](const Gate* x) { return x->get_type()->get_name() == "LUT5"; }).front();
            Gate* lut6 = nl->get_gates([](const Gate* x) { return x->get_type()->get_name() == "LUT6"; }).front();

            // O6 gets the full 64-bit INIT string.
            auto lut6_init = lut6->get_init_string("O");
            ASSERT_TRUE(lut6_init.is_ok());
            EXPECT_EQ(lut6_init.get(), "FEDCBA9876543210");

            // O5 gets bits [0, 31] of the INIT = lower 8 hex chars.
            auto lut5_init = lut5->get_init_string("O");
            ASSERT_TRUE(lut5_init.is_ok());
            EXPECT_EQ(lut5_init.get(), "76543210");

            // original_init metadata is set to the respective slice on each gate.
            EXPECT_EQ(std::get<1>(lut5->get_data("xilinx_preprocessing_information", "original_init")), "76543210");
            EXPECT_EQ(std::get<1>(lut6->get_data("xilinx_preprocessing_information", "original_init")), "FEDCBA9876543210");
        }
        TEST_END
    }

    TEST_F(XilinxPreprocessingTest, check_split_lut_input_connections)
    {
        TEST_START
        {
            auto nl = make_netlist();
            Gate* g = make_lut6_2(nl.get(), "g");
            add_sink(nl.get(), g, "O5");
            add_sink(nl.get(), g, "O6");

            // Record which net drives each input of the LUT6_2.
            std::vector<Net*> in_nets(6);
            for (int i = 0; i < 6; ++i)
                in_nets[i] = g->get_fan_in_net("I" + std::to_string(i));

            ASSERT_TRUE(xilinx_toolbox::split_lut(g).is_ok());

            Gate* lut5 = nl->get_gates([](const Gate* x) { return x->get_type()->get_name() == "LUT5"; }).front();
            Gate* lut6 = nl->get_gates([](const Gate* x) { return x->get_type()->get_name() == "LUT6"; }).front();

            // LUT5 receives I0-I4 from the same nets; I5 is excluded.
            for (int i = 0; i < 5; ++i)
                EXPECT_EQ(lut5->get_fan_in_net("I" + std::to_string(i)), in_nets[i]) << "LUT5 I" << i;

            // LUT6 receives all I0-I5 from the same nets.
            for (int i = 0; i < 6; ++i)
                EXPECT_EQ(lut6->get_fan_in_net("I" + std::to_string(i)), in_nets[i]) << "LUT6 I" << i;
        }
        TEST_END
    }

    TEST_F(XilinxPreprocessingTest, check_split_lut_module_assignment)
    {
        TEST_START
        {
            auto nl   = make_netlist();
            Gate* g   = make_lut6_2(nl.get(), "g");
            add_sink(nl.get(), g, "O5");
            add_sink(nl.get(), g, "O6");

            Module* mod = nl->create_module("sub", nl->get_top_module());
            mod->assign_gate(g);

            ASSERT_TRUE(xilinx_toolbox::split_lut(g).is_ok());

            Gate* lut5 = nl->get_gates([](const Gate* x) { return x->get_type()->get_name() == "LUT5"; }).front();
            Gate* lut6 = nl->get_gates([](const Gate* x) { return x->get_type()->get_name() == "LUT6"; }).front();

            EXPECT_EQ(lut5->get_module(), mod);
            EXPECT_EQ(lut6->get_module(), mod);
        }
        TEST_END
    }

    // -------------------------------------------------------------------------
    // split_luts — vector overload
    // -------------------------------------------------------------------------

    TEST_F(XilinxPreprocessingTest, check_split_luts_vector)
    {
        TEST_START
        {
            auto nl  = make_netlist();
            Gate* g1 = make_lut6_2(nl.get(), "g1");
            Gate* g2 = make_lut6_2(nl.get(), "g2");
            add_sink(nl.get(), g1, "O6");
            add_sink(nl.get(), g2, "O6");

            auto res = xilinx_toolbox::split_luts({g1, g2});
            ASSERT_TRUE(res.is_ok());
            EXPECT_EQ(res.get(), 2u);

            EXPECT_EQ(nl->get_gates([](const Gate* x) { return x->get_type()->get_name() == "LUT6_2"; }).size(), 0u);
            EXPECT_EQ(nl->get_gates([](const Gate* x) { return x->get_type()->get_name() == "LUT6"; }).size(), 2u);
        }
        TEST_END
    }

    TEST_F(XilinxPreprocessingTest, check_split_luts_vector_wrong_type)
    {
        TEST_START
        {
            auto  nl  = make_netlist();
            Gate* buf = nl->create_gate(m_buf_type, "buf");

            auto res = xilinx_toolbox::split_luts({buf});
            EXPECT_TRUE(res.is_error());
        }
        TEST_END
    }

    // -------------------------------------------------------------------------
    // split_luts — all LUT6_2 gates in netlist
    // -------------------------------------------------------------------------

    TEST_F(XilinxPreprocessingTest, check_split_luts_all)
    {
        TEST_START
        {
            auto nl  = make_netlist();
            Gate* g1 = make_lut6_2(nl.get(), "g1");
            Gate* g2 = make_lut6_2(nl.get(), "g2");
            Gate* g3 = make_lut6_2(nl.get(), "g3");
            add_sink(nl.get(), g1, "O5");
            add_sink(nl.get(), g2, "O6");
            add_sink(nl.get(), g3, "O5");
            add_sink(nl.get(), g3, "O6");

            auto res = xilinx_toolbox::split_luts(nl.get());
            ASSERT_TRUE(res.is_ok());
            EXPECT_EQ(res.get(), 3u);

            EXPECT_EQ(nl->get_gates([](const Gate* x) { return x->get_type()->get_name() == "LUT6_2"; }).size(), 0u);
        }
        TEST_END
    }

    TEST_F(XilinxPreprocessingTest, check_split_luts_none_present)
    {
        TEST_START
        {
            auto nl  = make_netlist();
            auto res = xilinx_toolbox::split_luts(nl.get());
            ASSERT_TRUE(res.is_ok());
            EXPECT_EQ(res.get(), 0u);
        }
        TEST_END
    }

}    // namespace hal
