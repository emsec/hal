#include "netlist_test_utils.h"
#include "hal_core/netlist/event_system/net_event_handler.h"

namespace hal {
    using test_utils::MIN_GATE_ID;
    using test_utils::MIN_NET_ID;
    using test_utils::MIN_MODULE_ID;
    using test_utils::MIN_NETLIST_ID;

    class NetTest : public ::testing::Test {
    protected:
        virtual void SetUp() {
            test_utils::init_log_channels();
        }

        virtual void TearDown() {
        }
    };


    /**
     * Testing the constructor of the Net
     *
     * Functions: constructor, get_id, get_name, get_netlist
     */
    TEST_F(NetTest, check_constructor) {
        TEST_START
            // Create a Net (id = 100) and append it to its netlist
            auto nl = test_utils::create_empty_netlist(MIN_NETLIST_ID + 0);
            Net* test_net = nl->create_net(MIN_NET_ID + 100, "test_net");

            EXPECT_EQ(test_net->get_id(), (u32) (MIN_NET_ID + 100));
            EXPECT_EQ(test_net->get_name(), "test_net");
            EXPECT_EQ(test_net->get_netlist()->get_id(), (u32) (MIN_NETLIST_ID + 0));

        TEST_END
    }

    /**
     * Testing the function set_name and get_name
     *
     * Functions: get_name, set_name
     */
    TEST_F(NetTest, check_set_and_get_name) {
        TEST_START
            // Create a Net and append it to its netlist
            auto nl = test_utils::create_empty_netlist(MIN_NETLIST_ID + 0);
            Net* test_net = nl->create_net(MIN_NET_ID + 1, "test_net");

            EXPECT_EQ(test_net->get_name(), "test_net");

            // Set a new name
            NO_COUT(test_net->set_name("new_name"));
            EXPECT_EQ(test_net->get_name(), "new_name");

            // Set the name to the same new name again
            NO_COUT(test_net->set_name("new_name"));
            EXPECT_EQ(test_net->get_name(), "new_name");

            // Set an empty name (should do nothing)
            NO_COUT(test_net->set_name("name"));
            NO_COUT(test_net->set_name(""));
            EXPECT_EQ(test_net->get_name(), "name");

        TEST_END
    }

    /**
     * Testing the function add_src
     *
     * Functions: add_src
     */
    TEST_F(NetTest, check_add_src) {
        TEST_START
            {
                // Add a source of the Net (using a valid Gate and pin_type)
                auto nl = test_utils::create_empty_netlist(MIN_NETLIST_ID + 0);
                Net* test_net = nl->create_net(MIN_NET_ID + 1, "test_net");
                Gate* t_gate = test_utils::create_test_gate(nl.get(), MIN_GATE_ID + 1);
                bool suc = test_net->add_source(t_gate, "O");
                EXPECT_TRUE(suc);
                ASSERT_EQ(test_net->get_sources().size(), 1);
                EXPECT_EQ(test_net->get_sources()[0], test_utils::get_endpoint(t_gate, "O"));
            }
            // Negative
            {
                // Set the source of the Net (Gate is nullptr)
                NO_COUT_TEST_BLOCK;
                auto nl = test_utils::create_empty_netlist(MIN_NETLIST_ID + 0);
                Net* test_net = nl->create_net(MIN_NET_ID + 1, "test_net");
                bool suc = test_net->add_source(nullptr, "O");
                EXPECT_FALSE(suc);
                EXPECT_EQ(test_net->get_sources().size(), 0);
            }
            {
                // Pin is an input pin (not an output/inout pin)
                NO_COUT_TEST_BLOCK;
                auto nl = test_utils::create_empty_netlist(MIN_NETLIST_ID + 0);
                Net* test_net = nl->create_net(MIN_NET_ID + 1, "test_net");
                auto t_gate_0 = test_utils::create_test_gate(nl.get(), MIN_GATE_ID + 1);
                bool suc = test_net->add_source(t_gate_0, "I0");    // <- input pin
                EXPECT_FALSE(suc);
                EXPECT_EQ(test_net->get_sources().size(), 0);
            }
            {
                // Pin is already occupied (example netlist is used)
                NO_COUT_TEST_BLOCK;
                auto nl = test_utils::create_example_netlist(MIN_NETLIST_ID + 0);
                Net* test_net = nl->create_net(MIN_NET_ID + 1, "test_net");
                bool suc = test_net->add_source(nl->get_gate_by_id(MIN_NET_ID + 1), "O");
                EXPECT_FALSE(suc);
                EXPECT_EQ(test_net->get_sources().size(), 0);
            }
            {
                // Set the source of the Net (invalid pin type)
                auto nl = test_utils::create_empty_netlist(0);
                Net* test_net = nl->create_net(MIN_NET_ID + 1, "test_net");
                auto t_gate = test_utils::create_test_gate(nl.get(), MIN_GATE_ID + 1);
                testing::internal::CaptureStdout();
                bool suc = test_net->add_source(t_gate, "NEx_PIN");
                testing::internal::GetCapturedStdout();
                EXPECT_FALSE(suc);
                EXPECT_EQ(test_net->get_sources().size(), 0);
            }

        TEST_END
    }

