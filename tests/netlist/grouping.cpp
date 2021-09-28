#include "hal_core/netlist/grouping.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/netlist/netlist_factory.h"
#include "netlist_test_utils.h"

namespace hal {
    class GroupingTest : public ::testing::Test {
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
     * Testing the access on the id, the type and the stored netlist after calling the constructor
     *
     * Functions: constructor, get_id, get_name
     */
    TEST_F(GroupingTest, check_constructor) {
        TEST_START
            {
                // Creating a grouping with ID 123 and name "test_grouping"
                auto nl = test_utils::create_empty_netlist();
                Grouping* test_grouping = nl->create_grouping("test_grouping");

                EXPECT_EQ(test_grouping->get_id(), 1);
                EXPECT_EQ(test_grouping->get_name(), "test_grouping");
                EXPECT_EQ(test_grouping->get_netlist(), nl.get());
            }
            {
                // Creating a grouping with ID 123 and name "test_grouping"
                auto nl = test_utils::create_empty_netlist();
                Grouping* test_grouping = nl->create_grouping(123, "test_grouping");

                EXPECT_EQ(test_grouping->get_id(), 123);
                EXPECT_EQ(test_grouping->get_name(), "test_grouping");
                EXPECT_EQ(test_grouping->get_netlist(), nl.get());
            }
            // NEGATIVE
            {
                NO_COUT_TEST_BLOCK;

                // Creating a grouping with invalid ID
                auto nl = test_utils::create_empty_netlist();
                Grouping* test_grouping = nl->create_grouping(0, "test_grouping");
                EXPECT_EQ(test_grouping, nullptr);
            }
        TEST_END
    }

    /**
     * Testing the set_name function of Grouping
     *
     * Functions: set_name
     */
    TEST_F(GroupingTest, check_set_name) {
        TEST_START
            {
                // Set a new name for grouping
                auto nl = test_utils::create_empty_netlist();
                Grouping* test_grouping = nl->create_grouping("test_grouping");

                test_grouping->set_name("new_name");
                EXPECT_EQ(test_grouping->get_name(), "new_name");
            }
            {
                // Set an empty name
                NO_COUT_TEST_BLOCK;
                auto nl = test_utils::create_empty_netlist();
                Grouping* test_grouping = nl->create_grouping("test_grouping");

                test_grouping->set_name("");
                EXPECT_EQ(test_grouping->get_name(), "test_grouping");
            }
        TEST_END
    }

    /**
     * Testing retrieving the contained gates of a grouping.
     * 
     * Functions: get_gates, get_gate_ids
     */
    TEST_F(GroupingTest, check_get_gates) {
        TEST_START
            {
                auto nl = test_utils::create_empty_netlist();
                Grouping* test_grouping = nl->create_grouping("test_grouping");

                EXPECT_EQ(test_grouping->get_gates(), std::vector<Gate*>());
                EXPECT_EQ(test_grouping->get_gate_ids(), std::vector<u32>());

                Gate* gate_0 = nl->create_gate(1, nl->get_gate_library()->get_gate_type_by_name("BUF"), "gate_0");
                Gate* gate_1 = nl->create_gate(2, nl->get_gate_library()->get_gate_type_by_name("BUF"), "gate_1");
                Gate* gate_2 = nl->create_gate(3, nl->get_gate_library()->get_gate_type_by_name("BUF"), "gate_2");

                test_grouping->assign_gate(gate_0);
                test_grouping->assign_gate(gate_1);
                test_grouping->assign_gate(gate_2);

                std::vector<Gate*> exp_res_0 = {gate_0, gate_1, gate_2};
                std::vector<u32> exp_res_1 = {1, 2, 3};

                EXPECT_EQ(test_grouping->get_gates(), exp_res_0);
                EXPECT_EQ(test_grouping->get_gate_ids(), exp_res_1);
            }
        TEST_END
    }

