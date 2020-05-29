#include "netlist/persistent/netlist_serializer.h"
#include "netlist/gate_library/gate_library_manager.h"
#include "netlist/netlist.h"
#include "netlist/netlist_factory.h"
#include "netlist_test_utils.h"
#include "gtest/gtest.h"
#include <experimental/filesystem>
#include <core/log.h>
#include <core/utils.h>
#include <iostream>
#include <netlist/gate.h>
#include <netlist/net.h>
#include <netlist/module.h>

#include <fstream>
#include <streambuf>
#include <string>

using namespace test_utils;

class netlist_serializer_test : public ::testing::Test
{
protected:
    hal::path test_hal_file_path;

    virtual void SetUp()
    {
        NO_COUT_BLOCK;
        //gate_library_manager::load_all();
        test_hal_file_path = core_utils::get_binary_directory() / "tmp.hal";
    }

    virtual void TearDown()
    {
        fs::remove(test_hal_file_path);
    }
};

TEST_F(netlist_serializer_test, check_empty){
    EXPECT_TRUE(true);
}

// NOTE: Due to current work on the gate_library_manager, the serializer does not work. (gate_library name can't be used
//       in get_gate_library)
#ifdef DONT_BUILD
/**
 * Testing the serialization and a followed deserialization of the example
 * netlist.
 *
 * Functions: serialize_netlist, deserialize_netlist
 */
TEST_F(netlist_serializer_test, check_serialize_and_deserialize){
    TEST_START
        {
            // Serialize and deserialize the example netlist (with some additions) and compare the result with the original netlist
            std::shared_ptr<netlist> nl = create_example_netlist();

            // Add a module
            std::shared_ptr<module> test_m = nl->create_module(MIN_MODULE_ID+1, "test_type", nl->get_top_module());
            test_m->assign_gate(nl->get_gate_by_id(MIN_GATE_ID+1));
            test_m->assign_gate(nl->get_gate_by_id(MIN_GATE_ID+2));
            //test_m->assign_net(nl->get_net_by_id(MIN_GATE_ID+13));

            // Store some data in a gate, net and module
            nl->get_gate_by_id(MIN_GATE_ID+1)->set_data("category_0", "key_0", "data_type", "test_value");
            nl->get_gate_by_id(MIN_GATE_ID+1)->set_data("category_1", "key_1", "data_type", "test_value_1");
            nl->get_gate_by_id(MIN_GATE_ID+1)->set_data("category_1", "key_0", "data_type", "test_value_2");
            nl->get_net_by_id(MIN_NET_ID+13)->set_data("category", "key_2", "data_type", "test_value");
            test_m->set_data("category", "key_3", "data_type", "test_value");

            // Mark some gates as global gates
            nl->mark_gnd_gate(nl->get_gate_by_id(MIN_GATE_ID+1));
            nl->mark_vcc_gate(nl->get_gate_by_id(MIN_GATE_ID+2));

            // Mark some nets as global nets
            nl->mark_global_input_net(nl->get_net_by_id(MIN_NET_ID+13));
            nl->mark_global_output_net(nl->get_net_by_id(MIN_NET_ID+30));

            // Serialize and deserialize the netlist now

            bool suc                        = netlist_serializer::serialize_to_file(nl, test_hal_file_path);
            std::shared_ptr<netlist> des_nl = netlist_serializer::deserialize_from_file(test_hal_file_path);


            EXPECT_TRUE(suc);

            ASSERT_NE(des_nl, nullptr);

            // Check if the original netlist and the deserialized one (from file) are equal

            // -- Check if the id and the gate library is the same
            EXPECT_EQ(nl->get_id(), des_nl->get_id());
            EXPECT_EQ(nl->get_gate_library()->get_name(), des_nl->get_gate_library()->get_name());

            // -- Check if gates and nets are the same
            EXPECT_EQ(nl->get_gates().size(), des_nl->get_gates().size());
            for (auto g_0 : nl->get_gates())
            {
                EXPECT_TRUE(gates_are_equal(g_0, des_nl->get_gate_by_id(g_0->get_id())));
            }


            EXPECT_EQ(nl->get_nets().size(), des_nl->get_nets().size());

            for (auto n_0 : nl->get_nets())
            {
                EXPECT_TRUE(nets_are_equal(n_0, des_nl->get_net_by_id(n_0->get_id())));
            }

            // -- Check if global gates are the same
            EXPECT_EQ(nl->get_gnd_gates().size(), des_nl->get_gnd_gates().size());
            for (auto gl_gnd_0 : nl->get_gnd_gates())
            {
                EXPECT_TRUE(des_nl->is_gnd_gate(des_nl->get_gate_by_id(gl_gnd_0->get_id())));
            }

            EXPECT_EQ(nl->get_vcc_gates().size(), des_nl->get_vcc_gates().size());
            for (auto gl_vcc_0 : nl->get_vcc_gates())
            {
                EXPECT_TRUE(des_nl->is_vcc_gate(des_nl->get_gate_by_id(gl_vcc_0->get_id())));
            }

            // -- Check if global nets are the same
            EXPECT_EQ(nl->get_global_input_nets().size(), des_nl->get_global_input_nets().size());
            for (auto gl_in_net : nl->get_global_input_nets())
            {
                EXPECT_TRUE(des_nl->is_global_input_net(des_nl->get_net_by_id(gl_in_net->get_id())));
            }

            EXPECT_EQ(nl->get_global_output_nets().size(), des_nl->get_global_output_nets().size());
            for (auto gl_out_net : nl->get_global_output_nets())
            {
                EXPECT_TRUE(des_nl->is_global_output_net(des_nl->get_net_by_id(gl_out_net->get_id())));
            }

            // -- Check if the modules are the same
            EXPECT_EQ(nl->get_modules().size(), des_nl->get_modules().size());
            std::set<std::shared_ptr<module>> mods_1 = des_nl->get_modules();
            for(auto m_0 : nl->get_modules()){
                EXPECT_TRUE(modules_are_equal(m_0, des_nl->get_module_by_id(m_0->get_id())));
            }

        }
        /*{ NOTE: SIGABRT
            // Serialize and deserialize an empty netlist and compare the result with the original netlist
            std::shared_ptr<netlist> nl = create_empty_netlist();

            test_def::capture_stdout();
            bool suc                        = netlist_serializer::serialize_to_file(nl, test_hal_file_path);
            std::shared_ptr<netlist> des_nl = netlist_serializer::deserialize_from_file(test_hal_file_path);
            test_def::get_captured_stdout();

            EXPECT_TRUE(suc);
            //EXPECT_TRUE(netlists_are_equal(nl, des_nl));
        }*/



    TEST_END
}

