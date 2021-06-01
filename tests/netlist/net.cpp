#include "netlist_test_utils.h"
#include "hal_core/netlist/event_handler.h"

namespace hal {
    using test_utils::MIN_GATE_ID;
    using test_utils::MIN_NET_ID;
    using test_utils::MIN_MODULE_ID;
    using test_utils::MIN_NETLIST_ID;

    class NetTest : public ::testing::Test 
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
     * Testing the constructor of the Net
     *
     * Functions: constructor, get_id, get_name, get_netlist
     */
    TEST_F(NetTest, check_constructor) 
    {
        TEST_START
        {
            auto nl = test_utils::create_empty_netlist();
            Net* test_net = nl->create_net(100, "test_net");

            EXPECT_EQ(test_net->get_id(), 100);
            EXPECT_EQ(test_net->get_name(), "test_net");
        }
        TEST_END
    }

    /**
     * Test operators for equality and inequality.
     * 
     * Functions: operator==, operator!=
     */
    TEST_F(NetTest, check_operators)
    {
        TEST_START
        {
            std::unique_ptr<Netlist> nl_1 = test_utils::create_empty_netlist();
            ASSERT_NE(nl_1, nullptr);
            std::unique_ptr<Netlist> nl_2 = test_utils::create_empty_netlist();
            ASSERT_NE(nl_2, nullptr);
            const GateLibrary* gl = nl_1->get_gate_library();
            ASSERT_NE(gl, nullptr);
            ASSERT_EQ(gl, nl_2->get_gate_library());

            Gate* nl1_dummy_1 = nl_1->create_gate(gl->get_gate_type_by_name("BUF"), "dummy_1");
            ASSERT_NE(nl1_dummy_1, nullptr);
            Gate* nl1_dummy_2 = nl_1->create_gate(gl->get_gate_type_by_name("BUF"), "dummy_2");
            ASSERT_NE(nl1_dummy_2, nullptr);
            Gate* nl1_dummy_3 = nl_1->create_gate(gl->get_gate_type_by_name("BUF"), "dummy_3");
            ASSERT_NE(nl1_dummy_3, nullptr);
            Gate* nl2_dummy_1 = nl_2->create_gate(gl->get_gate_type_by_name("BUF"), "dummy_1");
            ASSERT_NE(nl2_dummy_1, nullptr);
            Gate* nl2_dummy_2 = nl_2->create_gate(gl->get_gate_type_by_name("BUF"), "dummy_2");
            ASSERT_NE(nl2_dummy_2, nullptr);
            Gate* nl2_dummy_3 = nl_2->create_gate(gl->get_gate_type_by_name("BUF"), "dummy_3");
            ASSERT_NE(nl2_dummy_3, nullptr);

            // standard stuff
            Net* nl1_n1 = nl_1->create_net(1, "net_1");
            ASSERT_NE(nl1_n1, nullptr);
            Net* nl2_n1 = nl_2->create_net(1, "net_1");
            ASSERT_NE(nl2_n1, nullptr);
            Net* nl1_n2 = nl_1->create_net(2, "net_1");
            ASSERT_NE(nl1_n2, nullptr);
            Net* nl2_n2 = nl_2->create_net(2, "net_2");
            ASSERT_NE(nl2_n2, nullptr);

            EXPECT_TRUE(*nl1_n1 == *nl1_n1);        // identical gate pointer
            EXPECT_TRUE(*nl2_n1 == *nl2_n1);
            EXPECT_FALSE(*nl1_n1 != *nl1_n1);
            EXPECT_FALSE(*nl2_n1 != *nl2_n1);
            EXPECT_TRUE(*nl1_n1 == *nl2_n1);        // identical gates, but different netlists
            EXPECT_TRUE(*nl2_n1 == *nl1_n1);
            EXPECT_FALSE(*nl1_n1 != *nl2_n1);
            EXPECT_FALSE(*nl2_n1 != *nl1_n1);
            EXPECT_FALSE(*nl1_n1 == *nl1_n2);       // different IDs
            EXPECT_FALSE(*nl2_n1 == *nl2_n2);
            EXPECT_TRUE(*nl1_n1 != *nl1_n2);
            EXPECT_TRUE(*nl2_n1 != *nl2_n2);
            EXPECT_FALSE(*nl1_n2 == *nl2_n2);       // different names
            EXPECT_FALSE(*nl2_n2 == *nl1_n2);
            EXPECT_TRUE(*nl1_n2 != *nl2_n2);
            EXPECT_TRUE(*nl2_n2 != *nl1_n2);

            // global inputs/outputs
            Net* nl1_n3 = nl_1->create_net(3, "net_3");
            ASSERT_NE(nl1_n3, nullptr);
            nl1_n3->mark_global_input_net();
            Net* nl2_n3 = nl_2->create_net(3, "net_3");
            ASSERT_NE(nl2_n3, nullptr);
            EXPECT_FALSE(*nl1_n3 == *nl2_n3);       // one not marked as global in
            EXPECT_FALSE(*nl2_n3 == *nl1_n3);
            EXPECT_TRUE(*nl1_n3 != *nl2_n3);
            EXPECT_TRUE(*nl2_n3 != *nl1_n3);

            Net* nl1_n4 = nl_1->create_net(4, "net_4");
            ASSERT_NE(nl1_n4, nullptr);
            nl1_n4->mark_global_output_net();
            Net* nl2_n4 = nl_2->create_net(4, "net_4");
            ASSERT_NE(nl2_n4, nullptr);
            EXPECT_FALSE(*nl1_n4 == *nl2_n4);       // one not marked as global out
            EXPECT_FALSE(*nl2_n4 == *nl1_n4);
            EXPECT_TRUE(*nl1_n4 != *nl2_n4);
            EXPECT_TRUE(*nl2_n4 != *nl1_n4);

            // sources
            Net* nl1_n5 = nl_1->create_net(5, "net_5");
            ASSERT_NE(nl1_n5, nullptr);
            ASSERT_NE(nl1_n5->add_source(nl1_dummy_1, "O"), nullptr);
            ASSERT_NE(nl1_n5->add_source(nl1_dummy_2, "O"), nullptr);
            ASSERT_NE(nl1_n5->add_source(nl1_dummy_3, "O"), nullptr);
            Net* nl2_n5 = nl_2->create_net(5, "net_5");
            ASSERT_NE(nl2_n5, nullptr);
            ASSERT_NE(nl2_n5->add_source(nl2_dummy_1, "O"), nullptr);
            ASSERT_NE(nl2_n5->add_source(nl2_dummy_2, "O"), nullptr);
            ASSERT_NE(nl2_n5->add_source(nl2_dummy_3, "O"), nullptr);
            EXPECT_TRUE(*nl1_n5 == *nl2_n5);        // identical sources, identical order
            EXPECT_TRUE(*nl2_n5 == *nl1_n5); 
            EXPECT_FALSE(*nl1_n5 != *nl2_n5);
            EXPECT_FALSE(*nl2_n5 != *nl1_n5); 
            test_utils::clear_connections(nl1_n5);
            test_utils::clear_connections(nl2_n5);

            Net* nl1_n6 = nl_1->create_net(6, "net_6");
            ASSERT_NE(nl1_n6, nullptr);
            ASSERT_NE(nl1_n6->add_source(nl1_dummy_1, "O"), nullptr);
            ASSERT_NE(nl1_n6->add_source(nl1_dummy_2, "O"), nullptr);
            ASSERT_NE(nl1_n6->add_source(nl1_dummy_3, "O"), nullptr);
            Net* nl2_n6 = nl_2->create_net(6, "net_6");
            ASSERT_NE(nl2_n6, nullptr);
            ASSERT_NE(nl2_n6->add_source(nl2_dummy_3, "O"), nullptr);
            ASSERT_NE(nl2_n6->add_source(nl2_dummy_2, "O"), nullptr);
            ASSERT_NE(nl2_n6->add_source(nl2_dummy_1, "O"), nullptr);
            EXPECT_TRUE(*nl1_n6 == *nl2_n6);        // identical sources, different order
            EXPECT_TRUE(*nl2_n6 == *nl1_n6);
            EXPECT_FALSE(*nl1_n6 != *nl2_n6);
            EXPECT_FALSE(*nl2_n6 != *nl1_n6);
            test_utils::clear_connections(nl1_n6);
            test_utils::clear_connections(nl2_n6);

            Net* nl1_n7 = nl_1->create_net(7, "net_7");
            ASSERT_NE(nl1_n7, nullptr);
            ASSERT_NE(nl1_n7->add_source(nl1_dummy_1, "O"), nullptr);
            Net* nl2_n7 = nl_2->create_net(7, "net_7");
            ASSERT_NE(nl2_n7, nullptr);
            ASSERT_NE(nl2_n7->add_source(nl2_dummy_2, "O"), nullptr);
            ASSERT_NE(nl2_n7->add_source(nl2_dummy_1, "O"), nullptr);
            EXPECT_FALSE(*nl1_n7 == *nl2_n7);       // different number of sources
            EXPECT_FALSE(*nl2_n7 == *nl1_n7); 
            EXPECT_TRUE(*nl1_n7 != *nl2_n7);
            EXPECT_TRUE(*nl2_n7 != *nl1_n7); 
            test_utils::clear_connections(nl1_n7);
            test_utils::clear_connections(nl2_n7);

            Net* nl1_n8 = nl_1->create_net(8, "net_8");
            ASSERT_NE(nl1_n8, nullptr);
            ASSERT_NE(nl1_n8->add_source(nl1_dummy_1, "O"), nullptr);
            ASSERT_NE(nl1_n8->add_source(nl1_dummy_2, "O"), nullptr);
            Net* nl2_n8 = nl_2->create_net(8, "net_8");
            ASSERT_NE(nl2_n8, nullptr);
            ASSERT_NE(nl2_n8->add_source(nl2_dummy_2, "O"), nullptr);
            ASSERT_NE(nl2_n8->add_source(nl2_dummy_3, "O"), nullptr);
            EXPECT_FALSE(*nl1_n8 == *nl2_n8);       // different sources
            EXPECT_FALSE(*nl2_n8 == *nl1_n8);
            EXPECT_TRUE(*nl1_n8 != *nl2_n8);
            EXPECT_TRUE(*nl2_n8 != *nl1_n8);
            test_utils::clear_connections(nl1_n8);
            test_utils::clear_connections(nl2_n8);

            // destinations
            Net* nl1_n9 = nl_1->create_net(9, "net_9");
            ASSERT_NE(nl1_n9, nullptr);
            ASSERT_NE(nl1_n9->add_destination(nl1_dummy_1, "I"), nullptr);
            ASSERT_NE(nl1_n9->add_destination(nl1_dummy_2, "I"), nullptr);
            ASSERT_NE(nl1_n9->add_destination(nl1_dummy_3, "I"), nullptr);
            Net* nl2_n9 = nl_2->create_net(9, "net_9");
            ASSERT_NE(nl2_n9, nullptr);
            ASSERT_NE(nl2_n9->add_destination(nl2_dummy_1, "I"), nullptr);
            ASSERT_NE(nl2_n9->add_destination(nl2_dummy_2, "I"), nullptr);
            ASSERT_NE(nl2_n9->add_destination(nl2_dummy_3, "I"), nullptr);
            EXPECT_TRUE(*nl1_n9 == *nl2_n9);        // identical destinations, identical order
            EXPECT_TRUE(*nl2_n9 == *nl1_n9); 
            EXPECT_FALSE(*nl1_n9 != *nl2_n9);
            EXPECT_FALSE(*nl2_n9 != *nl1_n9); 
            test_utils::clear_connections(nl1_n9);
            test_utils::clear_connections(nl2_n9);

            Net* nl1_n10 = nl_1->create_net(10, "net_10");
            ASSERT_NE(nl1_n10, nullptr);
            ASSERT_NE(nl1_n10->add_destination(nl1_dummy_1, "I"), nullptr);
            ASSERT_NE(nl1_n10->add_destination(nl1_dummy_2, "I"), nullptr);
            ASSERT_NE(nl1_n10->add_destination(nl1_dummy_3, "I"), nullptr);
            Net* nl2_n10 = nl_2->create_net(10, "net_10");
            ASSERT_NE(nl2_n10, nullptr);
            ASSERT_NE(nl2_n10->add_destination(nl2_dummy_3, "I"), nullptr);
            ASSERT_NE(nl2_n10->add_destination(nl2_dummy_2, "I"), nullptr);
            ASSERT_NE(nl2_n10->add_destination(nl2_dummy_1, "I"), nullptr);
            EXPECT_TRUE(*nl1_n10 == *nl2_n10);        // identical destinations, different order
            EXPECT_TRUE(*nl2_n10 == *nl1_n10);
            EXPECT_FALSE(*nl1_n10 != *nl2_n10);
            EXPECT_FALSE(*nl2_n10 != *nl1_n10);
            test_utils::clear_connections(nl1_n10);
            test_utils::clear_connections(nl2_n10);

            Net* nl1_n11 = nl_1->create_net(11, "net_11");
            ASSERT_NE(nl1_n11, nullptr);
            ASSERT_NE(nl1_n11->add_destination(nl1_dummy_1, "I"), nullptr);
            Net* nl2_n11 = nl_2->create_net(11, "net_11");
            ASSERT_NE(nl2_n11, nullptr);
            ASSERT_NE(nl2_n11->add_destination(nl2_dummy_2, "I"), nullptr);
            ASSERT_NE(nl2_n11->add_destination(nl2_dummy_1, "I"), nullptr);
            EXPECT_FALSE(*nl1_n11 == *nl2_n11);       // different number of destinations
            EXPECT_FALSE(*nl2_n11 == *nl1_n11); 
            EXPECT_TRUE(*nl1_n11 != *nl2_n11);
            EXPECT_TRUE(*nl2_n11 != *nl1_n11); 
            test_utils::clear_connections(nl1_n11);
            test_utils::clear_connections(nl2_n11);

            Net* nl1_n12 = nl_1->create_net(12, "net_12");
            ASSERT_NE(nl1_n12, nullptr);
            ASSERT_NE(nl1_n12->add_destination(nl1_dummy_1, "I"), nullptr);
            ASSERT_NE(nl1_n12->add_destination(nl1_dummy_2, "I"), nullptr);
            Net* nl2_n12 = nl_2->create_net(12, "net_12");
            ASSERT_NE(nl2_n12, nullptr);
            ASSERT_NE(nl2_n12->add_destination(nl2_dummy_2, "I"), nullptr);
            ASSERT_NE(nl2_n12->add_destination(nl2_dummy_3, "I"), nullptr);
            EXPECT_FALSE(*nl1_n12 == *nl2_n12);       // different destinations
            EXPECT_FALSE(*nl2_n12 == *nl1_n12);
            EXPECT_TRUE(*nl1_n12 != *nl2_n12);
            EXPECT_TRUE(*nl2_n12 != *nl1_n12);
        }
        TEST_END
    }

