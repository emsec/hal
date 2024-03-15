#include "netlist_preprocessing/plugin_netlist_preprocessing.h"

#include "netlist_test_utils.h"
#include "gate_library_test_utils.h"

namespace hal {

    class NetlistPreprocessingTest : public ::testing::Test {
    protected:
        virtual void SetUp() 
        {
            NO_COUT_BLOCK;
            test_utils::init_log_channels();
            test_utils::create_sandbox_directory();
        }

        virtual void TearDown() 
        {
            test_utils::remove_sandbox_directory();
        }
    };

    /**
     * Test the deletion of LUT fan-in endpoints that are not present within the LUT's Boolean function.
     *
     * Functions: remove_unused_lut_inputs
     */
    TEST_F(NetlistPreprocessingTest, check_remove_unused_lut_inputs) 
    {
        TEST_START
        {
            std::unique_ptr<Netlist> nl = test_utils::create_empty_netlist();
            ASSERT_NE(nl, nullptr);
            const GateLibrary* gl = nl->get_gate_library();
            ASSERT_NE(gl, nullptr);

            Gate* gnd_gate = nl->create_gate(gl->get_gate_type_by_name("GND"), "gnd");
            nl->mark_gnd_gate(gnd_gate);
            Net* gnd_net = nl->create_net("gnd");
            gnd_net->add_source(gnd_gate, "O");

            GateType* lut4 = gl->get_gate_type_by_name("LUT4");

            Gate* l0 = nl->create_gate(lut4, "l0");
            Gate* l1 = nl->create_gate(lut4, "l1");
            Gate* l2 = nl->create_gate(lut4, "l2");
            Gate* l3 = nl->create_gate(lut4, "l3");
            Gate* l4 = nl->create_gate(lut4, "l4");
            Gate* l5 = nl->create_gate(lut4, "l5");
            l4->add_boolean_function("O", BooleanFunction::from_string("I0 & I1 & I2 & I3").get());
            l5->add_boolean_function("O", BooleanFunction::Var("I2"));

            test_utils::connect(nl.get(), l0, "O", l4, "I0");
            test_utils::connect(nl.get(), l1, "O", l4, "I1");
            test_utils::connect(nl.get(), l2, "O", l4, "I2");
            test_utils::connect(nl.get(), l3, "O", l4, "I3");

            test_utils::connect(nl.get(), l0, "O", l5, "I0");
            test_utils::connect(nl.get(), l1, "O", l5, "I1");
            test_utils::connect(nl.get(), l2, "O", l5, "I2");
            test_utils::connect(nl.get(), l3, "O", l5, "I3");

            EXPECT_EQ(l4->get_predecessor("I0")->get_gate(), l0);
            EXPECT_EQ(l4->get_predecessor("I1")->get_gate(), l1);
            EXPECT_EQ(l4->get_predecessor("I2")->get_gate(), l2);
            EXPECT_EQ(l4->get_predecessor("I3")->get_gate(), l3);

            EXPECT_EQ(l5->get_predecessor("I0")->get_gate(), l0);
            EXPECT_EQ(l5->get_predecessor("I1")->get_gate(), l1);
            EXPECT_EQ(l5->get_predecessor("I2")->get_gate(), l2);
            EXPECT_EQ(l5->get_predecessor("I3")->get_gate(), l3);

            auto res = NetlistPreprocessingPlugin::remove_unused_lut_inputs(nl.get());
            ASSERT_TRUE(res.is_ok());
            EXPECT_EQ(res.get(), 3);

            EXPECT_EQ(l4->get_predecessor("I0")->get_gate(), l0);
            EXPECT_EQ(l4->get_predecessor("I1")->get_gate(), l1);
            EXPECT_EQ(l4->get_predecessor("I2")->get_gate(), l2);
            EXPECT_EQ(l4->get_predecessor("I3")->get_gate(), l3);

            EXPECT_EQ(l5->get_predecessor("I0")->get_gate(), gnd_gate);
            EXPECT_EQ(l5->get_predecessor("I1")->get_gate(), gnd_gate);
            EXPECT_EQ(l5->get_predecessor("I2")->get_gate(), l2);
            EXPECT_EQ(l5->get_predecessor("I3")->get_gate(), gnd_gate);
        }
        TEST_END
    }

