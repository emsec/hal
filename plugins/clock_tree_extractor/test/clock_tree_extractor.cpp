#include "clock_tree_extractor/clock_tree.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/netlist.h"
#include "netlist_test_utils.h"

#include "gtest/gtest.h"

namespace hal
{
    class ClockTreeExtractorTest : public ::testing::Test
    {
    protected:
        virtual void SetUp()
        {
            test_utils::init_log_channels();
        }

        virtual void TearDown()
        {
        }

        static std::vector<const void*> upstream_of(const cte::ClockTree& tree, const Gate* gate)
        {
            auto res = tree.get_neighbors(gate, IGRAPH_IN);
            EXPECT_TRUE(res.is_ok());
            std::vector<const void*> ptrs;
            for (const auto& [ptr, type] : res.get())
            {
                ptrs.push_back(ptr);
            }
            return ptrs;
        }
    };

    /**
     * A flip-flop whose clock pin is driven by a buffer behind a global input net: the tree runs from the net through the
     * buffer to the flip-flop.
     *
     * Functions: from_netlist, get_neighbors
     */
    TEST_F(ClockTreeExtractorTest, check_buffered_clock)
    {
        TEST_START
        {
            std::unique_ptr<Netlist> nl = test_utils::create_empty_netlist();
            ASSERT_NE(nl, nullptr);
            const GateLibrary* gl = nl->get_gate_library();

            Gate* buf = nl->create_gate(gl->get_gate_type_by_name("BUF"), "buf");
            Gate* ff  = nl->create_gate(gl->get_gate_type_by_name("DFF"), "ff");

            Net* clk = nl->create_net("clk");
            clk->mark_global_input_net();
            clk->add_destination(buf, "I");
            test_utils::connect(nl.get(), buf, "O", ff, "CLK");

            auto res = cte::ClockTree::from_netlist(nl.get());
            ASSERT_TRUE(res.is_ok());
            std::unique_ptr<cte::ClockTree> tree = res.get();

            EXPECT_EQ(tree->get_gates().size(), 2);
            EXPECT_EQ(tree->get_nets().size(), 1);
            EXPECT_EQ(upstream_of(*tree, ff), std::vector<const void*>{buf});
            EXPECT_EQ(upstream_of(*tree, buf), std::vector<const void*>{clk});
        }
        TEST_END
    }

    /**
     * A flip-flop clocked straight from a global input net, without any gate in between: the net is the root of the tree and
     * the flip-flop hangs below it.
     *
     * Functions: from_netlist, get_neighbors
     */
    TEST_F(ClockTreeExtractorTest, check_direct_clock)
    {
        TEST_START
        {
            std::unique_ptr<Netlist> nl = test_utils::create_empty_netlist();
            ASSERT_NE(nl, nullptr);
            const GateLibrary* gl = nl->get_gate_library();

            Gate* ff = nl->create_gate(gl->get_gate_type_by_name("DFF"), "ff");

            Net* clk = nl->create_net("clk");
            clk->mark_global_input_net();
            clk->add_destination(ff, "CLK");

            auto res = cte::ClockTree::from_netlist(nl.get());
            ASSERT_TRUE(res.is_ok());
            std::unique_ptr<cte::ClockTree> tree = res.get();

            EXPECT_EQ(tree->get_gates().size(), 1);
            EXPECT_EQ(tree->get_nets().size(), 1);
            EXPECT_EQ(upstream_of(*tree, ff), std::vector<const void*>{clk});
        }
        TEST_END
    }
}    // namespace hal