    /**
     * Testing the function set_name and get_name
     *
     * Functions: get_name, set_name
     */
    TEST_F(NetTest, check_set_and_get_name) 
    {
        TEST_START
            {
                // Create a Net and append it to its netlist
                auto nl = test_utils::create_empty_netlist();
                ASSERT_NE(nl, nullptr);
                Net* test_net = nl->create_net("test_net");
                ASSERT_NE(test_net, nullptr);

                EXPECT_EQ(test_net->get_name(), "test_net");

                // Set a new name
                NO_COUT(test_net->set_name("new_name"));
                EXPECT_EQ(test_net->get_name(), "new_name");

                // Set an empty name (should do nothing)
                NO_COUT(test_net->set_name(""));
                EXPECT_EQ(test_net->get_name(), "new_name");
            }
        TEST_END
    }

    /**
     * Test adding and retrieving sources.
     *
     * Functions: add_source, get_sources, remove_source
     */
    TEST_F(NetTest, check_sources) {
        TEST_START
            {
                // get sources of net without sources
                auto nl = test_utils::create_empty_netlist();
                ASSERT_NE(nl, nullptr);
                Net* test_net = nl->create_net("test_net");
                ASSERT_NE(test_net, nullptr);
            }
            {
                // add source to net
                auto nl = test_utils::create_empty_netlist();
                ASSERT_NE(nl, nullptr);
                Net* test_net = nl->create_net("test_net");
                ASSERT_NE(test_net, nullptr);
                EXPECT_TRUE(test_net->get_sources().empty());
                Gate* test_gate = nl->create_gate(nl->get_gate_library()->get_gate_type_by_name("BUF"), "test_gate");
                ASSERT_NE(test_gate, nullptr);
                Endpoint* ep = test_net->add_source(test_gate, "O");
                EXPECT_NE(ep, nullptr);
                ASSERT_EQ(test_net->get_sources().size(), 1);
                EXPECT_EQ(test_net->get_sources().at(0), test_gate->get_fan_out_endpoint("O"));
            }
            {
                // get multiple sources (no filter applied)
                auto nl = test_utils::create_empty_netlist();
                ASSERT_NE(nl, nullptr);
                Net* test_net = nl->create_net("test_net");
                ASSERT_NE(test_net, nullptr);
                Gate* test_gate = nl->create_gate(nl->get_gate_library()->get_gate_type_by_name("RAM"), "test_gate");
                ASSERT_NE(test_gate, nullptr);
                test_net->add_source(test_gate, "DATA_OUT(0)");
                test_net->add_source(test_gate, "DATA_OUT(1)");
                test_net->add_source(test_gate, "DATA_OUT(2)");
                test_net->add_source(test_gate, "DATA_OUT(3)");
                EXPECT_EQ(test_net->get_sources(), std::vector<Endpoint*>(test_gate->get_fan_out_endpoints()));
            }
            {
                // get multiple sources (filter applied)
                auto nl = test_utils::create_empty_netlist();
                ASSERT_NE(nl, nullptr);
                Net* test_net = nl->create_net("test_net");
                ASSERT_NE(test_net, nullptr);
                Gate* test_gate_1 = nl->create_gate(nl->get_gate_library()->get_gate_type_by_name("RAM"), "test_gate_1");
                ASSERT_NE(test_gate_1, nullptr);
                Gate* test_gate_2 = nl->create_gate(nl->get_gate_library()->get_gate_type_by_name("RAM"), "test_gate_2");
                ASSERT_NE(test_gate_2, nullptr);
                EXPECT_NE(test_net->add_source(test_gate_1, "DATA_OUT(0)"), nullptr);
                EXPECT_NE(test_net->add_source(test_gate_1, "DATA_OUT(1)"), nullptr);
                EXPECT_NE(test_net->add_source(test_gate_2, "DATA_OUT(0)"), nullptr);
                EXPECT_NE(test_net->add_source(test_gate_2, "DATA_OUT(1)"), nullptr);
                EXPECT_EQ(test_net->get_sources([](const Endpoint* ep){return ep->get_gate()->get_name() == "test_gate_1";}),
                          std::vector<Endpoint*>(test_gate_1->get_fan_out_endpoints()));
            }
            {
                // remove a source by specifying gate and pin
                auto nl = test_utils::create_empty_netlist();
                ASSERT_NE(nl, nullptr);
                Net* test_net = nl->create_net("test_net");
                ASSERT_NE(test_net, nullptr);
                Gate* test_gate = nl->create_gate(nl->get_gate_library()->get_gate_type_by_name("BUF"), "test_gate");
                ASSERT_NE(test_gate, nullptr);
                EXPECT_NE(test_net->add_source(test_gate, "O"), nullptr);
                EXPECT_TRUE(test_net->remove_source(test_gate, "O"));
                EXPECT_TRUE(test_net->get_sources().empty());
            }
            {
                // remove a source by specifying endpoint
                auto nl = test_utils::create_empty_netlist();
                ASSERT_NE(nl, nullptr);
                Net* test_net = nl->create_net("test_net");
                ASSERT_NE(test_net, nullptr);
                Gate* test_gate = nl->create_gate(nl->get_gate_library()->get_gate_type_by_name("BUF"), "test_gate");
                ASSERT_NE(test_gate, nullptr);
                Endpoint* ep = test_net->add_source(test_gate, "O");
                ASSERT_NE(ep, nullptr);
                EXPECT_TRUE(test_net->remove_source(ep));
                EXPECT_TRUE(test_net->get_sources().empty());
            }
            // Negative
            {
                // add invalid source
                NO_COUT_TEST_BLOCK;
                auto nl = test_utils::create_empty_netlist();
                ASSERT_NE(nl, nullptr);
                Net* test_net = nl->create_net("test_net");
                ASSERT_NE(test_net, nullptr);
                Gate* test_gate = nl->create_gate(nl->get_gate_library()->get_gate_type_by_name("BUF"), "test_gate");
                ASSERT_NE(test_gate, nullptr);
                EXPECT_EQ(test_net->add_source(test_gate, "INVALID"), nullptr);  // invalid pin
                EXPECT_EQ(test_net->add_source(test_gate, "I"), nullptr);        // input pin
                EXPECT_EQ(test_net->add_source(test_gate, ""), nullptr);         // empty pin
                EXPECT_EQ(test_net->add_source(nullptr, "O"), nullptr);          // nullptr gate
                EXPECT_TRUE(test_net->get_sources().empty());
            }
            {
                // add source twice
                NO_COUT_TEST_BLOCK;
                auto nl = test_utils::create_empty_netlist();
                Net* test_net_1 = nl->create_net("test_net_1");
                ASSERT_NE(test_net_1, nullptr);
                Net* test_net_2 = nl->create_net("test_net_2");
                ASSERT_NE(test_net_2, nullptr);
                Gate* test_gate = nl->create_gate(nl->get_gate_library()->get_gate_type_by_name("BUF"), "test_gate");
                ASSERT_NE(test_gate, nullptr);
                EXPECT_NE(test_net_1->add_source(test_gate, "O"), nullptr);
                EXPECT_EQ(test_net_2->add_source(test_gate, "O"), nullptr);
                EXPECT_TRUE(test_net_2->get_sources().empty());
            }
            {
                // remove invalid source
                NO_COUT_TEST_BLOCK;
                auto nl = test_utils::create_empty_netlist();
                ASSERT_NE(nl, nullptr);
                Net* test_net = nl->create_net("test_net");
                ASSERT_NE(test_net, nullptr);
                Gate* test_gate = nl->create_gate(nl->get_gate_library()->get_gate_type_by_name("BUF"), "test_gate");
                ASSERT_NE(test_gate, nullptr);
                EXPECT_NE(test_net->add_source(test_gate, "O"), nullptr);
                EXPECT_FALSE(test_net->remove_source(test_gate, "INVALID"));  // invalid pin
                EXPECT_FALSE(test_net->remove_source(test_gate, "I"));        // input pin
                EXPECT_FALSE(test_net->remove_source(test_gate, ""));         // empty pin
                EXPECT_FALSE(test_net->remove_source(nullptr, "O"));          // nullptr gate
                EXPECT_FALSE(test_net->remove_source(nullptr));               // nullptr endpoint
                EXPECT_EQ(test_net->get_sources().size(), 1);
            }
            {
                // remove source twice by specifying gate and pin
                NO_COUT_TEST_BLOCK;
                auto nl = test_utils::create_empty_netlist();
                ASSERT_NE(nl, nullptr);
                Net* test_net = nl->create_net("test_net");
                ASSERT_NE(test_net, nullptr);
                Gate* test_gate = nl->create_gate(nl->get_gate_library()->get_gate_type_by_name("BUF"), "test_gate");
                ASSERT_NE(test_gate, nullptr);
                EXPECT_NE(test_net->add_source(test_gate, "O"), nullptr);
                EXPECT_TRUE(test_net->remove_source(test_gate, "O"));
                EXPECT_FALSE(test_net->remove_source(test_gate, "O"));
                EXPECT_TRUE(test_net->get_sources().empty());
            }
            {
                // remove source twice by specifying endpoint
                NO_COUT_TEST_BLOCK;
                auto nl = test_utils::create_empty_netlist();
                ASSERT_NE(nl, nullptr);
                Net* test_net = nl->create_net("test_net");
                ASSERT_NE(test_net, nullptr);
                Gate* test_gate = nl->create_gate(nl->get_gate_library()->get_gate_type_by_name("BUF"), "test_gate");
                ASSERT_NE(test_gate, nullptr);
                Endpoint* ep = test_net->add_source(test_gate, "O");
                ASSERT_NE(ep, nullptr);
                EXPECT_TRUE(test_net->remove_source(ep));
                EXPECT_FALSE(test_net->remove_source(ep));
                EXPECT_TRUE(test_net->get_sources().empty());
            }
        TEST_END
    }

