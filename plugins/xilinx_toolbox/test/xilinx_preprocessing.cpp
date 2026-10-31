#include "hal_core/netlist/gate_library/gate_library.h"
#include "hal_core/netlist/gate_library/gate_type.h"
#include "hal_core/netlist/gate_library/gate_type_component/gate_type_component.h"
#include "hal_core/netlist/gate_library/gate_type_component/lut_component.h"
#include "hal_core/netlist/module.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/netlist/parameter.h"
#include "netlist_test_utils.h"
#include "xilinx_toolbox/preprocessing.h"

#include "gtest/gtest.h"

namespace hal
{
    class XilinxPreprocessingTest : public ::testing::Test
    {
    protected:
        std::unique_ptr<GateLibrary> m_gl;

        // Gate types stored for easy access in tests.
        GateType* m_lut6_2_type  = nullptr;
        GateType* m_lut5_type    = nullptr;
        GateType* m_lut6_type    = nullptr;
        GateType* m_buf_type     = nullptr;    // generic single-input/output gate used as sink/source
        GateType* m_fde_type     = nullptr;
        GateType* m_srl16e_type  = nullptr;
        GateType* m_srl16_type   = nullptr;
        GateType* m_srlc16e_type = nullptr;
        GateType* m_srlc32e_type = nullptr;
        GateType* m_gnd_type     = nullptr;
        GateType* m_vcc_type     = nullptr;

        void SetUp() override
        {
            NO_COUT_BLOCK;
            test_utils::init_log_channels();

            m_gl = std::make_unique<GateLibrary>("no_path", "xilinx_test_lib");

            // LUT5: I0-I4 → O, 32-bit INIT
            m_lut5_type = m_gl->create_gate_type(
                "LUT5", {GateTypeProperty::combinational, GateTypeProperty::c_lut}, LUTComponent::create({{"O", LUTComponent::LUTOutputConfig("INIT", 0, 32, true, {"I0", "I1", "I2", "I3", "I4"})}}));
            for (int i = 0; i < 5; ++i)
            {
                m_lut5_type->create_pin("I" + std::to_string(i), PinDirection::input);
            }
            m_lut5_type->create_pin("O", PinDirection::output, PinType::lut);

            // LUT6: I0-I5 → O, 64-bit INIT
            m_lut6_type = m_gl->create_gate_type("LUT6",
                                                 {GateTypeProperty::combinational, GateTypeProperty::c_lut},
                                                 LUTComponent::create({{"O", LUTComponent::LUTOutputConfig("INIT", 0, 64, true, {"I0", "I1", "I2", "I3", "I4", "I5"})}}));
            for (int i = 0; i < 6; ++i)
            {
                m_lut6_type->create_pin("I" + std::to_string(i), PinDirection::input);
            }
            m_lut6_type->create_pin("O", PinDirection::output, PinType::lut);

            // LUT6_2: I0-I5 inputs; O6 = full 64-bit INIT, O5 = lower 32 bits of the same INIT
            m_lut6_2_type = m_gl->create_gate_type("LUT6_2",
                                                   {GateTypeProperty::combinational, GateTypeProperty::c_lut},
                                                   LUTComponent::create({{"O6", LUTComponent::LUTOutputConfig("INIT", 0, 64, true, {"I0", "I1", "I2", "I3", "I4", "I5"})},
                                                                         {"O5", LUTComponent::LUTOutputConfig("INIT", 0, 32, true, {"I0", "I1", "I2", "I3", "I4"})}}));
            for (int i = 0; i < 6; ++i)
            {
                m_lut6_2_type->create_pin("I" + std::to_string(i), PinDirection::input);
            }
            m_lut6_2_type->create_pin("O5", PinDirection::output, PinType::lut);
            m_lut6_2_type->create_pin("O6", PinDirection::output, PinType::lut);

            // BUF: A → O, used as a generic sink/source in tests
            m_buf_type = m_gl->create_gate_type("BUF", {GateTypeProperty::combinational});
            m_buf_type->create_pin("A", PinDirection::input);
            m_buf_type->create_pin("O", PinDirection::output);

            // FDCE: D flip-flop with clock enable
            m_fde_type = m_gl->create_gate_type("FDCE", {GateTypeProperty::sequential, GateTypeProperty::ff});
            m_fde_type->create_pin("C", PinDirection::input, PinType::clock);
            m_fde_type->create_pin("CE", PinDirection::input, PinType::enable);
            m_fde_type->create_pin("D", PinDirection::input, PinType::data);
            m_fde_type->create_pin("Q", PinDirection::output, PinType::state);
            m_fde_type->add_parameter(Parameter::BitVector("INIT", 1, "0x0").get());
            m_fde_type->add_parameter(Parameter::BitVector("IS_C_INVERTED", 1, "0x0").get());

            // SRL16E: 16-bit shift register with clock enable
            m_srl16e_type = m_gl->create_gate_type("SRL16E", {GateTypeProperty::sequential, GateTypeProperty::shift_register});
            m_srl16e_type->create_pin("A0", PinDirection::input, PinType::control);
            m_srl16e_type->create_pin("A1", PinDirection::input, PinType::control);
            m_srl16e_type->create_pin("A2", PinDirection::input, PinType::control);
            m_srl16e_type->create_pin("A3", PinDirection::input, PinType::control);
            m_srl16e_type->create_pin("CLK", PinDirection::input, PinType::clock);
            m_srl16e_type->create_pin("CE", PinDirection::input, PinType::enable);
            m_srl16e_type->create_pin("D", PinDirection::input, PinType::data);
            m_srl16e_type->create_pin("Q", PinDirection::output, PinType::state);
            m_srl16e_type->add_parameter(Parameter::BitVector("INIT", 16, "0x0000").get());
            m_srl16e_type->add_parameter(Parameter::BitVector("IS_CLK_INVERTED", 1, "0x0").get());

            // SRL16: same structure as SRL16E
            m_srl16_type = m_gl->create_gate_type("SRL16", {GateTypeProperty::sequential, GateTypeProperty::shift_register});
            m_srl16_type->create_pin("A0", PinDirection::input, PinType::control);
            m_srl16_type->create_pin("A1", PinDirection::input, PinType::control);
            m_srl16_type->create_pin("A2", PinDirection::input, PinType::control);
            m_srl16_type->create_pin("A3", PinDirection::input, PinType::control);
            m_srl16_type->create_pin("CLK", PinDirection::input, PinType::clock);
            m_srl16_type->create_pin("CE", PinDirection::input, PinType::enable);
            m_srl16_type->create_pin("D", PinDirection::input, PinType::data);
            m_srl16_type->create_pin("Q", PinDirection::output, PinType::state);

            // SRLC16E: 16-bit cascadeable shift register; Q15 is the fixed cascade output
            m_srlc16e_type = m_gl->create_gate_type("SRLC16E", {GateTypeProperty::sequential, GateTypeProperty::shift_register});
            m_srlc16e_type->create_pin("A0", PinDirection::input, PinType::control);
            m_srlc16e_type->create_pin("A1", PinDirection::input, PinType::control);
            m_srlc16e_type->create_pin("A2", PinDirection::input, PinType::control);
            m_srlc16e_type->create_pin("A3", PinDirection::input, PinType::control);
            m_srlc16e_type->create_pin("CLK", PinDirection::input, PinType::clock);
            m_srlc16e_type->create_pin("CE", PinDirection::input, PinType::enable);
            m_srlc16e_type->create_pin("D", PinDirection::input, PinType::data);
            m_srlc16e_type->create_pin("Q", PinDirection::output, PinType::state);
            m_srlc16e_type->create_pin("Q15", PinDirection::output, PinType::state);

            // SRLC32E: 32-bit cascadeable shift register with A(0)-A(4) address pins; Q31 is cascade
            m_srlc32e_type = m_gl->create_gate_type("SRLC32E", {GateTypeProperty::sequential, GateTypeProperty::shift_register});
            m_srlc32e_type->create_pin("A(0)", PinDirection::input, PinType::control);
            m_srlc32e_type->create_pin("A(1)", PinDirection::input, PinType::control);
            m_srlc32e_type->create_pin("A(2)", PinDirection::input, PinType::control);
            m_srlc32e_type->create_pin("A(3)", PinDirection::input, PinType::control);
            m_srlc32e_type->create_pin("A(4)", PinDirection::input, PinType::control);
            m_srlc32e_type->create_pin("CLK", PinDirection::input, PinType::clock);
            m_srlc32e_type->create_pin("CE", PinDirection::input, PinType::enable);
            m_srlc32e_type->create_pin("D", PinDirection::input, PinType::data);
            m_srlc32e_type->create_pin("Q", PinDirection::output, PinType::state);
            m_srlc32e_type->create_pin("Q31", PinDirection::output, PinType::state);

            // GND / VCC: single output used to drive constant nets in tests
            m_gnd_type = m_gl->create_gate_type("GND", {GateTypeProperty::ground});
            m_gnd_type->create_pin("O", PinDirection::output);
            m_vcc_type = m_gl->create_gate_type("VCC", {GateTypeProperty::power});
            m_vcc_type->create_pin("O", PinDirection::output);
        }

