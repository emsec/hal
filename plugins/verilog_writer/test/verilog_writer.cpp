#include "gtest/gtest.h"
#include "gate_library_test_utils.h"
#include "netlist_test_utils.h"
#include "verilog_parser/verilog_parser.h"
#include "verilog_writer/verilog_writer.h"
#include "hal_core/plugin_system/plugin_manager.h"
#include "hal_core/netlist/gate_library/gate_library_manager.h"

namespace hal {

    using test_utils::MIN_GATE_ID;
    using test_utils::MIN_NET_ID;

    class VerilogWriterTest : public ::testing::Test {
    protected:
        GateLibrary* m_gl;

        virtual void SetUp() 
        {
            NO_COUT_BLOCK;
            test_utils::init_log_channels();
            plugin_manager::load_all_plugins();
            test_utils::create_sandbox_directory();
            
            // gate library needs to be registered through gate_library_manager for serialization
            std::unique_ptr<GateLibrary> gl_tmp = test_utils::create_gate_library(test_utils::create_sandbox_path("testing_gate_library.hgl"));
            gate_library_manager::save(gl_tmp->get_path(), gl_tmp.get(), true);
            m_gl = gate_library_manager::load(gl_tmp->get_path());
        }

        virtual void TearDown() 
        {
            NO_COUT_BLOCK;
            plugin_manager::unload_all_plugins();
            // test_utils::remove_sandbox_directory();
        }
    };

    /**
     * Test writing a given netlist to file and subsequently parse it using the VerilogParser.
     *
     * Functions: write
     */
    TEST_F(VerilogWriterTest, check_main) {
        TEST_START
            {
                std::filesystem::path path_netlist = test_utils::create_sandbox_path("test.v");
                std::unique_ptr<Netlist> nl = std::make_unique<Netlist>(m_gl);
                
                {    
                    nl->set_design_name("top_module");
                    Module* top_module = nl->get_top_module();
                    top_module->set_type("top_module");

                    Gate* gate_0 = nl->create_gate(m_gl->get_gate_type_by_name("AND2"), "gate_0");
                    Gate* gate_1 = nl->create_gate(m_gl->get_gate_type_by_name("GND"), "gate_1");
                    Gate* gate_2 = nl->create_gate(m_gl->get_gate_type_by_name("VCC"), "gate_2");
                    Gate* gate_3 = nl->create_gate(m_gl->get_gate_type_by_name("INV"), "gate_3");
                    Gate* gate_4 = nl->create_gate(m_gl->get_gate_type_by_name("INV"), "gate_4");
                    Gate* gate_5 = nl->create_gate(m_gl->get_gate_type_by_name("AND2"), "gate_5");
                    Gate* gate_6 = nl->create_gate(m_gl->get_gate_type_by_name("AND2"), "gate_6");
                    Gate* gate_7 = nl->create_gate(m_gl->get_gate_type_by_name("AND2"), "gate_7");

                    gate_1->mark_gnd_gate();
                    gate_2->mark_vcc_gate();

                    Net* net_1_3 = test_utils::connect(nl.get(), gate_1, "O", gate_3, "I");
                    Net* net_3_0 = test_utils::connect(nl.get(), gate_3, "O", gate_0, "I0");
                    Net* net_2_0 = test_utils::connect(nl.get(), gate_2, "O", gate_0, "I1");
                    Net* net_0_4_5 = test_utils::connect(nl.get(), gate_0, "O", gate_4, "I");
                    test_utils::connect(nl.get(), gate_0, "O", gate_5, "I0");
                    Net* net_6_7 = test_utils::connect(nl.get(), gate_6, "O", gate_7, "I0");

                    Net* net_4_out = nl->create_net(MIN_NET_ID + 400, "net_4_out");
                    net_4_out->add_source(gate_4, "O");
                    net_4_out->mark_global_output_net();
                    top_module->set_output_port_name(net_4_out, net_4_out->get_name());

                    Net* net_5_out = nl->create_net(MIN_NET_ID + 500, "net_5_out");
                    net_5_out->add_source(gate_5, "O");
                    net_5_out->mark_global_output_net();
                    top_module->set_output_port_name(net_5_out, net_5_out->get_name());

                    Net* net_7_out = nl->create_net(MIN_NET_ID + 700, "net_7_out");
                    net_7_out->add_source(gate_7, "O");
                    net_7_out->mark_global_output_net();
                    top_module->set_output_port_name(net_7_out, net_7_out->get_name());
                }

                VerilogWriter verilog_writer;
                ASSERT_TRUE(verilog_writer.write(nl.get(), path_netlist));

                VerilogParser verilog_parser;
                std::unique_ptr<Netlist> parsed_nl = verilog_parser.parse_and_instantiate(path_netlist, m_gl);
                ASSERT_NE(parsed_nl, nullptr);

                // prepare comparison
                parsed_nl->set_device_name(nl->get_device_name());

                // compare netlists
                EXPECT_TRUE(test_utils::netlists_are_equal(nl.get(), parsed_nl.get(), true));
            }
        TEST_END
    }

