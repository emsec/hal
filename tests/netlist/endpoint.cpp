#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/netlist/netlist_factory.h"
#include "test_def.h"
#include "gate_library_test_utils.h"
#include "netlist_test_utils.h"

#include "gtest/gtest.h"

namespace hal {

    class EndpointTest : public ::testing::Test {
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
     * Test operators for equality and inequality.
     * 
     * Functions: operator==, operator!=
     */
    TEST_F(EndpointTest, check_operators)
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

            Gate* nl1_dummy_1 = nl_1->create_gate(gl->get_gate_type_by_name("AND2"), "dummy_1");
            ASSERT_NE(nl1_dummy_1, nullptr);
            Gate* nl1_dummy_2 = nl_1->create_gate(gl->get_gate_type_by_name("AND2"), "dummy_2");
            ASSERT_NE(nl1_dummy_2, nullptr);
            Gate* nl2_dummy_1 = nl_2->create_gate(gl->get_gate_type_by_name("AND2"), "dummy_1");
            ASSERT_NE(nl2_dummy_1, nullptr);
            Gate* nl2_dummy_2 = nl_2->create_gate(gl->get_gate_type_by_name("AND2"), "dummy_2");
            ASSERT_NE(nl2_dummy_2, nullptr);

            // standard stuff
            Net* nl1_n1 = nl_1->create_net(1, "net_1");
            ASSERT_NE(nl1_n1, nullptr);
            Net* nl2_n1 = nl_2->create_net(1, "net_1");
            ASSERT_NE(nl2_n1, nullptr);
            Endpoint* nl1_ep1 = nl1_n1->add_source(nl1_dummy_1, "O");
            ASSERT_NE(nl1_ep1, nullptr);
            Endpoint* nl2_ep1 = nl2_n1->add_source(nl2_dummy_1, "O");
            ASSERT_NE(nl2_ep1, nullptr);

            EXPECT_TRUE(*nl1_ep1 == *nl1_ep1);      // identical endpoint pointer
            EXPECT_TRUE(*nl2_ep1 == *nl2_ep1);
            EXPECT_FALSE(*nl1_ep1 != *nl1_ep1);
            EXPECT_FALSE(*nl2_ep1 != *nl2_ep1);
            EXPECT_TRUE(*nl1_ep1 == *nl2_ep1);        // identical endpoints, but different netlists
            EXPECT_TRUE(*nl2_ep1 == *nl1_ep1);
            EXPECT_FALSE(*nl1_ep1 != *nl2_ep1);
            EXPECT_FALSE(*nl2_ep1 != *nl1_ep1);
            test_utils::clear_connections(nl1_n1);
            test_utils::clear_connections(nl2_n1);

            // different nets
            Net* nl1_n2 = nl_1->create_net(2, "net_2");
            ASSERT_NE(nl1_n2, nullptr);
            Net* nl2_n2 = nl_2->create_net(2, "net_3");
            ASSERT_NE(nl2_n2, nullptr);
            Endpoint* nl1_ep2 = nl1_n2->add_source(nl1_dummy_1, "O");
            ASSERT_NE(nl1_ep2, nullptr);
            Endpoint* nl2_ep2 = nl2_n2->add_source(nl2_dummy_1, "O");
            ASSERT_NE(nl2_ep2, nullptr);
            EXPECT_FALSE(*nl1_ep2 == *nl2_ep2);      // different nets
            EXPECT_FALSE(*nl2_ep2 == *nl1_ep2);
            EXPECT_TRUE(*nl1_ep2 != *nl2_ep2);
            EXPECT_TRUE(*nl2_ep2 != *nl1_ep2);
            test_utils::clear_connections(nl1_n2);
            test_utils::clear_connections(nl2_n2);