    /**
     * Testing the contains_gate function
     *
     * Functions: contains_gate, contains_gate_by_id
     */
    TEST_F(GroupingTest, check_contains_gate) {
        TEST_START
            // POSITIVE
            {
                // Check a Gate that is part of the grouping
                auto nl = test_utils::create_empty_netlist();
                Grouping* g_0 = nl->create_grouping("test_grouping");
                Gate* gate_0 = nl->create_gate(1, nl->get_gate_library()->get_gate_type_by_name("BUF"), "gate_0");
                g_0->assign_gate(gate_0);

                EXPECT_TRUE(g_0->contains_gate(gate_0));
                EXPECT_TRUE(g_0->contains_gate_by_id(1));
            }
            {
                // Check a Gate that isn't part of the grouping
                auto nl = test_utils::create_empty_netlist();
                Grouping* g_0 = nl->create_grouping("test_grouping");
                Gate* gate_0 = nl->create_gate(1, nl->get_gate_library()->get_gate_type_by_name("BUF"), "gate_0");

                EXPECT_FALSE(g_0->contains_gate(gate_0));
                EXPECT_FALSE(g_0->contains_gate_by_id(1));
            }
        TEST_END
    }

    /**
     * Testing the addition of gates to the grouping. Verify the addition by calling the
     * get_gates function and the contains_gate function
     *
     * Functions: assign_gate, assign_gate_by_id
     */
    TEST_F(GroupingTest, check_assign_gate) {
        TEST_START
            {
                // add some gates to the grouping
                auto nl = test_utils::create_empty_netlist();
                Gate* gate_0 = nl->create_gate(1, nl->get_gate_library()->get_gate_type_by_name("BUF"), "gate_0");
                Gate* gate_1 = nl->create_gate(2, nl->get_gate_library()->get_gate_type_by_name("BUF"), "gate_1");
                Gate* gate_2 = nl->create_gate(3, nl->get_gate_library()->get_gate_type_by_name("BUF"), "gate_2");
                Gate* gate_3 = nl->create_gate(4, nl->get_gate_library()->get_gate_type_by_name("BUF"), "gate_3");

                // add two gates to grouping
                Grouping* grouping_0 = nl->create_grouping("grouping_0");
                grouping_0->assign_gate(gate_0);
                grouping_0->assign_gate(gate_1);

                // add two gates to another grouping by ID
                Grouping* grouping_1 = nl->create_grouping("grouping_1");
                grouping_1->assign_gate_by_id(3);
                grouping_1->assign_gate_by_id(4);

                std::vector<Gate*> exp_res_0 = {gate_0, gate_1};
                std::vector<Gate*> exp_res_1 = {gate_2, gate_3};

                EXPECT_TRUE(test_utils::vectors_have_same_content(grouping_0->get_gates(), exp_res_0));
                EXPECT_TRUE(grouping_0->contains_gate(gate_0));
                EXPECT_TRUE(grouping_0->contains_gate(gate_1));
                EXPECT_FALSE(grouping_0->contains_gate(gate_2));

                EXPECT_TRUE(test_utils::vectors_have_same_content(grouping_1->get_gates(), exp_res_1));
                EXPECT_TRUE(grouping_1->contains_gate(gate_2));
                EXPECT_TRUE(grouping_1->contains_gate(gate_3));
                EXPECT_FALSE(grouping_1->contains_gate(gate_0));
            }
            {
                // Add the same gate twice
                NO_COUT_TEST_BLOCK;
                auto nl = test_utils::create_empty_netlist();
                Gate* gate_0 = nl->create_gate(1, nl->get_gate_library()->get_gate_type_by_name("BUF"), "gate_0");
                Gate* gate_1 = nl->create_gate(2, nl->get_gate_library()->get_gate_type_by_name("BUF"), "gate_1");

                // Add gate_0 twice
                Grouping* test_grouping = nl->create_grouping("test_grouping");
                test_grouping->assign_gate(gate_0);
                test_grouping->assign_gate(gate_0);
                test_grouping->assign_gate_by_id(2);
                test_grouping->assign_gate_by_id(2);

                std::vector<Gate*> exp_res = {gate_0, gate_1};

                EXPECT_EQ(test_grouping->get_gates(), exp_res);
                EXPECT_TRUE(test_grouping->contains_gate(gate_0));
                EXPECT_TRUE(test_grouping->contains_gate(gate_1));
            }
            {
                // assign gate that is already part of other grouping
                NO_COUT_TEST_BLOCK;
                auto nl = test_utils::create_empty_netlist();
                Gate* gate_0 = nl->create_gate(1, nl->get_gate_library()->get_gate_type_by_name("BUF"), "gate_0");
                Gate* gate_1 = nl->create_gate(2, nl->get_gate_library()->get_gate_type_by_name("BUF"), "gate_1");


                Grouping* grouping_0 = nl->create_grouping("grouping_0");
                Grouping* grouping_1 = nl->create_grouping("grouping_1");

                std::vector<Gate*> exp_res_0 = {gate_0, gate_1};
                std::vector<Gate*> exp_res_1 = {gate_1, gate_0};

                grouping_0->assign_gate(gate_0);
                grouping_0->assign_gate_by_id(2);
                ASSERT_TRUE(grouping_0->contains_gate(gate_0));
                ASSERT_FALSE(grouping_1->contains_gate(gate_0));
                ASSERT_TRUE(grouping_0->contains_gate(gate_1));
                ASSERT_FALSE(grouping_1->contains_gate(gate_1));

                grouping_1->assign_gate(gate_0);
                grouping_1->assign_gate_by_id(2);
                EXPECT_EQ(grouping_0->get_gates(), exp_res_0);
                EXPECT_FALSE(grouping_1->contains_gate(gate_0));
                EXPECT_FALSE(grouping_1->contains_gate(gate_1));

                grouping_1->assign_gate_by_id(2, true);
                grouping_1->assign_gate(gate_0, true);
                EXPECT_EQ(grouping_1->get_gates(), exp_res_1);
                EXPECT_FALSE(grouping_0->contains_gate(gate_0));
                EXPECT_FALSE(grouping_0->contains_gate(gate_1));
            }

            // NEGATIVE
            {
                // Assigned Gate is a nullptr
                NO_COUT_TEST_BLOCK;
                auto nl = test_utils::create_empty_netlist();
                Grouping* test_grouping = nl->create_grouping("test_grouping");
                test_grouping->assign_gate(nullptr);
                EXPECT_TRUE(test_grouping->get_gates().empty());
            }
            {
                // Assigned gate has invalid ID
                NO_COUT_TEST_BLOCK;
                auto nl = test_utils::create_empty_netlist();
                Grouping* test_grouping = nl->create_grouping("test_grouping");
                test_grouping->assign_gate_by_id(0);
                EXPECT_TRUE(test_grouping->get_gates().empty());
            }
            {
                // Call contains_gate with a nullptr
                NO_COUT_TEST_BLOCK;
                auto nl = test_utils::create_empty_netlist();
                Grouping* test_grouping = nl->create_grouping("test_grouping");
                EXPECT_FALSE(test_grouping->contains_gate(nullptr));
            }
            {
                // Call contains_gate with an invalid ID
                NO_COUT_TEST_BLOCK;
                auto nl = test_utils::create_empty_netlist();
                Grouping* test_grouping = nl->create_grouping("test_grouping");
                EXPECT_FALSE(test_grouping->contains_gate_by_id(0));
            }
        TEST_END
    }

