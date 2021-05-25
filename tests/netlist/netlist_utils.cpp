#include "hal_core/netlist/netlist_utils.h"

#include "gate_library_test_utils.h"
#include "hal_core/netlist/netlist_factory.h"
#include "netlist_test_utils.h"

namespace hal
{
    using test_utils::MIN_GATE_ID;
    using test_utils::MIN_MODULE_ID;
    using test_utils::MIN_NET_ID;

    class NetlistUtilsTest : public ::testing::Test
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
     * Testing the get_subgraph_function
     *
     * Functions: get_subgraph_function
     */
    TEST_F(NetlistUtilsTest, check_get_subgraph_function)
    {
        TEST_START
        // Use the example netlist, that is filled with boolean functions for this test
        // (3) becomes a NOT gate, (0) an AND gate, (7) a NOT gate
        // TODO do not use example netlist anymore
        std::unique_ptr<Netlist> test_nl = test_utils::create_example_netlist();

        // -- Set the boolean functions for these gates
        Gate* gate_3 = test_nl->get_gate_by_id(MIN_GATE_ID + 3);
        Gate* gate_0 = test_nl->get_gate_by_id(MIN_GATE_ID + 0);
        Gate* gate_7 = test_nl->get_gate_by_id(MIN_GATE_ID + 7);
        gate_3->add_boolean_function("O", BooleanFunction::from_string("!I", std::vector<std::string>({"I"})));
        gate_0->add_boolean_function("O", BooleanFunction::from_string("I0 & I1", std::vector<std::string>({"I0", "I1"})));
        gate_7->add_boolean_function("O", BooleanFunction::from_string("!I", std::vector<std::string>({"I"})));
        // -- Get the names of the connected nets
        std::string net_13_name = std::to_string(MIN_NET_ID + 13);
        std::string net_20_name = std::to_string(MIN_NET_ID + 20);
        std::string net_78_name = std::to_string(MIN_NET_ID + 78);
        std::string net_30_name = std::to_string(MIN_NET_ID + 30);

        {
            // Get the boolean function of a normal sub-graph
            const std::vector<const Gate*> subgraph_gates({gate_0, gate_3});
            const Net* output_net        = test_nl->get_net_by_id(MIN_NET_ID + 045);
            BooleanFunction sub_graph_bf = netlist_utils::get_subgraph_function(output_net, subgraph_gates);

            BooleanFunction expected_bf = BooleanFunction::from_string(("!" + net_13_name + " & " + net_20_name), std::vector<std::string>({net_13_name, net_20_name}));

            EXPECT_EQ(sub_graph_bf, expected_bf);
        }
        // NEGATIVE
        {
            NO_COUT_BLOCK;
            // No subgraph gates are passed
            const std::vector<const Gate*> subgraph_gates({});
            const Net* output_net        = test_nl->get_net_by_id(MIN_NET_ID + 045);
            BooleanFunction sub_graph_bf = netlist_utils::get_subgraph_function(output_net, subgraph_gates);

            EXPECT_TRUE(sub_graph_bf.is_empty());
        }
        {
            NO_COUT_BLOCK;
            // One of the gates is a nullptr
            const std::vector<const Gate*> subgraph_gates({gate_0, nullptr, gate_3});
            const Net* output_net        = test_nl->get_net_by_id(MIN_NET_ID + 045);
            BooleanFunction sub_graph_bf = netlist_utils::get_subgraph_function(output_net, subgraph_gates);

            EXPECT_TRUE(sub_graph_bf.is_empty());
        }
        {
            NO_COUT_BLOCK;
            // The output net is a nullptr
            const std::vector<const Gate*> subgraph_gates({gate_0, gate_3});
            const Net* output_net        = nullptr;
            BooleanFunction sub_graph_bf = netlist_utils::get_subgraph_function(output_net, subgraph_gates);

            EXPECT_TRUE(sub_graph_bf.is_empty());
        }
        {
            NO_COUT_BLOCK;
            // The output net has multiple sources
            // -- create such a net
            Net* multi_src_net = test_nl->create_net("muli_src_net");
            multi_src_net->add_source(test_nl->get_gate_by_id(MIN_GATE_ID + 4), "O");
            multi_src_net->add_source(test_nl->get_gate_by_id(MIN_GATE_ID + 5), "O");

            const std::vector<const Gate*> subgraph_gates({gate_0, gate_3});
            const Net* output_net        = multi_src_net;
            BooleanFunction sub_graph_bf = netlist_utils::get_subgraph_function(output_net, subgraph_gates);

            EXPECT_TRUE(sub_graph_bf.is_empty());
            // -- remove the net
            test_nl->delete_net(multi_src_net);
        }
        {
            NO_COUT_BLOCK;
            // The output net has no source
            // -- create such a net
            Net* no_src_net = test_nl->create_net("muli_src_net");

            const std::vector<const Gate*> subgraph_gates({gate_0, gate_3});
            const Net* output_net        = no_src_net;
            BooleanFunction sub_graph_bf = netlist_utils::get_subgraph_function(output_net, subgraph_gates);

            EXPECT_TRUE(sub_graph_bf.is_empty());
            // -- remove the net
            test_nl->delete_net(no_src_net);
        }
        {
            NO_COUT_BLOCK;
            // A net in between has multiple sources (expansion should stop in this direction)
            // -- add a source to net 30 temporarily
            test_nl->get_net_by_id(MIN_NET_ID + 30)->add_source(test_nl->get_gate_by_id(MIN_GATE_ID + 8), "O");

            const std::vector<const Gate*> subgraph_gates({gate_0, gate_3});
            const Net* output_net        = test_nl->get_net_by_id(MIN_NET_ID + 045);
            BooleanFunction sub_graph_bf = netlist_utils::get_subgraph_function(output_net, subgraph_gates);

            BooleanFunction expected_bf = BooleanFunction::from_string((net_30_name + " & " + net_20_name), std::vector<std::string>({net_30_name, net_20_name}));

            EXPECT_EQ(sub_graph_bf, expected_bf);
        }
        {
            NO_COUT_BLOCK;
            // The netlist contains a cycle
            // -- create such a netlist:
            /*   .-=|gate_0|=----.
            *    '------------.  |
            *    .-=|gate_1|=-'  |
            *    '---------------'
            */
            std::unique_ptr<Netlist> netlist = test_utils::create_empty_netlist();
            Gate* cy_gate_0 = netlist->create_gate(netlist->get_gate_library()->get_gate_type_by_name("BUF"), "gate_0");
            Gate* cy_gate_1 = netlist->create_gate(netlist->get_gate_library()->get_gate_type_by_name("BUF"), "gate_1");

            Net* cy_net_0 = test_utils::connect(netlist.get(), cy_gate_0, "O", cy_gate_1, "I");
            Net* cy_net_1 = test_utils::connect(netlist.get(), cy_gate_1, "O", cy_gate_0, "I");

            const std::vector<const Gate*> subgraph_gates({cy_gate_0, cy_gate_1});
            const Net* output_net        = cy_net_0;
            BooleanFunction sub_graph_bf = netlist_utils::get_subgraph_function(output_net, subgraph_gates);

            EXPECT_TRUE(sub_graph_bf.is_empty());
        }
        {
            NO_COUT_BLOCK;
            // A gate of the subgraph has unconnected input pins
            const std::vector<const Gate*> subgraph_gates({gate_7});
            const Net* output_net        = test_nl->get_net_by_id(MIN_NET_ID + 78);
            BooleanFunction sub_graph_bf = netlist_utils::get_subgraph_function(output_net, subgraph_gates);

            EXPECT_EQ(sub_graph_bf, gate_7->get_boolean_function("O"));
        }
        TEST_END
    }