    /**
     * Test writing global input and output nets.
     *
     * Functions: write
     */
    TEST_F(VerilogWriterTest, check_global_nets) {
        TEST_START
            {
                std::filesystem::path path_netlist = test_utils::create_sandbox_path("test.v");
                std::unique_ptr<Netlist> nl = std::make_unique<Netlist>(m_gl);
                
                {   
                    Module* top_module = nl->get_top_module();

                    Gate* gate_0 = nl->create_gate(m_gl->get_gate_type_by_name("BUF"), "gate_0");
                    Net* global_in_0 = test_utils::connect_global_in(nl.get(), gate_0, "I", "global_in_0");
                    top_module->set_input_port_name(global_in_0, global_in_0->get_name());

                    Gate* gate_1 = nl->create_gate(m_gl->get_gate_type_by_name("BUF"), "gate_1");
                    Net* global_in_1 = test_utils::connect_global_in(nl.get(), gate_1, "I", "global_in_1");
                    top_module->set_input_port_name(global_in_1, global_in_1->get_name());

                    Gate* gate_2 = nl->create_gate(m_gl->get_gate_type_by_name("BUF"), "gate_2");
                    Net* global_out_0 = test_utils::connect_global_out(nl.get(), gate_2, "O", "global_out_0");
                    top_module->set_output_port_name(global_out_0, global_out_0->get_name());

                    Gate* gate_3 = nl->create_gate(m_gl->get_gate_type_by_name("BUF"), "gate_3");
                    Net* global_out_1 = test_utils::connect_global_out(nl.get(), gate_3, "O", "global_out_1");
                    top_module->set_output_port_name(global_out_1, global_out_1->get_name());
                }


                VerilogWriter verilog_writer;
                ASSERT_TRUE(verilog_writer.write(nl.get(), path_netlist));

                VerilogParser verilog_parser;
                std::unique_ptr<Netlist> parsed_nl = verilog_parser.parse_and_instantiate(path_netlist, m_gl);
                ASSERT_NE(parsed_nl, nullptr);

                EXPECT_EQ(parsed_nl->get_global_input_nets().size(), 2);
                EXPECT_EQ(parsed_nl->get_global_output_nets().size(), 2);

                std::vector<Net*> nets = parsed_nl->get_nets();
                ASSERT_EQ(nets.size(), 4);

                auto global_in_0_it = std::find_if(nets.begin(), nets.end(), [](const Net* net){ return net->get_name() == "global_in_0"; });
                ASSERT_NE(global_in_0_it, nets.end());
                EXPECT_TRUE((*global_in_0_it)->is_global_input_net());

                auto global_in_1_it = std::find_if(nets.begin(), nets.end(), [](const Net* net){ return net->get_name() == "global_in_1"; });
                ASSERT_NE(global_in_1_it, nets.end());
                EXPECT_TRUE((*global_in_1_it)->is_global_input_net());

                auto global_out_0_it = std::find_if(nets.begin(), nets.end(), [](const Net* net){ return net->get_name() == "global_out_0"; });
                ASSERT_NE(global_out_0_it, nets.end());
                EXPECT_TRUE((*global_out_0_it)->is_global_output_net());

                auto global_out_1_it = std::find_if(nets.begin(), nets.end(), [](const Net* net){ return net->get_name() == "global_out_1"; });
                ASSERT_NE(global_out_1_it, nets.end());
                EXPECT_TRUE((*global_out_1_it)->is_global_output_net());
            }
        TEST_END
    }