    /**
     * Testing the deletion of gates from groupings
     *
     * Functions: remove_gate, remove_gate_by_id
     */
    TEST_F(GroupingTest, check_remove_gate) {
        TEST_START
            {
                // Delete a Gate from a grouping
                auto nl = test_utils::create_empty_netlist();
                Grouping* test_grouping = nl->create_grouping("test_grouping");
                Gate* gate_0 = nl->create_gate(1, nl->get_gate_library()->get_gate_type_by_name("BUF"), "gate_0");
                Gate* gate_1 = nl->create_gate(2, nl->get_gate_library()->get_gate_type_by_name("BUF"), "gate_1");

                test_grouping->assign_gate(gate_0);
                test_grouping->assign_gate(gate_1);

                ASSERT_TRUE(test_grouping->contains_gate(gate_0));
                ASSERT_TRUE(test_grouping->contains_gate(gate_1));
                test_grouping->remove_gate(gate_0);
                test_grouping->remove_gate_by_id(2);
                EXPECT_FALSE(test_grouping->contains_gate(gate_0));
                EXPECT_FALSE(test_grouping->contains_gate(gate_1));
            }
            {
                // Try to delete a Gate from a grouping that is not part of the grouping
                NO_COUT_TEST_BLOCK;
                auto nl = test_utils::create_empty_netlist();
                Grouping* grouping_0 = nl->create_grouping("grouping_0");
                Grouping* grouping_1 = nl->create_grouping("grouping_1");
                Gate* gate_0 = nl->create_gate(1, nl->get_gate_library()->get_gate_type_by_name("BUF"), "gate_0");
                Gate* gate_1 = nl->create_gate(2, nl->get_gate_library()->get_gate_type_by_name("BUF"), "gate_1");

                grouping_1->assign_gate(gate_0);
                grouping_1->assign_gate(gate_1);

                grouping_0->remove_gate(gate_0);
                grouping_0->remove_gate_by_id(2);
                EXPECT_FALSE(grouping_0->contains_gate(gate_0));
                EXPECT_TRUE(grouping_1->contains_gate(gate_0));
                EXPECT_FALSE(grouping_0->contains_gate(gate_1));
                EXPECT_TRUE(grouping_1->contains_gate(gate_1));
            }
            {
                // Try to delete a nullptr (should not crash)
                NO_COUT_TEST_BLOCK;
                auto nl = test_utils::create_empty_netlist();
                Grouping* test_grouping = nl->create_grouping("test_grouping");

                test_grouping->remove_gate(nullptr);
            }
            {
                // Try to delete an invalid ID
                NO_COUT_TEST_BLOCK;
                auto nl = test_utils::create_empty_netlist();
                Grouping* test_grouping = nl->create_grouping("test_grouping");

                test_grouping->remove_gate_by_id(0);
            }
        TEST_END
    }