    /**
     * Testing the deep copying of netlists
     *
     * Functions: copy_netlist
     */
    TEST_F(NetlistUtilsTest, check_copy_netlist)
    {
        TEST_START
        // Create an example netlist that should be copied
        // TODO do not use example netlist anymore
        std::unique_ptr<Netlist> test_nl = test_utils::create_example_netlist();

        // -- Add some modules to the example netlist
        Module* mod_0 = test_nl->create_module(test_utils::MIN_MODULE_ID + 0, "mod_0", test_nl->get_top_module(), std::vector<Gate*>{
                                                                                                                      test_nl->get_gate_by_id(MIN_GATE_ID + 0),
                                                                                                                      test_nl->get_gate_by_id(MIN_GATE_ID + 1),
                                                                                                                      test_nl->get_gate_by_id(MIN_GATE_ID + 2),
                                                                                                                      test_nl->get_gate_by_id(MIN_GATE_ID + 3),
                                                                                                                      test_nl->get_gate_by_id(MIN_GATE_ID + 4),
                                                                                                                      test_nl->get_gate_by_id(MIN_GATE_ID + 5)
                                                                                                                  });
        Module* mod_1 = test_nl->create_module(test_utils::MIN_MODULE_ID + 1, "mod_1", mod_0, std::vector<Gate*>{
                                                                                                  test_nl->get_gate_by_id(MIN_GATE_ID + 0),
                                                                                                  test_nl->get_gate_by_id(MIN_GATE_ID + 4),
                                                                                                  test_nl->get_gate_by_id(MIN_GATE_ID + 5),
                                                                                              });

        // -- Add some groupings to the netlist
        Grouping* prime_grouping = test_nl->create_grouping("prime_gates");
        prime_grouping->assign_gate_by_id(MIN_GATE_ID + 2);
        prime_grouping->assign_gate_by_id(MIN_GATE_ID + 3);
        prime_grouping->assign_gate_by_id(MIN_GATE_ID + 5);
        prime_grouping->assign_gate_by_id(MIN_GATE_ID + 7);
        prime_grouping->assign_net_by_id(MIN_NET_ID + 13);
        prime_grouping->assign_module_by_id(MIN_MODULE_ID + 1);    // (I know 1 is not prime :P)
        Grouping* empty_grouping = test_nl->create_grouping("empty_grouping");

        // -- Mark Gates as GND/VCC
        test_nl->mark_gnd_gate(test_nl->get_gate_by_id(MIN_GATE_ID + 1));
        test_nl->mark_vcc_gate(test_nl->get_gate_by_id(MIN_GATE_ID + 2));

        // -- Mark nets as global inputs/outputs
        test_nl->mark_global_input_net(test_nl->get_net_by_id(MIN_NET_ID + 20));
        test_nl->mark_global_output_net(test_nl->get_net_by_id(MIN_NET_ID + 78));

        // -- Add some boolean functions to the gates
        test_nl->get_gate_by_id(MIN_GATE_ID + 0)->add_boolean_function("O", BooleanFunction::from_string("I0 & I1"));
        test_nl->get_gate_by_id(MIN_GATE_ID + 4)->add_boolean_function("O", BooleanFunction::from_string("!I"));

        // Copy and compare the netlist
        std::unique_ptr<Netlist> test_nl_copy = netlist_utils::copy_netlist(test_nl.get());

        EXPECT_TRUE(*test_nl_copy == *test_nl);
        TEST_END
    }