    /**
     * Test adding and retrieving destinations.
     *
     * Functions: add_destination, get_destinations, remove_destination
     */
    TEST_F(NetTest, check_destinations) {
        TEST_START
            {
                // get destinations of net without destinations
                auto nl = test_utils::create_empty_netlist();
                ASSERT_NE(nl, nullptr);
                Net* test_net = nl->create_net("test_net");
                ASSERT_NE(test_net, nullptr);
                EXPECT_TRUE(test_net->get_destinations().empty());
            }
            {
                // add destination to net
                auto nl = test_utils::create_empty_netlist();
                ASSERT_NE(nl, nullptr);
                Net* test_net = nl->create_net("test_net");
                ASSERT_NE(test_net, nullptr);
                Gate* test_gate = nl->create_gate(nl->get_gate_library()->get_gate_type_by_name("BUF"), "test_gate");
                ASSERT_NE(test_gate, nullptr);
                EXPECT_TRUE(test_net->get_destinations().empty());
                EXPECT_NE(test_net->add_destination(test_gate, "I"), nullptr);
                ASSERT_EQ(test_net->get_destinations().size(), 1);
                EXPECT_EQ(test_net->get_destinations().at(0), test_gate->get_fan_in_endpoint("I"));
            }
            {
                // get multiple destinations (no filter applied)
                auto nl = test_utils::create_empty_netlist();
                ASSERT_NE(nl, nullptr);
                Net* test_net = nl->create_net("test_net");
                ASSERT_NE(test_net, nullptr);
                Gate* test_gate = nl->create_gate(nl->get_gate_library()->get_gate_type_by_name("RAM"), "test_gate");
                ASSERT_NE(test_gate, nullptr);
                EXPECT_NE(test_net->add_destination(test_gate, "DATA_IN(0)"), nullptr);
                EXPECT_NE(test_net->add_destination(test_gate, "DATA_IN(1)"), nullptr);
                EXPECT_NE(test_net->add_destination(test_gate, "DATA_IN(2)"), nullptr);
                EXPECT_NE(test_net->add_destination(test_gate, "DATA_IN(3)"), nullptr);
                EXPECT_EQ(test_net->get_destinations(), std::vector<Endpoint*>(test_gate->get_fan_in_endpoints()));
            }
            {
                // get multiple destinations (filter applied)
                auto nl = test_utils::create_empty_netlist();
                ASSERT_NE(nl, nullptr);
                Net* test_net = nl->create_net("test_net");
                ASSERT_NE(test_net, nullptr);
                Gate* test_gate_1 = nl->create_gate(nl->get_gate_library()->get_gate_type_by_name("RAM"), "test_gate_1");
                ASSERT_NE(test_gate_1, nullptr);
                Gate* test_gate_2 = nl->create_gate(nl->get_gate_library()->get_gate_type_by_name("RAM"), "test_gate_2");
                ASSERT_NE(test_gate_2, nullptr);
                EXPECT_NE(test_net->add_destination(test_gate_1, "DATA_IN(0)"), nullptr);
                EXPECT_NE(test_net->add_destination(test_gate_1, "DATA_IN(1)"), nullptr);
                EXPECT_NE(test_net->add_destination(test_gate_2, "DATA_IN(0)"), nullptr);
                EXPECT_NE(test_net->add_destination(test_gate_2, "DATA_IN(1)"), nullptr);
                EXPECT_EQ(test_net->get_destinations([](const Endpoint* ep){return ep->get_gate()->get_name() == "test_gate_1";}),
                          std::vector<Endpoint*>(test_gate_1->get_fan_in_endpoints()));
            }
            {
                // remove a destination by specifying gate and pin
                auto nl = test_utils::create_empty_netlist();
                ASSERT_NE(nl, nullptr);
                Net* test_net = nl->create_net("test_net");
                ASSERT_NE(test_net, nullptr);
                Gate* test_gate = nl->create_gate(nl->get_gate_library()->get_gate_type_by_name("BUF"), "test_gate");
                ASSERT_NE(test_gate, nullptr);
                EXPECT_NE(test_net->add_destination(test_gate, "I"), nullptr);
                EXPECT_TRUE(test_net->remove_destination(test_gate, "I"));
                EXPECT_TRUE(test_net->get_destinations().empty());
            }
            {
                // remove a destination by specifying endpoint
                auto nl = test_utils::create_empty_netlist();
                ASSERT_NE(nl, nullptr);
                Net* test_net = nl->create_net("test_net");
                ASSERT_NE(test_net, nullptr);
                Gate* test_gate = nl->create_gate(nl->get_gate_library()->get_gate_type_by_name("BUF"), "test_gate");
                ASSERT_NE(test_gate, nullptr);
                Endpoint* ep = test_net->add_destination(test_gate, "I");
                ASSERT_NE(ep, nullptr);
                EXPECT_TRUE(test_net->remove_destination(ep));
                EXPECT_TRUE(test_net->get_destinations().empty());
            }
            // Negative
            {
                // add invalid destination
                NO_COUT_TEST_BLOCK;
                auto nl = test_utils::create_empty_netlist();
                ASSERT_NE(nl, nullptr);
                Net* test_net = nl->create_net("test_net");
                ASSERT_NE(test_net, nullptr);
                Gate* test_gate = nl->create_gate(nl->get_gate_library()->get_gate_type_by_name("BUF"), "test_gate");
                ASSERT_NE(test_gate, nullptr);
                EXPECT_EQ(test_net->add_destination(test_gate, "INVALID"), nullptr);  // invalid pin
                EXPECT_EQ(test_net->add_destination(test_gate, "O"), nullptr);        // input pin
                EXPECT_EQ(test_net->add_destination(test_gate, ""), nullptr);         // empty pin
                EXPECT_EQ(test_net->add_destination(nullptr, "I"), nullptr);          // nullptr gate
                EXPECT_TRUE(test_net->get_destinations().empty());
            }
            {
                // add destination twice
                NO_COUT_TEST_BLOCK;
                auto nl = test_utils::create_empty_netlist();
                ASSERT_NE(nl, nullptr);
                Net* test_net_1 = nl->create_net("test_net_1");
                ASSERT_NE(test_net_1, nullptr);
                Net* test_net_2 = nl->create_net("test_net_2");
                ASSERT_NE(test_net_2, nullptr);
                Gate* test_gate = nl->create_gate(nl->get_gate_library()->get_gate_type_by_name("BUF"), "test_gate");
                ASSERT_NE(test_gate, nullptr);
                EXPECT_NE(test_net_1->add_destination(test_gate, "I"), nullptr);
                EXPECT_EQ(test_net_2->add_destination(test_gate, "I"), nullptr);
                EXPECT_TRUE(test_net_2->get_destinations().empty());
            }
            {
                // remove invalid destination
                NO_COUT_TEST_BLOCK;
                auto nl = test_utils::create_empty_netlist();
                ASSERT_NE(nl, nullptr);
                Net* test_net = nl->create_net("test_net");
                ASSERT_NE(test_net, nullptr);
                Gate* test_gate = nl->create_gate(nl->get_gate_library()->get_gate_type_by_name("BUF"), "test_gate");
                ASSERT_NE(test_gate, nullptr);
                EXPECT_NE(test_net->add_destination(test_gate, "I"), nullptr);
                EXPECT_FALSE(test_net->remove_destination(test_gate, "INVALID"));  // invalid pin
                EXPECT_FALSE(test_net->remove_destination(test_gate, "O"));        // input pin
                EXPECT_FALSE(test_net->remove_destination(test_gate, ""));         // empty pin
                EXPECT_FALSE(test_net->remove_destination(nullptr, "I"));          // nullptr gate
                EXPECT_FALSE(test_net->remove_destination(nullptr));               // nullptr endpoint
                EXPECT_EQ(test_net->get_destinations().size(), 1);
            }
            {
                // remove destination twice by specifying gate and pin
                NO_COUT_TEST_BLOCK;
                auto nl = test_utils::create_empty_netlist();
                ASSERT_NE(nl, nullptr);
                Net* test_net = nl->create_net("test_net");
                ASSERT_NE(test_net, nullptr);
                Gate* test_gate = nl->create_gate(nl->get_gate_library()->get_gate_type_by_name("BUF"), "test_gate");
                ASSERT_NE(test_gate, nullptr);
                EXPECT_NE(test_net->add_destination(test_gate, "I"), nullptr);
                EXPECT_TRUE(test_net->remove_destination(test_gate, "I"));
                EXPECT_FALSE(test_net->remove_destination(test_gate, "I"));
                EXPECT_TRUE(test_net->get_destinations().empty());
            }
            {
                // remove destination twice by specifying endpoint
                NO_COUT_TEST_BLOCK;
                auto nl = test_utils::create_empty_netlist();
                ASSERT_NE(nl, nullptr);
                Net* test_net = nl->create_net("test_net");
                ASSERT_NE(test_net, nullptr);
                Gate* test_gate = nl->create_gate(nl->get_gate_library()->get_gate_type_by_name("BUF"), "test_gate");
                ASSERT_NE(test_gate, nullptr);
                Endpoint* ep = test_net->add_destination(test_gate, "I");
                ASSERT_NE(ep, nullptr);
                EXPECT_TRUE(test_net->remove_destination(ep));
                EXPECT_FALSE(test_net->remove_destination(ep));
                EXPECT_TRUE(test_net->get_destinations().empty());
            }
        TEST_END
    }