    /**
     * Testing retrieving the contained nets of a grouping.
     * 
     * Functions: get_nets, get_net_ids
     */
    TEST_F(GroupingTest, check_get_nets) {
        TEST_START
            {
                auto nl = test_utils::create_empty_netlist();
                Grouping* test_grouping = nl->create_grouping("test_grouping");

                EXPECT_EQ(test_grouping->get_nets(), std::vector<Net*>());
                EXPECT_EQ(test_grouping->get_net_ids(), std::vector<u32>());

                Net* net_0 = nl->create_net(1, "net_0");
                Net* net_1 = nl->create_net(2, "net_1");
                Net* net_2 = nl->create_net(3, "net_2");

                test_grouping->assign_net(net_0);
                test_grouping->assign_net(net_1);
                test_grouping->assign_net(net_2);

                std::vector<Net*> exp_res_0 = {net_0, net_1, net_2};
                std::vector<u32> exp_res_1 = {1, 2, 3};

                EXPECT_EQ(test_grouping->get_nets(), exp_res_0);
                EXPECT_EQ(test_grouping->get_net_ids(), exp_res_1);
            }
        TEST_END
    }

    /**
     * Testing the contains_net function
     *
     * Functions: contains_net, contains_net_by_id
     */
    TEST_F(GroupingTest, check_contains_net) {
        TEST_START
            // POSITIVE
            {
                // Check a Gate that is part of the grouping
                auto nl = test_utils::create_empty_netlist();
                Grouping* g_0 = nl->create_grouping("test_grouping");
                Net* net_0 = nl->create_net(1, "net_0");
                g_0->assign_net(net_0);

                EXPECT_TRUE(g_0->contains_net(net_0));
                EXPECT_TRUE(g_0->contains_net_by_id(1));
            }
            {
                // Check a Gate that isn't part of the grouping
                auto nl = test_utils::create_empty_netlist();
                Grouping* g_0 = nl->create_grouping("test_grouping");
                Net* net_0 = nl->create_net(1, "net_0");

                EXPECT_FALSE(g_0->contains_net(net_0));
                EXPECT_FALSE(g_0->contains_net_by_id(1));
            }
        TEST_END
    }

