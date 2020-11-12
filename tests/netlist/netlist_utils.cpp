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