        void TearDown() override
        {
        }

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
            {
                test_utils::connect_global_in(nl, g, "I" + std::to_string(i));
            }
            auto* lc = m_lut6_2_type->get_component_as<LUTComponent>([](const GateTypeComponent* c) { return c->get_type() == GateTypeComponent::ComponentType::lut; });
            if (lc != nullptr)
            {
                lc->set_init_string(g, "O6", init);
            }
            return g;
        }

        // Add a BUF sink connected to the given output pin of g; returns the net.
        Net* add_sink(Netlist* nl, Gate* g, const std::string& out_pin) const
        {
            Gate* sink = nl->create_gate(m_buf_type, g->get_name() + "_sink_" + out_pin);
            return test_utils::connect(nl, g, out_pin, sink, "A");
        }

        // Create a GND net and a VCC net; the caller connects them to address pins as needed.
        std::pair<Net*, Net*> make_constant_nets(Netlist* nl) const
        {
            Gate* gnd_gate = nl->create_gate(m_gnd_type, "GND");
            gnd_gate->mark_gnd_gate();
            Net* gnd_net = nl->create_net("GND");
            gnd_net->add_source(gnd_gate, "O");

            Gate* vcc_gate = nl->create_gate(m_vcc_type, "VCC");
            vcc_gate->mark_vcc_gate();
            Net* vcc_net = nl->create_net("VCC");
            vcc_net->add_source(vcc_gate, "O");

            return {gnd_net, vcc_net};
        }

        // Create an SRL16E gate with CLK, CE, D driven by global inputs.
        // Address pins are left unconnected; the caller connects them via make_constant_nets.
        Gate* make_srl16e(Netlist* nl, const std::string& name) const
        {
            Gate* g = nl->create_gate(m_srl16e_type, name);
            test_utils::connect_global_in(nl, g, "CLK");
            test_utils::connect_global_in(nl, g, "CE");
            test_utils::connect_global_in(nl, g, "D");
            return g;
        }

        // Connect all four address pins (A0-A3) of a gate to the given constant nets.
        // addr_bits is a 4-bit value: bit 0 = A0, bit 1 = A1, ..., bit 3 = A3.
        void connect_addr4(Gate* g, Net* gnd, Net* vcc, u32 addr_bits) const
        {
            for (u32 i = 0; i < 4; i++)
            {
                ((addr_bits >> i) & 1u ? vcc : gnd)->add_destination(g, "A" + std::to_string(i));
            }
        }

        // Connect all five address pins A(0)-A(4) of an SRLC32E gate.
        void connect_addr5(Gate* g, Net* gnd, Net* vcc, u32 addr_bits) const
        {
            for (u32 i = 0; i < 5; i++)
            {
                ((addr_bits >> i) & 1u ? vcc : gnd)->add_destination(g, "A(" + std::to_string(i) + ")");
            }
        }
    };

    // -------------------------------------------------------------------------
    // split_lut — single gate
    // -------------------------------------------------------------------------

    TEST_F(XilinxPreprocessingTest, check_split_lut_both_outputs){TEST_START{auto nl = make_netlist();
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
}    // namespace hal
TEST_END
}

TEST_F(XilinxPreprocessingTest, check_split_lut_only_o6){TEST_START{auto nl = make_netlist();
Gate* g = make_lut6_2(nl.get(), "g");
add_sink(nl.get(), g, "O6");
// O5 has no destinations.

ASSERT_TRUE(xilinx_toolbox::split_lut(g).is_ok());

EXPECT_TRUE(nl->get_gates([](const Gate* x) { return x->get_type()->get_name() == "LUT5"; }).empty());
EXPECT_EQ(nl->get_gates([](const Gate* x) { return x->get_type()->get_name() == "LUT6"; }).size(), 1u);
}
TEST_END
}

TEST_F(XilinxPreprocessingTest, check_split_lut_only_o5){TEST_START{auto nl = make_netlist();
Gate* g = make_lut6_2(nl.get(), "g");
add_sink(nl.get(), g, "O5");
// O6 has no destinations.

ASSERT_TRUE(xilinx_toolbox::split_lut(g).is_ok());

EXPECT_EQ(nl->get_gates([](const Gate* x) { return x->get_type()->get_name() == "LUT5"; }).size(), 1u);
EXPECT_TRUE(nl->get_gates([](const Gate* x) { return x->get_type()->get_name() == "LUT6"; }).empty());
}
TEST_END
}