    /**
     * Test writing a modularized netlist.
     *
     * Functions: write
     */
    TEST_F(VerilogWriterTest, check_modules) {
        TEST_START
            {
                std::filesystem::path path_netlist = test_utils::create_sandbox_path("test.v");
                std::unique_ptr<Netlist> nl = std::make_unique<Netlist>(m_gl);
                
                {
                    Gate* buf_0 = nl->create_gate(m_gl->get_gate_type_by_name("BUF"), "buf_0");
                    Gate* buf_1 = nl->create_gate(m_gl->get_gate_type_by_name("BUF"), "buf_1");
                    Gate* buf_2 = nl->create_gate(m_gl->get_gate_type_by_name("BUF"), "buf_2");
                    Gate* buf_3 = nl->create_gate(m_gl->get_gate_type_by_name("BUF"), "buf_3");
                    Gate* inv_0 = nl->create_gate(m_gl->get_gate_type_by_name("INV"), "inv_0");
                    Gate* and2_0 = nl->create_gate(m_gl->get_gate_type_by_name("AND2"), "and2_0");
                    Gate* and2_1 = nl->create_gate(m_gl->get_gate_type_by_name("AND2"), "and2_1");
                    Gate* or2_0 = nl->create_gate(m_gl->get_gate_type_by_name("OR2"), "or2_0");
                    
                    Net* net_0 = test_utils::connect(nl.get(), buf_0, "O", and2_0, "I0", "net_0");
                    Net* net_1 = test_utils::connect(nl.get(), buf_1, "O", and2_0, "I1", "net_1");
                    Net* net_2 = test_utils::connect(nl.get(), and2_0, "O", or2_0, "I0", "net_2");
                    test_utils::connect(nl.get(), and2_0, "O", and2_1, "I0", "net_3");
                    Net* net_3 = test_utils::connect(nl.get(), inv_0, "O", or2_0, "I1", "net_3");
                    test_utils::connect(nl.get(), inv_0, "O", and2_1, "I1", "net_5");
                    Net* net_4 = test_utils::connect(nl.get(), or2_0, "O", buf_2, "I", "net_4");
                    Net* net_5 = test_utils::connect(nl.get(), and2_1, "O", buf_3, "I", "net_5");

                    Net* net_6 = test_utils::connect_global_in(nl.get(), buf_0, "I", "net_6");
                    Net* net_7 = test_utils::connect_global_in(nl.get(), buf_1, "I", "net_7");
                    Net* net_8 = test_utils::connect_global_in(nl.get(), inv_0, "I", "net_8");

                    Net* net_9 = test_utils::connect_global_out(nl.get(), buf_2, "O", "net_9");
                    Net* net_10 = test_utils::connect_global_out(nl.get(), buf_3, "O", "net_10");

                    Module* mod_0 = nl->create_module("mod_0", nl->get_top_module(), {buf_0, buf_1, and2_0});
                    Module* mod_1 = nl->create_module("mod_1", nl->get_top_module());
                    Module* mod_2 = nl->create_module("mod_2", mod_1, {or2_0, buf_2});
                    Module* mod_3 = nl->create_module("mod_3", mod_1, {and2_1, buf_3});
                }

                VerilogWriter verilog_writer;
                ASSERT_TRUE(verilog_writer.write(nl.get(), path_netlist));

                VerilogParser verilog_parser;
                std::unique_ptr<Netlist> parsed_nl = verilog_parser.parse_and_instantiate(path_netlist, m_gl);
                ASSERT_NE(parsed_nl, nullptr);

                EXPECT_EQ(parsed_nl->get_nets().size(), 11);
                EXPECT_EQ(parsed_nl->get_global_input_nets().size(), 3);
                EXPECT_EQ(parsed_nl->get_global_output_nets().size(), 2);
                EXPECT_EQ(parsed_nl->get_gates().size(), 8);
                EXPECT_EQ(parsed_nl->get_modules().size(), 5);
            }
        TEST_END
    } 

    // /**
    //  * Testing the storage of generic data within gates
    //  *
    //  * IMPORTANT: If an error occurs, first run the HDLParserVerilog test to check, that
    //  * the issue isn't within the parser, but in the writer...
    //  *
    //  * Functions: write, parse
    //  */
    // TEST_F(VerilogWriterTest, check_generic_data_storage) {
    //     TEST_START
    //         {
    //             // Add a Gate to the netlist and store some data
    //             std::unique_ptr<Netlist> nl = test_utils::create_empty_netlist(0);

    //             std::filesystem::path path_netlist = test_utils::create_sandbox_path("test.v");

    //             Net* global_in = nl->create_net(MIN_NET_ID + 0, "global_in");
    //             nl->mark_global_input_net(global_in);

    //             Gate* test_gate_0 =
    //                 nl->create_gate(MIN_GATE_ID + 0, nl->get_gate_library()->get_gate_type_by_name("gate_1_to_1"), "test_gate_0");
    //             Gate* test_gate_1 =
    //                 nl->create_gate(MIN_GATE_ID + 1, nl->get_gate_library()->get_gate_type_by_name("gate_1_to_1"), "test_gate_1");
    //             Gate* test_gate_2 =
    //                 nl->create_gate(MIN_GATE_ID + 2, nl->get_gate_library()->get_gate_type_by_name("gate_1_to_1"), "test_gate_2");
    //             Gate* test_gate_3 =
    //                 nl->create_gate(MIN_GATE_ID + 3, nl->get_gate_library()->get_gate_type_by_name("gate_1_to_1"), "test_gate_3");
    //             Gate* test_gate_4 =
    //                 nl->create_gate(MIN_GATE_ID + 4, nl->get_gate_library()->get_gate_type_by_name("gate_1_to_1"), "test_gate_4");
    //             Gate* test_gate_5 =
    //                 nl->create_gate(MIN_GATE_ID + 5, nl->get_gate_library()->get_gate_type_by_name("gate_1_to_1"), "test_gate_5");
    //             Gate* test_gate_6 =
    //                 nl->create_gate(MIN_GATE_ID + 6, nl->get_gate_library()->get_gate_type_by_name("gate_1_to_1"), "test_gate_6");