    /**
     * Testing the addition of nets to the grouping. Verify the addition by calling the
     * get_nets function and the contains_net function
     *
     * Functions: assign_net, assign_net_by_id
     */
    TEST_F(GroupingTest, check_assign_net) {
        TEST_START
            {
                // add some nets to the grouping
                auto nl = test_utils::create_empty_netlist();
                Net* net_0 = nl->create_net(1, "net_0");
                Net* net_1 = nl->create_net(2, "net_1");
                Net* net_2 = nl->create_net(3, "net_2");
                Net* net_3 = nl->create_net(4, "net_3");

                // add two nets to grouping
                Grouping* grouping_0 = nl->create_grouping("grouping_0");
                grouping_0->assign_net(net_0);
                grouping_0->assign_net(net_1);

                // add two nets to another grouping by ID
                Grouping* grouping_1 = nl->create_grouping("grouping_1");
                grouping_1->assign_net_by_id(3);
                grouping_1->assign_net_by_id(4);

                std::vector<Net*> exp_res_0 = {net_0, net_1};
                std::vector<Net*> exp_res_1 = {net_2, net_3};

                EXPECT_TRUE(test_utils::vectors_have_same_content(grouping_0->get_nets(), exp_res_0));
                EXPECT_TRUE(grouping_0->contains_net(net_0));
                EXPECT_TRUE(grouping_0->contains_net(net_1));
                EXPECT_FALSE(grouping_0->contains_net(net_2));

                EXPECT_TRUE(test_utils::vectors_have_same_content(grouping_1->get_nets(), exp_res_1));
                EXPECT_TRUE(grouping_1->contains_net(net_2));
                EXPECT_TRUE(grouping_1->contains_net(net_3));
                EXPECT_FALSE(grouping_1->contains_net(net_0));
            }
            {
                // Add the same net twice
                NO_COUT_TEST_BLOCK;
                auto nl = test_utils::create_empty_netlist();
                Net* net_0 = nl->create_net(1, "net_0");
                Net* net_1 = nl->create_net(2, "net_1");

                // Add gate_0 twice
                Grouping* test_grouping = nl->create_grouping("test_grouping");
                test_grouping->assign_net(net_0);
                test_grouping->assign_net(net_0);
                test_grouping->assign_net_by_id(2);
                test_grouping->assign_net_by_id(2);

                std::vector<Net*> exp_res = {net_0, net_1};

                EXPECT_EQ(test_grouping->get_nets(), exp_res);
                EXPECT_TRUE(test_grouping->contains_net(net_0));
                EXPECT_TRUE(test_grouping->contains_net(net_1));
            }
            {
                // assign net that is already part of other grouping
                NO_COUT_TEST_BLOCK;
                auto nl = test_utils::create_empty_netlist();
                Net* net_0 = nl->create_net(1, "net_0");
                Net* net_1 = nl->create_net(2, "net_1");


                Grouping* grouping_0 = nl->create_grouping("grouping_0");
                Grouping* grouping_1 = nl->create_grouping("grouping_1");

                std::vector<Net*> exp_res_0 = {net_0, net_1};
                std::vector<Net*> exp_res_1 = {net_1, net_0};

                grouping_0->assign_net(net_0);
                grouping_0->assign_net_by_id(2);
                ASSERT_TRUE(grouping_0->contains_net(net_0));
                ASSERT_FALSE(grouping_1->contains_net(net_0));
                ASSERT_TRUE(grouping_0->contains_net(net_1));
                ASSERT_FALSE(grouping_1->contains_net(net_1));

                grouping_1->assign_net(net_0);
                grouping_1->assign_net_by_id(2);
                EXPECT_EQ(grouping_0->get_nets(), exp_res_0);
                EXPECT_FALSE(grouping_1->contains_net(net_0));
                EXPECT_FALSE(grouping_1->contains_net(net_1));

                grouping_1->assign_net_by_id(2, true);
                grouping_1->assign_net(net_0, true);
                EXPECT_EQ(grouping_1->get_nets(), exp_res_1);
                EXPECT_FALSE(grouping_0->contains_net(net_0));
                EXPECT_FALSE(grouping_0->contains_net(net_1));
            }

            // NEGATIVE
            {
                // Assigned net is a nullptr
                NO_COUT_TEST_BLOCK;
                auto nl = test_utils::create_empty_netlist();
                Grouping* test_grouping = nl->create_grouping("test_grouping");
                test_grouping->assign_net(nullptr);
                EXPECT_TRUE(test_grouping->get_nets().empty());
            }
            {
                // Assigned net has invalid ID
                NO_COUT_TEST_BLOCK;
                auto nl = test_utils::create_empty_netlist();
                Grouping* test_grouping = nl->create_grouping("test_grouping");
                test_grouping->assign_gate_by_id(0);
                EXPECT_TRUE(test_grouping->get_nets().empty());
            }
            {
                // Call contains_net with a nullptr
                NO_COUT_TEST_BLOCK;
                auto nl = test_utils::create_empty_netlist();
                Grouping* test_grouping = nl->create_grouping("test_grouping");
                EXPECT_FALSE(test_grouping->contains_net(nullptr));
            }
            {
                // Call contains_net with an invalid ID
                NO_COUT_TEST_BLOCK;
                auto nl = test_utils::create_empty_netlist();
                Grouping* test_grouping = nl->create_grouping("test_grouping");
                EXPECT_FALSE(test_grouping->contains_net_by_id(0));
            }
        TEST_END
    }