TEST_F(XilinxPreprocessingTest, check_split_lut_no_outputs_used){TEST_START{auto nl = make_netlist();
Gate* g = make_lut6_2(nl.get(), "g");
u32 id  = g->get_id();

ASSERT_TRUE(xilinx_toolbox::split_lut(g).is_ok());

// Gate still deleted even though nothing was connected.
EXPECT_EQ(nl->get_gate_by_id(id), nullptr);
EXPECT_TRUE(nl->get_gates([](const Gate* x) { return x->get_type()->get_name() == "LUT5" || x->get_type()->get_name() == "LUT6"; }).empty());
}
TEST_END
}

TEST_F(XilinxPreprocessingTest, check_split_lut_wrong_type){TEST_START{auto nl = make_netlist();
Gate* buf = nl->create_gate(m_buf_type, "buf");

auto res = xilinx_toolbox::split_lut(buf);
EXPECT_TRUE(res.is_error());
// Gate not deleted.
EXPECT_NE(nl->get_gate_by_id(buf->get_id()), nullptr);
}
TEST_END
}

TEST_F(XilinxPreprocessingTest, check_split_lut_init_strings){TEST_START{auto nl = make_netlist();
Gate* g = make_lut6_2(nl.get(), "g", "FEDCBA9876543210");
add_sink(nl.get(), g, "O5");
add_sink(nl.get(), g, "O6");

ASSERT_TRUE(xilinx_toolbox::split_lut(g).is_ok());

Gate* lut5 = nl->get_gates([](const Gate* x) { return x->get_type()->get_name() == "LUT5"; }).front();
Gate* lut6 = nl->get_gates([](const Gate* x) { return x->get_type()->get_name() == "LUT6"; }).front();

auto* lc6 = lut6->get_type()->get_component_as<LUTComponent>([](const GateTypeComponent* c) { return c->get_type() == GateTypeComponent::ComponentType::lut; });
auto* lc5 = lut5->get_type()->get_component_as<LUTComponent>([](const GateTypeComponent* c) { return c->get_type() == GateTypeComponent::ComponentType::lut; });
ASSERT_NE(lc6, nullptr);
ASSERT_NE(lc5, nullptr);

// O6 gets the full 64-bit INIT string.
auto lut6_init = lc6->get_init_string(lut6, "O");
ASSERT_TRUE(lut6_init.is_ok());
EXPECT_EQ(lut6_init.get(), "FEDCBA9876543210");

// O5 gets bits [0, 31] of the INIT = lower 8 hex chars.
auto lut5_init = lc5->get_init_string(lut5, "O");
ASSERT_TRUE(lut5_init.is_ok());
EXPECT_EQ(lut5_init.get(), "76543210");

// original_init metadata is set to the respective slice on each gate.
EXPECT_EQ(std::get<1>(lut5->get_data("xilinx_preprocessing_information", "original_init")), "76543210");
EXPECT_EQ(std::get<1>(lut6->get_data("xilinx_preprocessing_information", "original_init")), "FEDCBA9876543210");
}
TEST_END
}

TEST_F(XilinxPreprocessingTest, check_split_lut_input_connections){TEST_START{auto nl = make_netlist();
Gate* g = make_lut6_2(nl.get(), "g");
add_sink(nl.get(), g, "O5");
add_sink(nl.get(), g, "O6");

// Record which net drives each input of the LUT6_2.
std::vector<Net*> in_nets(6);
for (int i = 0; i < 6; ++i)
{
    in_nets[i] = g->get_fan_in_net("I" + std::to_string(i));
}

ASSERT_TRUE(xilinx_toolbox::split_lut(g).is_ok());

Gate* lut5 = nl->get_gates([](const Gate* x) { return x->get_type()->get_name() == "LUT5"; }).front();
Gate* lut6 = nl->get_gates([](const Gate* x) { return x->get_type()->get_name() == "LUT6"; }).front();

// LUT5 receives I0-I4 from the same nets; I5 is excluded.
for (int i = 0; i < 5; ++i)
{
    EXPECT_EQ(lut5->get_fan_in_net("I" + std::to_string(i)), in_nets[i]) << "LUT5 I" << i;
}

// LUT6 receives all I0-I5 from the same nets.
for (int i = 0; i < 6; ++i)
{
    EXPECT_EQ(lut6->get_fan_in_net("I" + std::to_string(i)), in_nets[i]) << "LUT6 I" << i;
}
}
TEST_END
}