    //             // Create output nets for all gates to create a valid netlist
    //             unsigned int idx = 0;
    //             for (auto g : nl->get_gates()) {
    //                 Net* out_net = nl->create_net("net_" + std::to_string(idx));
    //                 out_net->add_source(g, "O");
    //                 idx++;
    //             }

    //             global_in->add_destination(test_gate_0, "I");
    //             global_in->add_destination(test_gate_1, "I");
    //             global_in->add_destination(test_gate_2, "I");
    //             global_in->add_destination(test_gate_3, "I");
    //             global_in->add_destination(test_gate_4, "I");
    //             global_in->add_destination(test_gate_5, "I");
    //             global_in->add_destination(test_gate_6, "I");

    //             // Store some data in the test_gate
    //             test_gate_0->set_data("generic", "1_key_bit_vector", "bit_vector", "123ABC");
    //             test_gate_0->set_data("generic", "2_key_bit_vector", "bit_vector", "456DEF");

    //             test_gate_1->set_data("generic", "0_key_bit_vector", "bit_vector", "123ABC");
    //             test_gate_1->set_data("generic", "1_key_string", "string", "one_two_three");

    //             test_gate_2->set_data("generic", "0_key_string", "string", "one_two_three");
    //             test_gate_2->set_data("generic", "1_key_boolean", "boolean", "TRUE");

    //             test_gate_3->set_data("generic", "0_key_boolean", "boolean", "TRUE");
    //             test_gate_3->set_data("generic", "1_key_integer", "integer", "123");

    //             test_gate_4->set_data("generic", "0_key_integer", "integer", "123");

    //             test_gate_5->set_data("generic", "0_key_invalid", "invalid", "ignore_me"); // Should be ignored
    //             test_gate_5->set_data("generic", "1_key_bit_vector", "bit_value", "1");

    //             test_gate_6->set_data("generic", "0_key_bit_value", "bit_value", "10101100");
    //             test_gate_6->set_data("generic", "1_key_bit_value", "bit_value", "01010011");

    //             // Write and parse the netlist
    //             VerilogWriter verilog_writer;

    //             // Writes the netlist in the sstream
    //             bool writer_suc = verilog_writer.write(nl.get(), path_netlist);

    //             ASSERT_TRUE(writer_suc);

    //             VerilogParser verilog_parser;

    //             // Parse the .verilog file
    //             std::unique_ptr<Netlist> parsed_nl = verilog_parser.parse_and_instantiate(path_netlist, m_gl);

    //             ASSERT_NE(parsed_nl, nullptr);

    //             // -- Verilog Specific --
    //             // Since bit_vectors and bit_values are stored in the same format and both parsed as bit_vectors,
    //             // the data must be changed in the original netlist for comparison
    //             test_gate_6->set_data("generic", "0_key_bit_value", "bit_vector", "AC");
    //             test_gate_6->set_data("generic", "1_key_bit_value", "bit_vector", "53");

    //             // Since boolean values are written and parsed as integervalues (0 and 1),
    //             // the data must be changed in the original netlist for comparison
    //             test_gate_2->set_data("generic", "1_key_boolean", "integer", "1");
    //             test_gate_3->set_data("generic", "0_key_boolean", "integer", "1");

    //             // Check if the data is written/parsed correctly
    //             Gate* p_test_gate_0 = test_utils::get_gate_by_subname(parsed_nl.get(), "test_gate_0");
    //             ASSERT_NE(p_test_gate_0, nullptr);
    //             EXPECT_EQ(p_test_gate_0->get_data_map(), test_gate_0->get_data_map());

    //             Gate* p_test_gate_1 = test_utils::get_gate_by_subname(parsed_nl.get(), "test_gate_1");
    //             ASSERT_NE(p_test_gate_1, nullptr);
    //             EXPECT_EQ(p_test_gate_1->get_data_map(), test_gate_1->get_data_map());

    //             Gate* p_test_gate_2 = test_utils::get_gate_by_subname(parsed_nl.get(), "test_gate_2");
    //             ASSERT_NE(p_test_gate_2, nullptr);
    //             EXPECT_EQ(p_test_gate_2->get_data_map(), test_gate_2->get_data_map());

    //             Gate* p_test_gate_3 = test_utils::get_gate_by_subname(parsed_nl.get(), "test_gate_3");
    //             ASSERT_NE(p_test_gate_3, nullptr);
    //             EXPECT_EQ(p_test_gate_3->get_data_map(), test_gate_3->get_data_map());

    //             Gate* p_test_gate_4 = test_utils::get_gate_by_subname(parsed_nl.get(), "test_gate_4");
    //             ASSERT_NE(p_test_gate_4, nullptr);
    //             EXPECT_EQ(p_test_gate_4->get_data_map(), test_gate_4->get_data_map());

