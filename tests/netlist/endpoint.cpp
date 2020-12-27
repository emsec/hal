#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/gate_library/gate_library_parser/gate_library_parser_manager.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/netlist.h"
#include "netlist_test_utils.h"

#include "gtest/gtest.h"
#include <iostream>

namespace hal {

    class EndpointTest : public ::testing::Test {
    protected:
        virtual void SetUp() {
            test_utils::init_log_channels();
        }

        virtual void TearDown() {
        }
    };

    /**
     * Testing the access on the endpoints content
     *
     * Functions: get_gate, get_pin, is_destination_pin, is_source_pin
     */
    TEST_F(EndpointTest, check_set_get_gate) {
        TEST_START
            auto nl = test_utils::create_empty_netlist(0);
            Gate* test_gate = nl->create_gate(123, test_utils::get_gate_type_by_name("gate_1_to_1"), "test_gate");
            Net* test_net = nl->create_net("test_net");
            auto ep = test_net->add_destination(test_gate, "I");
            EXPECT_EQ(ep->get_gate(), test_gate);
            EXPECT_EQ(ep->get_pin(), "I");
            EXPECT_EQ(ep->get_net(), test_net);
            EXPECT_TRUE(ep->is_destination_pin());
            EXPECT_FALSE(ep->is_source_pin());
        TEST_END
    }
} //namespace hal