    /**
     * Test the deletion of buffer gates.
     *
     * Functions: remove_buffers
     */
    TEST_F(NetlistPreprocessingTest, check_remove_buffers)
    {
        TEST_START
        {
            std::unique_ptr<Netlist> nl = test_utils::create_empty_netlist();
            ASSERT_NE(nl, nullptr);
            const GateLibrary* gl = nl->get_gate_library();
            ASSERT_NE(gl, nullptr);

            Gate* gnd_gate = nl->create_gate(gl->get_gate_type_by_name("GND"), "gnd");
            nl->mark_gnd_gate(gnd_gate);
            Net* gnd_net = nl->create_net("gnd");
            gnd_net->add_source(gnd_gate, "O");
            Gate* vcc_gate = nl->create_gate(gl->get_gate_type_by_name("VCC"), "vcc");
            nl->mark_vcc_gate(vcc_gate);
            Net* vcc_net = nl->create_net("vcc");
            vcc_net->add_source(vcc_gate, "O");

            Gate* g0 = nl->create_gate(gl->get_gate_type_by_name("AND2"), "g0");
            Gate* g1 = nl->create_gate(gl->get_gate_type_by_name("BUF"), "g1");
            Gate* g2 = nl->create_gate(gl->get_gate_type_by_name("AND2"), "g2");

            Net* n0 = nl->create_net("n0");
            n0->add_destination(g0, "I0");
            n0->mark_global_input_net();

            Net* n1 = nl->create_net("n1");
            n1->add_destination(g0, "I1");
            n1->mark_global_input_net();

            Net* n2 = nl->create_net("n2");
            n2->add_destination(g2, "I1");
            n2->mark_global_input_net();

            Net* n3 = test_utils::connect(nl.get(), g0, "O", g1, "I");
            Net* n4 = test_utils::connect(nl.get(), g1, "O", g2, "I0");

            auto res = NetlistPreprocessingPlugin::remove_buffers(nl.get());
            ASSERT_TRUE(res.is_ok());
            EXPECT_EQ(res.get(), 1);

            ASSERT_EQ(nl->get_gates().size(), 4);
            ASSERT_EQ(nl->get_nets().size(), 6);

            EXPECT_EQ(g0->get_successor("O")->get_gate(), g2);
        }
        {
            std::unique_ptr<Netlist> nl = test_utils::create_empty_netlist();
            ASSERT_NE(nl, nullptr);
            const GateLibrary* gl = nl->get_gate_library();
            ASSERT_NE(gl, nullptr);

            Gate* gnd_gate = nl->create_gate(gl->get_gate_type_by_name("GND"), "gnd");
            nl->mark_gnd_gate(gnd_gate);
            Net* gnd_net = nl->create_net("gnd");
            gnd_net->add_source(gnd_gate, "O");
            Gate* vcc_gate = nl->create_gate(gl->get_gate_type_by_name("VCC"), "vcc");
            nl->mark_vcc_gate(vcc_gate);
            Net* vcc_net = nl->create_net("vcc");
            vcc_net->add_source(vcc_gate, "O");

            Gate* g0 = nl->create_gate(gl->get_gate_type_by_name("AND2"), "g0");
            Gate* g1 = nl->create_gate(gl->get_gate_type_by_name("LUT2"), "g1");
            g1->add_boolean_function("O", BooleanFunction::Var("I1"));
            Gate* g2 = nl->create_gate(gl->get_gate_type_by_name("AND2"), "g2");

            Net* n0 = nl->create_net("n0");
            n0->add_destination(g0, "I0");
            n0->mark_global_input_net();

            Net* n1 = nl->create_net("n1");
            n1->add_destination(g0, "I1");
            n1->mark_global_input_net();

            Net* n2 = nl->create_net("n2");
            n2->add_destination(g2, "I1");
            n2->mark_global_input_net();

            gnd_net->add_destination(g1, "I0");

            Net* n3 = test_utils::connect(nl.get(), g0, "O", g1, "I1");
            Net* n4 = test_utils::connect(nl.get(), g1, "O", g2, "I0");

            auto res = NetlistPreprocessingPlugin::remove_buffers(nl.get());
            ASSERT_TRUE(res.is_ok());
            EXPECT_EQ(res.get(), 1);

            ASSERT_EQ(nl->get_gates().size(), 4);
            ASSERT_EQ(nl->get_nets().size(), 6);

            EXPECT_EQ(g0->get_successor("O")->get_gate(), g2);
        }
        {
            std::unique_ptr<Netlist> nl = test_utils::create_empty_netlist();
            ASSERT_NE(nl, nullptr);
            const GateLibrary* gl       = nl->get_gate_library();
            ASSERT_NE(gl, nullptr);

            Gate* gnd_gate = nl->create_gate(gl->get_gate_type_by_name("GND"), "gnd");
            nl->mark_gnd_gate(gnd_gate);
            Net* gnd_net = nl->create_net("gnd");
            gnd_net->add_source(gnd_gate, "O");
            Gate* vcc_gate = nl->create_gate(gl->get_gate_type_by_name("VCC"), "vcc");
            nl->mark_vcc_gate(vcc_gate);
            Net* vcc_net = nl->create_net("vcc");
            vcc_net->add_source(vcc_gate, "O");

            Gate* g0 = nl->create_gate(gl->get_gate_type_by_name("AND2"), "g0");
            Gate* g1 = nl->create_gate(gl->get_gate_type_by_name("AND2"), "g1");
            Gate* g2 = nl->create_gate(gl->get_gate_type_by_name("AND2"), "g2");

            Net* n0 = nl->create_net("n0");
            n0->add_destination(g0, "I0");
            n0->mark_global_input_net();

            Net* n1 = nl->create_net("n1");
            n1->add_destination(g0, "I1");
            n1->mark_global_input_net();

            Net* n2 = nl->create_net("n2");
            n2->add_destination(g2, "I1");
            n2->mark_global_input_net();

            vcc_net->add_destination(g1, "I0");

            Net* n3 = test_utils::connect(nl.get(), g0, "O", g1, "I1");
            Net* n4 = test_utils::connect(nl.get(), g1, "O", g2, "I0");

            auto res = NetlistPreprocessingPlugin::remove_buffers(nl.get());
            ASSERT_TRUE(res.is_ok());
            EXPECT_EQ(res.get(), 1);

            ASSERT_EQ(nl->get_gates().size(), 4);
            ASSERT_EQ(nl->get_nets().size(), 6);

            EXPECT_EQ(g0->get_successor("O")->get_gate(), g2);
        }

        TEST_END
    }