    //             Gate* p_test_gate_5 = test_utils::get_gate_by_subname(parsed_nl.get(), "test_gate_5");
    //             ASSERT_NE(p_test_gate_5, nullptr);
    //             auto test_gate_5_data_without_invalid = test_gate_5->get_data_map();
    //             test_gate_5_data_without_invalid.erase(std::make_tuple("generic", "0_key_invalid"));
    //             EXPECT_EQ(p_test_gate_5->get_data_map(), test_gate_5_data_without_invalid);

    //             Gate* p_test_gate_6 = test_utils::get_gate_by_subname(parsed_nl.get(), "test_gate_6");
    //             ASSERT_NE(p_test_gate_6, nullptr);
    //             EXPECT_EQ(p_test_gate_6->get_data_map(), test_gate_6->get_data_map());
    //         }
    //     TEST_END
    // }

    // /**
    //  * Testing the handling of Net names which contains only digits (i.e. 123 should become NET_123)
    //  *
    //  * IMPORTANT: If an error occurs, first run the HDLParserVerilog test to check, that
    //  * the issue isn't within the parser, but in the writer...
    //  *
    //  * Functions: write, parse
    //  */
    // TEST_F(VerilogWriterTest, check_digit_net_name) {
    //     TEST_START
    //         {
    //             // Add a Gate to the netlist and store some data
    //             std::unique_ptr<Netlist> nl = test_utils::create_empty_netlist(0);

    //             std::filesystem::path path_netlist = test_utils::create_sandbox_path("test.v");

    //             Net* global_in = nl->create_net(test_utils::MIN_NET_ID + 0, "123");
    //             nl->mark_global_input_net(global_in);

    //             // Write and parse the netlist
    //             VerilogWriter verilog_writer;

    //             // Writes the netlist in the sstream
    //             bool writer_suc = verilog_writer.write(nl.get(), path_netlist);

    //             ASSERT_TRUE(writer_suc);

    //             VerilogParser verilog_parser;

    //             // Parse the .verilog file
    //             std::unique_ptr<Netlist> parsed_nl = verilog_parser.parse_and_instantiate(path_netlist, m_gl);

    //             ASSERT_NE(parsed_nl, nullptr);

    //             // Check if the net_name is written/parsed correctly
    //             ASSERT_EQ(parsed_nl->get_nets().size(), (size_t) 1);
    //             Net* p_global_in = *parsed_nl->get_nets().begin();
    //             EXPECT_EQ(p_global_in->get_name(), "NET_123");

    //         }
    //     TEST_END
    // }

    // /**
    //  * Testing the handling of Net/Gate names with special characters and their translation.
    //  * Special characters: '(', ')', ',', ', ', '/', '\', '[', ']', '<', '>', '__', '_'
    //  * Other special cases: only digits, '_' at the beginning or at the end
    //  *
    //  * IMPORTANT: If an error occurs, first run the HDLParserVerilog test to check, that
    //  * the issue isn't within the parser, but in the writer...
    //  *
    //  * Functions: write, parse
    //  */
    // TEST_F(VerilogWriterTest, check_special_net_names) {
    //     TEST_START
    //         // {
    //         //     // Testing the handling of special Net names
    //         //     std::unique_ptr<Netlist> nl = test_utils::create_empty_netlist(0);

    //         //     Net* bracket_net = nl->create_net(test_utils::MIN_NET_ID + 0, "net(0)");
    //         //     Net* comma_net = nl->create_net(test_utils::MIN_NET_ID + 1, "net,1");
    //         //     Net* comma_space_net = nl->create_net(test_utils::MIN_NET_ID + 2, "net, 2");
    //         //     Net* slash_net = nl->create_net(test_utils::MIN_NET_ID + 3, "net/_3");
    //         //     Net* backslash_net = nl->create_net(test_utils::MIN_NET_ID + 4, "\\net\\_4");
    //         //     Net* curly_bracket_net = nl->create_net(test_utils::MIN_NET_ID + 5, "net[5]");
    //         //     Net* angle_bracket_net = nl->create_net(test_utils::MIN_NET_ID + 6, "net<6>");
    //         //     Net* double_underscore_net = nl->create_net(test_utils::MIN_NET_ID + 7, "net__7");
    //         //     Net* edges_underscore_net = nl->create_net(test_utils::MIN_NET_ID + 8, "_net_8_");
    //         //     Net*
    //         //         digit_only_net = nl->create_net(test_utils::MIN_NET_ID + 9, "9"); // should be converted to NET_9

    //         //     // Connect the nets to dummy gates (nets must be connected, else they are removed)
    //         //     Gate* dummy_gate_l_0 =
    //         //         nl->create_gate(MIN_GATE_ID + 0, test_utils::get_gate_type_by_name("gate_8_to_8"), "l_0");
    //         //     Gate* dummy_gate_l_1 =
    //         //         nl->create_gate(MIN_GATE_ID + 1, test_utils::get_gate_type_by_name("gate_8_to_8"), "l_1");
    //         //     Gate* dummy_gate_r_0 =
    //         //         nl->create_gate(MIN_GATE_ID + 2, test_utils::get_gate_type_by_name("gate_8_to_8"), "r_0");
    //         //     Gate* dummy_gate_r_1 =
    //         //         nl->create_gate(MIN_GATE_ID + 3, test_utils::get_gate_type_by_name("gate_8_to_8"), "r_1");