    /**
     * Test identifying whether an endpoint is a source or a destination.
     *
     * Functions: is_a_destination, is_a_source
     */
    TEST_F(NetTest, check_is_a_dest_or_src) {
        TEST_START
            {
                auto nl = test_utils::create_empty_netlist();
                ASSERT_NE(nl, nullptr);
                Net* test_net = nl->create_net("test_net");
                ASSERT_NE(test_net, nullptr);
                Gate* test_gate = nl->create_gate(nl->get_gate_library()->get_gate_type_by_name("BUF"), "test_gate");
                ASSERT_NE(test_gate, nullptr);
                Endpoint* ep = test_net->add_source(test_gate, "O");
                ASSERT_NE(ep, nullptr);

                EXPECT_TRUE(test_net->is_a_source(test_gate, "O"));
                EXPECT_FALSE(test_net->is_a_destination(test_gate, "O"));

                EXPECT_TRUE(test_net->is_a_source(ep));
                EXPECT_FALSE(test_net->is_a_destination(ep));
            }
            {
                auto nl = test_utils::create_empty_netlist();
                ASSERT_NE(nl, nullptr);
                Net* test_net = nl->create_net("test_net");
                ASSERT_NE(test_net, nullptr);
                Gate* test_gate = nl->create_gate(nl->get_gate_library()->get_gate_type_by_name("BUF"), "test_gate");
                ASSERT_NE(test_gate, nullptr);
                Endpoint* ep = test_net->add_destination(test_gate, "I");
                ASSERT_NE(ep, nullptr);

                EXPECT_FALSE(test_net->is_a_source(test_gate, "I"));
                EXPECT_TRUE(test_net->is_a_destination(test_gate, "I"));

                EXPECT_FALSE(test_net->is_a_source(ep));
                EXPECT_TRUE(test_net->is_a_destination(ep));
            }
            // NEGATIVE
            {
                // invalid source
                auto nl = test_utils::create_empty_netlist();
                ASSERT_NE(nl, nullptr);
                Net* test_net = nl->create_net("test_net");
                ASSERT_NE(test_net, nullptr);
                Gate* test_gate = nl->create_gate(nl->get_gate_library()->get_gate_type_by_name("BUF"), "test_gate");
                ASSERT_NE(test_gate, nullptr);
                Endpoint* ep = test_net->add_source(test_gate, "O");
                ASSERT_NE(ep, nullptr);

                EXPECT_FALSE(test_net->is_a_source(test_gate, "INVALID"));
                EXPECT_FALSE(test_net->is_a_source(test_gate, ""));
                EXPECT_FALSE(test_net->is_a_source(test_gate, "I"));
                EXPECT_FALSE(test_net->is_a_source(nullptr, "O"));
                EXPECT_FALSE(test_net->is_a_source(nullptr));
            }
            {
                // invalid destination
                auto nl = test_utils::create_empty_netlist();
                ASSERT_NE(nl, nullptr);
                Net* test_net = nl->create_net("test_net");
                ASSERT_NE(test_net, nullptr);
                Gate* test_gate = nl->create_gate(nl->get_gate_library()->get_gate_type_by_name("BUF"), "test_gate");
                ASSERT_NE(test_gate, nullptr);
                Endpoint* ep = test_net->add_destination(test_gate, "I");
                ASSERT_NE(ep, nullptr);

                EXPECT_FALSE(test_net->is_a_destination(test_gate, "INVALID"));
                EXPECT_FALSE(test_net->is_a_destination(test_gate, ""));
                EXPECT_FALSE(test_net->is_a_destination(test_gate, "O"));
                EXPECT_FALSE(test_net->is_a_destination(nullptr, "I"));
                EXPECT_FALSE(test_net->is_a_destination(nullptr));
            }
        TEST_END
    }