    /**
     * Test the deletion of redundant logic gates.
     *
     * Functions: remove_redundant_gates
     */
    TEST_F(NetlistPreprocessingTest, check_remove_redundant_gates)
    {
        TEST_START
        {
            std::unique_ptr<Netlist> nl = test_utils::create_empty_netlist();
            ASSERT_NE(nl, nullptr);
            const GateLibrary* gl = nl->get_gate_library();
            ASSERT_NE(gl, nullptr);

            Gate* g0 = nl->create_gate(gl->get_gate_type_by_name("AND2"), "g0");
            Gate* g1 = nl->create_gate(gl->get_gate_type_by_name("AND2"), "g1");
            Gate* g2 = nl->create_gate(gl->get_gate_type_by_name("DFF"), "g2");
            Gate* g3 = nl->create_gate(gl->get_gate_type_by_name("DFF"), "g3");
            Gate* g4 = nl->create_gate(gl->get_gate_type_by_name("XOR2"), "g4");

            Net* n0 = nl->create_net("n0");
            n0->add_destination(g0, "I0");
            n0->add_destination(g1, "I0");
            n0->mark_global_input_net();

            Net* n1 = nl->create_net("n1");
            n1->add_destination(g0, "I1");
            n1->add_destination(g1, "I1");
            n1->mark_global_input_net();

            Net* n2 = nl->create_net("clk");
            n2->add_destination(g2, "CLK");
            n2->add_destination(g3, "CLK");
            n2->mark_global_input_net();

            Net* n3 = test_utils::connect(nl.get(), g0, "O", g2, "D");
            Net* n4 = test_utils::connect(nl.get(), g1, "O", g3, "D");
            
            Net* n5 = test_utils::connect(nl.get(), g2, "Q", g4, "I0");
            Net* n6 = test_utils::connect(nl.get(), g3, "QN", g4, "I1");

            auto res = NetlistPreprocessingPlugin::remove_redundant_gates(nl.get());
            ASSERT_TRUE(res.is_ok());
            EXPECT_EQ(res.get(), 2);

            ASSERT_EQ(nl->get_gates().size(), 3);
            ASSERT_EQ(nl->get_nets().size(), 6);

            auto and2_gates = nl->get_gates([](const auto* g){ return g->get_type()->get_name() == "AND2"; });
            ASSERT_EQ(and2_gates.size(), 1);
            auto and2 = and2_gates.front();
            auto dff_gates = nl->get_gates([](const auto* g){ return g->get_type()->get_name() == "DFF"; });
            ASSERT_EQ(dff_gates.size(), 1);
            auto dff = dff_gates.front();
            auto xor2_gates = nl->get_gates([](const auto* g){ return g->get_type()->get_name() == "XOR2"; });
            ASSERT_EQ(xor2_gates.size(), 1);
            auto xor2 = xor2_gates.front();

            auto and2_suc = and2->get_successor("O");
            ASSERT_NE(and2_suc, nullptr);
            EXPECT_EQ(and2_suc->get_gate(), dff);
            EXPECT_EQ(and2_suc->get_pin()->get_name(), "D");

            auto dff_suc_0 = dff->get_successor("Q");
            ASSERT_NE(dff_suc_0, nullptr);
            EXPECT_EQ(dff_suc_0->get_gate(), xor2);
            EXPECT_EQ(dff_suc_0->get_pin()->get_name(), "I0");

            auto dff_suc_1 = dff->get_successor("QN");
            ASSERT_NE(dff_suc_1, nullptr);
            EXPECT_EQ(dff_suc_1->get_gate(), xor2);
            EXPECT_EQ(dff_suc_1->get_pin()->get_name(), "I1");
        }
        TEST_END
    }
} // namespace hal
