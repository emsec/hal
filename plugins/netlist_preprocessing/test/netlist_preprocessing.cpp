#include "netlist_preprocessing/netlist_preprocessing.h"
#include "hal_core/netlist/decorators/boolean_function_decorator.h"
#include "hal_core/netlist/gate_library/gate_library.h"
#include "hal_core/netlist/gate_library/gate_type_component/lut_component.h"
#include "hal_core/netlist/netlist_factory.h"

#include "netlist_preprocessing/plugin_netlist_preprocessing.h"

#include "netlist_test_utils.h"
#include "gate_library_test_utils.h"

#include <random>

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

            auto res = netlist_preprocessing::remove_unused_lut_inputs(nl.get());
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

            auto res = netlist_preprocessing::remove_buffers(nl.get());
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

            auto res = netlist_preprocessing::remove_buffers(nl.get());
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

            auto res = netlist_preprocessing::remove_buffers(nl.get());
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

            auto res = netlist_preprocessing::remove_redundant_gates(nl.get());
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

    /**
     * Test that the gate scope restricts which buffers are removed.
     *
     * Functions: remove_buffers
     */
    TEST_F(NetlistPreprocessingTest, check_remove_buffers_scoped)
    {
        TEST_START
        {
            std::unique_ptr<Netlist> nl = test_utils::create_empty_netlist();
            ASSERT_NE(nl, nullptr);
            const GateLibrary* gl = nl->get_gate_library();
            ASSERT_NE(gl, nullptr);

            Gate* g0 = nl->create_gate(gl->get_gate_type_by_name("AND2"), "g0");
            Gate* b1 = nl->create_gate(gl->get_gate_type_by_name("BUF"), "b1");
            Gate* b2 = nl->create_gate(gl->get_gate_type_by_name("BUF"), "b2");
            Gate* g3 = nl->create_gate(gl->get_gate_type_by_name("AND2"), "g3");

            Net* n0 = nl->create_net("n0");
            n0->add_destination(g0, "I0");
            n0->mark_global_input_net();

            Net* n1 = nl->create_net("n1");
            n1->add_destination(g0, "I1");
            n1->mark_global_input_net();

            Net* n2 = nl->create_net("n2");
            n2->add_destination(g3, "I1");
            n2->mark_global_input_net();

            test_utils::connect(nl.get(), g0, "O", b1, "I");
            test_utils::connect(nl.get(), b1, "O", b2, "I");
            test_utils::connect(nl.get(), b2, "O", g3, "I0");

            // only the buffer within the scope is removed, the second one is left untouched
            auto res = netlist_preprocessing::remove_buffers(nl.get(), {b1});
            ASSERT_TRUE(res.is_ok());
            EXPECT_EQ(res.get(), 1);

            ASSERT_EQ(nl->get_gates().size(), 3);
            ASSERT_NE(nl->get_gate_by_id(b2->get_id()), nullptr);
            ASSERT_NE(g0->get_successor("O"), nullptr);
            EXPECT_EQ(g0->get_successor("O")->get_gate(), b2);

            // without a scope the remaining buffer is removed as well
            res = netlist_preprocessing::remove_buffers(nl.get());
            ASSERT_TRUE(res.is_ok());
            EXPECT_EQ(res.get(), 1);

            ASSERT_EQ(nl->get_gates().size(), 2);
            ASSERT_NE(g0->get_successor("O"), nullptr);
            EXPECT_EQ(g0->get_successor("O")->get_gate(), g3);
        }
        TEST_END
    }

    /**
     * Test that the gate scope restricts which LUT fan-in endpoints are removed.
     *
     * Functions: remove_unused_lut_inputs
     */
    TEST_F(NetlistPreprocessingTest, check_remove_unused_lut_inputs_scoped)
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
            l4->add_boolean_function("O", BooleanFunction::Var("I2"));
            l5->add_boolean_function("O", BooleanFunction::Var("I2"));

            for (Gate* dst : {l4, l5})
            {
                test_utils::connect(nl.get(), l0, "O", dst, "I0");
                test_utils::connect(nl.get(), l1, "O", dst, "I1");
                test_utils::connect(nl.get(), l2, "O", dst, "I2");
                test_utils::connect(nl.get(), l3, "O", dst, "I3");
            }

            // both LUTs ignore three of their inputs, but only the one within the scope is cleaned up
            auto res = netlist_preprocessing::remove_unused_lut_inputs(nl.get(), {l5});
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
        {
            // a scope that contains no LUT at all leaves the netlist alone
            std::unique_ptr<Netlist> nl = test_utils::create_empty_netlist();
            ASSERT_NE(nl, nullptr);
            const GateLibrary* gl = nl->get_gate_library();
            ASSERT_NE(gl, nullptr);

            Gate* gnd_gate = nl->create_gate(gl->get_gate_type_by_name("GND"), "gnd");
            nl->mark_gnd_gate(gnd_gate);
            Net* gnd_net = nl->create_net("gnd");
            gnd_net->add_source(gnd_gate, "O");

            GateType* lut4 = gl->get_gate_type_by_name("LUT4");
            Gate* l0       = nl->create_gate(lut4, "l0");
            Gate* l1       = nl->create_gate(lut4, "l1");
            l1->add_boolean_function("O", BooleanFunction::Var("I2"));
            test_utils::connect(nl.get(), l0, "O", l1, "I0");

            auto res = netlist_preprocessing::remove_unused_lut_inputs(nl.get(), {gnd_gate});
            ASSERT_TRUE(res.is_ok());
            EXPECT_EQ(res.get(), 0);
            EXPECT_EQ(l1->get_predecessor("I0")->get_gate(), l0);
        }
        TEST_END
    }

    /**
     * Test that the gate scope restricts which redundant gates are removed, that the gate kept in their stead may lie
     * outside of the scope, and that the scope is not the same as the filter.
     *
     * Functions: remove_redundant_gates
     */
    TEST_F(NetlistPreprocessingTest, check_remove_redundant_gates_scoped)
    {
        // builds two functionally equivalent AND2 gates 'g0' and 'g1' driving an XOR2 gate
        auto build = [](Netlist* nl) -> std::vector<Gate*> {
            const GateLibrary* gl = nl->get_gate_library();

            Gate* g0 = nl->create_gate(gl->get_gate_type_by_name("AND2"), "g0");
            Gate* g1 = nl->create_gate(gl->get_gate_type_by_name("AND2"), "g1");
            Gate* x  = nl->create_gate(gl->get_gate_type_by_name("XOR2"), "x");

            Net* n0 = nl->create_net("n0");
            n0->add_destination(g0, "I0");
            n0->add_destination(g1, "I0");
            n0->mark_global_input_net();

            Net* n1 = nl->create_net("n1");
            n1->add_destination(g0, "I1");
            n1->add_destination(g1, "I1");
            n1->mark_global_input_net();

            test_utils::connect(nl, g0, "O", x, "I0");
            test_utils::connect(nl, g1, "O", x, "I1");

            return {g0, g1, x};
        };

        TEST_START
        {
            // the gate inside the scope is deleted, the equivalent gate outside of it survives
            std::unique_ptr<Netlist> nl = test_utils::create_empty_netlist();
            ASSERT_NE(nl, nullptr);
            auto gates = build(nl.get());
            Gate *g0 = gates[0], *g1 = gates[1];

            auto res = netlist_preprocessing::remove_redundant_gates(nl.get(), nullptr, {g1});
            ASSERT_TRUE(res.is_ok());
            EXPECT_EQ(res.get(), 1);

            EXPECT_EQ(nl->get_gates([](const Gate* g) { return g->get_name() == "g0"; }).size(), 1);
            EXPECT_EQ(nl->get_gates([](const Gate* g) { return g->get_name() == "g1"; }).size(), 0);
        }
        {
            // the survivor is determined by the scope, not by the gate name
            std::unique_ptr<Netlist> nl = test_utils::create_empty_netlist();
            ASSERT_NE(nl, nullptr);
            auto gates = build(nl.get());
            Gate* g0   = gates[0];

            auto res = netlist_preprocessing::remove_redundant_gates(nl.get(), nullptr, {g0});
            ASSERT_TRUE(res.is_ok());
            EXPECT_EQ(res.get(), 1);

            EXPECT_EQ(nl->get_gates([](const Gate* g) { return g->get_name() == "g0"; }).size(), 0);
            EXPECT_EQ(nl->get_gates([](const Gate* g) { return g->get_name() == "g1"; }).size(), 1);
        }
        {
            // the filter and the scope are not interchangeable: the filter also hides a gate from being used as the
            // survivor, so restricting it to a single gate leaves that gate without an equivalent partner
            std::unique_ptr<Netlist> nl = test_utils::create_empty_netlist();
            ASSERT_NE(nl, nullptr);
            auto gates = build(nl.get());
            Gate* g1   = gates[1];

            auto res = netlist_preprocessing::remove_redundant_gates(nl.get(), [g1](const Gate* g) { return g == g1; });
            ASSERT_TRUE(res.is_ok());
            EXPECT_EQ(res.get(), 0);

            EXPECT_EQ(nl->get_gates([](const Gate* g) { return g->get_name() == "g0"; }).size(), 1);
            EXPECT_EQ(nl->get_gates([](const Gate* g) { return g->get_name() == "g1"; }).size(), 1);
        }
        {
            // a scope holding both gates behaves like an unscoped call
            std::unique_ptr<Netlist> nl = test_utils::create_empty_netlist();
            ASSERT_NE(nl, nullptr);
            auto gates = build(nl.get());
            Gate *g0 = gates[0], *g1 = gates[1];

            auto res = netlist_preprocessing::remove_redundant_gates(nl.get(), nullptr, {g0, g1});
            ASSERT_TRUE(res.is_ok());
            EXPECT_EQ(res.get(), 1);
            EXPECT_EQ(nl->get_gates([](const Gate* g) { return g->get_type()->get_name() == "AND2"; }).size(), 1);
        }
        TEST_END
    }

    /**
     * Test that the removal of unconnected gates does not cascade beyond the gate scope.
     *
     * Functions: remove_unconnected_gates
     */
    TEST_F(NetlistPreprocessingTest, check_remove_unconnected_gates_scoped)
    {
        // builds a chain 'g0' -> 'g1' -> 'g2' whose last gate has no fan-out at all
        auto build = [](Netlist* nl) -> std::vector<Gate*> {
            const GateLibrary* gl = nl->get_gate_library();

            Gate* g0 = nl->create_gate(gl->get_gate_type_by_name("AND2"), "g0");
            Gate* g1 = nl->create_gate(gl->get_gate_type_by_name("AND2"), "g1");
            Gate* g2 = nl->create_gate(gl->get_gate_type_by_name("AND2"), "g2");

            for (const auto& [gate, pin] : std::vector<std::pair<Gate*, std::string>>{{g0, "I0"}, {g0, "I1"}, {g1, "I1"}, {g2, "I1"}})
            {
                Net* n = nl->create_net("in_" + gate->get_name() + "_" + pin);
                n->add_destination(gate, pin);
                n->mark_global_input_net();
            }

            test_utils::connect(nl, g0, "O", g1, "I0");
            test_utils::connect(nl, g1, "O", g2, "I0");

            return {g0, g1, g2};
        };

        TEST_START
        {
            // without a scope the whole chain collapses
            std::unique_ptr<Netlist> nl = test_utils::create_empty_netlist();
            ASSERT_NE(nl, nullptr);
            build(nl.get());

            auto res = netlist_preprocessing::remove_unconnected_gates(nl.get());
            ASSERT_TRUE(res.is_ok());
            EXPECT_EQ(res.get(), 3);
            EXPECT_TRUE(nl->get_gates().empty());
        }
        {
            // the cascade stops at the scope boundary, so 'g0' survives even though it is unconnected afterwards
            std::unique_ptr<Netlist> nl = test_utils::create_empty_netlist();
            ASSERT_NE(nl, nullptr);
            auto gates = build(nl.get());
            Gate *g1 = gates[1], *g2 = gates[2];

            auto res = netlist_preprocessing::remove_unconnected_gates(nl.get(), {g1, g2});
            ASSERT_TRUE(res.is_ok());
            EXPECT_EQ(res.get(), 2);

            ASSERT_EQ(nl->get_gates().size(), 1);
            EXPECT_EQ(nl->get_gates().front()->get_name(), "g0");
        }
        TEST_END
    }

    /**
     * Test that constant propagation does not cascade beyond the gate scope.
     *
     * Functions: propagate_constants
     */
    TEST_F(NetlistPreprocessingTest, check_propagate_constants_scoped)
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

            // both AND2 gates have one input tied to GND, so both outputs are constant 0
            Gate* g0 = nl->create_gate(gl->get_gate_type_by_name("AND2"), "g0");
            Gate* g1 = nl->create_gate(gl->get_gate_type_by_name("AND2"), "g1");
            Gate* x  = nl->create_gate(gl->get_gate_type_by_name("XOR2"), "x");

            gnd_net->add_destination(g0, "I0");
            gnd_net->add_destination(g1, "I0");

            Net* n0 = nl->create_net("n0");
            n0->add_destination(g0, "I1");
            n0->mark_global_input_net();

            Net* n1 = nl->create_net("n1");
            n1->add_destination(g1, "I1");
            n1->mark_global_input_net();

            test_utils::connect(nl.get(), g0, "O", x, "I0");
            test_utils::connect(nl.get(), g1, "O", x, "I1");

            auto res = netlist_preprocessing::propagate_constants(nl.get(), {g0});
            ASSERT_TRUE(res.is_ok());
            EXPECT_EQ(res.get(), 1);

            EXPECT_EQ(nl->get_gates([](const Gate* g) { return g->get_name() == "g0"; }).size(), 0);
            EXPECT_EQ(nl->get_gates([](const Gate* g) { return g->get_name() == "g1"; }).size(), 1);
            ASSERT_NE(x->get_predecessor("I0"), nullptr);
            EXPECT_EQ(x->get_predecessor("I0")->get_gate(), gnd_gate);
        }
        TEST_END
    }

    /**
     * Test that both inverters of a pair have to be inside the gate scope for the pair to be removed.
     *
     * Functions: remove_consecutive_inverters
     */
    TEST_F(NetlistPreprocessingTest, check_remove_consecutive_inverters_scoped)
    {
        // builds 'i0' -> 'i1' followed by an AND2 gate consuming the result
        auto build = [](Netlist* nl) -> std::vector<Gate*> {
            const GateLibrary* gl = nl->get_gate_library();

            Gate* i0 = nl->create_gate(gl->get_gate_type_by_name("INV"), "i0");
            Gate* i1 = nl->create_gate(gl->get_gate_type_by_name("INV"), "i1");
            Gate* g  = nl->create_gate(gl->get_gate_type_by_name("AND2"), "g");

            Net* n0 = nl->create_net("n0");
            n0->add_destination(i0, "I");
            n0->mark_global_input_net();

            Net* n1 = nl->create_net("n1");
            n1->add_destination(g, "I1");
            n1->mark_global_input_net();

            test_utils::connect(nl, i0, "O", i1, "I");
            test_utils::connect(nl, i1, "O", g, "I0");

            return {i0, i1, g};
        };

        TEST_START
        {
            // only the second inverter is in scope, so the pair is left alone
            std::unique_ptr<Netlist> nl = test_utils::create_empty_netlist();
            ASSERT_NE(nl, nullptr);
            auto gates = build(nl.get());
            Gate* i1   = gates[1];

            auto res = netlist_preprocessing::remove_consecutive_inverters(nl.get(), {i1});
            ASSERT_TRUE(res.is_ok());
            EXPECT_EQ(res.get(), 0);
            EXPECT_EQ(nl->get_gates().size(), 3);
        }
        {
            // with both inverters in scope the pair is removed and the AND2 gate is fed directly
            std::unique_ptr<Netlist> nl = test_utils::create_empty_netlist();
            ASSERT_NE(nl, nullptr);
            auto gates = build(nl.get());
            Gate *i0 = gates[0], *i1 = gates[1], *g = gates[2];

            auto res = netlist_preprocessing::remove_consecutive_inverters(nl.get(), {i0, i1});
            ASSERT_TRUE(res.is_ok());
            EXPECT_EQ(res.get(), 2);

            ASSERT_EQ(nl->get_gates().size(), 1);
            ASSERT_NE(g->get_fan_in_net("I0"), nullptr);
            EXPECT_TRUE(g->get_fan_in_net("I0")->is_global_input_net());
        }
        TEST_END
    }

    /**
     * Test that the gate scope restricts which LUT INIT strings are simplified.
     *
     * Functions: simplify_lut_inits
     */
    TEST_F(NetlistPreprocessingTest, check_simplify_lut_inits_scoped)
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

            GateType* lut2 = gl->get_gate_type_by_name("LUT2");
            const LUTComponent* lc = lut2->get_component_as<LUTComponent>([](const GateTypeComponent* c) { return LUTComponent::is_class_of(c); });
            ASSERT_NE(lc, nullptr);

            // both LUTs compute 'I0 & I1' with 'I0' tied to GND, so both are constant 0
            Gate* l0 = nl->create_gate(lut2, "l0");
            Gate* l1 = nl->create_gate(lut2, "l1");
            ASSERT_TRUE(lc->set_init_string(l0, "O", "8").is_ok());
            ASSERT_TRUE(lc->set_init_string(l1, "O", "8").is_ok());

            gnd_net->add_destination(l0, "I0");
            gnd_net->add_destination(l1, "I0");

            Net* n0 = nl->create_net("n0");
            n0->add_destination(l0, "I1");
            n0->add_destination(l1, "I1");
            n0->mark_global_input_net();

            Gate* x = nl->create_gate(gl->get_gate_type_by_name("XOR2"), "x");
            test_utils::connect(nl.get(), l0, "O", x, "I0");
            test_utils::connect(nl.get(), l1, "O", x, "I1");

            auto res = netlist_preprocessing::simplify_lut_inits(nl.get(), {l0});
            ASSERT_TRUE(res.is_ok());
            EXPECT_EQ(res.get(), 1);

            auto l0_init = lc->get_init_string(l0, "O");
            ASSERT_TRUE(l0_init.is_ok());
            EXPECT_NE(l0_init.get(), "8");

            auto l1_init = lc->get_init_string(l1, "O");
            ASSERT_TRUE(l1_init.is_ok());
            EXPECT_EQ(l1_init.get(), "8");
        }
        {
            // a LUT whose output pin is unconnected is skipped instead of crashing
            std::unique_ptr<Netlist> nl = test_utils::create_empty_netlist();
            ASSERT_NE(nl, nullptr);
            const GateLibrary* gl = nl->get_gate_library();
            ASSERT_NE(gl, nullptr);

            Gate* gnd_gate = nl->create_gate(gl->get_gate_type_by_name("GND"), "gnd");
            nl->mark_gnd_gate(gnd_gate);
            Net* gnd_net = nl->create_net("gnd");
            gnd_net->add_source(gnd_gate, "O");

            GateType* lut2 = gl->get_gate_type_by_name("LUT2");
            const LUTComponent* lc = lut2->get_component_as<LUTComponent>([](const GateTypeComponent* c) { return LUTComponent::is_class_of(c); });
            ASSERT_NE(lc, nullptr);

            Gate* l0 = nl->create_gate(lut2, "l0");
            ASSERT_TRUE(lc->set_init_string(l0, "O", "8").is_ok());
            gnd_net->add_destination(l0, "I0");

            Net* n0 = nl->create_net("n0");
            n0->add_destination(l0, "I1");
            n0->mark_global_input_net();

            auto res = netlist_preprocessing::simplify_lut_inits(nl.get());
            ASSERT_TRUE(res.is_ok());
            EXPECT_EQ(res.get(), 0);

            auto l0_init = lc->get_init_string(l0, "O");
            ASSERT_TRUE(l0_init.is_ok());
            EXPECT_EQ(l0_init.get(), "8");
        }
        TEST_END
    }

    /**
     * Test the INIT strings written for LUTs the test library does not have: a LUT1, whose two-entry truth table is
     * shorter than one hex digit, in both bit orders, and a LUT2 that reads its INIT with its input pins in reverse.
     *
     * Functions: simplify_lut_inits
     */
    TEST_F(NetlistPreprocessingTest, check_simplify_lut_inits_lut1_and_pin_order)
    {
        TEST_START
        {
            auto gl = std::make_unique<GateLibrary>("imaginary_path", "lut_library");

            GateType* gnd = gl->create_gate_type("GND", {GateTypeProperty::combinational, GateTypeProperty::ground});
            ASSERT_TRUE(gnd->create_pin("O", PinDirection::output, PinType::ground).is_ok());
            gnd->add_boolean_function("O", BooleanFunction::Const(BooleanFunction::Value::ZERO));
            ASSERT_TRUE(gl->mark_gnd_gate_type(gnd));

            GateType* vcc = gl->create_gate_type("VCC", {GateTypeProperty::combinational, GateTypeProperty::power});
            ASSERT_TRUE(vcc->create_pin("O", PinDirection::output, PinType::power).is_ok());
            vcc->add_boolean_function("O", BooleanFunction::Const(BooleanFunction::Value::ONE));
            ASSERT_TRUE(gl->mark_vcc_gate_type(vcc));

            // the input pins are created as I0, I1, whatever order the INIT configuration reads them in
            auto create_lut = [&gl](const std::string& name, u32 bit_count, bool is_ascending, const std::vector<std::string>& init_pins) -> GateType* {
                GateType* gt = gl->create_gate_type(name,
                                                    {GateTypeProperty::combinational, GateTypeProperty::c_lut},
                                                    LUTComponent::create({{"O", LUTComponent::LUTOutputConfig("INIT", 0, bit_count, is_ascending, init_pins)}}));
                for (const std::string pin : {"I0", "I1"})
                {
                    if (std::find(init_pins.begin(), init_pins.end(), pin) != init_pins.end() && gt->create_pin(pin, PinDirection::input).is_error())
                    {
                        return nullptr;
                    }
                }
                if (gt->create_pin("O", PinDirection::output, PinType::lut).is_error())
                {
                    return nullptr;
                }
                return gt;
            };
            GateType* lut1_desc = create_lut("LUT1_desc", 2, false, {"I0"});
            GateType* lut1_asc  = create_lut("LUT1_asc", 2, true, {"I0"});
            GateType* lut2_rev  = create_lut("LUT2_rev", 4, false, {"I1", "I0"});
            ASSERT_NE(lut1_desc, nullptr);
            ASSERT_NE(lut1_asc, nullptr);
            ASSERT_NE(lut2_rev, nullptr);

            std::unique_ptr<Netlist> nl = netlist_factory::create_netlist(gl.get());
            ASSERT_NE(nl, nullptr);

            Gate* gnd_gate = nl->create_gate(gnd, "gnd");
            nl->mark_gnd_gate(gnd_gate);
            Net* gnd_net = nl->create_net("gnd");
            gnd_net->add_source(gnd_gate, "O");
            Gate* vcc_gate = nl->create_gate(vcc, "vcc");
            nl->mark_vcc_gate(vcc_gate);
            Net* vcc_net = nl->create_net("vcc");
            vcc_net->add_source(vcc_gate, "O");

            struct Case
            {
                GateType* type;
                std::string init;
                std::vector<std::pair<std::string, Net*>> inputs;    // a nullptr is a fresh global input
                std::string expected_init;
            };
            const std::vector<Case> cases = {
                // descending: entry r is bit r
                {lut1_desc, "2", {{"I0", gnd_net}}, "0"},    // buffer of GND: constant 0
                {lut1_desc, "1", {{"I0", gnd_net}}, "3"},    // inverter of GND: constant 1
                {lut1_desc, "2", {{"I0", vcc_net}}, "3"},    // buffer of VCC: constant 1
                // ascending: entry 0 is the most significant bit
                {lut1_asc, "1", {{"I0", gnd_net}}, "0"},    // buffer of GND
                {lut1_asc, "2", {{"I0", gnd_net}}, "3"},    // inverter of GND
                // INIT pin order (I1, I0): entry r sets I1 to bit 0 and I0 to bit 1 of r. "4" is I0 & !I1, which
                // is I0 with I1 tied to GND: entries 2 and 3.
                {lut2_rev, "4", {{"I0", nullptr}, {"I1", gnd_net}}, "C"},
            };

            std::vector<Gate*> luts;
            for (u32 i = 0; i < cases.size(); i++)
            {
                const auto& c = cases.at(i);
                Gate* g       = nl->create_gate(c.type, "lut_" + std::to_string(i));
                const auto* lc = c.type->get_component_as<LUTComponent>([](const GateTypeComponent* comp) { return LUTComponent::is_class_of(comp); });
                ASSERT_NE(lc, nullptr);
                ASSERT_TRUE(lc->set_init_string(g, "O", c.init).is_ok());
                for (const auto& [pin, net] : c.inputs)
                {
                    Net* in = net;
                    if (in == nullptr)
                    {
                        in = nl->create_net(g->get_name() + "_" + pin);
                        in->mark_global_input_net();
                    }
                    in->add_destination(g, pin);
                }
                Net* out = nl->create_net(g->get_name() + "_O");
                out->add_source(g, "O");
                out->mark_global_output_net();
                luts.push_back(g);
            }

            auto res = netlist_preprocessing::simplify_lut_inits(nl.get());
            ASSERT_TRUE(res.is_ok()) << res.get_error().get();
            EXPECT_EQ(res.get(), cases.size());

            for (u32 i = 0; i < cases.size(); i++)
            {
                Gate* g        = luts.at(i);
                const auto* lc = g->get_type()->get_component_as<LUTComponent>([](const GateTypeComponent* comp) { return LUTComponent::is_class_of(comp); });
                auto init      = lc->get_init_string(g, "O");
                ASSERT_TRUE(init.is_ok());
                EXPECT_EQ(init.get(), cases.at(i).expected_init) << g->get_name();
            }
        }
        TEST_END
    }

    /**
     * Test that simplifying the INIT strings keeps the function of each LUT, given the constants at its inputs, for
     * every LUT type of the test library: LUT2 to LUT6 with ascending INIT strings and LUT3_desc with a descending one.
     *
     * Functions: simplify_lut_inits
     */
    TEST_F(NetlistPreprocessingTest, check_simplify_lut_inits_keeps_function)
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

            std::mt19937 rng(20261109);
            std::vector<std::pair<Gate*, BooleanFunction>> luts;
            for (const std::string type_name : {"LUT2", "LUT3", "LUT3_desc", "LUT4", "LUT5", "LUT6"})
            {
                GateType* gt = gl->get_gate_type_by_name(type_name);
                ASSERT_NE(gt, nullptr) << type_name;
                const auto* lc = gt->get_component_as<LUTComponent>([](const GateTypeComponent* c) { return LUTComponent::is_class_of(c); });
                ASSERT_NE(lc, nullptr);
                const auto* cfg = lc->get_output_pin_config("O");
                ASSERT_NE(cfg, nullptr);

                for (u32 k = 0; k < 16; k++)
                {
                    Gate* g = nl->create_gate(gt, type_name + "_" + std::to_string(k));
                    std::string init;
                    for (u32 c = 0; c < cfg->bit_count / 4; c++)
                    {
                        init += "0123456789abcdef"[rng() % 16];
                    }
                    ASSERT_TRUE(lc->set_init_string(g, "O", init).is_ok());

                    // every input goes to GND, VCC or a global input; the first one always to a constant
                    for (u32 i = 0; i < cfg->input_pins.size(); i++)
                    {
                        const std::string& pin = cfg->input_pins.at(i);
                        const u32 choice       = (i == 0) ? rng() % 2 : rng() % 3;
                        if (choice == 0)
                        {
                            gnd_net->add_destination(g, pin);
                        }
                        else if (choice == 1)
                        {
                            vcc_net->add_destination(g, pin);
                        }
                        else
                        {
                            Net* in = nl->create_net(g->get_name() + "_" + pin);
                            in->add_destination(g, pin);
                            in->mark_global_input_net();
                        }
                    }
                    Net* out = nl->create_net(g->get_name() + "_O");
                    out->add_source(g, "O");
                    out->mark_global_output_net();

                    auto expected = BooleanFunctionDecorator(g->get_boolean_function("O")).substitute_power_ground_pins(g);
                    ASSERT_TRUE(expected.is_ok());
                    luts.emplace_back(g, expected.get());
                }
            }

            auto res = netlist_preprocessing::simplify_lut_inits(nl.get());
            ASSERT_TRUE(res.is_ok()) << res.get_error().get();
            EXPECT_GT(res.get(), 0);

            for (const auto& [g, expected] : luts)
            {
                const auto* lc  = g->get_type()->get_component_as<LUTComponent>([](const GateTypeComponent* c) { return LUTComponent::is_class_of(c); });
                const auto& pins = lc->get_output_pin_config("O")->input_pins;
                auto actual      = BooleanFunctionDecorator(g->get_boolean_function("O")).substitute_power_ground_pins(g);
                ASSERT_TRUE(actual.is_ok());
                auto tt_actual   = actual.get().compute_truth_table(pins);
                auto tt_expected = expected.compute_truth_table(pins);
                ASSERT_TRUE(tt_actual.is_ok());
                ASSERT_TRUE(tt_expected.is_ok());
                EXPECT_TRUE(tt_actual.get() == tt_expected.get()) << g->get_name() << ": " << g->get_boolean_function("O").to_string() << " instead of " << expected.to_string();
            }
        }
        TEST_END
    }

    /**
     * Test that the gate scope restricts which gates get nets created at their unconnected output pins.
     *
     * Functions: create_nets_at_unconnected_pins
     */
    TEST_F(NetlistPreprocessingTest, check_create_nets_at_unconnected_pins_scoped)
    {
        TEST_START
        {
            std::unique_ptr<Netlist> nl = test_utils::create_empty_netlist();
            ASSERT_NE(nl, nullptr);
            const GateLibrary* gl = nl->get_gate_library();
            ASSERT_NE(gl, nullptr);

            Gate* g0 = nl->create_gate(gl->get_gate_type_by_name("AND2"), "g0");
            Gate* g1 = nl->create_gate(gl->get_gate_type_by_name("AND2"), "g1");

            auto res = netlist_preprocessing::create_nets_at_unconnected_pins(nl.get(), {g0});
            ASSERT_TRUE(res.is_ok());
            ASSERT_EQ(res.get().size(), 1);

            EXPECT_NE(g0->get_fan_out_net("O"), nullptr);
            EXPECT_EQ(g1->get_fan_out_net("O"), nullptr);

            // without a scope the remaining pin is covered as well
            res = netlist_preprocessing::create_nets_at_unconnected_pins(nl.get());
            ASSERT_TRUE(res.is_ok());
            ASSERT_EQ(res.get().size(), 1);
            EXPECT_NE(g1->get_fan_out_net("O"), nullptr);
        }
        TEST_END
    }

    /**
     * Test that the inverter created for a 'neg_state' output is assigned to the module of its flip-flop.
     *
     * Functions: unify_ff_outputs
     */
    TEST_F(NetlistPreprocessingTest, check_unify_ff_outputs_module_assignment)
    {
        TEST_START
        {
            std::unique_ptr<Netlist> nl = test_utils::create_empty_netlist();
            ASSERT_NE(nl, nullptr);
            const GateLibrary* gl = nl->get_gate_library();
            ASSERT_NE(gl, nullptr);

            Gate* ff = nl->create_gate(gl->get_gate_type_by_name("DFF"), "ff");
            Gate* g  = nl->create_gate(gl->get_gate_type_by_name("XOR2"), "g");

            Net* clk = nl->create_net("clk");
            clk->add_destination(ff, "CLK");
            clk->mark_global_input_net();

            Net* d = nl->create_net("d");
            d->add_destination(ff, "D");
            d->mark_global_input_net();

            test_utils::connect(nl.get(), ff, "Q", g, "I0");
            test_utils::connect(nl.get(), ff, "QN", g, "I1");

            Module* mod = nl->create_module("mod", nl->get_top_module(), {ff});
            ASSERT_NE(mod, nullptr);

            auto res = netlist_preprocessing::unify_ff_outputs(nl.get(), {ff});
            ASSERT_TRUE(res.is_ok());
            EXPECT_EQ(res.get(), 1);

            auto inverters = nl->get_gates([](const Gate* g) { return g->get_type()->has_property(GateTypeProperty::c_inverter); });
            ASSERT_EQ(inverters.size(), 1);

            // the new inverter belongs to the flip-flop it was created for, not to the top module
            EXPECT_EQ(inverters.front()->get_module(), mod);
        }
        TEST_END
    }

    /**
     * Test unifying every flip-flop of a netlist, without a list of flip-flops. The inverters created on the way grow
     * the gate vector of the netlist far beyond its capacity, which must not disturb the loop over the flip-flops.
     *
     * Functions: unify_ff_outputs
     */
    TEST_F(NetlistPreprocessingTest, check_unify_ff_outputs_netlist)
    {
        TEST_START
        {
            std::unique_ptr<Netlist> nl = test_utils::create_empty_netlist();
            ASSERT_NE(nl, nullptr);
            const GateLibrary* gl = nl->get_gate_library();
            ASSERT_NE(gl, nullptr);

            Net* clk = nl->create_net("clk");
            clk->mark_global_input_net();
            Net* d = nl->create_net("d");
            d->mark_global_input_net();

            const u32 num_ffs = 1000;
            std::vector<Gate*> ffs;
            for (u32 i = 0; i < num_ffs; i++)
            {
                Gate* ff = nl->create_gate(gl->get_gate_type_by_name("DFF"), "ff_" + std::to_string(i));
                clk->add_destination(ff, "CLK");
                d->add_destination(ff, "D");
                Net* qn = nl->create_net("qn_" + std::to_string(i));
                qn->add_source(ff, "QN");
                qn->mark_global_output_net();
                ffs.push_back(ff);
            }

            auto res = netlist_preprocessing::unify_ff_outputs(nl.get());
            ASSERT_TRUE(res.is_ok());
            EXPECT_EQ(res.get(), num_ffs);
            EXPECT_EQ(nl->get_gates().size(), 2 * num_ffs);

            for (Gate* ff : ffs)
            {
                // QN is unconnected, and an inverter of Q drives its net instead
                EXPECT_EQ(ff->get_fan_out_net("QN"), nullptr);
                Net* q = ff->get_fan_out_net("Q");
                ASSERT_NE(q, nullptr);
                ASSERT_EQ(q->get_destinations().size(), 1);
                Gate* inv = q->get_destinations().front()->get_gate();
                EXPECT_TRUE(inv->get_type()->has_property(GateTypeProperty::c_inverter));
                ASSERT_EQ(inv->get_fan_out_nets().size(), 1);
                EXPECT_TRUE(inv->get_fan_out_nets().front()->is_global_output_net());
            }
        }
        TEST_END
    }

    /**
     * Test the context menu entries contributed to the GUI.
     *
     * Functions: GuiExtensionNetlistPreprocessing::get_context_contribution, GuiExtensionNetlistPreprocessing::execute_function
     */
    TEST_F(NetlistPreprocessingTest, check_gui_extension)
    {
        TEST_START
        {
            NetlistPreprocessingPlugin plugin;

            GuiExtensionNetlistPreprocessing* gui = nullptr;
            for (auto* ext : plugin.get_extensions())
            {
                if (auto* casted = dynamic_cast<GuiExtensionNetlistPreprocessing*>(ext); casted != nullptr)
                {
                    gui = casted;
                }
            }
            ASSERT_NE(gui, nullptr);

            std::unique_ptr<Netlist> nl = test_utils::create_empty_netlist();
            ASSERT_NE(nl, nullptr);
            const GateLibrary* gl = nl->get_gate_library();
            ASSERT_NE(gl, nullptr);

            Gate* g0 = nl->create_gate(gl->get_gate_type_by_name("AND2"), "g0");
            Gate* b1 = nl->create_gate(gl->get_gate_type_by_name("BUF"), "b1");
            Gate* b2 = nl->create_gate(gl->get_gate_type_by_name("BUF"), "b2");
            Gate* g3 = nl->create_gate(gl->get_gate_type_by_name("AND2"), "g3");

            Net* n0 = nl->create_net("n0");
            n0->add_destination(g0, "I0");
            n0->mark_global_input_net();

            Net* n1 = nl->create_net("n1");
            n1->add_destination(g0, "I1");
            n1->mark_global_input_net();

            Net* n2 = nl->create_net("n2");
            n2->add_destination(g3, "I1");
            n2->mark_global_input_net();

            test_utils::connect(nl.get(), g0, "O", b1, "I");
            test_utils::connect(nl.get(), b1, "O", b2, "I");
            test_utils::connect(nl.get(), b2, "O", g3, "I0");

            // without a selection the netlist-wide entries are offered
            auto without_selection = gui->get_context_contribution(nl.get(), {}, {}, {});
            ASSERT_EQ(without_selection.size(), 2);
            for (const auto& cmc : without_selection)
            {
                EXPECT_EQ(cmc.mContributer, gui);
                EXPECT_FALSE(cmc.mEntry.empty());
                EXPECT_NE(cmc.mTagname.find("_netlist"), std::string::npos);
            }

            // with a selection only the entries operating on it are offered
            auto with_selection = gui->get_context_contribution(nl.get(), {}, {b1->get_id()}, {});
            ASSERT_EQ(with_selection.size(), 2);
            for (const auto& cmc : with_selection)
            {
                EXPECT_EQ(cmc.mContributer, gui);
                EXPECT_FALSE(cmc.mEntry.empty());
                EXPECT_NE(cmc.mTagname.find("_selection"), std::string::npos);
            }

            // running the entry on the selected gate removes only that buffer
            gui->execute_function("remove_buffers_selection", nl.get(), {}, {b1->get_id()}, {});
            EXPECT_EQ(nl->get_gates().size(), 3);
            ASSERT_NE(g0->get_successor("O"), nullptr);
            EXPECT_EQ(g0->get_successor("O")->get_gate(), b2);

            // the netlist-wide entry then removes the remaining one
            gui->execute_function("remove_buffers_netlist", nl.get(), {}, {}, {});
            EXPECT_EQ(nl->get_gates().size(), 2);
            ASSERT_NE(g0->get_successor("O"), nullptr);
            EXPECT_EQ(g0->get_successor("O")->get_gate(), g3);
        }
        {
            // a module selection is expanded into the gates it contains
            NetlistPreprocessingPlugin plugin;
            auto* gui = dynamic_cast<GuiExtensionNetlistPreprocessing*>(plugin.get_extensions().front());
            ASSERT_NE(gui, nullptr);

            std::unique_ptr<Netlist> nl = test_utils::create_empty_netlist();
            ASSERT_NE(nl, nullptr);
            const GateLibrary* gl = nl->get_gate_library();
            ASSERT_NE(gl, nullptr);

            Gate* g0 = nl->create_gate(gl->get_gate_type_by_name("AND2"), "g0");
            Gate* b1 = nl->create_gate(gl->get_gate_type_by_name("BUF"), "b1");
            Gate* b2 = nl->create_gate(gl->get_gate_type_by_name("BUF"), "b2");
            Gate* g3 = nl->create_gate(gl->get_gate_type_by_name("AND2"), "g3");

            Net* n0 = nl->create_net("n0");
            n0->add_destination(g0, "I0");
            n0->mark_global_input_net();

            Net* n1 = nl->create_net("n1");
            n1->add_destination(g0, "I1");
            n1->mark_global_input_net();

            Net* n2 = nl->create_net("n2");
            n2->add_destination(g3, "I1");
            n2->mark_global_input_net();

            test_utils::connect(nl.get(), g0, "O", b1, "I");
            test_utils::connect(nl.get(), b1, "O", b2, "I");
            test_utils::connect(nl.get(), b2, "O", g3, "I0");

            // only the first buffer lives inside the module
            Module* mod = nl->create_module("mod", nl->get_top_module(), {b1});
            ASSERT_NE(mod, nullptr);

            gui->execute_function("remove_buffers_selection", nl.get(), {mod->get_id()}, {}, {});
            EXPECT_EQ(nl->get_gates().size(), 3);
            ASSERT_NE(g0->get_successor("O"), nullptr);
            EXPECT_EQ(g0->get_successor("O")->get_gate(), b2);
        }
        TEST_END
    }

    /**
     * Test that two flip-flops which differ only in the value they start out at are not treated as
     * duplicates of one another.
     *
     * Functions: remove_redundant_gates
     */
    TEST_F(NetlistPreprocessingTest, check_remove_redundant_gates_keeps_differing_initial_values)
    {
        TEST_START
        {
            // Two flip-flops of the same type, driven by the same clock and the same data net, so that
            // everything the fan-in can tell them apart by is identical.
            const auto build = [](const std::string& init_0, const std::string& init_1) {
                std::unique_ptr<Netlist> nl = test_utils::create_empty_netlist();
                const GateLibrary* gl       = nl->get_gate_library();

                Gate* ff_0 = nl->create_gate(gl->get_gate_type_by_name("DFF"), "ff_0");
                Gate* ff_1 = nl->create_gate(gl->get_gate_type_by_name("DFF"), "ff_1");

                Net* clk = nl->create_net("clk");
                clk->add_destination(ff_0, "CLK");
                clk->add_destination(ff_1, "CLK");
                clk->mark_global_input_net();

                Net* data = nl->create_net("data");
                data->add_destination(ff_0, "D");
                data->add_destination(ff_1, "D");
                data->mark_global_input_net();

                ff_0->set_data("generic", "INIT", "bit_vector", init_0);
                ff_1->set_data("generic", "INIT", "bit_vector", init_1);
                return nl;
            };

            {
                // The same initial value: the two really are interchangeable and one may go.
                std::unique_ptr<Netlist> nl = build("0", "0");
                ASSERT_NE(nl, nullptr);

                auto res = netlist_preprocessing::remove_redundant_gates(nl.get());
                ASSERT_TRUE(res.is_ok());
                EXPECT_EQ(res.get(), 1);
                EXPECT_EQ(nl->get_gates().size(), 1);
            }
            {
                // Different initial values: one starts at 0 and the other at 1, so neither can stand in
                // for the other and both have to survive.
                std::unique_ptr<Netlist> nl = build("0", "1");
                ASSERT_NE(nl, nullptr);

                auto res = netlist_preprocessing::remove_redundant_gates(nl.get());
                ASSERT_TRUE(res.is_ok());
                EXPECT_EQ(res.get(), 0);
                EXPECT_EQ(nl->get_gates().size(), 2);
            }
        }
        TEST_END
    }
} // namespace hal