    /**
     * Testing the function is_unrouted
     *
     * Functions: is_unrouted
     */
    TEST_F(NetTest, check_is_unrouted) {
        TEST_START
            {
                // has source and destination
                auto nl = test_utils::create_empty_netlist();
                ASSERT_NE(nl, nullptr);
                Net* test_net = nl->create_net("test_net");
                ASSERT_NE(test_net, nullptr);
                Gate* test_gate_src = nl->create_gate(nl->get_gate_library()->get_gate_type_by_name("BUF"), "test_gate_src");
                ASSERT_NE(test_gate_src, nullptr);
                Gate* test_gate_dst = nl->create_gate(nl->get_gate_library()->get_gate_type_by_name("BUF"), "test_gate_dst");
                ASSERT_NE(test_gate_dst, nullptr);
                EXPECT_NE(test_net->add_source(test_gate_src, "O"), nullptr);
                EXPECT_NE(test_net->add_destination(test_gate_dst, "I"), nullptr);

                EXPECT_FALSE(test_net->is_unrouted());
            }
            {
                // has source
                auto nl = test_utils::create_empty_netlist();
                ASSERT_NE(nl, nullptr);
                Net* test_net = nl->create_net("test_net");
                ASSERT_NE(test_net, nullptr);
                Gate* test_gate_src = nl->create_gate(nl->get_gate_library()->get_gate_type_by_name("BUF"), "test_gate_src");
                ASSERT_NE(test_gate_src, nullptr);
                EXPECT_NE(test_net->add_source(test_gate_src, "O"), nullptr);

                EXPECT_TRUE(test_net->is_unrouted());
            }
            {
                // has destination
                auto nl = test_utils::create_empty_netlist();
                ASSERT_NE(nl, nullptr);
                Net* test_net = nl->create_net("test_net");
                ASSERT_NE(test_net, nullptr);
                Gate* test_gate_dst = nl->create_gate(nl->get_gate_library()->get_gate_type_by_name("BUF"), "test_gate_dst");
                ASSERT_NE(test_gate_dst, nullptr);
                EXPECT_NE(test_net->add_destination(test_gate_dst, "I"), nullptr);

                EXPECT_TRUE(test_net->is_unrouted());
            }
            {
                // has no source and no destination
                auto nl = test_utils::create_empty_netlist();
                ASSERT_NE(nl, nullptr);
                Net* test_net = nl->create_net("test_net");
                ASSERT_NE(test_net, nullptr);

                EXPECT_TRUE(test_net->is_unrouted());
            }

        TEST_END
    }