    //         //     std::vector<Net*> all_nets =
    //         //         {bracket_net, comma_net, comma_space_net, slash_net, backslash_net, curly_bracket_net,
    //         //          curly_bracket_net,
    //         //          angle_bracket_net, double_underscore_net, edges_underscore_net, digit_only_net};
    //         //     std::vector<Gate*> dummy_gates_l = {dummy_gate_l_0, dummy_gate_l_1};
    //         //     std::vector<Gate*> dummy_gates_r = {dummy_gate_r_0, dummy_gate_r_1};
    //         //     // Connect the nets in a loop
    //         //     for (size_t i = 0; i < all_nets.size(); i++) {
    //         //         all_nets[i]->add_source(dummy_gates_l[i / 8], "O" + std::to_string(i % 8));
    //         //         all_nets[i]->add_destination(dummy_gates_r[i / 8], "I" + std::to_string(i % 8));
    //         //     }

    //         //     // Write and parse the netlist
    //         //     std::stringstream parser_input;
    //         //     VerilogWriter verilog_writer;

    //         //     // Writes the netlist in the sstream
    //         //     bool writer_suc = verilog_writer.write(nl.get(), parser_input);

    //         //     ASSERT_TRUE(writer_suc);

    //         //     VerilogParser verilog_parser;
    //         //     // Parse the .verilog file
    //         //     auto verilog_file = test_utils::create_sandbox_file("netlist.v", parser_input.str());
    //         //     std::unique_ptr<Netlist> parsed_nl = verilog_parser.parse_and_instantiate(verilog_file, m_gl);

    //         //     ASSERT_NE(parsed_nl, nullptr);

    //         //     // Check if the net_name is translated correctly
    //         //     EXPECT_FALSE(parsed_nl->get_nets(test_utils::net_name_filter("net_0")).empty());
    //         //     EXPECT_FALSE(parsed_nl->get_nets(test_utils::net_name_filter("net_1")).empty());
    //         //     EXPECT_FALSE(parsed_nl->get_nets(test_utils::net_name_filter("net_2")).empty());
    //         //     EXPECT_FALSE(parsed_nl->get_nets(test_utils::net_name_filter("net_3")).empty());
    //         //     EXPECT_FALSE(parsed_nl->get_nets(test_utils::net_name_filter("net_4")).empty());
    //         //     EXPECT_FALSE(parsed_nl->get_nets(test_utils::net_name_filter("net_5")).empty());
    //         //     EXPECT_FALSE(parsed_nl->get_nets(test_utils::net_name_filter("net_6")).empty());
    //         //     EXPECT_FALSE(parsed_nl->get_nets(test_utils::net_name_filter("net_7")).empty());
    //         //     EXPECT_FALSE(parsed_nl->get_nets(test_utils::net_name_filter("net_8")).empty());
    //         //     EXPECT_FALSE(parsed_nl->get_nets(test_utils::net_name_filter("NET_9")).empty());
    //         // }
    //         {
    //             // Testing the handling of special Gate names
    //             std::unique_ptr<Netlist> nl = test_utils::create_empty_netlist(0);

    //             std::filesystem::path path_netlist = test_utils::create_sandbox_path("test.v");

    //             GateLibrary* gl = m_gl;

    //             // Create various gates with special Gate name characters
    //             Gate*
    //                 bracket_gate =
    //                 nl->create_gate(test_utils::MIN_GATE_ID + 0, gl->get_gate_types().at("gate_1_to_1"), "gate(0)");
    //             Gate*
    //                 comma_gate =
    //                 nl->create_gate(test_utils::MIN_GATE_ID + 1, gl->get_gate_types().at("gate_1_to_1"), "gate,1");
    //             Gate*
    //                 comma_space_gate =
    //                 nl->create_gate(test_utils::MIN_GATE_ID + 2, gl->get_gate_types().at("gate_1_to_1"), "gate, 2");
    //             Gate*
    //                 slash_gate =
    //                 nl->create_gate(test_utils::MIN_GATE_ID + 3, gl->get_gate_types().at("gate_1_to_1"), "gate/_3");
    //             Gate*
    //                 backslash_gate =
    //                 nl->create_gate(test_utils::MIN_GATE_ID + 4, gl->get_gate_types().at("gate_1_to_1"), "\\gate\\_4");
    //             Gate*
    //                 curly_bracket_gate =
    //                 nl->create_gate(test_utils::MIN_GATE_ID + 5, gl->get_gate_types().at("gate_1_to_1"), "gate[5]");
    //             Gate*
    //                 angle_bracket_gate =
    //                 nl->create_gate(test_utils::MIN_GATE_ID + 6, gl->get_gate_types().at("gate_1_to_1"), "gate<6>");
    //             Gate* double_underscore_gate =
    //                 nl->create_gate(test_utils::MIN_GATE_ID + 7, gl->get_gate_types().at("gate_1_to_1"), "gate__7");
    //             Gate* edges_underscore_gate =
    //                 nl->create_gate(test_utils::MIN_GATE_ID + 8, gl->get_gate_types().at("gate_1_to_1"), "_gate_8_");
    //             Gate* digit_only_gate = nl->create_gate(test_utils::MIN_GATE_ID + 9,
    //                                                     gl->get_gate_types().at("gate_1_to_1"),
    //                                                     "9"); // should be converted to GATE_9