    /**
     * Testing the access on sources
     *
     * Functions: get_sources, get_source
     */
    // disable get_source() deprecated warning for this test (because get_source() is also tested)
    #pragma GCC diagnostic ignored "-Wdeprecated-declarations"
    #pragma warning(disable:1478)
    TEST_F(NetTest, check_get_sources) {
        TEST_START
            {
                // Get a single source
                auto nl = test_utils::create_empty_netlist(MIN_NETLIST_ID + 0);
                Net* test_net = nl->create_net(MIN_NET_ID + 1, "test_net");
                auto t_gate =
                    nl->create_gate(MIN_GATE_ID + 0, test_utils::get_gate_type_by_name("gate_4_to_4"), "test_gate");
                test_net->add_source(t_gate, "O0");
                EXPECT_EQ(test_net->get_source(), test_utils::get_endpoint(t_gate, "O0")); // (compiler warning intended)
            }
            {
                // Get all sources (no filter)
                auto nl = test_utils::create_empty_netlist(MIN_NETLIST_ID + 0);
                Net* test_net = nl->create_net(MIN_NET_ID + 1, "test_net");
                auto t_gate =
                    nl->create_gate(MIN_GATE_ID + 0, test_utils::get_gate_type_by_name("gate_4_to_4"), "test_gate");
                test_net->add_source(t_gate, "O0");
                test_net->add_source(t_gate, "O1");
                test_net->add_source(t_gate, "O3");
                EXPECT_EQ(test_net->get_sources(), std::vector<Endpoint*>({test_utils::get_endpoint(t_gate, "O0"),
                                                                          test_utils::get_endpoint(t_gate, "O1"),
                                                                          test_utils::get_endpoint(t_gate, "O3")}));
            }
            {
                // Get all sources by using a filter
                auto nl = test_utils::create_empty_netlist(MIN_NETLIST_ID + 0);
                Net* test_net = nl->create_net(MIN_NET_ID + 1, "test_net");
                auto t_gate_0 =
                    nl->create_gate(MIN_GATE_ID + 0, test_utils::get_gate_type_by_name("gate_4_to_4"), "test_gate_0");
                auto t_gate_1 =
                    nl->create_gate(MIN_GATE_ID + 1, test_utils::get_gate_type_by_name("gate_4_to_4"), "test_gate_1");
                test_net->add_source(t_gate_0, "O0");
                test_net->add_source(t_gate_0, "O1");
                test_net->add_source(t_gate_1, "O1");
                test_net->add_source(t_gate_1, "O2");
                EXPECT_EQ(test_net->get_sources(test_utils::endpoint_gate_name_filter("test_gate_0")),
                          std::vector<Endpoint*>({test_utils::get_endpoint(t_gate_0, "O0"),
                                                 test_utils::get_endpoint(t_gate_0, "O1")}));
            }
            {
                // Get the source(s) if the Gate has no source
                auto nl = test_utils::create_empty_netlist(MIN_NETLIST_ID + 0);
                Net* test_net = nl->create_net(MIN_NET_ID + 1, "test_net");
                EXPECT_TRUE(test_utils::is_empty(test_net->get_source())); // (compiler warning intended)
                EXPECT_TRUE(test_net->get_sources().empty());
            }
            // NEGATIVE
            {
                // Get source, if there are multiple sources (only the first one is returned)
                NO_COUT_TEST_BLOCK;
                auto nl = test_utils::create_empty_netlist(MIN_NETLIST_ID + 0);
                Net* test_net = nl->create_net(MIN_NET_ID + 1, "test_net");
                auto t_gate =
                    nl->create_gate(MIN_GATE_ID + 0, test_utils::get_gate_type_by_name("gate_4_to_4"), "test_gate");
                test_net->add_source(t_gate, "O0");
                test_net->add_source(t_gate, "O1");
                EXPECT_EQ(test_net->get_source(), test_utils::get_endpoint(t_gate, "O0")); // (compiler warning intended)
            }
        TEST_END
    }
    // enable get_source() deprecated warning
    #pragma GCC diagnostic warning "-Wdeprecated-declarations"
    #pragma warning(enable:1478) // enable get_source() deprecated warning