    /**
     * Test the handling of global input and output nets.
     *
     * Functions: mark_global_input_net, mark_global_input_net,
     *            unmark_global_input_net, unmark_global_input_net,
     *            is_global_input_net, is_global_input_net
     */
    TEST_F(NetTest, check_global_inout_nets) {
        TEST_START
            {
                // mark and unmark a global input net
                auto nl = test_utils::create_empty_netlist();
                ASSERT_NE(nl, nullptr);
                Net* test_net = nl->create_net("test_net");
                ASSERT_NE(test_net, nullptr);

                EXPECT_TRUE(test_net->mark_global_input_net());
                EXPECT_TRUE(test_net->is_global_input_net());
                EXPECT_TRUE(nl->is_global_input_net(test_net));
                EXPECT_FALSE(test_net->is_global_output_net());
                EXPECT_FALSE(nl->is_global_output_net(test_net));

                EXPECT_TRUE(test_net->unmark_global_input_net());
                EXPECT_FALSE(test_net->is_global_input_net());
                EXPECT_FALSE(nl->is_global_input_net(test_net));
                EXPECT_FALSE(test_net->is_global_output_net());
                EXPECT_FALSE(nl->is_global_output_net(test_net));
            }
            {
                // mark and unmark a global output net
                auto nl = test_utils::create_empty_netlist();
                ASSERT_NE(nl, nullptr);
                Net* test_net = nl->create_net("test_net");
                ASSERT_NE(test_net, nullptr);

                EXPECT_TRUE(test_net->mark_global_output_net());
                EXPECT_FALSE(test_net->is_global_input_net());
                EXPECT_FALSE(nl->is_global_input_net(test_net));
                EXPECT_TRUE(test_net->is_global_output_net());
                EXPECT_TRUE(nl->is_global_output_net(test_net));

                EXPECT_TRUE(test_net->unmark_global_output_net());
                EXPECT_FALSE(test_net->is_global_output_net());
                EXPECT_FALSE(nl->is_global_input_net(test_net));
                EXPECT_FALSE(test_net->is_global_output_net());
                EXPECT_FALSE(nl->is_global_output_net(test_net));
            }
            {
                // mark and unmark a global input and output net
                auto nl = test_utils::create_empty_netlist();
                ASSERT_NE(nl, nullptr);
                Net* test_net = nl->create_net("test_net");
                ASSERT_NE(test_net, nullptr);

                EXPECT_TRUE(test_net->mark_global_input_net());
                EXPECT_TRUE(test_net->mark_global_output_net());
                EXPECT_TRUE(test_net->is_global_input_net());
                EXPECT_TRUE(nl->is_global_input_net(test_net));
                EXPECT_TRUE(test_net->is_global_output_net());
                EXPECT_TRUE(nl->is_global_output_net(test_net));

                EXPECT_TRUE(test_net->unmark_global_input_net());
                EXPECT_TRUE(test_net->unmark_global_output_net());
                EXPECT_FALSE(test_net->is_global_output_net());
                EXPECT_FALSE(nl->is_global_input_net(test_net));
                EXPECT_FALSE(test_net->is_global_output_net());
                EXPECT_FALSE(nl->is_global_output_net(test_net));
            }
            {
                // unmark unmarked nets
                auto nl = test_utils::create_empty_netlist();
                ASSERT_NE(nl, nullptr);
                Net* test_net = nl->create_net("test_net");
                ASSERT_NE(test_net, nullptr);

                EXPECT_FALSE(test_net->is_global_output_net());
                EXPECT_FALSE(nl->is_global_input_net(test_net));
                EXPECT_FALSE(test_net->is_global_output_net());
                EXPECT_FALSE(nl->is_global_output_net(test_net));

                EXPECT_FALSE(test_net->unmark_global_input_net());
                EXPECT_FALSE(test_net->unmark_global_output_net());
                EXPECT_FALSE(test_net->is_global_output_net());
                EXPECT_FALSE(nl->is_global_input_net(test_net));
                EXPECT_FALSE(test_net->is_global_output_net());
                EXPECT_FALSE(nl->is_global_output_net(test_net));
            }
        TEST_END
    }