    //             // Create output nets for all gates to create a valid netlist
    //             unsigned int idx = 0;
    //             for (auto g : nl->get_gates()) {
    //                 Net* out_net = nl->create_net("net_" + std::to_string(idx));
    //                 out_net->add_source(g, "O");
    //                 idx++;
    //             }

    //             // Write and parse the netlist
    //             VerilogWriter verilog_writer;

    //             // Writes the netlist in the sstream
    //             bool writer_suc = verilog_writer.write(nl.get(), path_netlist);

    //             ASSERT_TRUE(writer_suc);

    //             VerilogParser verilog_parser;
    //             // Parse the .verilog file
    //             std::unique_ptr<Netlist> parsed_nl = verilog_parser.parse_and_instantiate(path_netlist, m_gl);

    //             ASSERT_NE(parsed_nl, nullptr);

    //             EXPECT_FALSE(parsed_nl->get_gates(test_utils::gate_name_filter("gate_0" + m_gate_suffix)).empty());
    //             EXPECT_FALSE(parsed_nl->get_gates(test_utils::gate_name_filter("gate_1" + m_gate_suffix)).empty());
    //             EXPECT_FALSE(parsed_nl->get_gates(test_utils::gate_name_filter("gate_2" + m_gate_suffix)).empty());
    //             EXPECT_FALSE(parsed_nl->get_gates(test_utils::gate_name_filter("gate_3" + m_gate_suffix)).empty());
    //             EXPECT_FALSE(parsed_nl->get_gates(test_utils::gate_name_filter("gate_4" + m_gate_suffix)).empty());
    //             EXPECT_FALSE(parsed_nl->get_gates(test_utils::gate_name_filter("gate_5" + m_gate_suffix)).empty());
    //             EXPECT_FALSE(parsed_nl->get_gates(test_utils::gate_name_filter("gate_6" + m_gate_suffix)).empty());
    //             EXPECT_FALSE(parsed_nl->get_gates(test_utils::gate_name_filter("gate_7" + m_gate_suffix)).empty());
    //             EXPECT_FALSE(parsed_nl->get_gates(test_utils::gate_name_filter("gate_8" + m_gate_suffix)).empty());
    //             EXPECT_FALSE(parsed_nl->get_gates(test_utils::gate_name_filter("GATE_9" + m_gate_suffix)).empty());
    //         }
    //     TEST_END
    // }

    // /**
    //  * Testing the handling of collisions with Gate and Net names
    //  *
    //  * IMPORTANT: If an error occurs, first run the HDLParserVerilog test to check, that
    //  * the issue isn't within the parser, but in the writer...
    //  *
    //  * Functions: write, parse
    //  */
    // TEST_F(VerilogWriterTest, check_gate_net_name_collision) {
    //     TEST_START
    //         {
    //             // Testing the handling of two gates with the same name
    //             std::unique_ptr<Netlist> nl = test_utils::create_empty_netlist(0);

    //             std::filesystem::path path_netlist = test_utils::create_sandbox_path("test.v");

    //             Net* test_net = nl->create_net(test_utils::MIN_NET_ID + 0, "gate_net_name");
    //             Gate*
    //                 test_gate = nl->create_gate(test_utils::MIN_GATE_ID + 0,
    //                                             nl->get_gate_library()->get_gate_type_by_name("gate_1_to_1"),
    //                                             "gate_net_name");

    //             test_net->add_destination(test_gate, "I");

    //             // Write and parse the netlist now
    //             VerilogWriter verilog_writer;

    //             // Writes the netlist in the sstream
    //             bool writer_suc = verilog_writer.write(nl.get(), path_netlist);

    //             ASSERT_TRUE(writer_suc);

    //             VerilogParser verilog_parser;

    //             // Parse the .verilog file
    //             std::unique_ptr<Netlist> parsed_nl = verilog_parser.parse_and_instantiate(path_netlist, m_gl);

    //             ASSERT_NE(parsed_nl, nullptr);