    /**
     * Testing the function remove_src
     *
     * Functions: remove_src
     */
    TEST_F(NetTest, check_remove_src) {
        TEST_START
            {
                // Remove an existing source (passing a Gate and a pin type)
                auto nl = test_utils::create_empty_netlist(MIN_NETLIST_ID + 0);
                Net* test_net = nl->create_net(MIN_NET_ID + 1, "test_net");
                auto t_gate = test_utils::create_test_gate(nl.get(), MIN_GATE_ID + 1);
                test_net->add_source(t_gate, "O");
                bool suc = test_net->remove_source(t_gate, "O");
                EXPECT_EQ(test_net->get_sources().size(), 0);
                EXPECT_TRUE(suc);
            }
            // NEGATIVE
            {
                // Remove the source if the passed parameters do not define any source
                NO_COUT_TEST_BLOCK;
                auto nl = test_utils::create_empty_netlist(MIN_NETLIST_ID + 0);
                Net* test_net = nl->create_net(MIN_NET_ID + 1, "test_net");
                bool suc = test_net->remove_source(nullptr, "");
                EXPECT_EQ(test_net->get_sources().size(), 0);
                EXPECT_FALSE(suc);
            }
        TEST_END
    }

    /**
     * Testing the function which removes a destination
     *
     * Functions: remove_destination, get_num_of_destinations
     */
    TEST_F(NetTest, check_add_remove_destination) {
        TEST_START
            {
                // Remove a destination in the normal way
                auto nl = test_utils::create_empty_netlist(MIN_NETLIST_ID + 0);
                Net* test_net = nl->create_net(MIN_NET_ID + 1, "test_net");
                auto t_gate = test_utils::create_test_gate(nl.get(), MIN_GATE_ID + 1);
                test_net->add_destination(t_gate, "I0");

                bool suc = test_net->remove_destination(t_gate, "I0");

                EXPECT_TRUE(suc);
                EXPECT_TRUE(test_net->get_destinations().empty());
                EXPECT_EQ(test_net->get_num_of_destinations(), (size_t) 0);
            }
            {
                // Remove the same destination twice
                auto nl = test_utils::create_empty_netlist(MIN_NETLIST_ID + 0);
                Net* test_net = nl->create_net(MIN_NET_ID + 1, "test_net");
                auto t_gate = test_utils::create_test_gate(nl.get(), MIN_GATE_ID + 1);
                test_net->add_destination(t_gate, "I0");

                test_net->remove_destination(t_gate, "I0");
                NO_COUT_TEST_BLOCK;
                bool suc = test_net->remove_destination(t_gate, "I0");

                EXPECT_FALSE(suc);
                EXPECT_TRUE(test_net->get_destinations().empty());
                EXPECT_EQ(test_net->get_num_of_destinations(), 0);
            }
            // NEGATIVE
            {
                // The Gate is a nullptr
                NO_COUT_TEST_BLOCK;
                auto nl = test_utils::create_empty_netlist(MIN_NETLIST_ID + 0);
                Net* test_net = nl->create_net(MIN_NET_ID + 1, "test_net");

                bool suc = test_net->remove_destination(nullptr, "I0");

                EXPECT_FALSE(suc);
            }
            {
                // The Gate wasn't added to the netlist
                NO_COUT_TEST_BLOCK;
                auto nl = test_utils::create_empty_netlist(MIN_NETLIST_ID + 0);
                Net* test_net = nl->create_net(MIN_NET_ID + 1, "test_net");
                Gate*
                    t_gate =
                    nl->create_gate(MIN_GATE_ID + 0, test_utils::get_gate_type_by_name("gate_1_to_1"), "t_gate");

                bool suc = test_net->remove_destination(t_gate, "I0");

                EXPECT_FALSE(suc);
            }

        TEST_END
    }