    /**
     * Testing the get_next_sequential_gates variants.
     * Testing only the gate functions for now as they call the net functions.
     * TODO: test both variants
     *
     * Functions: get_next_sequential_gates
     */
    TEST_F(NetlistUtilsTest, check_get_next_sequential_gates)
    {
        TEST_START
        {
            std::unique_ptr<Netlist> nl = test_utils::create_empty_netlist();
            ASSERT_NE(nl, nullptr);
            const GateLibrary* gl       = nl->get_gate_library();
            ASSERT_NE(gl, nullptr);

            Gate* gate_0     = nl->create_gate(gl->get_gate_type_by_name("GND"), "gate_0");
            Gate* gate_1     = nl->create_gate(gl->get_gate_type_by_name("VCC"), "gate_1");
            Gate* gate_2     = nl->create_gate(gl->get_gate_type_by_name("BUF"), "gate_2");
            Gate* gate_3     = nl->create_gate(gl->get_gate_type_by_name("AND2"), "gate_3");
            Gate* gate_4_seq = nl->create_gate(gl->get_gate_type_by_name("DFFE"), "gate_4_seq");
            Gate* gate_5_seq = nl->create_gate(gl->get_gate_type_by_name("DFFE"), "gate_5_seq");
            Gate* gate_6     = nl->create_gate(gl->get_gate_type_by_name("AND2"), "gate_6");

            test_utils::connect(nl.get(), gate_0, "O", gate_2, "I");
            test_utils::connect(nl.get(), gate_0, "O", gate_3, "I1");
            test_utils::connect(nl.get(), gate_1, "O", gate_3, "I0");
            test_utils::connect(nl.get(), gate_3, "O", gate_4_seq, "D");
            test_utils::connect(nl.get(), gate_4_seq, "Q", gate_4_seq, "EN");
            test_utils::connect(nl.get(), gate_4_seq, "Q", gate_5_seq, "EN");
            test_utils::connect(nl.get(), gate_0, "O", gate_5_seq, "D");
            test_utils::connect(nl.get(), gate_4_seq, "Q", gate_6, "I0");
            test_utils::connect(nl.get(), gate_5_seq, "Q", gate_6, "I1");

            std::map<Gate*, std::vector<Gate*>> test_successors = 
            {
                {gate_0, {gate_4_seq, gate_5_seq}},
                {gate_1, {gate_4_seq}},
                {gate_2, {}},
                {gate_3, {gate_4_seq}},
                {gate_4_seq, {gate_4_seq, gate_5_seq}},
                {gate_5_seq, {}},
                {gate_6, {}},
            };

            std::map<Gate*, std::vector<Gate*>> test_predecessors = 
            {
                {gate_0, {}},
                {gate_1, {}},
                {gate_2, {}},
                {gate_3, {}},
                {gate_4_seq, {gate_4_seq}},
                {gate_5_seq, {gate_4_seq}},
                {gate_6, {gate_5_seq, gate_4_seq}},
            };

            for (const auto& [start, expected] : test_successors)
            {
                auto successors = netlist_utils::get_next_sequential_gates(start, true);
                std::unordered_map<u32, std::vector<Gate*>> cache;
                auto successors_cached = netlist_utils::get_next_sequential_gates(start, true, cache);
                EXPECT_EQ(successors, successors_cached);
                EXPECT_TRUE(test_utils::vectors_have_same_content(successors, expected));

                // std::cout << "successors of: " << start->get_name() << std::endl;
                // std::cout << "computed: " << utils::join(", ", successors, [](auto x) { return x->get_name(); }) << std::endl;
                // std::cout << "expected: " << utils::join(", ", expected, [](auto x) { return x->get_name(); }) << std::endl;
            }

            for (const auto& [start, expected] : test_predecessors)
            {
                auto predecessors = netlist_utils::get_next_sequential_gates(start, false);
                std::unordered_map<u32, std::vector<Gate*>> cache;
                auto predecessors_cached = netlist_utils::get_next_sequential_gates(start, false, cache);
                EXPECT_EQ(predecessors, predecessors_cached);
                EXPECT_TRUE(test_utils::vectors_have_same_content(predecessors, expected));

                // std::cout << "predecessors of: " << start->get_name() << std::endl;
                // std::cout << "computed: " << utils::join(", ", predecessors, [](auto x) { return x->get_name(); }) << std::endl;
                // std::cout << "expected: " << utils::join(", ", expected, [](auto x) { return x->get_name(); }) << std::endl;
            }
        }
        TEST_END
    }