/**
 * Testing the serialization and deserialization of a netlist with invalid input
 *
 * Functions: serialize_netlist, deserialize_netlist
 */
TEST_F(netlist_serializer_test, check_serialize_and_deserialize_negative)
{
    TEST_START
        /*{ ISSUE: Failed with SIGSEGV
                // Serialize a netlist which is a nullptr
                bool suc = netlist_serializer::serialize_netlist(nullptr, test_hal_file_path);
                EXPECT_FALSE(suc);
            }*/
        {
            // Serialize a netlist to an invalid path
            NO_COUT_TEST_BLOCK;
            std::shared_ptr<netlist> nl = create_example_netlist(0);
            bool suc                    = netlist_serializer::serialize_to_file(nl, hal::path(""));
            EXPECT_FALSE(suc);
        }
        {
            // Deserialize a netlist from a non existing path
            NO_COUT_TEST_BLOCK;
            std::shared_ptr<netlist> des_nl = netlist_serializer::deserialize_from_file(hal::path("/using/this/file/is/let.hal"));
            EXPECT_EQ(des_nl, nullptr);
        }
        {
            // Deserialize invalid input
            NO_COUT_TEST_BLOCK;
            std::shared_ptr<netlist> nl     = create_example_netlist(0);
            std::ofstream myfile;
            myfile.open (test_hal_file_path.string());
            myfile << "I h4ve no JSON f0rmat!!!\n(Temporary file for testing. Should be already deleted...)";
            myfile.close();
            std::shared_ptr<netlist> des_nl = netlist_serializer::deserialize_from_file(test_hal_file_path);
            EXPECT_EQ(des_nl, nullptr);
        }
    TEST_END
}
#endif //DONT_BUILD