    /**
     * Testing the function which adds a destination
     *
     * Functions: add_destination, get_num_of_destinations
     */
    TEST_F(NetTest, check_add_dst) {
        TEST_START
            {
                // Add a destination in the normal way
                auto nl = test_utils::create_empty_netlist(MIN_NETLIST_ID + 0);
                Net* test_net = nl->create_net(MIN_NET_ID + 1, "test_net");

                auto t_gate = test_utils::create_test_gate(nl.get(), MIN_GATE_ID + 1);
                bool suc = test_net->add_destination(t_gate, "I0");
                std::vector<Endpoint*> dsts = {test_utils::get_endpoint(t_gate, "I0")};
                EXPECT_EQ(test_net->get_destinations(), dsts);
                EXPECT_EQ(test_net->get_num_of_destinations(), (size_t) 1);
                EXPECT_TRUE(suc);
            }
            {
                // Add the same destination twice
                NO_COUT_TEST_BLOCK;
                auto nl = test_utils::create_empty_netlist(MIN_NETLIST_ID + 0);
                Net* test_net = nl->create_net(MIN_NET_ID + 1, "test_net");

                auto t_gate = test_utils::create_test_gate(nl.get(), MIN_GATE_ID + 1);
                test_net->add_destination(t_gate, "I0");
                bool suc = test_net->add_destination(t_gate, "I0");
                std::vector<Endpoint*> dsts = {test_utils::get_endpoint(t_gate, "I0")};
                EXPECT_EQ(test_net->get_destinations(), dsts);
                EXPECT_EQ(test_net->get_num_of_destinations(), (size_t) 1);
                EXPECT_FALSE(suc);
            }

            // NEGATIVE
            {
                // The Gate is a nullptr
                NO_COUT_TEST_BLOCK;
                auto nl = test_utils::create_empty_netlist(MIN_NETLIST_ID + 0);
                Net* test_net = nl->create_net(MIN_NET_ID + 1, "test_net");
                bool suc = test_net->add_destination(nullptr, "I0");

                EXPECT_FALSE(suc);
                EXPECT_TRUE(test_net->get_destinations().empty());
                EXPECT_EQ(test_net->get_num_of_destinations(), (size_t) 0);
            }
            {
                // The Gate isn't part of the netlist
                NO_COUT_TEST_BLOCK;
                auto nl = test_utils::create_empty_netlist(MIN_NETLIST_ID + 0);
                Net* test_net = nl->create_net(MIN_NET_ID + 1, "test_net");
                Gate*
                    t_gate =
                    nl->create_gate(MIN_GATE_ID + 0, test_utils::get_gate_type_by_name("gate_1_to_1"), "t_gate");
                // Gate isn't added
                bool suc = test_net->add_destination(t_gate, "I0");

                EXPECT_FALSE(suc);
                EXPECT_TRUE(test_net->get_destinations().empty());
                EXPECT_EQ(test_net->get_num_of_destinations(), (size_t) 0);
            }
            {
                // The pin to connect is weather an input pin nor an inout pin (but an output pin)
                NO_COUT_TEST_BLOCK;
                auto nl = test_utils::create_empty_netlist(MIN_NETLIST_ID + 0);
                Net* test_net = nl->create_net(MIN_NET_ID + 1, "test_net");
                auto t_gate = test_utils::create_test_gate(nl.get(), MIN_GATE_ID + 1);
                bool suc = test_net->add_destination(t_gate, "O");

                EXPECT_FALSE(suc);
                EXPECT_TRUE(test_net->get_destinations().empty());
                EXPECT_EQ(test_net->get_num_of_destinations(), (size_t) 0);
            }
            {
                // The pin is already occupied (example netlist is used)
                NO_COUT_TEST_BLOCK;
                auto nl = test_utils::create_example_netlist(MIN_NETLIST_ID + 0);
                Net* test_net = nl->create_net(MIN_NET_ID + 1, "test_net");
                bool suc = test_net->add_destination(nl->get_gate_by_id(MIN_GATE_ID + 0), "I1");

                EXPECT_FALSE(suc);
                EXPECT_TRUE(test_net->get_destinations().empty());
                EXPECT_EQ(test_net->get_num_of_destinations(), (size_t) 0);
            }
        TEST_END
    }