TEST_F(XilinxPreprocessingTest, check_split_lut_module_assignment){TEST_START{auto nl = make_netlist();
Gate* g = make_lut6_2(nl.get(), "g");
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

TEST_F(XilinxPreprocessingTest, check_split_luts_vector){TEST_START{auto nl = make_netlist();
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

TEST_F(XilinxPreprocessingTest, check_split_luts_vector_skips_on_error){TEST_START{// A non-LUT6_2 gate should be skipped; the valid gate in the same batch must still be split.
                                                                                   auto nl = make_netlist();
Gate* good = make_lut6_2(nl.get(), "good");
Gate* bad  = nl->create_gate(m_buf_type, "bad");
add_sink(nl.get(), good, "O6");

auto res = xilinx_toolbox::split_luts({good, bad});
ASSERT_TRUE(res.is_ok());
EXPECT_EQ(res.get(), 1u);    // only "good" was split

EXPECT_EQ(nl->get_gates([](const Gate* g) { return g->get_name() == "good"; }).size(), 0u);
EXPECT_EQ(nl->get_gates([](const Gate* g) { return g->get_name() == "bad"; }).size(), 1u);
EXPECT_EQ(nl->get_gates([](const Gate* g) { return g->get_type()->get_name() == "LUT6"; }).size(), 1u);
}
TEST_END
}

TEST_F(XilinxPreprocessingTest, check_split_luts_vector_all_wrong_type){TEST_START{// All gates invalid — should return OK(0), not an error.
                                                                                   auto nl = make_netlist();
Gate* buf = nl->create_gate(m_buf_type, "buf");

auto res = xilinx_toolbox::split_luts({buf});
ASSERT_TRUE(res.is_ok());
EXPECT_EQ(res.get(), 0u);
}
TEST_END
}

// -------------------------------------------------------------------------
// split_luts — all LUT6_2 gates in netlist
// -------------------------------------------------------------------------

TEST_F(XilinxPreprocessingTest, check_split_luts_all){TEST_START{auto nl = make_netlist();
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

TEST_F(XilinxPreprocessingTest, check_split_luts_none_present){TEST_START{auto nl = make_netlist();
auto res = xilinx_toolbox::split_luts(nl.get());
ASSERT_TRUE(res.is_ok());
EXPECT_EQ(res.get(), 0u);
}
TEST_END
}

// -------------------------------------------------------------------------
// split_shift_register — SRL16E
// -------------------------------------------------------------------------

TEST_F(XilinxPreprocessingTest, check_split_shift_register_srl16e_depth1){TEST_START{// A3:A0 = 0000 → select_value = 0 → depth 1 (one FF)
                                                                                     auto nl = make_netlist();
auto [gnd, vcc] = make_constant_nets(nl.get());
Gate* srl       = make_srl16e(nl.get(), "srl");
connect_addr4(srl, gnd, vcc, 0b0000);
Net* q_net = add_sink(nl.get(), srl, "Q");
u32 srl_id = srl->get_id();

auto res = xilinx_toolbox::split_shift_register(srl);
ASSERT_TRUE(res.is_ok());

EXPECT_EQ(nl->get_gate_by_id(srl_id), nullptr);

auto fdes = nl->get_gates([](const Gate* g) { return g->get_type()->get_name() == "FDCE"; });
ASSERT_EQ(fdes.size(), 1u);
EXPECT_EQ(fdes.front()->get_name(), "srl_split_ff_0");

// Q is sourced by ff_0
ASSERT_EQ(q_net->get_sources().size(), 1u);
EXPECT_EQ(q_net->get_sources().front()->get_gate(), fdes.front());
}
TEST_END
}

TEST_F(XilinxPreprocessingTest, check_split_shift_register_srl16e_depth5){TEST_START{// A3:A0 = 0100 → select_value = 4 → depth 5 (five FFs: ff_0 .. ff_4)
                                                                                     auto nl = make_netlist();
auto [gnd, vcc] = make_constant_nets(nl.get());
Gate* srl       = make_srl16e(nl.get(), "srl");
connect_addr4(srl, gnd, vcc, 0b0100);    // A2=1, others=0 → value 4
Net* q_net = add_sink(nl.get(), srl, "Q");

ASSERT_TRUE(xilinx_toolbox::split_shift_register(srl).is_ok());

auto fdes = nl->get_gates([](const Gate* g) { return g->get_type()->get_name() == "FDCE"; });
ASSERT_EQ(fdes.size(), 5u);

// Q is sourced by ff_4 (the last one)
ASSERT_EQ(q_net->get_sources().size(), 1u);
EXPECT_EQ(q_net->get_sources().front()->get_gate()->get_name(), "srl_split_ff_4");
}
TEST_END
}

TEST_F(XilinxPreprocessingTest, check_split_shift_register_srl16e_max_depth){TEST_START{// A3:A0 = 1111 → select_value = 15 → depth 16 (FFs ff_0 .. ff_15)
                                                                                        auto nl = make_netlist();
auto [gnd, vcc] = make_constant_nets(nl.get());
Gate* srl       = make_srl16e(nl.get(), "srl");
connect_addr4(srl, gnd, vcc, 0b1111);
Net* q_net = add_sink(nl.get(), srl, "Q");

ASSERT_TRUE(xilinx_toolbox::split_shift_register(srl).is_ok());

auto fdes = nl->get_gates([](const Gate* g) { return g->get_type()->get_name() == "FDCE"; });
EXPECT_EQ(fdes.size(), 16u);
ASSERT_EQ(q_net->get_sources().size(), 1u);
EXPECT_EQ(q_net->get_sources().front()->get_gate()->get_name(), "srl_split_ff_15");
}
TEST_END
}

TEST_F(XilinxPreprocessingTest, check_split_shift_register_srl16e_chain_connections){TEST_START{// Verify that FFs are chained correctly: ff_0.D = D, ff_i.D = ff_{i-1}.Q for i>0
                                                                                                auto nl = make_netlist();
auto [gnd, vcc] = make_constant_nets(nl.get());
Gate* srl       = make_srl16e(nl.get(), "srl");
connect_addr4(srl, gnd, vcc, 0b0010);    // A1=1 → select_value = 2 → 3 FFs
Net* data_net = srl->get_fan_in_net("D");
add_sink(nl.get(), srl, "Q");

ASSERT_TRUE(xilinx_toolbox::split_shift_register(srl).is_ok());

auto get_ff = [&](const std::string& n) -> Gate* {
    auto v = nl->get_gates([&](const Gate* g) { return g->get_name() == n; });
    return v.empty() ? nullptr : v.front();
};
Gate* ff0 = get_ff("srl_split_ff_0");
Gate* ff1 = get_ff("srl_split_ff_1");
Gate* ff2 = get_ff("srl_split_ff_2");
ASSERT_NE(ff0, nullptr);
ASSERT_NE(ff1, nullptr);
ASSERT_NE(ff2, nullptr);

// ff_0.D is driven by the original data net
EXPECT_EQ(ff0->get_fan_in_net("D"), data_net);

// ff_1.D is driven by ff_0.Q
EXPECT_EQ(ff1->get_fan_in_net("D"), ff0->get_fan_out_net("Q"));

// ff_2.D is driven by ff_1.Q (which is also the Q output net)
EXPECT_EQ(ff2->get_fan_in_net("D"), ff1->get_fan_out_net("Q"));
}
TEST_END
}

TEST_F(XilinxPreprocessingTest, check_split_shift_register_srl16e_clk_ce_shared){TEST_START{// All FFs must be connected to the same CLK and CE nets as the original gate
                                                                                            auto nl = make_netlist();
auto [gnd, vcc] = make_constant_nets(nl.get());
Gate* srl       = make_srl16e(nl.get(), "srl");
connect_addr4(srl, gnd, vcc, 0b0001);    // select_value = 1 → 2 FFs
Net* clk_net = srl->get_fan_in_net("CLK");
Net* ce_net  = srl->get_fan_in_net("CE");
add_sink(nl.get(), srl, "Q");

ASSERT_TRUE(xilinx_toolbox::split_shift_register(srl).is_ok());

for (const auto& ff : nl->get_gates([](const Gate* g) { return g->get_type()->get_name() == "FDCE"; }))
{
    EXPECT_EQ(ff->get_fan_in_net("C"), clk_net) << ff->get_name();
    EXPECT_EQ(ff->get_fan_in_net("CE"), ce_net) << ff->get_name();
}
}
TEST_END
}

TEST_F(XilinxPreprocessingTest, check_split_shift_register_wrong_type){TEST_START{auto nl = make_netlist();
Gate* buf = nl->create_gate(m_buf_type, "buf");

auto res = xilinx_toolbox::split_shift_register(buf);
EXPECT_TRUE(res.is_error());
EXPECT_NE(nl->get_gate_by_id(buf->get_id()), nullptr);    // not deleted
}
TEST_END
}

TEST_F(XilinxPreprocessingTest, check_split_shift_register_unconnected_addr_pin){TEST_START{// An address pin with no net must produce an error, not a silent treat-as-0
                                                                                            auto nl = make_netlist();
auto [gnd, vcc] = make_constant_nets(nl.get());
Gate* srl       = make_srl16e(nl.get(), "srl");
// Connect only A1-A3, leave A0 unconnected
gnd->add_destination(srl, "A1");
gnd->add_destination(srl, "A2");
gnd->add_destination(srl, "A3");
add_sink(nl.get(), srl, "Q");

u32 srl_id = srl->get_id();
auto res   = xilinx_toolbox::split_shift_register(srl);
EXPECT_TRUE(res.is_error());
EXPECT_NE(nl->get_gate_by_id(srl_id), nullptr);    // gate not deleted on error
}
TEST_END
}

TEST_F(XilinxPreprocessingTest, check_split_shift_register_nonconstant_addr_pin){TEST_START{// An address pin driven by a non-constant net must produce an error
                                                                                            auto nl = make_netlist();
auto [gnd, vcc] = make_constant_nets(nl.get());
Gate* srl       = make_srl16e(nl.get(), "srl");
// A0 driven by a regular (non-GND/VCC) net
Net* dynamic = test_utils::connect_global_in(nl.get(), srl, "A0");
gnd->add_destination(srl, "A1");
gnd->add_destination(srl, "A2");
gnd->add_destination(srl, "A3");
add_sink(nl.get(), srl, "Q");

u32 srl_id = srl->get_id();
auto res   = xilinx_toolbox::split_shift_register(srl);
EXPECT_TRUE(res.is_error());
EXPECT_NE(nl->get_gate_by_id(srl_id), nullptr);    // gate not deleted on error
}
TEST_END
}

TEST_F(XilinxPreprocessingTest, check_split_shift_registers_vector_skips_on_error){TEST_START{// A gate with a non-constant address pin should be skipped;
                                                                                              // the valid gate in the same batch must still be split.
                                                                                              auto nl = make_netlist();
auto [gnd, vcc] = make_constant_nets(nl.get());

Gate* good = make_srl16e(nl.get(), "good");
connect_addr4(good, gnd, vcc, 0b0001);
add_sink(nl.get(), good, "Q");

Gate* bad = make_srl16e(nl.get(), "bad");
test_utils::connect_global_in(nl.get(), bad, "A0");    // non-constant
gnd->add_destination(bad, "A1");
gnd->add_destination(bad, "A2");
gnd->add_destination(bad, "A3");
add_sink(nl.get(), bad, "Q");

auto res = xilinx_toolbox::split_shift_registers({good, bad});
ASSERT_TRUE(res.is_ok());
EXPECT_EQ(res.get(), 1u);    // only "good" was split

// "good" is gone, replaced by FDEs; "bad" still present
EXPECT_EQ(nl->get_gates([](const Gate* g) { return g->get_name() == "good"; }).size(), 0u);
EXPECT_EQ(nl->get_gates([](const Gate* g) { return g->get_name() == "bad"; }).size(), 1u);
EXPECT_EQ(nl->get_gates([](const Gate* g) { return g->get_type()->get_name() == "FDCE"; }).size(), 2u);
}
TEST_END
}

TEST_F(XilinxPreprocessingTest, check_split_shift_register_module_assignment){TEST_START{auto nl = make_netlist();
auto [gnd, vcc] = make_constant_nets(nl.get());
Gate* srl       = make_srl16e(nl.get(), "srl");
connect_addr4(srl, gnd, vcc, 0b0001);

Module* sub = nl->create_module("sub", nl->get_top_module());
sub->assign_gate(srl);
add_sink(nl.get(), srl, "Q");

ASSERT_TRUE(xilinx_toolbox::split_shift_register(srl).is_ok());

for (const auto& ff : nl->get_gates([](const Gate* g) { return g->get_type()->get_name() == "FDCE"; }))
{
    EXPECT_EQ(ff->get_module(), sub) << ff->get_name();
}
}
TEST_END
}

// -------------------------------------------------------------------------
// split_shift_register — SRL16 (no-CE variant, same structure as SRL16E in HAL)
// -------------------------------------------------------------------------

TEST_F(XilinxPreprocessingTest, check_split_shift_register_srl16){TEST_START{// SRL16 should be handled identically to SRL16E
                                                                             auto nl = make_netlist();
auto [gnd, vcc] = make_constant_nets(nl.get());
Gate* srl       = nl->create_gate(m_srl16_type, "srl");
test_utils::connect_global_in(nl.get(), srl, "CLK");
test_utils::connect_global_in(nl.get(), srl, "CE");
test_utils::connect_global_in(nl.get(), srl, "D");
connect_addr4(srl, gnd, vcc, 0b0011);    // select_value = 3 → 4 FFs
add_sink(nl.get(), srl, "Q");

auto res = xilinx_toolbox::split_shift_register(srl);
ASSERT_TRUE(res.is_ok());
EXPECT_EQ(nl->get_gates([](const Gate* g) { return g->get_type()->get_name() == "FDCE"; }).size(), 4u);
}
TEST_END
}

// -------------------------------------------------------------------------
// split_shift_register — SRLC16E (cascadeable 16-bit)
// -------------------------------------------------------------------------

TEST_F(XilinxPreprocessingTest, check_split_shift_register_srlc16e_no_cascade){TEST_START{// When Q15 has no consumers it behaves like SRL16E
                                                                                          auto nl = make_netlist();
auto [gnd, vcc] = make_constant_nets(nl.get());
Gate* srl       = nl->create_gate(m_srlc16e_type, "srl");
test_utils::connect_global_in(nl.get(), srl, "CLK");
test_utils::connect_global_in(nl.get(), srl, "CE");
test_utils::connect_global_in(nl.get(), srl, "D");
connect_addr4(srl, gnd, vcc, 0b0101);    // select_value = 5 → 6 FFs
add_sink(nl.get(), srl, "Q");
// Q15 left unconnected

ASSERT_TRUE(xilinx_toolbox::split_shift_register(srl).is_ok());
EXPECT_EQ(nl->get_gates([](const Gate* g) { return g->get_type()->get_name() == "FDCE"; }).size(), 6u);
}
TEST_END
}

TEST_F(XilinxPreprocessingTest, check_split_shift_register_srlc16e_cascade){TEST_START{// When Q15 is used, all 16 stages must be created; Q15 sources ff_15
                                                                                       auto nl = make_netlist();
auto [gnd, vcc] = make_constant_nets(nl.get());
Gate* srl       = nl->create_gate(m_srlc16e_type, "srl");
test_utils::connect_global_in(nl.get(), srl, "CLK");
test_utils::connect_global_in(nl.get(), srl, "CE");
test_utils::connect_global_in(nl.get(), srl, "D");
connect_addr4(srl, gnd, vcc, 0b0010);    // select_value = 2 → Q at ff_2
add_sink(nl.get(), srl, "Q");
Net* q15_net = add_sink(nl.get(), srl, "Q15");

ASSERT_TRUE(xilinx_toolbox::split_shift_register(srl).is_ok());

// All 16 FFs materialised
auto fdes = nl->get_gates([](const Gate* g) { return g->get_type()->get_name() == "FDCE"; });
EXPECT_EQ(fdes.size(), 16u);

// Q sourced by ff_2
auto gates_ff2 = nl->get_gates([](const Gate* g) { return g->get_name() == "srl_split_ff_2"; });
ASSERT_EQ(gates_ff2.size(), 1u);
Gate* ff_2 = gates_ff2.front();
Net* q_net = ff_2->get_fan_out_net("Q");
ASSERT_NE(q_net, nullptr);
EXPECT_GT(q_net->get_num_of_destinations(), 0u);

// Q15 sourced by ff_15
ASSERT_EQ(q15_net->get_sources().size(), 1u);
EXPECT_EQ(q15_net->get_sources().front()->get_gate()->get_name(), "srl_split_ff_15");
}
TEST_END
}

// -------------------------------------------------------------------------
// split_shift_register — SRLC32E (cascadeable 32-bit, "A(i)" pin naming)
// -------------------------------------------------------------------------

TEST_F(XilinxPreprocessingTest, check_split_shift_register_srlc32e_no_cascade){TEST_START{// Q31 has no consumers: only select_value+1 FFs are created
                                                                                          auto nl = make_netlist();
auto [gnd, vcc] = make_constant_nets(nl.get());
Gate* srl       = nl->create_gate(m_srlc32e_type, "srl");
test_utils::connect_global_in(nl.get(), srl, "CLK");
test_utils::connect_global_in(nl.get(), srl, "CE");
test_utils::connect_global_in(nl.get(), srl, "D");
connect_addr5(srl, gnd, vcc, 0b00111);    // select_value = 7 → 8 FFs
add_sink(nl.get(), srl, "Q");

ASSERT_TRUE(xilinx_toolbox::split_shift_register(srl).is_ok());
EXPECT_EQ(nl->get_gates([](const Gate* g) { return g->get_type()->get_name() == "FDCE"; }).size(), 8u);
}
TEST_END
}

TEST_F(XilinxPreprocessingTest, check_split_shift_register_srlc32e_cascade){TEST_START{// Q31 used: all 32 FFs must be created
                                                                                       auto nl = make_netlist();
auto [gnd, vcc] = make_constant_nets(nl.get());
Gate* srl       = nl->create_gate(m_srlc32e_type, "srl");
test_utils::connect_global_in(nl.get(), srl, "CLK");
test_utils::connect_global_in(nl.get(), srl, "CE");
test_utils::connect_global_in(nl.get(), srl, "D");
connect_addr5(srl, gnd, vcc, 0b00011);    // select_value = 3 → Q at ff_3
add_sink(nl.get(), srl, "Q");
Net* q31_net = add_sink(nl.get(), srl, "Q31");

ASSERT_TRUE(xilinx_toolbox::split_shift_register(srl).is_ok());

EXPECT_EQ(nl->get_gates([](const Gate* g) { return g->get_type()->get_name() == "FDCE"; }).size(), 32u);

ASSERT_EQ(q31_net->get_sources().size(), 1u);
EXPECT_EQ(q31_net->get_sources().front()->get_gate()->get_name(), "srl_split_ff_31");
}
TEST_END
}

TEST_F(XilinxPreprocessingTest, check_split_shift_register_srlc32e_both_at_max){TEST_START{// select_value = 31 = max_depth: Q and Q31 tap the same physical stage.
                                                                                           // A single FF output pin can source only one net in HAL, so cascade_out's
                                                                                           // consumers are merged onto state_out. Verify q_net sources ff_31 and carries
                                                                                           // both original consumers; q31_net is left with no source.
                                                                                           auto nl = make_netlist();
auto [gnd, vcc] = make_constant_nets(nl.get());
Gate* srl       = nl->create_gate(m_srlc32e_type, "srl");
test_utils::connect_global_in(nl.get(), srl, "CLK");
test_utils::connect_global_in(nl.get(), srl, "CE");
test_utils::connect_global_in(nl.get(), srl, "D");
connect_addr5(srl, gnd, vcc, 0b11111);    // select_value = 31

Gate* q_sink   = nl->create_gate(m_buf_type, "q_sink");
Gate* q31_sink = nl->create_gate(m_buf_type, "q31_sink");
Net* q_net     = test_utils::connect(nl.get(), srl, "Q", q_sink, "A");
Net* q31_net   = test_utils::connect(nl.get(), srl, "Q31", q31_sink, "A");

ASSERT_TRUE(xilinx_toolbox::split_shift_register(srl).is_ok());

EXPECT_EQ(nl->get_gates([](const Gate* g) { return g->get_type()->get_name() == "FDCE"; }).size(), 32u);

auto gates_ff31 = nl->get_gates([](const Gate* g) { return g->get_name() == "srl_split_ff_31"; });
ASSERT_EQ(gates_ff31.size(), 1u);
Gate* ff_31 = gates_ff31.front();

// q_net is sourced by ff_31 and now carries both the Q and Q31 consumers
ASSERT_EQ(q_net->get_sources().size(), 1u);
EXPECT_EQ(q_net->get_sources().front()->get_gate(), ff_31);
EXPECT_TRUE(q_net->is_a_destination(q_sink, "A"));
EXPECT_TRUE(q_net->is_a_destination(q31_sink, "A"));

// q31_net had its consumers moved away; it has no source anymore
EXPECT_EQ(q31_net->get_sources().size(), 0u);
EXPECT_EQ(q31_net->get_num_of_destinations(), 0u);
}
TEST_END
}

// -------------------------------------------------------------------------
// split_shift_registers — vector overload
// -------------------------------------------------------------------------

TEST_F(XilinxPreprocessingTest, check_split_shift_registers_vector){TEST_START{auto nl = make_netlist();
auto [gnd, vcc] = make_constant_nets(nl.get());
Gate* srl1      = make_srl16e(nl.get(), "srl1");
Gate* srl2      = make_srl16e(nl.get(), "srl2");
connect_addr4(srl1, gnd, vcc, 0b0001);    // 2 FFs
connect_addr4(srl2, gnd, vcc, 0b0010);    // 3 FFs
add_sink(nl.get(), srl1, "Q");
add_sink(nl.get(), srl2, "Q");

auto res = xilinx_toolbox::split_shift_registers({srl1, srl2});
ASSERT_TRUE(res.is_ok());
EXPECT_EQ(res.get(), 2u);
EXPECT_EQ(nl->get_gates([](const Gate* g) { return g->get_type()->get_name() == "SRL16E"; }).size(), 0u);
EXPECT_EQ(nl->get_gates([](const Gate* g) { return g->get_type()->get_name() == "FDCE"; }).size(), 5u);
}
TEST_END
}

TEST_F(XilinxPreprocessingTest, check_split_shift_registers_vector_wrong_type){TEST_START{// Wrong-type gate is skipped with a warning; the call succeeds with count 0.
                                                                                          auto nl = make_netlist();
Gate* buf = nl->create_gate(m_buf_type, "buf");
auto res  = xilinx_toolbox::split_shift_registers({buf});
ASSERT_TRUE(res.is_ok());
EXPECT_EQ(res.get(), 0u);
EXPECT_NE(nl->get_gate_by_id(buf->get_id()), nullptr);    // not deleted
}
TEST_END
}

// -------------------------------------------------------------------------
// split_shift_registers — all supported types in netlist
// -------------------------------------------------------------------------

TEST_F(XilinxPreprocessingTest, check_split_shift_registers_all_types){TEST_START{auto nl = make_netlist();
auto [gnd, vcc] = make_constant_nets(nl.get());

// One gate of each supported type
Gate* srl16e  = make_srl16e(nl.get(), "srl16e");
Gate* srl16   = nl->create_gate(m_srl16_type, "srl16");
Gate* srlc16e = nl->create_gate(m_srlc16e_type, "srlc16e");
Gate* srlc32e = nl->create_gate(m_srlc32e_type, "srlc32e");

for (Gate* g : {srl16, srlc16e})
{
    test_utils::connect_global_in(nl.get(), g, "CLK");
    test_utils::connect_global_in(nl.get(), g, "CE");
    test_utils::connect_global_in(nl.get(), g, "D");
}
test_utils::connect_global_in(nl.get(), srlc32e, "CLK");
test_utils::connect_global_in(nl.get(), srlc32e, "CE");
test_utils::connect_global_in(nl.get(), srlc32e, "D");

connect_addr4(srl16e, gnd, vcc, 0b0001);
connect_addr4(srl16, gnd, vcc, 0b0001);
connect_addr4(srlc16e, gnd, vcc, 0b0001);
connect_addr5(srlc32e, gnd, vcc, 0b00001);

add_sink(nl.get(), srl16e, "Q");
add_sink(nl.get(), srl16, "Q");
add_sink(nl.get(), srlc16e, "Q");
add_sink(nl.get(), srlc32e, "Q");

auto res = xilinx_toolbox::split_shift_registers(nl.get());
ASSERT_TRUE(res.is_ok());
EXPECT_EQ(res.get(), 4u);

// All shift register primitives replaced
EXPECT_EQ(nl->get_gates([](const Gate* g) { return g->get_type()->get_name() == "SRL16E"; }).size(), 0u);
EXPECT_EQ(nl->get_gates([](const Gate* g) { return g->get_type()->get_name() == "SRL16"; }).size(), 0u);
EXPECT_EQ(nl->get_gates([](const Gate* g) { return g->get_type()->get_name() == "SRLC16E"; }).size(), 0u);
EXPECT_EQ(nl->get_gates([](const Gate* g) { return g->get_type()->get_name() == "SRLC32E"; }).size(), 0u);
}
TEST_END
}

TEST_F(XilinxPreprocessingTest, check_split_shift_registers_none_present){TEST_START{auto nl = make_netlist();
auto res = xilinx_toolbox::split_shift_registers(nl.get());
ASSERT_TRUE(res.is_ok());
EXPECT_EQ(res.get(), 0u);
}
TEST_END
}

// -------------------------------------------------------------------------
// Parameter copying — split_shift_register
// -------------------------------------------------------------------------

TEST_F(XilinxPreprocessingTest, check_split_shift_register_copies_init){// INIT is N bits wide; bit i becomes the 1-bit INIT of FF i.
                                                                        // Use INIT=0x0002 (binary ...0010): FF0 gets bit 0 = 0, FF1 gets bit 1 = 1.
                                                                        TEST_START{auto nl = make_netlist();
auto [gnd, vcc] = make_constant_nets(nl.get());
Gate* srl       = make_srl16e(nl.get(), "srl");
connect_addr4(srl, gnd, vcc, 0b0001);    // select_value=1 → 2 FFs (indices 0 and 1)
add_sink(nl.get(), srl, "Q");

auto init_decl = m_srl16e_type->get_parameter("INIT");
ASSERT_TRUE(init_decl.is_ok());
ASSERT_TRUE(srl->set_parameter(init_decl.get(), "0x0002").is_ok());

ASSERT_TRUE(xilinx_toolbox::split_shift_register(srl).is_ok());

auto ffs = nl->get_gates([](const Gate* g) { return g->get_type()->get_name() == "FDCE"; });
ASSERT_EQ(ffs.size(), 2u);

for (const Gate* ff : ffs)
{
    ASSERT_TRUE(ff->has_parameter("INIT")) << ff->get_name();
    // Determine index from the FF name suffix.
    const u32 idx              = ff->get_name().back() - '0';
    const std::string expected = (idx == 1) ? "0x1" : "0x0";
    EXPECT_EQ(ff->get_parameter_value("INIT").get(), expected) << ff->get_name();
}
}
TEST_END
}

TEST_F(XilinxPreprocessingTest, check_split_shift_register_copies_clk_inverted){TEST_START{auto nl = make_netlist();
auto [gnd, vcc] = make_constant_nets(nl.get());
Gate* srl       = make_srl16e(nl.get(), "srl");
connect_addr4(srl, gnd, vcc, 0b0001);    // 2 FFs
add_sink(nl.get(), srl, "Q");

auto clk_decl = m_srl16e_type->get_parameter("IS_CLK_INVERTED");
ASSERT_TRUE(clk_decl.is_ok());
ASSERT_TRUE(srl->set_parameter(clk_decl.get(), "0x1").is_ok());

ASSERT_TRUE(xilinx_toolbox::split_shift_register(srl).is_ok());

auto ffs = nl->get_gates([](const Gate* g) { return g->get_type()->get_name() == "FDCE"; });
ASSERT_EQ(ffs.size(), 2u);
for (const auto* ff : ffs)
{
    ASSERT_TRUE(ff->has_parameter("IS_C_INVERTED")) << ff->get_name();
    EXPECT_EQ(ff->get_parameter_value("IS_C_INVERTED").get(), "0x1") << ff->get_name();
}
}
TEST_END
}

TEST_F(XilinxPreprocessingTest, check_split_shift_register_no_params_no_copy){// When source gate has no parameters set, FFs must not get any either.
                                                                              TEST_START{auto nl = make_netlist();
auto [gnd, vcc] = make_constant_nets(nl.get());
Gate* srl       = make_srl16e(nl.get(), "srl");
connect_addr4(srl, gnd, vcc, 0b0000);
add_sink(nl.get(), srl, "Q");

ASSERT_TRUE(xilinx_toolbox::split_shift_register(srl).is_ok());

auto ffs = nl->get_gates([](const Gate* g) { return g->get_type()->get_name() == "FDCE"; });
ASSERT_EQ(ffs.size(), 1u);
EXPECT_FALSE(ffs.front()->has_parameter("INIT"));
EXPECT_FALSE(ffs.front()->has_parameter("IS_C_INVERTED"));
}
TEST_END
}

// -------------------------------------------------------------------------
// create_module flag — split_lut
// -------------------------------------------------------------------------

TEST_F(XilinxPreprocessingTest, check_split_lut_create_module){TEST_START{auto nl = make_netlist();
Gate* g = make_lut6_2(nl.get(), "myLUT");
add_sink(nl.get(), g, "O5");
add_sink(nl.get(), g, "O6");

ASSERT_TRUE(xilinx_toolbox::split_lut(g, /*create_module=*/true).is_ok());

// A module named after the original gate must exist.
auto mods = nl->get_modules([](const Module* m) { return m->get_name() == "myLUT"; });
ASSERT_EQ(mods.size(), 1u);
Module* mod = mods.front();

// Both replacement gates must be in that module.
Gate* lut5 = nl->get_gates([](const Gate* x) { return x->get_type()->get_name() == "LUT5"; }).front();
Gate* lut6 = nl->get_gates([](const Gate* x) { return x->get_type()->get_name() == "LUT6"; }).front();
EXPECT_EQ(lut5->get_module(), mod);
EXPECT_EQ(lut6->get_module(), mod);
}
TEST_END
}

TEST_F(XilinxPreprocessingTest, check_split_lut_create_module_parent_preserved){TEST_START{// When create_module=true the new module must be a child of the original gate's parent.
                                                                                           auto nl = make_netlist();
Gate* g = make_lut6_2(nl.get(), "myLUT");
add_sink(nl.get(), g, "O6");

Module* parent = nl->create_module("parent_mod", nl->get_top_module());
parent->assign_gate(g);

ASSERT_TRUE(xilinx_toolbox::split_lut(g, /*create_module=*/true).is_ok());

auto mods = nl->get_modules([](const Module* m) { return m->get_name() == "myLUT"; });
ASSERT_EQ(mods.size(), 1u);
EXPECT_EQ(mods.front()->get_parent_module(), parent);
}
TEST_END
}

TEST_F(XilinxPreprocessingTest, check_split_lut_no_create_module_default){// Default (create_module=false) must not create any extra module.
                                                                          TEST_START{auto nl = make_netlist();
Gate* g = make_lut6_2(nl.get(), "g");
add_sink(nl.get(), g, "O6");
u32 module_count = nl->get_modules().size();

ASSERT_TRUE(xilinx_toolbox::split_lut(g).is_ok());

EXPECT_EQ(nl->get_modules().size(), module_count);
}
TEST_END
}

// -------------------------------------------------------------------------
// create_module flag — split_shift_register
// -------------------------------------------------------------------------

TEST_F(XilinxPreprocessingTest, check_split_shift_register_create_module){TEST_START{auto nl = make_netlist();
auto [gnd, vcc] = make_constant_nets(nl.get());
Gate* srl       = make_srl16e(nl.get(), "mySRL");
connect_addr4(srl, gnd, vcc, 0b0011);    // select_value = 3 → 4 FFs
add_sink(nl.get(), srl, "Q");

ASSERT_TRUE(xilinx_toolbox::split_shift_register(srl, /*create_module=*/true).is_ok());

// A module named after the original gate must exist.
auto mods = nl->get_modules([](const Module* m) { return m->get_name() == "mySRL"; });
ASSERT_EQ(mods.size(), 1u);
Module* mod = mods.front();

// All replacement FFs must be in that module.
auto ffs = nl->get_gates([](const Gate* g) { return g->get_type()->get_name() == "FDCE"; });
ASSERT_EQ(ffs.size(), 4u);
for (const auto* ff : ffs)
{
    EXPECT_EQ(ff->get_module(), mod) << ff->get_name();
}
}
TEST_END
}

TEST_F(XilinxPreprocessingTest, check_split_shift_register_create_module_parent_preserved){TEST_START{auto nl = make_netlist();
auto [gnd, vcc] = make_constant_nets(nl.get());
Gate* srl       = make_srl16e(nl.get(), "mySRL");
connect_addr4(srl, gnd, vcc, 0b0000);
add_sink(nl.get(), srl, "Q");

Module* parent = nl->create_module("parent_mod", nl->get_top_module());
parent->assign_gate(srl);

ASSERT_TRUE(xilinx_toolbox::split_shift_register(srl, /*create_module=*/true).is_ok());

auto mods = nl->get_modules([](const Module* m) { return m->get_name() == "mySRL"; });
ASSERT_EQ(mods.size(), 1u);
EXPECT_EQ(mods.front()->get_parent_module(), parent);
}
TEST_END
}

TEST_F(XilinxPreprocessingTest, check_split_shift_register_no_create_module_default)
{
    // Default (create_module=false) must not create any extra module.
    TEST_START
    {
        auto nl         = make_netlist();
        auto [gnd, vcc] = make_constant_nets(nl.get());
        Gate* srl       = make_srl16e(nl.get(), "srl");
        connect_addr4(srl, gnd, vcc, 0b0000);
        add_sink(nl.get(), srl, "Q");
        u32 module_count = nl->get_modules().size();

        ASSERT_TRUE(xilinx_toolbox::split_shift_register(srl).is_ok());

        EXPECT_EQ(nl->get_modules().size(), module_count);
    }
    TEST_END
}

}    // namespace hal
