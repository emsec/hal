#include "netlist_test_utils.h"
#include "hal_core/netlist/netlist_utils.h"

namespace hal {

    using test_utils::MIN_MODULE_ID;
    using test_utils::MIN_GATE_ID;
    using test_utils::MIN_NET_ID;

    class NetlistUtilsTest : public ::testing::Test {
    protected:
        virtual void SetUp() {
            test_utils::init_log_channels();
        }

        virtual void TearDown() {
        }
    };

    /**
     * Testing the get_subgraph_function
     *
     * Functions: get_subgraph_function
     */
    TEST_F(NetlistUtilsTest, check_get_subgraph_function) {
        TEST_START
            // Use the example netlist, that is filled with boolean functions for this test
            // (3) becomes a NOT gate, (0) an AND gate, (7) a NOT gate
            std::unique_ptr<Netlist> test_nl = test_utils::create_example_netlist();

            // -- Set the boolean functions for these gates
            Gate* gate_3 = test_nl->get_gate_by_id(MIN_GATE_ID+3);
            Gate* gate_0 = test_nl->get_gate_by_id(MIN_GATE_ID+0);
            Gate* gate_7 = test_nl->get_gate_by_id(MIN_GATE_ID+7);
            gate_3->add_boolean_function("O",
                BooleanFunction::from_string("!I", std::vector<std::string>({"I"})));
            gate_0->add_boolean_function("O",
                BooleanFunction::from_string("I0 & I1", std::vector<std::string>({"I0", "I1"})));
            gate_7->add_boolean_function("O",
                BooleanFunction::from_string("!I", std::vector<std::string>({"I"})));
            // -- Get the names of the connected nets
            std::string net_13_name = std::to_string(MIN_NET_ID+13);
            std::string net_20_name = std::to_string(MIN_NET_ID+20);
            std::string net_78_name = std::to_string(MIN_NET_ID+78);
            std::string net_30_name = std::to_string(MIN_NET_ID+30);

            {
                // Get the boolean function of a normal sub-graph
                const std::vector<const Gate*> subgraph_gates({gate_0, gate_3});
                const Net* output_net = test_nl->get_net_by_id(MIN_NET_ID+045);
                BooleanFunction sub_graph_bf = netlist_utils::get_subgraph_function(subgraph_gates, output_net);

                BooleanFunction expected_bf = BooleanFunction::from_string(("!"+net_13_name+" & "+net_20_name),
                    std::vector<std::string>({net_13_name, net_20_name}));

                EXPECT_EQ(sub_graph_bf, expected_bf);
            }
            // NEGATIVE
            {
                // No subgraph gates are passed
                const std::vector<const Gate*> subgraph_gates({});
                const Net* output_net = test_nl->get_net_by_id(MIN_NET_ID+045);
                BooleanFunction sub_graph_bf = netlist_utils::get_subgraph_function(subgraph_gates, output_net);

                EXPECT_TRUE(sub_graph_bf.is_empty());
            }
            {
                // One of the gates is a nullptr
                const std::vector<const Gate*> subgraph_gates({gate_0, nullptr, gate_3});
                const Net* output_net = test_nl->get_net_by_id(MIN_NET_ID+045);
                BooleanFunction sub_graph_bf = netlist_utils::get_subgraph_function(subgraph_gates, output_net);

                EXPECT_TRUE(sub_graph_bf.is_empty());
            }
            {
                // The output net is a nullptr
                const std::vector<const Gate*> subgraph_gates({gate_0, gate_3});
                const Net* output_net = nullptr;
                BooleanFunction sub_graph_bf = netlist_utils::get_subgraph_function(subgraph_gates, output_net);

                EXPECT_TRUE(sub_graph_bf.is_empty());
            }
            {
                // The output net has multiple sources
                // -- create such a net
                Net* multi_src_net = test_nl->create_net("muli_src_net");
                multi_src_net->add_source(test_nl->get_gate_by_id(MIN_GATE_ID+4), "O");
                multi_src_net->add_source(test_nl->get_gate_by_id(MIN_GATE_ID+5), "O");

                const std::vector<const Gate*> subgraph_gates({gate_0, gate_3});
                const Net* output_net = multi_src_net;
                BooleanFunction sub_graph_bf = netlist_utils::get_subgraph_function(subgraph_gates, output_net);

                EXPECT_TRUE(sub_graph_bf.is_empty());
                // -- remove the net
                test_nl->delete_net(multi_src_net);
            }
            {
                // The output net has no source
                // -- create such a net
                Net* no_src_net = test_nl->create_net("muli_src_net");

                const std::vector<const Gate*> subgraph_gates({gate_0, gate_3});
                const Net* output_net = no_src_net;
                BooleanFunction sub_graph_bf = netlist_utils::get_subgraph_function(subgraph_gates, output_net);

                EXPECT_TRUE(sub_graph_bf.is_empty());
                // -- remove the net
                test_nl->delete_net(no_src_net);
            }
            {
                // A net in between has multiple sources (expansion should stop in this direction)
                // -- add a source to net 30 temporarily
                test_nl->get_net_by_id(MIN_NET_ID+30)->add_source(test_nl->get_gate_by_id(MIN_GATE_ID+8),"O");

                const std::vector<const Gate*> subgraph_gates({gate_0, gate_3});
                const Net* output_net = test_nl->get_net_by_id(MIN_NET_ID+045);
                BooleanFunction sub_graph_bf = netlist_utils::get_subgraph_function(subgraph_gates, output_net);

                BooleanFunction expected_bf = BooleanFunction::from_string((net_30_name+" & "+net_20_name),
                                                                           std::vector<std::string>({net_30_name, net_20_name}));

                EXPECT_EQ(sub_graph_bf, expected_bf);
            }
            {
                // The netlist contains a cycle
                // -- create such a netlist:
                 /*   .-=|gate_0|=----.
                 *    '------------.  |
                 *    .-=|gate_1|=-'  |
                 *    '---------------'
                 */
                std::unique_ptr<Netlist> cyclic_nl = test_utils::create_empty_netlist();
                Gate* cy_gate_0 = cyclic_nl->create_gate(test_utils::get_gate_type_by_name("gate_1_to_1"), "gate_0");
                Gate* cy_gate_1 = cyclic_nl->create_gate(test_utils::get_gate_type_by_name("gate_1_to_1"), "gate_1");
                cy_gate_0->add_boolean_function("O",BooleanFunction::from_string("O",{"I"}));
                cy_gate_1->add_boolean_function("O",BooleanFunction::from_string("O",{"I"}));

                Net* cy_net_0 = cyclic_nl->create_net("net_0");
                Net* cy_net_1 = cyclic_nl->create_net("net_1");
                cy_net_0->add_source(cy_gate_0, "O");
                cy_net_0->add_destination(cy_gate_1, "I");
                cy_net_1->add_source(cy_gate_1, "O");
                cy_net_1->add_destination(cy_gate_0, "I");

                const std::vector<const Gate*> subgraph_gates({cy_gate_0, cy_gate_1});
                const Net* output_net = cy_net_0;
                BooleanFunction sub_graph_bf = netlist_utils::get_subgraph_function(subgraph_gates, output_net);

                EXPECT_TRUE(sub_graph_bf.is_empty());
            }
            {
                // A gate of the subgraph has unconnected input pins
                const std::vector<const Gate*> subgraph_gates({gate_7});
                const Net* output_net = test_nl->get_net_by_id(MIN_NET_ID+78);
                BooleanFunction sub_graph_bf = netlist_utils::get_subgraph_function(subgraph_gates, output_net);

                EXPECT_EQ(sub_graph_bf, gate_7->get_boolean_function("O"));
            }
        TEST_END
    }