    /**
     * Testing the functions is_a_destination and is_a_source
     *
     * Functions: is_a_destination, is_a_source
     */
    TEST_F(NetTest, check_is_a_dest_or_src) {
        TEST_START
            {
                // Gate is a destination
                auto nl = test_utils::create_empty_netlist(MIN_NETLIST_ID + 0);
                Net* test_net = nl->create_net(MIN_NET_ID + 1, "test_net");
                auto t_gate = test_utils::create_test_gate(nl.get(), MIN_GATE_ID + 1);
                test_net->add_destination(t_gate, "I2");
                // Pass the Gate and the pin type
                EXPECT_TRUE(test_net->is_a_destination(t_gate, "I2"));
                EXPECT_FALSE(test_net->is_a_source(t_gate, "I2"));
                // Pass the Endpoint
                EXPECT_TRUE(test_net->is_a_destination(test_utils::get_endpoint(t_gate, "I2")));
                NO_COUT_TEST_BLOCK;
                EXPECT_FALSE(test_net->is_a_source(test_utils::get_endpoint(t_gate, "I2")));
            }
            {
                // Gate is a source
                auto nl = test_utils::create_empty_netlist(MIN_NETLIST_ID + 0);
                Net* test_net = nl->create_net(MIN_NET_ID + 1, "test_net");
                auto t_gate = test_utils::create_test_gate(nl.get(), MIN_GATE_ID + 1);
                test_net->add_source(t_gate, "O");
                // Pass the Gate and the pin type
                EXPECT_TRUE(test_net->is_a_source(t_gate, "O"));
                EXPECT_FALSE(test_net->is_a_destination(t_gate, "O"));
                // Pass the Endpoint
                EXPECT_TRUE(test_net->is_a_source(test_utils::get_endpoint(t_gate, "O")));
                NO_COUT_TEST_BLOCK;
                EXPECT_FALSE(test_net->is_a_destination(test_utils::get_endpoint(t_gate, "O")));
            }
            {
                // Gate is a destination but the pin type doesn't match
                auto nl = test_utils::create_empty_netlist(MIN_NETLIST_ID + 0);
                Net* test_net = nl->create_net(MIN_NET_ID + 1, "test_net");
                auto t_gate = test_utils::create_test_gate(nl.get(), MIN_GATE_ID + 1);
                test_net->add_destination(t_gate, "I2");
                EXPECT_TRUE(test_net->is_a_destination(t_gate, "I2"));
                EXPECT_FALSE(test_net->is_a_destination(t_gate, "I1"));
            }
            {
                // Gate is a destination but the pin type doesn't exist
                auto nl = test_utils::create_empty_netlist(MIN_NETLIST_ID + 0);
                Net* test_net = nl->create_net(MIN_NET_ID + 1, "test_net");
                auto t_gate = test_utils::create_test_gate(nl.get(), MIN_GATE_ID + 1);
                test_net->add_destination(t_gate, "I2");
                EXPECT_TRUE(test_net->is_a_destination(t_gate, "I2"));
                EXPECT_FALSE(test_net->is_a_destination(t_gate, "NEx_PIN"));
            }
            // NEGATIVE
            {
                // Gate is a nullptr
                NO_COUT_TEST_BLOCK;
                auto nl = test_utils::create_empty_netlist(MIN_NETLIST_ID + 0);
                Net* test_net = nl->create_net(MIN_NET_ID + 1, "test_net");

                EXPECT_FALSE(test_net->is_a_destination(nullptr, ""));
                EXPECT_FALSE(test_net->is_a_source(nullptr, ""));
            }
        TEST_END
    }