    //             // Check if the Gate name was added a "_inst"
    //             EXPECT_NE(test_utils::get_net_by_subname(parsed_nl.get(), "gate_net_name"), nullptr);
    //             EXPECT_NE(test_utils::get_gate_by_subname(parsed_nl.get(), "gate_net_name_inst"), nullptr);

    //         }
    //     TEST_END
    // }

    // // /**
    // //  * Testing the correct handling of pin groups (e.g. I(0), I(1), I(2), I(3)).
    // //  *
    // //  * IMPORTANT: If an error occurs, first run the HDLParserVerilog test to check, that
    // //  * the issue isn't within the parser, but in the writer...
    // //  *
    // //  * Functions: write, parse
    // //  */
    // // TEST_F(VerilogWriterTest, check_pin_vector) {
    // //     TEST_START
    // //         {
    // //             /*                      .-------------------.
    // //              *    test_gnd_net ----=|I(0)               |
    // //              *    test_vcc_net ----=|I(1)  test_gate   O|=---- global_out
    // //              *    test_gnd_net ----=|I(2)               |
    // //              *    test_vcc_net ----=|I(3)               |
    // //              *                      '-------------------'
    // //              */

    // //             std::unique_ptr<Netlist> nl(new Netlist(m_gl));
    // //             nl->set_design_name("design_name");

    // //             Gate*
    // //                 gnd_gate = nl->create_gate(MIN_GATE_ID + 0, test_utils::get_gate_type_by_name("gnd"), "gnd_gate");
    // //             Gate*
    // //                 vcc_gate = nl->create_gate(MIN_GATE_ID + 1, test_utils::get_gate_type_by_name("vcc"), "vcc_gate");
    // //             Net* global_out = nl->create_net(MIN_NET_ID + 0, "global_out");
    // //             nl->mark_global_output_net(global_out);
    // //             Gate* test_gate = nl->create_gate(MIN_GATE_ID + 2,
    // //                                               test_utils::get_gate_type_by_name(
    // //                                                   "pin_group_gate_4_to_4"),
    // //                                               "test_gate");
    // //             global_out->add_source(test_gate, "O(0)");

    // //             Net* gnd_net = nl->create_net(MIN_NET_ID + 2, "test_gnd_net");
    // //             Net* vcc_net = nl->create_net(MIN_NET_ID + 3, "test_vcc_net");

    // //             gnd_net->add_source(gnd_gate, "O");
    // //             vcc_net->add_source(vcc_gate, "O");

    // //             gnd_net->add_destination(test_gate, "I(0)");
    // //             vcc_net->add_destination(test_gate, "I(1)");
    // //             gnd_net->add_destination(test_gate, "I(2)");
    // //             vcc_net->add_destination(test_gate, "I(3)");


    // //             // Write and parse the netlist
    // //             std::stringstream parser_input;
    // //             VerilogWriter verilog_writer;

    // //             // Writes the netlist in the sstream
    // //             bool writer_suc = verilog_writer.write(nl.get(), parser_input);

    // //             ASSERT_TRUE(writer_suc);

    // //             VerilogParser verilog_parser;

    // //             // Parse the .verilog file
    // //             auto verilog_file = test_utils::create_sandbox_file("netlist.v", parser_input.str());
    // //             std::unique_ptr<Netlist> parsed_nl = verilog_parser.parse_and_instantiate(verilog_file, m_gl);

    // //             ASSERT_NE(parsed_nl, nullptr);

    // //             ASSERT_EQ(parsed_nl->get_nets(test_utils::net_name_filter("test_gnd_net")).size(), 1);
    // //             ASSERT_EQ(parsed_nl->get_nets(test_utils::net_name_filter("test_vcc_net")).size(), 1);
    // //             Net*
    // //                 test_gnd_net_ref = *parsed_nl->get_nets(test_utils::net_name_filter("test_gnd_net")).begin();
    // //             Net*
    // //                 test_vcc_net_ref = *parsed_nl->get_nets(test_utils::net_name_filter("test_vcc_net")).begin();

    // //             ASSERT_EQ(parsed_nl->get_gates(test_utils::gate_type_filter("pin_group_gate_4_to_4")).size(), 1);
    // //             Gate* test_gate_ref =
    // //                 *parsed_nl->get_gates(test_utils::gate_type_filter("pin_group_gate_4_to_4")).begin();
    // //             //EXPECT_EQ(test_gate_ref->get_fan_in_net("I(0)"), test_gnd_net_ref);
    // //             //EXPECT_EQ(test_gate_ref->get_fan_in_net("I(1)"), test_vcc_net_ref);
    // //             //EXPECT_EQ(test_gate_ref->get_fan_in_net("I(2)"), test_gnd_net_ref);
    // //             //EXPECT_EQ(test_gate_ref->get_fan_in_net("I(3)"), test_vcc_net_ref); // ISSUE: wrong order (verilog writer: l.557 useless?)

    // //         }
    // //     TEST_END
    // // }
} //namespace hal
