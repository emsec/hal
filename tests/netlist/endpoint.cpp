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
     * Testing the access on the endpoints content
     *
     * Functions: get_gate, get_pin, is_destination_pin, is_source_pin
     */
    TEST_F(EndpointTest, check_set_get_gate) {
        TEST_START
        {
            const GateLibrary* gl       = test_utils::get_gate_library();
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
        TEST_END
    }
} //namespace hal