    /**
     * Testing the deletion of nets from groupings
     *
     * Functions: remove_net, remove_net_by_id
     */
    TEST_F(GroupingTest, check_remove_net) {
        TEST_START
            {
                // Delete a net from a grouping
                auto nl = test_utils::create_empty_netlist();
                Grouping* test_grouping = nl->create_grouping("test_grouping");
                Net* net_0 = nl->create_net(1, "net_0");
                Net* net_1 = nl->create_net(2, "net_1");

                test_grouping->assign_net(net_0);
                test_grouping->assign_net(net_1);

                ASSERT_TRUE(test_grouping->contains_net(net_0));
                ASSERT_TRUE(test_grouping->contains_net(net_1));
                test_grouping->remove_net(net_0);
                test_grouping->remove_net_by_id(2);
                EXPECT_FALSE(test_grouping->contains_net(net_0));
                EXPECT_FALSE(test_grouping->contains_net(net_1));
            }
            {
                // Try to delete a net from a grouping that is not part of the grouping
                NO_COUT_TEST_BLOCK;
                auto nl = test_utils::create_empty_netlist();
                Grouping* grouping_0 = nl->create_grouping("grouping_0");
                Grouping* grouping_1 = nl->create_grouping("grouping_1");
                Net* net_0 = nl->create_net(1, "net_0");
                Net* net_1 = nl->create_net(2, "net_1");

                grouping_1->assign_net(net_0);
                grouping_1->assign_net(net_1);

                grouping_0->remove_net(net_0);
                grouping_0->remove_net_by_id(2);
                EXPECT_FALSE(grouping_0->contains_net(net_0));
                EXPECT_TRUE(grouping_1->contains_net(net_0));
                EXPECT_FALSE(grouping_0->contains_net(net_1));
                EXPECT_TRUE(grouping_1->contains_net(net_1));
            }
            {
                // Try to delete a nullptr (should not crash)
                NO_COUT_TEST_BLOCK;
                auto nl = test_utils::create_empty_netlist();
                Grouping* test_grouping = nl->create_grouping("test_grouping");

                test_grouping->remove_net(nullptr);
            }
            {
                // Try to delete an invalid ID
                NO_COUT_TEST_BLOCK;
                auto nl = test_utils::create_empty_netlist();
                Grouping* test_grouping = nl->create_grouping("test_grouping");

                test_grouping->remove_net_by_id(0);
            }
        TEST_END
    }

    /**
     * Testing retrieving the contained modules of a grouping.
     * 
     * Functions: get_modules, get_module_ids
     */
    TEST_F(GroupingTest, check_get_modules) {
        TEST_START
            {
                auto nl = test_utils::create_empty_netlist();
                Grouping* test_grouping = nl->create_grouping("test_grouping");

                EXPECT_EQ(test_grouping->get_modules(), std::vector<Module*>());
                EXPECT_EQ(test_grouping->get_module_ids(), std::vector<u32>());

                Module* module_0 = nl->create_module(2, "module_0", nl->get_top_module());
                Module* module_1 = nl->create_module(3, "module_1", nl->get_top_module());
                Module* module_2 = nl->create_module(4, "module_2", nl->get_top_module());

                test_grouping->assign_module(module_0);
                test_grouping->assign_module(module_1);
                test_grouping->assign_module(module_2);

                std::vector<Module*> exp_res_0 = {module_0, module_1, module_2};
                std::vector<u32> exp_res_1 = {2, 3, 4};

                EXPECT_EQ(test_grouping->get_modules(), exp_res_0);
                EXPECT_EQ(test_grouping->get_module_ids(), exp_res_1);
            }
        TEST_END
    }

    /**
     * Testing the contains_module function
     *
     * Functions: contains_module, contains_module_by_id
     */
    TEST_F(GroupingTest, check_contains_module) {
        TEST_START
            // POSITIVE
            {
                // Check a Gate that is part of the grouping
                auto nl = test_utils::create_empty_netlist();
                Grouping* g_0 = nl->create_grouping("test_grouping");
                Module* module_0 = nl->create_module(2, "module_0", nl->get_top_module());
                g_0->assign_module(module_0);

                EXPECT_TRUE(g_0->contains_module(module_0));
                EXPECT_TRUE(g_0->contains_module_by_id(2));
            }
            {
                // Check a Gate that isn't part of the grouping
                auto nl = test_utils::create_empty_netlist();
                Grouping* g_0 = nl->create_grouping("test_grouping");
                Module* module_0 = nl->create_module(2, "module_0", nl->get_top_module());

                EXPECT_FALSE(g_0->contains_module(module_0));
                EXPECT_FALSE(g_0->contains_module_by_id(2));
            }
        TEST_END
    }