    /**
     * Testing the deep copying of netlists
     *
     * Functions: copy_netlist
     */
    TEST_F(NetlistUtilsTest, check_copy_netlist) {
        TEST_START
            // Create an example netlist that should be copied
            std::unique_ptr<Netlist> test_nl = test_utils::create_example_netlist();

            // -- Add some modules to the example netlist
            Module* mod_0 = test_nl->create_module(test_utils::MIN_MODULE_ID+0, "mod_0", test_nl->get_top_module(), std::vector<Gate*>{
                test_nl->get_gate_by_id(MIN_GATE_ID+0), test_nl->get_gate_by_id(MIN_GATE_ID+1),
                test_nl->get_gate_by_id(MIN_GATE_ID+2), test_nl->get_gate_by_id(MIN_GATE_ID+3),
                test_nl->get_gate_by_id(MIN_GATE_ID+4), test_nl->get_gate_by_id(MIN_GATE_ID+5)
            });
            Module* mod_1 = test_nl->create_module(test_utils::MIN_MODULE_ID+1, "mod_1", mod_0, std::vector<Gate*>{
                test_nl->get_gate_by_id(MIN_GATE_ID+0), test_nl->get_gate_by_id(MIN_GATE_ID+4),
                test_nl->get_gate_by_id(MIN_GATE_ID+5),
            });

            // -- Add some groupings to the netlist
            Grouping* prime_grouping = test_nl->create_grouping("prime_gates");
            prime_grouping->assign_gate_by_id(MIN_GATE_ID+2);
            prime_grouping->assign_gate_by_id(MIN_GATE_ID+3);
            prime_grouping->assign_gate_by_id(MIN_GATE_ID+5);
            prime_grouping->assign_gate_by_id(MIN_GATE_ID+7);
            prime_grouping->assign_net_by_id(MIN_NET_ID+13);
            prime_grouping->assign_module_by_id(MIN_MODULE_ID+1); // (I know 1 is not prime :P)
            Grouping* empty_grouping = test_nl->create_grouping("empty_grouping");

            // -- Mark Gates as GND/VCC
            test_nl->mark_gnd_gate(test_nl->get_gate_by_id(MIN_GATE_ID+1));
            test_nl->mark_vcc_gate(test_nl->get_gate_by_id(MIN_GATE_ID+2));

            // -- Mark nets as global inputs/outputs
            test_nl->mark_global_input_net(test_nl->get_net_by_id(MIN_NET_ID+20));
            test_nl->mark_global_output_net(test_nl->get_net_by_id(MIN_NET_ID+78));

            // -- Add some boolean functions to the gates
            test_nl->get_gate_by_id(MIN_GATE_ID+0)->add_boolean_function("O", BooleanFunction::from_string("I0 & I1"));
            test_nl->get_gate_by_id(MIN_GATE_ID+4)->add_boolean_function("O", BooleanFunction::from_string("!I"));

            // Copy and compare the netlist
            std::unique_ptr<Netlist> test_nl_copy = netlist_utils::copy_netlist(test_nl.get());

            EXPECT_TRUE(test_utils::netlists_are_equal(test_nl.get(), test_nl_copy.get()));
        TEST_END
    }


} //namespace hal