    /**
     * Testing getting the nets connected to a set of pins.
     *
     * Functions: get_nets_at_pins
     */
    TEST_F(NetlistUtilsTest, check_get_nets_at_pins)
    {
        TEST_START
        {
            std::unique_ptr<Netlist> nl = test_utils::create_empty_netlist();
            ASSERT_NE(nl, nullptr);
            const GateLibrary* gl       = nl->get_gate_library();
            ASSERT_NE(gl, nullptr);

            Gate* l0 = nl->create_gate(gl->get_gate_type_by_name("LUT4"), "l0");
            Gate* l1 = nl->create_gate(gl->get_gate_type_by_name("LUT4"), "l1");
            Gate* l2 = nl->create_gate(gl->get_gate_type_by_name("LUT4"), "l2");
            Gate* l3 = nl->create_gate(gl->get_gate_type_by_name("LUT4"), "l3");
            Gate* l4 = nl->create_gate(gl->get_gate_type_by_name("LUT4"), "l4");

            Net* n0 = test_utils::connect(nl.get(), l0, "O", l4, "I0");
            Net* n1 = test_utils::connect(nl.get(), l1, "O", l4, "I1");
            Net* n2 = test_utils::connect(nl.get(), l2, "O", l4, "I2");
            Net* n3 = test_utils::connect(nl.get(), l3, "O", l4, "I3");

            EXPECT_EQ(netlist_utils::get_nets_at_pins(l4, {"I0", "I2"}, true), std::unordered_set<Net*>({n0, n2}));
            EXPECT_EQ(netlist_utils::get_nets_at_pins(l4, {"I1", "I2", "I4"}, true), std::unordered_set<Net*>({n1, n2}));
            EXPECT_EQ(netlist_utils::get_nets_at_pins(l4, {"I1", "I2", "I3"}, true), std::unordered_set<Net*>({n1, n2, n3}));
            EXPECT_EQ(netlist_utils::get_nets_at_pins(l0, {"O"}, false), std::unordered_set<Net*>({n0}));
            EXPECT_EQ(netlist_utils::get_nets_at_pins(l0, {"A", "B", "C"}, true), std::unordered_set<Net*>());
        }
        TEST_END
    }