    /**
     * Testing the addition of modules to the grouping. Verify the addition by calling the
     * get_modules function and the contains_module function
     *
     * Functions: assign_module, assign_module_by_id
     */
    TEST_F(GroupingTest, check_assign_module) {
        TEST_START
            {
                // add some modules to the grouping
                auto nl = test_utils::create_empty_netlist();
                Module* module_0 = nl->create_module(2, "module_0", nl->get_top_module());
                Module* module_1 = nl->create_module(3, "module_1", nl->get_top_module());
                Module* module_2 = nl->create_module(4, "module_2", nl->get_top_module());
                Module* module_3 = nl->create_module(5, "module_3", nl->get_top_module());

                // add two modules to grouping
                Grouping* grouping_0 = nl->create_grouping("grouping_0");
                grouping_0->assign_module(module_0);
                grouping_0->assign_module(module_1);

                // add two modules to another grouping by ID
                Grouping* grouping_1 = nl->create_grouping("grouping_1");
                grouping_1->assign_module_by_id(4);
                grouping_1->assign_module_by_id(5);

                std::vector<Module*> exp_res_0 = {module_0, module_1};
                std::vector<Module*> exp_res_1 = {module_2, module_3};

                EXPECT_TRUE(test_utils::vectors_have_same_content(grouping_0->get_modules(), exp_res_0));

                // for(auto test : grouping_0->get_modules()) {
                //     std::cout << test->get_name() << std::endl;;
                // }

                // for(auto test : exp_res_0) {
                //     std::cout << test->get_name() << std::endl;;
                // }
                EXPECT_TRUE(grouping_0->contains_module(module_0));
                EXPECT_TRUE(grouping_0->contains_module(module_1));
                EXPECT_FALSE(grouping_0->contains_module(module_2));

                EXPECT_TRUE(test_utils::vectors_have_same_content(grouping_1->get_modules(), exp_res_1));
                EXPECT_TRUE(grouping_1->contains_module(module_2));
                EXPECT_TRUE(grouping_1->contains_module(module_3));
                EXPECT_FALSE(grouping_1->contains_module(module_0));
            }
            {
                // Add the same net twice
                NO_COUT_TEST_BLOCK;
                auto nl = test_utils::create_empty_netlist();
                Module* module_0 = nl->create_module(2, "module_0", nl->get_top_module());
                Module* module_1 = nl->create_module(3, "module_1", nl->get_top_module());

                // Add gate_0 twice
                Grouping* test_grouping = nl->create_grouping("test_grouping");
                test_grouping->assign_module(module_0);
                test_grouping->assign_module(module_0);
                test_grouping->assign_module_by_id(3);
                test_grouping->assign_module_by_id(3);

                std::vector<Module*> exp_res = {module_0, module_1};

                EXPECT_EQ(test_grouping->get_modules(), exp_res);
                EXPECT_TRUE(test_grouping->contains_module(module_0));
                EXPECT_TRUE(test_grouping->contains_module(module_1));
            }
            {
                // assign net that is already part of other grouping
                NO_COUT_TEST_BLOCK;
                auto nl = test_utils::create_empty_netlist();
                Module* module_0 = nl->create_module(2, "module_0", nl->get_top_module());
                Module* module_1 = nl->create_module(3, "module_1", nl->get_top_module());


                Grouping* grouping_0 = nl->create_grouping("grouping_0");
                Grouping* grouping_1 = nl->create_grouping("grouping_1");

                std::vector<Module*> exp_res_0 = {module_0, module_1};
                std::vector<Module*> exp_res_1 = {module_1, module_0};

                grouping_0->assign_module(module_0);
                grouping_0->assign_module_by_id(3);
                ASSERT_TRUE(grouping_0->contains_module(module_0));
                ASSERT_FALSE(grouping_1->contains_module(module_0));
                ASSERT_TRUE(grouping_0->contains_module(module_1));
                ASSERT_FALSE(grouping_1->contains_module(module_1));

                grouping_1->assign_module(module_0);
                grouping_1->assign_module_by_id(3);
                EXPECT_EQ(grouping_0->get_modules(), exp_res_0);
                EXPECT_FALSE(grouping_1->contains_module(module_0));
                EXPECT_FALSE(grouping_1->contains_module(module_1));

                grouping_1->assign_module_by_id(3, true);
                grouping_1->assign_module(module_0, true);
                EXPECT_EQ(grouping_1->get_modules(), exp_res_1);
                EXPECT_FALSE(grouping_0->contains_module(module_0));
                EXPECT_FALSE(grouping_0->contains_module(module_1));
            }

            // NEGATIVE
            {
                // Assigned net is a nullptr
                NO_COUT_TEST_BLOCK;
                auto nl = test_utils::create_empty_netlist();
                Grouping* test_grouping = nl->create_grouping("test_grouping");
                test_grouping->assign_module(nullptr);
                EXPECT_TRUE(test_grouping->get_modules().empty());
            }
            {
                // Assigned net has invalid ID
                NO_COUT_TEST_BLOCK;
                auto nl = test_utils::create_empty_netlist();
                Grouping* test_grouping = nl->create_grouping("test_grouping");
                test_grouping->assign_gate_by_id(0);
                EXPECT_TRUE(test_grouping->get_modules().empty());
            }
            {
                // Call contains_module with a nullptr
                NO_COUT_TEST_BLOCK;
                auto nl = test_utils::create_empty_netlist();
                Grouping* test_grouping = nl->create_grouping("test_grouping");
                EXPECT_FALSE(test_grouping->contains_module(nullptr));
            }
            {
                // Call contains_module with an invalid ID
                NO_COUT_TEST_BLOCK;
                auto nl = test_utils::create_empty_netlist();
                Grouping* test_grouping = nl->create_grouping("test_grouping");
                EXPECT_FALSE(test_grouping->contains_module_by_id(0));
            }
        TEST_END
    }

