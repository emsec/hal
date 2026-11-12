#include "dataflow_analysis/api/configuration.h"
#include "dataflow_analysis/api/dataflow.h"
#include "dataflow_analysis/api/result.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/gate_library/gate_library.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/netlist.h"
#include "netlist_test_utils.h"

#include <unordered_set>

namespace hal
{
    class DataflowTest : public ::testing::Test
    {
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

    namespace
    {
        /**
         * Three 4-bit registers on one clock that shift into each other, each bit through a buffer:
         * input i -> a[i] -> BUF -> b[i] -> BUF -> c[i] -> output i.
         */
        struct Chain
        {
            std::unique_ptr<Netlist> nl;
            std::vector<Gate*> a, b, c;
        };

        Chain create_chain()
        {
            Chain chain;
            chain.nl              = test_utils::create_empty_netlist();
            Netlist* nl           = chain.nl.get();
            const GateLibrary* gl = nl->get_gate_library();
            GateType* dff         = gl->get_gate_type_by_name("DFF");
            GateType* buf         = gl->get_gate_type_by_name("BUF");

            Net* clk = nl->create_net("clk");
            nl->mark_global_input_net(clk);
            for (u32 i = 0; i < 4; i++)
            {
                const std::string bit = std::to_string(i);
                Gate* a               = nl->create_gate(dff, "a_" + bit);
                Gate* b               = nl->create_gate(dff, "b_" + bit);
                Gate* c               = nl->create_gate(dff, "c_" + bit);
                for (Gate* ff : {a, b, c})
                {
                    clk->add_destination(ff, "CLK");
                }

                Net* in = nl->create_net("in_" + bit);
                nl->mark_global_input_net(in);
                in->add_destination(a, "D");

                Gate* ab = nl->create_gate(buf, "ab_" + bit);
                Gate* bc = nl->create_gate(buf, "bc_" + bit);
                test_utils::connect(nl, a, "Q", ab, "I");
                test_utils::connect(nl, ab, "O", b, "D");
                test_utils::connect(nl, b, "Q", bc, "I");
                test_utils::connect(nl, bc, "O", c, "D");

                Net* out = nl->create_net("out_" + bit);
                out->add_source(c, "Q");
                nl->mark_global_output_net(out);

                chain.a.push_back(a);
                chain.b.push_back(b);
                chain.c.push_back(c);
            }
            return chain;
        }

        dataflow::Configuration configuration(Netlist* nl)
        {
            dataflow::Configuration config(nl);
            config.with_flip_flops().with_min_group_size(2);
            return config;
        }

        std::unordered_set<Gate*> grouped_gates(const dataflow::Result& result)
        {
            const auto gates = result.get_gates();
            return std::unordered_set<Gate*>(gates.begin(), gates.end());
        }

        /* the gates of `left_out` are not grouped, and a[i] has no successor: the dataflow ends at b[i] instead of passing through to c[i] */
        void expect_b_left_out(const Chain& chain, const dataflow::Result& result)
        {
            const auto grouped = grouped_gates(result);
            EXPECT_EQ(grouped.size(), 8);
            for (u32 i = 0; i < 4; i++)
            {
                EXPECT_TRUE(grouped.find(chain.a.at(i)) != grouped.end());
                EXPECT_TRUE(grouped.find(chain.c.at(i)) != grouped.end());
                EXPECT_TRUE(grouped.find(chain.b.at(i)) == grouped.end());
                EXPECT_TRUE(result.get_group_id_of_gate(chain.b.at(i)).is_error());

                const auto successors = result.get_gate_successors(chain.a.at(i));
                ASSERT_TRUE(successors.is_ok());
                EXPECT_TRUE(successors.get().empty());
            }
        }
    }    // namespace