    /**
     * Testing removal of buffer gates.
     *
     * Functions: remove_buffers
     */
    TEST_F(NetlistUtilsTest, check_remove_buffers)
    {
        TEST_START
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

            netlist_utils::remove_buffers(nl.get());

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
            Gate* g1 = nl->create_gate(gl->get_gate_type_by_name("LUT2"), "g1");
            g1->add_boolean_function("O", BooleanFunction::from_string("I1"));
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

            netlist_utils::remove_buffers(nl.get());

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

            netlist_utils::remove_buffers(nl.get(), true);

            ASSERT_EQ(nl->get_gates().size(), 4);
            ASSERT_EQ(nl->get_nets().size(), 6);

            EXPECT_EQ(g0->get_successor("O")->get_gate(), g2);
        }

        TEST_END
    }

    /**
     * Testing removal of LUT fan-in endpoints that are not present within the LUT's Boolean function.
     *
     * Functions: remove_buffers
     */
    TEST_F(NetlistUtilsTest, check_remove_unused_lut_endpoints)
    {
        TEST_START
        {
            std::unique_ptr<Netlist> nl = test_utils::create_empty_netlist();
            ASSERT_NE(nl, nullptr);
            const GateLibrary* gl       = nl->get_gate_library();
            ASSERT_NE(gl, nullptr);

            Gate* gnd_gate = nl->create_gate(gl->get_gate_type_by_name("GND"), "gnd");
            nl->mark_gnd_gate(gnd_gate);
            Net* gnd_net = nl->create_net("gnd");
            gnd_net->add_source(gnd_gate, "O");

            Gate* l0 = nl->create_gate(gl->get_gate_type_by_name("LUT4"), "l0");
            Gate* l1 = nl->create_gate(gl->get_gate_type_by_name("LUT4"), "l1");
            Gate* l2 = nl->create_gate(gl->get_gate_type_by_name("LUT4"), "l2");
            Gate* l3 = nl->create_gate(gl->get_gate_type_by_name("LUT4"), "l3");
            Gate* l4 = nl->create_gate(gl->get_gate_type_by_name("LUT4"), "l4");
            Gate* l5 = nl->create_gate(gl->get_gate_type_by_name("LUT4"), "l5");
            l4->add_boolean_function("O", BooleanFunction::from_string("I0 & I1 & I2 & I3"));
            l5->add_boolean_function("O", BooleanFunction::from_string("I2"));

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

            netlist_utils::remove_unused_lut_endpoints(nl.get());

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
     * Testing detection of common inputs of gates.
     *
     * Functions: get_common_inputs
     */
    TEST_F(NetlistUtilsTest, check_get_common_inputs)
    {
        TEST_START
        {
            std::unique_ptr<Netlist> nl = test_utils::create_empty_netlist();
            ASSERT_NE(nl, nullptr);
            const GateLibrary* gl       = nl->get_gate_library();
            ASSERT_NE(gl, nullptr);

            Gate* gnd_gate = nl->create_gate(gl->get_gate_type_by_name("GND"), "gnd");
            nl->mark_gnd_gate(gnd_gate);

            Gate* l0 = nl->create_gate(gl->get_gate_type_by_name("LUT4"), "l0");
            Gate* l1 = nl->create_gate(gl->get_gate_type_by_name("LUT4"), "l1");
            Gate* l2 = nl->create_gate(gl->get_gate_type_by_name("LUT4"), "l2");
            Gate* l3 = nl->create_gate(gl->get_gate_type_by_name("LUT4"), "l3");
            Gate* l4 = nl->create_gate(gl->get_gate_type_by_name("LUT4"), "l4");
            Gate* l5 = nl->create_gate(gl->get_gate_type_by_name("LUT4"), "l4");

            Net* gnd_net = test_utils::connect(nl.get(), gnd_gate, "O", l2, "I0");
            test_utils::connect(nl.get(), gnd_gate, "O", l3, "I0");
            test_utils::connect(nl.get(), gnd_gate, "O", l4, "I0");
            test_utils::connect(nl.get(), gnd_gate, "O", l5, "I0");
            Net* common_net4 = test_utils::connect(nl.get(), l0, "O", l2, "I2");
            test_utils::connect(nl.get(), l0, "O", l3, "I2");
            test_utils::connect(nl.get(), l0, "O", l4, "I2");
            test_utils::connect(nl.get(), l0, "O", l5, "I2");
            Net* common_net2 = test_utils::connect(nl.get(), l1, "O", l2, "I1");
            test_utils::connect(nl.get(), l1, "O", l3, "I1");

            std::vector<Gate*> gates       = {l2, l3, l4, l5};
            std::vector<Net*> common_nets4 = netlist_utils::get_common_inputs(gates);
            std::vector<Net*> common_nets2 = netlist_utils::get_common_inputs(gates, 2);

            ASSERT_EQ(common_nets4.size(), 1);
            EXPECT_TRUE(std::find(common_nets4.begin(), common_nets4.end(), common_net4) != common_nets4.end());

            ASSERT_EQ(common_nets2.size(), 2);
            EXPECT_TRUE(std::find(common_nets2.begin(), common_nets2.end(), common_net2) != common_nets2.end());
            EXPECT_TRUE(std::find(common_nets2.begin(), common_nets2.end(), common_net4) != common_nets2.end());
        }
        TEST_END
    }

    /**
     * Testing replacement of gate.
     *
     * Functions: replace_gate
     */
    TEST_F(NetlistUtilsTest, check_replace_gate)
    {
        TEST_START
        {
            std::unique_ptr<Netlist> nl = test_utils::create_empty_netlist();
            ASSERT_NE(nl, nullptr);
            const GateLibrary* gl       = nl->get_gate_library();
            ASSERT_NE(gl, nullptr);

            // TODO implement test
        }
        TEST_END
    }

    /**
     * Testing detection of gate chains.
     *
     * Functions: get_gate_chain
     */
    TEST_F(NetlistUtilsTest, check_get_gate_chain)
    {
        TEST_START
        {
            // test default functionality
            std::unique_ptr<Netlist> nl = test_utils::create_empty_netlist();
            ASSERT_NE(nl, nullptr);
            const GateLibrary* gl       = nl->get_gate_library();
            ASSERT_NE(gl, nullptr);

            Gate* c0 = nl->create_gate(gl->get_gate_type_by_name("CARRY"), "c0");
            Gate* c1 = nl->create_gate(gl->get_gate_type_by_name("CARRY"), "c1");
            Gate* c2 = nl->create_gate(gl->get_gate_type_by_name("CARRY"), "c2");
            Gate* c3 = nl->create_gate(gl->get_gate_type_by_name("CARRY"), "c3");

            Gate* g0 = nl->create_gate(gl->get_gate_type_by_name("BUF"), "g0");
            Gate* g1 = nl->create_gate(gl->get_gate_type_by_name("BUF"), "g1");

            test_utils::connect(nl.get(), c0, "CO", c1, "CI");
            test_utils::connect(nl.get(), c1, "CO", c2, "CI");
            test_utils::connect(nl.get(), c2, "CO", c3, "CI");

            test_utils::connect(nl.get(), g0, "O", c0, "CI");
            test_utils::connect(nl.get(), c3, "CO", g1, "I");

            std::vector<Gate*> expected_chain = {c0, c1, c2, c3};

            std::vector<Gate*> chain_0 = netlist_utils::get_gate_chain(c0, {"CI"}, {"CO"});
            EXPECT_EQ(chain_0, expected_chain);

            std::vector<Gate*> chain_1 = netlist_utils::get_gate_chain(c1, {"CI"}, {"CO"});
            EXPECT_EQ(chain_1, expected_chain);

            std::vector<Gate*> chain_2 = netlist_utils::get_gate_chain(c2, {"CI"}, {"CO"});
            EXPECT_EQ(chain_2, expected_chain);

            std::vector<Gate*> chain_3 = netlist_utils::get_gate_chain(c3, {"CI"}, {"CO"});
            EXPECT_EQ(chain_3, expected_chain);

            std::vector<Gate*> chain_4 = netlist_utils::get_gate_chain(g0, {"CI"}, {"CO"}, [](const Gate* g) {
                return g->get_type()->has_property(GateTypeProperty::carry);
            });
            EXPECT_EQ(chain_4, std::vector<Gate*>());
        }
        {
            // test connection through unspecified pins
            std::unique_ptr<Netlist> nl = test_utils::create_empty_netlist();
            ASSERT_NE(nl, nullptr);
            const GateLibrary* gl       = nl->get_gate_library();
            ASSERT_NE(gl, nullptr);

            Gate* c0 = nl->create_gate(gl->get_gate_type_by_name("CARRY"), "c0");
            Gate* c1 = nl->create_gate(gl->get_gate_type_by_name("CARRY"), "c1");
            Gate* c2 = nl->create_gate(gl->get_gate_type_by_name("CARRY"), "c2");
            Gate* c3 = nl->create_gate(gl->get_gate_type_by_name("CARRY"), "c3");

            Gate* g0 = nl->create_gate(gl->get_gate_type_by_name("BUF"), "g0");
            Gate* g1 = nl->create_gate(gl->get_gate_type_by_name("BUF"), "g1");

            test_utils::connect(nl.get(), c0, "CO", c1, "I0");
            test_utils::connect(nl.get(), c1, "CO", c2, "CI");
            test_utils::connect(nl.get(), c2, "CO", c3, "I1");

            test_utils::connect(nl.get(), g0, "O", c0, "CI");
            test_utils::connect(nl.get(), c3, "CO", g1, "I");

            std::vector<Gate*> expected_chain = {c0, c1, c2, c3};

            std::vector<Gate*> chain_0 = netlist_utils::get_gate_chain(c0);
            EXPECT_EQ(chain_0, expected_chain);

            std::vector<Gate*> chain_1 = netlist_utils::get_gate_chain(c1);
            EXPECT_EQ(chain_1, expected_chain);

            std::vector<Gate*> chain_2 = netlist_utils::get_gate_chain(c2);
            EXPECT_EQ(chain_2, expected_chain);

            std::vector<Gate*> chain_3 = netlist_utils::get_gate_chain(c3);
            EXPECT_EQ(chain_3, expected_chain);
        }
        {
            // test loop
            std::unique_ptr<Netlist> nl = test_utils::create_empty_netlist();
            ASSERT_NE(nl, nullptr);
            const GateLibrary* gl       = nl->get_gate_library();
            ASSERT_NE(gl, nullptr);

            Gate* c0 = nl->create_gate(gl->get_gate_type_by_name("CARRY"), "c0");
            Gate* c1 = nl->create_gate(gl->get_gate_type_by_name("CARRY"), "c1");
            Gate* c2 = nl->create_gate(gl->get_gate_type_by_name("CARRY"), "c2");
            Gate* c3 = nl->create_gate(gl->get_gate_type_by_name("CARRY"), "c3");

            test_utils::connect(nl.get(), c0, "CO", c1, "CI");
            test_utils::connect(nl.get(), c1, "CO", c2, "CI");
            test_utils::connect(nl.get(), c2, "CO", c3, "CI");
            test_utils::connect(nl.get(), c3, "CO", c0, "CI");

            std::vector<Gate*> chain = netlist_utils::get_gate_chain(c0);
            EXPECT_EQ(chain, std::vector<Gate*>({c0, c1, c2, c3}));
        }
        {
            // test multiple successors
            std::unique_ptr<Netlist> nl = test_utils::create_empty_netlist();
            ASSERT_NE(nl, nullptr);
            const GateLibrary* gl       = nl->get_gate_library();
            ASSERT_NE(gl, nullptr);

            Gate* c0 = nl->create_gate(gl->get_gate_type_by_name("CARRY"), "c0");
            Gate* c1 = nl->create_gate(gl->get_gate_type_by_name("CARRY"), "c1");
            Gate* c2 = nl->create_gate(gl->get_gate_type_by_name("CARRY"), "c2");
            Gate* c3 = nl->create_gate(gl->get_gate_type_by_name("CARRY"), "c3");
            Gate* c4 = nl->create_gate(gl->get_gate_type_by_name("CARRY"), "c4");
            Gate* c5 = nl->create_gate(gl->get_gate_type_by_name("CARRY"), "c5");

            test_utils::connect(nl.get(), c0, "CO", c1, "CI");
            test_utils::connect(nl.get(), c1, "CO", c2, "CI");
            test_utils::connect(nl.get(), c2, "CO", c3, "CI");
            test_utils::connect(nl.get(), c1, "CO", c4, "CI");
            test_utils::connect(nl.get(), c4, "CO", c5, "CI");

            std::vector<Gate*> chain_0 = netlist_utils::get_gate_chain(c0);
            EXPECT_EQ(chain_0, std::vector<Gate*>({c0, c1}));

            std::vector<Gate*> chain_1 = netlist_utils::get_gate_chain(c1);
            EXPECT_EQ(chain_1, std::vector<Gate*>({c0, c1}));

            std::vector<Gate*> chain_2 = netlist_utils::get_gate_chain(c2);
            EXPECT_EQ(chain_2, std::vector<Gate*>({c0, c1, c2, c3}));

            std::vector<Gate*> chain_3 = netlist_utils::get_gate_chain(c3);
            EXPECT_EQ(chain_3, std::vector<Gate*>({c0, c1, c2, c3}));

            std::vector<Gate*> chain_4 = netlist_utils::get_gate_chain(c4);
            EXPECT_EQ(chain_4, std::vector<Gate*>({c0, c1, c4, c5}));

            std::vector<Gate*> chain_5 = netlist_utils::get_gate_chain(c5);
            EXPECT_EQ(chain_5, std::vector<Gate*>({c0, c1, c4, c5}));
        }
        TEST_END
    }

    /**
     * Testing detection of complex gate chains.
     *
     * Functions: get_complex_gate_chain
     */
    TEST_F(NetlistUtilsTest, check_get_complex_gate_chain)
    {
        TEST_START
        {
            std::unique_ptr<Netlist> nl = test_utils::create_empty_netlist();
            ASSERT_NE(nl, nullptr);
            const GateLibrary* gl       = nl->get_gate_library();
            ASSERT_NE(gl, nullptr);

            // TODO implement test
        }
        TEST_END
    }

}    //namespace hal