    /**
     * Testing the function get_destinations
     *
     * Functions: get_destinations, get_destinations_by_type
     */
    TEST_F(NetTest, check_get_destinations) {
        TEST_START
            // Create a Net with two different destinations (AND3 and INV Gate)
            auto nl = test_utils::create_empty_netlist(MIN_NETLIST_ID + 0);
            Net* test_net = nl->create_net(MIN_NET_ID + 1, "test_net");
            auto mult_gate = test_utils::create_test_gate(nl.get(), MIN_GATE_ID + 1);
            Gate*
                inv_gate = nl->create_gate(MIN_GATE_ID + 2, test_utils::get_gate_type_by_name("gate_1_to_1"), "gate_1");
            test_net->add_destination(mult_gate, "I0");
            test_net->add_destination(inv_gate, "I");

            {
                // Get the destinations
                std::vector<Endpoint*>
                    dsts = {test_utils::get_endpoint(mult_gate, "I0"), test_utils::get_endpoint(inv_gate, "I")};

                EXPECT_TRUE(test_utils::vectors_have_same_content(test_net->get_destinations(), dsts));
            }
            {
                // Get the destinations by passing a Gate type
                std::vector<Endpoint*> dsts = {test_utils::get_endpoint(inv_gate, "I")};
                EXPECT_TRUE(test_utils::vectors_have_same_content(test_net
                                                                      ->get_destinations(test_utils::endpoint_gate_type_filter(
                                                                          "gate_1_to_1")),
                                                                  dsts));
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
                // Net has a source and a destination
                auto nl = test_utils::create_empty_netlist(MIN_NETLIST_ID + 0);
                Net* test_net = nl->create_net(MIN_NET_ID + 1, "test_net");

                auto t_gate_src = test_utils::create_test_gate(nl.get(), MIN_GATE_ID + 1);
                auto t_gate_dst = test_utils::create_test_gate(nl.get(), MIN_GATE_ID + 2);
                test_net->add_source(t_gate_src, "O");
                test_net->add_destination(t_gate_dst, "I0");

                EXPECT_FALSE(test_net->is_unrouted());
            }
            {
                // Net has no destination
                auto nl = test_utils::create_empty_netlist(MIN_NETLIST_ID + 0);
                Net* test_net = nl->create_net(MIN_NET_ID + 1, "test_net");

                auto t_gate_src = test_utils::create_test_gate(nl.get(), MIN_GATE_ID + 1);
                test_net->add_source(t_gate_src, "O");

                EXPECT_TRUE(test_net->is_unrouted());
            }
            {
                // Net has no source
                auto nl = test_utils::create_empty_netlist(MIN_NETLIST_ID + 0);
                Net* test_net = nl->create_net(MIN_NET_ID + 1, "test_net");

                auto t_gate_dst = test_utils::create_test_gate(nl.get(), MIN_GATE_ID + 1);
                test_net->add_destination(t_gate_dst, "I0");

                EXPECT_TRUE(test_net->is_unrouted());
            }

        TEST_END
    }