            // different gates
            Net* nl1_n3 = nl_1->create_net(3, "net_3");
            ASSERT_NE(nl1_n3, nullptr);
            Net* nl2_n3 = nl_2->create_net(3, "net_3");
            ASSERT_NE(nl2_n3, nullptr);
            Endpoint* nl1_ep3 = nl1_n3->add_source(nl1_dummy_1, "O");
            ASSERT_NE(nl1_ep3, nullptr);
            Endpoint* nl2_ep3 = nl2_n3->add_source(nl2_dummy_2, "O");
            ASSERT_NE(nl2_ep3, nullptr);
            EXPECT_FALSE(*nl1_ep3 == *nl2_ep3);      // different gates
            EXPECT_FALSE(*nl2_ep3 == *nl1_ep3);
            EXPECT_TRUE(*nl1_ep3 != *nl2_ep3);
            EXPECT_TRUE(*nl2_ep3 != *nl1_ep3);
            test_utils::clear_connections(nl1_n3);
            test_utils::clear_connections(nl2_n3);

            // different pins
            Net* nl1_n4 = nl_1->create_net(4, "net_4");
            ASSERT_NE(nl1_n4, nullptr);
            Net* nl2_n4 = nl_2->create_net(4, "net_4");
            ASSERT_NE(nl2_n4, nullptr);
            Endpoint* nl1_ep4 = nl1_n4->add_destination(nl1_dummy_1, "I0");
            ASSERT_NE(nl1_ep4, nullptr);
            Endpoint* nl2_ep4 = nl2_n4->add_destination(nl2_dummy_1, "I1");
            ASSERT_NE(nl2_ep4, nullptr);
            EXPECT_FALSE(*nl1_ep4 == *nl2_ep4);      // different pins
            EXPECT_FALSE(*nl2_ep4 == *nl1_ep4);
            EXPECT_TRUE(*nl1_ep4 != *nl2_ep4);
            EXPECT_TRUE(*nl2_ep4 != *nl1_ep4);
        }
        TEST_END
    }

    /**
     * Testing the access on the endpoints content
     *
     * Functions: get_gate, get_pin, is_destination_pin, is_source_pin
     */
    TEST_F(EndpointTest, check_set_get_gate) {
        TEST_START
        {
            const GateLibrary* gl       = test_utils::get_gate_library();
            ASSERT_NE(gl, nullptr);
            std::unique_ptr<Netlist> nl = netlist_factory::create_netlist(gl);
            ASSERT_NE(nl, nullptr);
            
            Gate* test_gate = nl->create_gate(gl->get_gate_type_by_name("BUF"), "test_gate");
            ASSERT_NE(test_gate, nullptr);

            Net* test_net = nl->create_net("test_net");
            ASSERT_NE(test_net, nullptr);

            Endpoint* ep = test_net->add_destination(test_gate, "I");
            ASSERT_NE(ep, nullptr);

            EXPECT_EQ(ep->get_gate(), test_gate);
            EXPECT_EQ(ep->get_pin(), "I");
            EXPECT_EQ(ep->get_net(), test_net);
            EXPECT_TRUE(ep->is_destination_pin());
            EXPECT_FALSE(ep->is_source_pin());
        }
        {
            const GateLibrary* gl       = test_utils::get_gate_library();
            ASSERT_NE(gl, nullptr);
            std::unique_ptr<Netlist> nl = netlist_factory::create_netlist(gl);
            ASSERT_NE(nl, nullptr);
            
            Gate* test_gate = nl->create_gate(gl->get_gate_type_by_name("BUF"), "test_gate");
            ASSERT_NE(test_gate, nullptr);

            Net* test_net = nl->create_net("test_net");
            ASSERT_NE(test_net, nullptr);

            Endpoint* ep = test_net->add_source(test_gate, "O");
            ASSERT_NE(ep, nullptr);

            EXPECT_EQ(ep->get_gate(), test_gate);
            EXPECT_EQ(ep->get_pin(), "O");
            EXPECT_EQ(ep->get_net(), test_net);
            EXPECT_FALSE(ep->is_destination_pin());
            EXPECT_TRUE(ep->is_source_pin());
        }
        TEST_END
    }
} //namespace hal