    /**
     * Without a selection or exclusion, every flip-flop is grouped and a[i] -> b[i] -> c[i].
     *
     * Functions: analyze
     */
    TEST_F(DataflowTest, check_all_flip_flops_by_default)
    {
        TEST_START
        {
            auto chain = create_chain();
            auto res   = dataflow::analyze(configuration(chain.nl.get()));
            ASSERT_TRUE(res.is_ok()) << res.get_error().get();
            const auto result = res.get();

            EXPECT_EQ(grouped_gates(result).size(), 12);
            for (u32 i = 0; i < 4; i++)
            {
                const auto a_successors = result.get_gate_successors(chain.a.at(i));
                ASSERT_TRUE(a_successors.is_ok());
                EXPECT_EQ(a_successors.get(), std::unordered_set<Gate*>({chain.b.at(i)}));
                const auto b_successors = result.get_gate_successors(chain.b.at(i));
                ASSERT_TRUE(b_successors.is_ok());
                EXPECT_EQ(b_successors.get(), std::unordered_set<Gate*>({chain.c.at(i)}));
            }
        }
        TEST_END
    }

    /**
     * Excluded flip-flops are not grouped, and the dataflow does not pass through them.
     *
     * Functions: Configuration::with_excluded_gates, analyze
     */
    TEST_F(DataflowTest, check_excluded_gates)
    {
        TEST_START
        {
            auto chain  = create_chain();
            auto config = configuration(chain.nl.get());
            config.with_excluded_gates(chain.b);
            auto res = dataflow::analyze(config);
            ASSERT_TRUE(res.is_ok()) << res.get_error().get();
            expect_b_left_out(chain, res.get());
        }
        TEST_END
    }

    /**
     * Only selected flip-flops are grouped; the others are left out as if excluded. Selection and exclusion combine.
     *
     * Functions: Configuration::with_gates, Configuration::with_excluded_gates, analyze
     */
    TEST_F(DataflowTest, check_gate_selection)
    {
        TEST_START
        {
            {
                auto chain  = create_chain();
                auto config = configuration(chain.nl.get());
                config.with_gates(chain.a).with_gates(chain.c);
                EXPECT_EQ(config.gates.size(), 8);
                auto res = dataflow::analyze(config);
                ASSERT_TRUE(res.is_ok()) << res.get_error().get();
                expect_b_left_out(chain, res.get());
            }
            {
                // a selection of every flip-flop, of which b is excluded; the buffers in the selection are ignored
                auto chain  = create_chain();
                auto config = configuration(chain.nl.get());
                config.with_gates(chain.nl->get_gates()).with_excluded_gates(chain.b);
                auto res = dataflow::analyze(config);
                ASSERT_TRUE(res.is_ok()) << res.get_error().get();
                expect_b_left_out(chain, res.get());
            }
            {
                // overwrite replaces the selection
                auto chain  = create_chain();
                auto config = configuration(chain.nl.get());
                config.with_gates(chain.b).with_gates(chain.a, true);
                EXPECT_EQ(config.gates.size(), 4);
                EXPECT_TRUE(config.gates.find(chain.b.front()) == config.gates.end());
            }
        }
        TEST_END
    }

    /**
     * A selection or exclusion with a gate of another netlist, or one that leaves nothing to group, is refused.
     *
     * Functions: analyze
     */
    TEST_F(DataflowTest, check_invalid_selection)
    {
        TEST_START
        {
            auto chain = create_chain();
            auto other = create_chain();
            {
                auto config = configuration(chain.nl.get());
                config.with_gates({other.a.front()});
                EXPECT_TRUE(dataflow::analyze(config).is_error());
            }
            {
                auto config = configuration(chain.nl.get());
                config.with_excluded_gates({other.a.front()});
                EXPECT_TRUE(dataflow::analyze(config).is_error());
            }
            {
                auto config = configuration(chain.nl.get());
                config.with_excluded_gates(chain.a).with_excluded_gates(chain.b).with_excluded_gates(chain.c);
                EXPECT_TRUE(dataflow::analyze(config).is_error());
            }
        }
        TEST_END
    }
}    // namespace hal