    /**
     * Testing the handling of global nets
     *
     * Functions: mark_global_input_net, mark_global_input_net, mark_global_inout_net,
     *            unmark_global_input_net, unmark_global_input_net, unmark_global_inout_net
     *            is_global_input_net, is_global_input_net, is_global_inout_net
     */
    TEST_F(NetTest, check_global_nets) {
        TEST_START
            {
                // mark and unmark a global input Net
                auto nl = test_utils::create_empty_netlist(MIN_NETLIST_ID + 0);
                Net* test_net = nl->create_net(MIN_NET_ID + 1, "test_net");

                test_net->mark_global_input_net();
                EXPECT_TRUE(test_net->is_global_input_net());
                EXPECT_TRUE(nl->is_global_input_net(test_net));

                test_net->unmark_global_input_net();
                EXPECT_FALSE(test_net->is_global_input_net());
                EXPECT_FALSE(nl->is_global_input_net(test_net));
            }
            {
                // mark and unmark a global output Net
                auto nl = test_utils::create_empty_netlist(MIN_NETLIST_ID + 0);
                Net* test_net = nl->create_net(MIN_NET_ID + 1, "test_net");

                test_net->mark_global_output_net();
                EXPECT_TRUE(test_net->is_global_output_net());
                EXPECT_TRUE(nl->is_global_output_net(test_net));

                test_net->unmark_global_output_net();
                EXPECT_FALSE(test_net->is_global_output_net());
                EXPECT_FALSE(nl->is_global_output_net(test_net));
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
            Gate* new_gate = test_nl->create_gate(test_utils::get_gate_type_by_name("gate_1_to_1"), "new_gate");

            // Small functions that should trigger certain events exactly once (these operations are executed in this order)
            std::function<void(void)> trigger_name_changed = [=](){test_net->set_name("new_name");};
            std::function<void(void)> trigger_src_added = [=](){test_net->add_source(new_gate, "O");};
            std::function<void(void)> trigger_src_removed = [=](){test_net->remove_source(new_gate, "O");};
            std::function<void(void)> trigger_dst_added = [=](){test_net->add_destination(new_gate, "I");};
            std::function<void(void)> trigger_dst_removed = [=](){test_net->remove_destination(new_gate, "I");};

            // The events that are tested
            std::vector<net_event_handler::event> event_type = {
                net_event_handler::event::name_changed, net_event_handler::event::src_added,
                net_event_handler::event::src_removed, net_event_handler::event::dst_added,
                net_event_handler::event::dst_removed};

            // A list of the functions that will trigger its associated event exactly once
            std::vector<std::function<void(void)>> trigger_event = { trigger_name_changed, trigger_src_added,
                 trigger_src_removed, trigger_dst_added, trigger_dst_removed };

            // The parameters of the events that are expected
            std::vector<std::tuple<net_event_handler::event, Net*, u32>> expected_parameter = {
                std::make_tuple(net_event_handler::event::name_changed, test_net, NO_DATA),
                std::make_tuple(net_event_handler::event::src_added, test_net, new_gate->get_id()),
                std::make_tuple(net_event_handler::event::src_removed, test_net, new_gate->get_id()),
                std::make_tuple(net_event_handler::event::dst_added, test_net, new_gate->get_id()),
                std::make_tuple(net_event_handler::event::dst_removed, test_net, new_gate->get_id())
            };

            // Check all events in a for-loop
            for(u32 event_idx = 0; event_idx < event_type.size(); event_idx++)
            {
                // Create the listener for the tested event
                test_utils::EventListener<void, net_event_handler::event, Net*, u32> listener;
                std::function<void(net_event_handler::event, Net*, u32)> cb = listener.get_conditional_callback(
                    [=](net_event_handler::event ev, Net* n, u32 id){return ev == event_type[event_idx] && n == test_net;}
                );
                std::string cb_name = "net_event_callback_" + std::to_string((u32)event_type[event_idx]);
                // Register a callback of the listener
                net_event_handler::register_callback(cb_name, cb);

                // Trigger the event
                trigger_event[event_idx]();

                EXPECT_EQ(listener.get_event_count(), 1);
                EXPECT_EQ(listener.get_last_parameters(), expected_parameter[event_idx]);

                // Unregister the callback
                net_event_handler::unregister_callback(cb_name);
            }

            // Test the events 'created' and 'removed'
            // -- 'created' event
            test_utils::EventListener<void, net_event_handler::event, Net*, u32> listener_created;
            std::function<void(net_event_handler::event, Net*, u32)> cb_created = listener_created.get_conditional_callback(
                [=](net_event_handler::event ev, Net* m, u32 id){return ev == net_event_handler::created;}
            );
            std::string cb_name_created = "net_event_callback_created";
            net_event_handler::register_callback(cb_name_created, cb_created);

            // Create a new mod
            Net* new_net = test_nl->create_net("new_net");
            EXPECT_EQ(listener_created.get_event_count(), 1);
            EXPECT_EQ(listener_created.get_last_parameters(), std::make_tuple(net_event_handler::event::created, new_net, NO_DATA));

            net_event_handler::unregister_callback(cb_name_created);

            // -- 'removed' event
            test_utils::EventListener<void, net_event_handler::event, Net*, u32> listener_removed;
            std::function<void(net_event_handler::event, Net*, u32)> cb_removed = listener_removed.get_conditional_callback(
                [=](net_event_handler::event ev, Net* m, u32 id){return ev == net_event_handler::removed;}
            );
            std::string cb_name_removed = "net_event_callback_removed";
            net_event_handler::register_callback(cb_name_removed, cb_removed);

            // Delete the module which was created in the previous part
            test_nl->delete_net(new_net);
            EXPECT_EQ(listener_removed.get_event_count(), 1);
            EXPECT_EQ(listener_removed.get_last_parameters(), std::make_tuple(net_event_handler::event::removed, new_net, NO_DATA));

            net_event_handler::unregister_callback(cb_name_removed);

        TEST_END
    }


} //namespace hal