    /**
     * Test detection of GND and VCC nets.
     *
     * Functions: is_gnd_net, is_vcc_net
     */
    TEST_F(NetTest, check_gnd_vcc_nets) {
        TEST_START
            {
                auto nl = test_utils::create_empty_netlist();
                ASSERT_NE(nl, nullptr);
                Gate* gnd = nl->create_gate(nl->get_gate_library()->get_gate_type_by_name("GND"), "gnd");
                ASSERT_NE(gnd, nullptr);
                EXPECT_TRUE(gnd->mark_gnd_gate());
                Gate* vcc = nl->create_gate(nl->get_gate_library()->get_gate_type_by_name("VCC"), "vcc");
                ASSERT_NE(vcc, nullptr);
                EXPECT_TRUE(vcc->mark_vcc_gate());

                Net* gnd_net = nl->create_net("gnd_net");
                ASSERT_NE(gnd_net, nullptr);
                Net* vcc_net = nl->create_net("vcc_net");
                ASSERT_NE(vcc_net, nullptr);

                EXPECT_FALSE(gnd_net->is_gnd_net());
                EXPECT_FALSE(vcc_net->is_gnd_net());
                EXPECT_FALSE(gnd_net->is_vcc_net());
                EXPECT_FALSE(vcc_net->is_vcc_net());

                EXPECT_NE(gnd_net->add_source(gnd, "O"), nullptr);
                EXPECT_NE(vcc_net->add_source(vcc, "O"), nullptr);

                EXPECT_TRUE(gnd_net->is_gnd_net());
                EXPECT_FALSE(vcc_net->is_gnd_net());
                EXPECT_FALSE(gnd_net->is_vcc_net());
                EXPECT_TRUE(vcc_net->is_vcc_net());

                EXPECT_TRUE(gnd_net->remove_source(gnd, "O"));
                EXPECT_TRUE(vcc_net->remove_source(vcc, "O"));

                EXPECT_FALSE(gnd_net->is_gnd_net());
                EXPECT_FALSE(vcc_net->is_gnd_net());
                EXPECT_FALSE(gnd_net->is_vcc_net());
                EXPECT_FALSE(vcc_net->is_vcc_net());
            }
        TEST_END
    }