    /**
     * Testing the deletion of gates from groupings
     *
     * Functions: remove_gate, remove_gate_by_id
     */
    TEST_F(GroupingTest, check_remove_module) {
        TEST_START
            {
                // Delete a Gate from a grouping
                auto nl = test_utils::create_empty_netlist();
                Grouping* test_grouping = nl->create_grouping("test_grouping");
                Module* module_0 = nl->create_module(2, "module_0", nl->get_top_module());
                Module* module_1 = nl->create_module(3, "module_1", nl->get_top_module());

                test_grouping->assign_module(module_0);
                test_grouping->assign_module(module_1);

                ASSERT_TRUE(test_grouping->contains_module(module_0));
                ASSERT_TRUE(test_grouping->contains_module(module_1));
                test_grouping->remove_module(module_0);
                test_grouping->remove_module_by_id(3);
                EXPECT_FALSE(test_grouping->contains_module(module_0));
                EXPECT_FALSE(test_grouping->contains_module(module_1));
            }
            {
                // Try to delete a Gate from a grouping that is not part of the grouping
                NO_COUT_TEST_BLOCK;
                auto nl = test_utils::create_empty_netlist();
                Grouping* grouping_0 = nl->create_grouping("grouping_0");
                Grouping* grouping_1 = nl->create_grouping("grouping_1");
                Module* module_0 = nl->create_module(2, "module_0", nl->get_top_module());
                Module* module_1 = nl->create_module(3, "module_1", nl->get_top_module());

                grouping_1->assign_module(module_0);
                grouping_1->assign_module(module_1);

                grouping_0->remove_module(module_0);
                grouping_0->remove_module_by_id(3);
                EXPECT_FALSE(grouping_0->contains_module(module_0));
                EXPECT_TRUE(grouping_1->contains_module(module_0));
                EXPECT_FALSE(grouping_0->contains_module(module_1));
                EXPECT_TRUE(grouping_1->contains_module(module_1));
            }
            {
                // Try to delete a nullptr (should not crash)
                NO_COUT_TEST_BLOCK;
                auto nl = test_utils::create_empty_netlist();
                Grouping* test_grouping = nl->create_grouping("test_grouping");

                test_grouping->remove_module(nullptr);
            }
            {
                // Try to delete an invalid ID
                NO_COUT_TEST_BLOCK;
                auto nl = test_utils::create_empty_netlist();
                Grouping* test_grouping = nl->create_grouping("test_grouping");

                test_grouping->remove_module_by_id(0);
            }
        TEST_END
    }


} //namespace hal