    /**
     * Test handling of groupings.
     *
     * Functions: get_grouping
     */
    TEST_F(NetTest, check_get_grouping) {
        TEST_START
            {
                auto nl = test_utils::create_empty_netlist();
                ASSERT_NE(nl, nullptr);
                Net* test_net = nl->create_net("test_net");
                ASSERT_NE(test_net, nullptr);

                EXPECT_EQ(test_net->get_grouping(), nullptr);

                // move the net in the test_grouping
                Grouping* test_grouping = nl->create_grouping("test_grouping");
                ASSERT_NE(test_grouping, nullptr);
                EXPECT_TRUE(test_grouping->assign_net(test_net));
                EXPECT_EQ(test_net->get_grouping(), test_grouping);

                // -- delete the test_grouping, so the net should be nullptr again
                EXPECT_TRUE(nl->delete_grouping(test_grouping));
                EXPECT_EQ(test_net->get_grouping(), nullptr);
            }
        TEST_END
    }
    
    /*************************************
     * Event System
     *************************************/

    /**
     * Testing the triggering of events.
     */
    TEST_F(NetTest, check_events) {
        TEST_START
            const u32 NO_DATA = 0xFFFFFFFF;

            std::unique_ptr<Netlist> test_nl = test_utils::create_example_netlist();
            Net* test_net = test_nl->get_net_by_id(MIN_NET_ID + 13);
            Gate* new_gate = test_nl->create_gate(test_nl->get_gate_library()->get_gate_type_by_name("gate_1_to_1"), "new_gate");

            // Small functions that should trigger certain events exactly once (these operations are executed in this order)
            std::function<void(void)> trigger_name_changed = [=](){test_net->set_name("new_name");};
            std::function<void(void)> trigger_src_added = [=](){test_net->add_source(new_gate, "O");};
            std::function<void(void)> trigger_src_removed = [=](){test_net->remove_source(new_gate, "O");};
            std::function<void(void)> trigger_dst_added = [=](){test_net->add_destination(new_gate, "I");};
            std::function<void(void)> trigger_dst_removed = [=](){test_net->remove_destination(new_gate, "I");};

            // The events that are tested
            std::vector<NetEvent::event> event_type = {
                NetEvent::event::name_changed, NetEvent::event::src_added,
                NetEvent::event::src_removed, NetEvent::event::dst_added,
                NetEvent::event::dst_removed};

            // A list of the functions that will trigger its associated event exactly once
            std::vector<std::function<void(void)>> trigger_event = { trigger_name_changed, trigger_src_added,
                 trigger_src_removed, trigger_dst_added, trigger_dst_removed };

            // The parameters of the events that are expected
            std::vector<std::tuple<NetEvent::event, Net*, u32>> expected_parameter = {
                std::make_tuple(NetEvent::event::name_changed, test_net, NO_DATA),
                std::make_tuple(NetEvent::event::src_added, test_net, new_gate->get_id()),
                std::make_tuple(NetEvent::event::src_removed, test_net, new_gate->get_id()),
                std::make_tuple(NetEvent::event::dst_added, test_net, new_gate->get_id()),
                std::make_tuple(NetEvent::event::dst_removed, test_net, new_gate->get_id())
            };

            // Check all events in a for-loop
            for(u32 event_idx = 0; event_idx < event_type.size(); event_idx++)
            {
                // Create the listener for the tested event
                test_utils::EventListener<void, NetEvent::event, Net*, u32> listener;
                std::function<void(NetEvent::event, Net*, u32)> cb = listener.get_conditional_callback(
                    [=](NetEvent::event ev, Net* n, u32 id){return ev == event_type[event_idx] && n == test_net;}
                );
                std::string cb_name = "net_event_callback_" + std::to_string((u32)event_type[event_idx]);
                // Register a callback of the listener
                test_nl->get_event_handler()->register_callback(cb_name, cb);

                // Trigger the event
                trigger_event[event_idx]();

                EXPECT_EQ(listener.get_event_count(), 1);
                EXPECT_EQ(listener.get_last_parameters(), expected_parameter[event_idx]);

                // Unregister the callback
                test_nl->get_event_handler()->unregister_callback(cb_name);
            }

            // Test the events 'created' and 'removed'
            // -- 'created' event
            test_utils::EventListener<void, NetEvent::event, Net*, u32> listener_created;
            std::function<void(NetEvent::event, Net*, u32)> cb_created = listener_created.get_conditional_callback(
                [=](NetEvent::event ev, Net*, u32){return ev == NetEvent::event::created;}
            );
            std::string cb_name_created = "net_event_callback_created";
            test_nl->get_event_handler()->register_callback(cb_name_created, cb_created);

            // Create a new mod
            Net* new_net = test_nl->create_net("new_net");
            EXPECT_EQ(listener_created.get_event_count(), 1);
            EXPECT_EQ(listener_created.get_last_parameters(), std::make_tuple(NetEvent::event::created, new_net, NO_DATA));

            test_nl->get_event_handler()->unregister_callback(cb_name_created);

            // -- 'removed' event
            test_utils::EventListener<void, NetEvent::event, Net*, u32> listener_removed;
            std::function<void(NetEvent::event, Net*, u32)> cb_removed = listener_removed.get_conditional_callback(
                [=](NetEvent::event ev, Net*, u32){return ev == NetEvent::event::removed;}
            );
            std::string cb_name_removed = "net_event_callback_removed";
            test_nl->get_event_handler()->register_callback(cb_name_removed, cb_removed);

            // Delete the module which was created in the previous part
            test_nl->delete_net(new_net);
            EXPECT_EQ(listener_removed.get_event_count(), 1);
            EXPECT_EQ(listener_removed.get_last_parameters(), std::make_tuple(NetEvent::event::removed, new_net, NO_DATA));

            test_nl->get_event_handler()->unregister_callback(cb_name_removed);

        TEST_END
    }


} //namespace hal
