#include "gtest/gtest.h"
#include "netlist_test_utils.h"
#include <iostream>
#include <netlist/gate.h>
#include <netlist/net.h>
#include "netlist/gate_library/gate_library_manager.h"
#include "netlist/netlist_factory.h"
#include "netlist/netlist.h"
#include "netlist/hdl_parser/hdl_parser_vhdl.h"
#include "netlist/hdl_writer/hdl_writer_vhdl.h"

namespace hal {
    using test_utils::MIN_GATE_ID;
    using test_utils::MIN_NET_ID;


    class HDLWriterVHDLTest : public ::testing::Test {
    protected:

        const std::string m_gate_suffix = "";
        std::shared_ptr<GateLibrary> m_gl;

        virtual void SetUp() {
            NO_COUT_BLOCK;
            test_utils::init_log_channels();
            m_gl = test_utils::get_testing_gate_library();
        }

        virtual void TearDown() {

        }
    };

    /**
     * Testing to write a given netlist in a sstream and parses it after, with
     * the HDLParserVHDL.
     * IMPORTANT: If an error occurs, first run the hdl_parser_vhdl_old test to check, that
     * the issue isn't within the parser, but in the writer...
     *
     * Functions: write, parse
     */
    TEST_F(HDLWriterVHDLTest, check_write_and_parse_main_example) {
        TEST_START
            { //NOTE: Fails because of an issue in the parser (empty 'port map' can't be parsed)
                // Write and parse the example netlist (with some additions) and compare the result with the original netlist
                std::shared_ptr<Netlist> nl = test_utils::create_example_parse_netlist(0);

                // Mark the global gates as such
                nl->mark_gnd_gate(nl->get_gate_by_id(MIN_GATE_ID + 1));
                nl->mark_vcc_gate(nl->get_gate_by_id(MIN_GATE_ID + 2));

                // Write and parse the netlist now
                test_def::capture_stdout();
                std::stringstream parser_input;
                HDLWriterVHDL vhdl_writer(parser_input);

                // Writes the netlist in the sstream
                bool writer_suc = vhdl_writer.write(nl);

                if (!writer_suc) {
                    std::cout << test_def::get_captured_stdout() << std::endl;
                }
                ASSERT_TRUE(writer_suc);

                HDLParserVHDL vhdl_parser(parser_input);

                // Parse the .vhdl file
                std::shared_ptr<Netlist> parsed_nl = vhdl_parser.parse_and_instantiate(m_gl);

                if (parsed_nl == nullptr) {
                    std::cout << test_def::get_captured_stdout() << std::endl;
                }
                ASSERT_NE(parsed_nl, nullptr);
                test_def::get_captured_stdout();

                // Check if the original netlist and the parsed one are equal

                // FIXME
                // -- Check if gates and nets are the same
                EXPECT_EQ(nl->get_gates().size(), parsed_nl->get_gates().size());
                for(auto g_0 : nl->get_gates()){
                    EXPECT_TRUE(test_utils::gates_are_equal(g_0, test_utils::get_gate_by_subname(parsed_nl, g_0->get_name()),true, true));
                }

                EXPECT_EQ(nl->get_nets().size(), parsed_nl->get_nets().size());

                for(auto n_0 : nl->get_nets()){
                    EXPECT_TRUE(test_utils::nets_are_equal(n_0, test_utils::get_net_by_subname(parsed_nl, n_0->get_name()), true, true));
                }

                // -- Check if global gates are the same
                EXPECT_EQ(nl->get_gnd_gates().size(), parsed_nl->get_gnd_gates().size());
                for(auto gl_gnd_0 : nl->get_gnd_gates()){
                    EXPECT_TRUE(parsed_nl->is_gnd_gate(test_utils::get_gate_by_subname(parsed_nl, gl_gnd_0->get_name())));
                }

                EXPECT_EQ(nl->get_vcc_gates().size(), parsed_nl->get_vcc_gates().size());
                for(auto gl_vcc_0 : nl->get_vcc_gates()){
                    EXPECT_TRUE(parsed_nl->is_vcc_gate(test_utils::get_gate_by_subname(parsed_nl, gl_vcc_0->get_name())));
                }//
            }
        TEST_END
    }

    /**
     * Testing the writing of global input/output nets
     *
     * IMPORTANT: If an error occurs, first run the hdl_parser_vhdl_old test to check, that
     * the issue isn't within the parser, but in the writer...
     *
     * Functions: write, parse
     */
    TEST_F(HDLWriterVHDLTest, check_global_nets) {
        TEST_START
            {
                // Add 2 global input nets to an empty netlist
                std::shared_ptr<Netlist> nl = test_utils::create_empty_netlist(0);

                std::shared_ptr<Net> global_in_0 = nl->create_net(MIN_NET_ID + 0, "0_global_in");
                std::shared_ptr<Net> global_in_1 = nl->create_net(MIN_NET_ID + 1, "1_global_in");

                nl->mark_global_input_net(global_in_0);
                nl->mark_global_input_net(global_in_1);

                // Write and parse the netlist now
                test_def::capture_stdout();
                std::stringstream parser_input;
                HDLWriterVHDL vhdl_writer(parser_input);

                // Writes the netlist in the sstream
                bool writer_suc = vhdl_writer.write(nl);
                if (!writer_suc) {
                    std::cout << test_def::get_captured_stdout() << std::endl;
                }
                ASSERT_TRUE(writer_suc);

                HDLParserVHDL vhdl_parser(parser_input);
                // Parse the .vhdl file
                std::shared_ptr<Netlist> parsed_nl = vhdl_parser.parse_and_instantiate(m_gl);

                if (parsed_nl == nullptr) {
                    std::cout << test_def::get_captured_stdout() << std::endl;
                }
                ASSERT_NE(parsed_nl, nullptr);
                test_def::get_captured_stdout();

                // Check if the nets are written/parsed correctly
                std::shared_ptr<Net> p_global_in_0 = test_utils::get_net_by_subname(parsed_nl, "0_global_in");
                ASSERT_NE(p_global_in_0, nullptr);
                EXPECT_TRUE(parsed_nl->is_global_input_net(p_global_in_0));

                std::shared_ptr<Net> p_global_in_1 = test_utils::get_net_by_subname(parsed_nl, "1_global_in");
                ASSERT_NE(p_global_in_1, nullptr);
                EXPECT_TRUE(parsed_nl->is_global_input_net(p_global_in_1));

            }
            {
                // Add 2 global output nets to an empty netlist
                std::shared_ptr<Netlist> nl = test_utils::create_empty_netlist(0);

                std::shared_ptr<Net> global_out_0 = nl->create_net(MIN_NET_ID + 0, "0_global_out");
                std::shared_ptr<Net> global_out_1 = nl->create_net(MIN_NET_ID + 1, "1_global_out");

                nl->mark_global_output_net(global_out_0);
                nl->mark_global_output_net(global_out_1);

                // Write and parse the netlist now
                test_def::capture_stdout();
                std::stringstream parser_input;
                HDLWriterVHDL vhdl_writer(parser_input);

                // Writes the netlist in the sstream
                bool writer_suc = vhdl_writer.write(nl);
                if (!writer_suc) {
                    std::cout << test_def::get_captured_stdout() << std::endl;
                }
                ASSERT_TRUE(writer_suc);

                HDLParserVHDL vhdl_parser(parser_input);
                // Parse the .vhdl file
                std::shared_ptr<Netlist> parsed_nl = vhdl_parser.parse_and_instantiate(m_gl);

                if (parsed_nl == nullptr) {
                    std::cout << test_def::get_captured_stdout() << std::endl;
                }
                ASSERT_NE(parsed_nl, nullptr);
                test_def::get_captured_stdout();

                // Check if the nets are written/parsed correctly
                std::shared_ptr<Net> p_global_out_0 = test_utils::get_net_by_subname(parsed_nl, "0_global_out");
                ASSERT_NE(p_global_out_0, nullptr);
                EXPECT_TRUE(parsed_nl->is_global_output_net(p_global_out_0));

                std::shared_ptr<Net> p_global_out_1 = test_utils::get_net_by_subname(parsed_nl, "1_global_out");
                ASSERT_NE(p_global_out_1, nullptr);
                EXPECT_TRUE(parsed_nl->is_global_output_net(p_global_out_1));

            }
        TEST_END
    }

    /** NOTE: DISABLED (Terminated)
     * Testing the storage of generic data within gates
     *
     * IMPORTANT: If an error occurs, first run the HDLParserVHDL test to check, that
     * the issue isn't within the parser, but in the writer...
     *
     * Functions: write, parse
     */
    TEST_F(HDLWriterVHDLTest, check_generic_data_storage) {
        TEST_START
            { //NOTE: Need to assure that the nl is valid. by passing output nets
                // Add a Gate to the netlist and store some data
                std::shared_ptr<Netlist> nl = test_utils::create_empty_netlist(0);

                std::shared_ptr<Net> global_in = nl->create_net(MIN_NET_ID + 0, "global_in");
                nl->mark_global_input_net(global_in);

                std::shared_ptr<Gate>
                    test_gate_0 = nl->create_gate(MIN_GATE_ID + 0, test_utils::get_gate_type_by_name("gate_1_to_1"), "test_gate_0");
                std::shared_ptr<Gate>
                    test_gate_1 = nl->create_gate(MIN_GATE_ID + 1, test_utils::get_gate_type_by_name("gate_1_to_1"), "test_gate_1");
                std::shared_ptr<Gate>
                    test_gate_2 = nl->create_gate(MIN_GATE_ID + 2, test_utils::get_gate_type_by_name("gate_1_to_1"), "test_gate_2");
                std::shared_ptr<Gate>
                    test_gate_3 = nl->create_gate(MIN_GATE_ID + 3, test_utils::get_gate_type_by_name("gate_1_to_1"), "test_gate_3");
                std::shared_ptr<Gate>
                    test_gate_4 = nl->create_gate(MIN_GATE_ID + 4, test_utils::get_gate_type_by_name("gate_1_to_1"), "test_gate_4");
                std::shared_ptr<Gate>
                    test_gate_5 = nl->create_gate(MIN_GATE_ID + 5, test_utils::get_gate_type_by_name("gate_1_to_1"), "test_gate_5");
                std::shared_ptr<Gate>
                    test_gate_6 = nl->create_gate(MIN_GATE_ID + 6, test_utils::get_gate_type_by_name("gate_1_to_1"), "test_gate_6");

                // Create output nets for all gates to create a valid netlist
                unsigned int idx = 0;
                for (auto g : nl->get_gates()) {
                    std::shared_ptr<Net> out_net = nl->create_net("net_" + std::to_string(idx));
                    out_net->add_source(g, "O");
                    idx++;
                }

                global_in->add_destination(test_gate_0, "I");
                global_in->add_destination(test_gate_1, "I");
                global_in->add_destination(test_gate_2, "I");
                global_in->add_destination(test_gate_3, "I");
                global_in->add_destination(test_gate_4, "I");
                global_in->add_destination(test_gate_5, "I");
                global_in->add_destination(test_gate_6, "I");

                // Store some data in the test_gate
                test_gate_0->set_data("generic", "0_key_time", "time", "123s");
                test_gate_0->set_data("generic", "1_key_bit_vector", "bit_vector", "123abc");
                test_gate_0->set_data("generic", "2_key_bit_vector", "bit_vector", "456def");

                test_gate_1->set_data("generic", "0_key_bit_vector", "bit_vector", "123abc");
                test_gate_1->set_data("generic", "1_key_string", "string", "one_two_three");

                test_gate_2->set_data("generic", "0_key_string", "string", "one_two_three");
                test_gate_2->set_data("generic", "1_key_boolean", "boolean", "true");

                test_gate_3->set_data("generic", "0_key_boolean", "boolean", "true");
                test_gate_3->set_data("generic", "1_key_integer", "integer", "123");

                test_gate_4->set_data("generic", "0_key_integer", "integer", "123");
                test_gate_4->set_data("generic", "1_key_time", "time", "123s");

                test_gate_5->set_data("generic", "0_key_invalid", "invalid", "ignore_me"); // Should be ignored
                test_gate_5->set_data("generic", "1_key_bit_vector", "bit_vector", "1");

                //test_gate_6->set_data("generic", "0_key_bit_value", "bit_value", "10101100");
                //test_gate_6->set_data("generic", "1_key_bit_value", "bit_value", "01010011");

                // Write and parse the netlist now
                test_def::capture_stdout();
                std::stringstream parser_input;
                HDLWriterVHDL vhdl_writer(parser_input);

                // Writes the netlist in the sstream
                bool writer_suc = vhdl_writer.write(nl);
                if (!writer_suc) {
                    std::cout << test_def::get_captured_stdout() << std::endl;
                }
                ASSERT_TRUE(writer_suc);

                HDLParserVHDL vhdl_parser(parser_input);

                // Parse the .vhdl file
                std::shared_ptr<Netlist> parsed_nl = vhdl_parser.parse_and_instantiate(m_gl);

                if (parsed_nl == nullptr) {
                    std::cout << test_def::get_captured_stdout() << std::endl;
                }
                ASSERT_NE(parsed_nl, nullptr);
                test_def::get_captured_stdout();

                // Check if the data is written/parsed correctly
                std::shared_ptr<Gate> p_test_gate_0 = test_utils::get_gate_by_subname(parsed_nl, "test_gate_0");
                ASSERT_NE(p_test_gate_0, nullptr);
                // EXPECT_EQ(p_test_gate_0->get_data(), test_gate_0->get_data()); //ISSUE: generic, time can't be used

                std::shared_ptr<Gate> p_test_gate_1 = test_utils::get_gate_by_subname(parsed_nl, "test_gate_1");
                ASSERT_NE(p_test_gate_1, nullptr);
                EXPECT_EQ(p_test_gate_1->get_data(), test_gate_1->get_data());

                std::shared_ptr<Gate> p_test_gate_2 = test_utils::get_gate_by_subname(parsed_nl, "test_gate_2");
                ASSERT_NE(p_test_gate_2, nullptr);
                EXPECT_EQ(p_test_gate_2->get_data(), test_gate_2->get_data());

                std::shared_ptr<Gate> p_test_gate_3 = test_utils::get_gate_by_subname(parsed_nl, "test_gate_3");
                ASSERT_NE(p_test_gate_3, nullptr);
                EXPECT_EQ(p_test_gate_3->get_data(), test_gate_3->get_data());

                std::shared_ptr<Gate> p_test_gate_4 = test_utils::get_gate_by_subname(parsed_nl, "test_gate_4");
                ASSERT_NE(p_test_gate_4, nullptr);
                // EXPECT_EQ(p_test_gate_4->get_data(), test_gate_4->get_data()); //ISSUE: generic, time can't be used

                std::shared_ptr<Gate> p_test_gate_5 = test_utils::get_gate_by_subname(parsed_nl, "test_gate_5");
                ASSERT_NE(p_test_gate_5, nullptr);
                auto test_gate_5_data_without_invalid = test_gate_5->get_data();
                test_gate_5_data_without_invalid.erase(std::make_tuple("generic", "0_key_invalid"));
                EXPECT_EQ(p_test_gate_5->get_data(), test_gate_5_data_without_invalid);

                //std::shared_ptr<Gate> p_test_gate_6 = test_utils::get_gate_by_subname(parsed_nl, "test_gate_6");
                //ASSERT_NE(p_test_gate_6, nullptr);
                //EXPECT_EQ(p_test_gate_6->get_data(), test_gate_6->get_data());
            }
        TEST_END
    }

    /**
     * Testing the handling of Net names which contains only digits (i.e. 123 should become NET_123)
     *
     * IMPORTANT: If an error occurs, first run the HDLParserVHDL test to check, that
     * the issue isn't within the parser, but in the writer...
     *
     * Functions: write, parse
     */
    TEST_F(HDLWriterVHDLTest, check_digit_net_name) {
        TEST_START
            {
                // Add a Gate to the netlist and store some data
                std::shared_ptr<Netlist> nl = test_utils::create_empty_netlist(0);

                std::shared_ptr<Net> global_in = nl->create_net(MIN_NET_ID + 0, "123");
                nl->mark_global_input_net(global_in);

                // Write and parse the netlist now
                test_def::capture_stdout();
                std::stringstream parser_input;
                HDLWriterVHDL vhdl_writer(parser_input);

                // Writes the netlist in the sstream
                bool writer_suc = vhdl_writer.write(nl);
                if (!writer_suc) {
                    std::cout << test_def::get_captured_stdout() << std::endl;
                }
                ASSERT_TRUE(writer_suc);

                HDLParserVHDL vhdl_parser(parser_input);

                // Parse the .vhdl file
                std::shared_ptr<Netlist> parsed_nl = vhdl_parser.parse_and_instantiate(m_gl);

                if (parsed_nl == nullptr) {
                    std::cout << test_def::get_captured_stdout() << std::endl;
                }
                ASSERT_NE(parsed_nl, nullptr);
                test_def::get_captured_stdout();

                // Check if the net_name is written/parsed correctly
                ASSERT_EQ(parsed_nl->get_nets().size(), (size_t) 1);
                std::shared_ptr<Net> p_global_in = *parsed_nl->get_nets().begin();
                EXPECT_EQ(p_global_in->get_name(), "NET_123");

            }
        TEST_END
    }

    /**
     * Testing the handling of vcc and gnd gates (ONE and ZERO)
     *
     * IMPORTANT: If an error occurs, first run the HDLParserVHDL test to check, that
     * the issue isn't within the parser, but in the writer...
     *
     * Functions: write, parse
     */
    TEST_F(HDLWriterVHDLTest, check_vcc_and_gnd_gates) {
        TEST_START
            {
                // Two nets with individual names connect a test_gate with a gnd/vcc Gate
                std::shared_ptr<Netlist> nl = test_utils::create_empty_netlist(0);

                // Add the gates
                std::shared_ptr<Gate> gnd_gate = nl->create_gate( MIN_GATE_ID+0, test_utils::get_gate_type_by_name("gnd"), "gnd_gate");
                std::shared_ptr<Gate> vcc_gate = nl->create_gate( MIN_GATE_ID+1, test_utils::get_gate_type_by_name("vcc"), "vcc_gate");
                std::shared_ptr<Gate> test_gate = nl->create_gate( MIN_GATE_ID+2, test_utils::get_gate_type_by_name("gate_2_to_1"), "test_gate");

                // Add and connect the nets
                std::shared_ptr<Net> zero_net = nl->create_net(MIN_GATE_ID+0,"zero_net");
                std::shared_ptr<Net> one_net = nl->create_net(MIN_GATE_ID+1,"one_net");

                zero_net->add_source(gnd_gate, "O");
                zero_net->add_destination(test_gate, "I0");

                one_net->add_source(vcc_gate, "O");
                one_net->add_destination(test_gate, "I1");

                // Write and parse the netlist now
                test_def::capture_stdout();
                std::stringstream parser_input;
                HDLWriterVHDL vhdl_writer(parser_input);

                // Writes the netlist in the sstream
                bool writer_suc = vhdl_writer.write(nl);
                if (!writer_suc) {
                    std::cout << test_def::get_captured_stdout() << std::endl;
                }
                ASSERT_TRUE(writer_suc);

                HDLParserVHDL vhdl_parser(parser_input);
                // Parse the .vhdl file
                std::shared_ptr<Netlist> parsed_nl = vhdl_parser.parse_and_instantiate(m_gl);

                if (parsed_nl == nullptr) {
                    std::cout << test_def::get_captured_stdout() << std::endl;
                }
                ASSERT_NE(parsed_nl, nullptr);
                test_def::get_captured_stdout();

                // Check if the GND/VCC gates and nets are written/parsed correctly
                std::shared_ptr<Gate> p_vcc_gate = test_utils::get_gate_by_subname(parsed_nl,  "vcc_gate");
                std::shared_ptr<Gate> p_gnd_gate = test_utils::get_gate_by_subname(parsed_nl,  "gnd_gate");
                std::shared_ptr<Gate> p_test_gate = test_utils::get_gate_by_subname(parsed_nl,  "test_gate");

                ASSERT_NE(vcc_gate, nullptr);
                ASSERT_NE(gnd_gate, nullptr);
                ASSERT_NE(test_gate, nullptr);

                ASSERT_EQ(parsed_nl->get_nets().size(), (size_t)2);
                std::shared_ptr<Net> p_zero_net = test_utils::get_net_by_subname(parsed_nl,  "zero_net");
                std::shared_ptr<Net> p_one_net = test_utils::get_net_by_subname(parsed_nl,  "one_net");
                ASSERT_NE(zero_net, nullptr);
                ASSERT_NE(one_net, nullptr);

                EXPECT_EQ(p_zero_net->get_source(), test_utils::get_endpoint(p_gnd_gate, "O"));
                EXPECT_EQ(p_one_net->get_source(), test_utils::get_endpoint(p_vcc_gate, "O"));

                EXPECT_TRUE(p_zero_net->is_a_destination(test_utils::get_endpoint(p_test_gate, "I0")));
                EXPECT_TRUE(p_one_net->is_a_destination(test_utils::get_endpoint(p_test_gate, "I1")));

            }
            {
                // Two nets ('0'/'1')  are connected with a test_gate. For these net names, a global_gnd/global_vcc gate
                // should be created.
                std::shared_ptr<Netlist> nl = test_utils::create_empty_netlist(0);

                // Add the gate
                std::shared_ptr<Gate> test_gate = nl->create_gate( MIN_GATE_ID+2, test_utils::get_gate_type_by_name("gate_2_to_1"), "test_gate");

                // Add and connect the nets
                std::shared_ptr<Net> zero_net = nl->create_net(MIN_NET_ID+0,"'0'");
                std::shared_ptr<Net> one_net = nl->create_net(MIN_NET_ID+1,"'1'");

                //zero_net->add_source(gnd_gate, "O");
                zero_net->add_destination(test_gate, "I0");

                //one_net->add_source(vcc_gate, "O");
                one_net->add_destination(test_gate, "I1");

                // Write and parse the netlist now
                std::stringstream parser_input;
                HDLWriterVHDL vhdl_writer(parser_input);

                // Writes the netlist in the sstream
                bool writer_suc = vhdl_writer.write(nl);
                ASSERT_TRUE(writer_suc);

                HDLParserVHDL vhdl_parser(parser_input);
                // Parse the .vhdl file
                std::shared_ptr<Netlist> parsed_nl = vhdl_parser.parse_and_instantiate(m_gl);

                ASSERT_NE(parsed_nl, nullptr);
                std::shared_ptr<Gate> global_gnd = test_utils::get_gate_by_subname(parsed_nl, "global_gnd");
                std::shared_ptr<Gate> global_vcc = test_utils::get_gate_by_subname(parsed_nl, "global_vcc");
                ASSERT_NE(global_gnd, nullptr);
                ASSERT_NE(global_vcc, nullptr);
                ASSERT_NE(global_gnd->get_fan_out_net("O"), nullptr);
                ASSERT_NE(global_vcc->get_fan_out_net("O"), nullptr);
                EXPECT_EQ(global_gnd->get_fan_out_net("O")->get_name(), "'0'");
                EXPECT_EQ(global_vcc->get_fan_out_net("O")->get_name(), "'1'");
                EXPECT_FALSE(global_gnd->get_fan_out_net("O")->get_destinations(test_utils::endpoint_pin_type_filter("I0")).empty());
                EXPECT_FALSE(global_vcc->get_fan_out_net("O")->get_destinations(test_utils::endpoint_pin_type_filter("I1")).empty());
            }
        TEST_END
    }

    /**
     * Testing the handling of Net/Gate names with special characters and their translation.
     * Special characters: '(', ')', ',', ', ', '/', '\', '[', ']', '<', '>', '__', '_'
     * Other special cases: only digits, '_' at the beginning or at the end
     *
     * IMPORTANT: If an error occurs, first run the HDLParserVHDL test to check, that
     * the issue isn't within the parser, but in the writer...
     *
     * Functions: write, parse
     */
    TEST_F(HDLWriterVHDLTest, check_special_net_names) {
        TEST_START
            {
                // Testing the handling of special Net names
                std::shared_ptr<Netlist> nl = test_utils::create_empty_netlist(0);

                std::shared_ptr<Net> bracket_net = nl->create_net(MIN_NET_ID + 0, "net(0)");
                std::shared_ptr<Net> comma_net = nl->create_net(MIN_NET_ID + 1, "net,1");
                std::shared_ptr<Net> comma_space_net = nl->create_net(MIN_NET_ID + 2, "net, 2");
                std::shared_ptr<Net> slash_net = nl->create_net(MIN_NET_ID + 3, "net/_3");
                std::shared_ptr<Net> backslash_net = nl->create_net(MIN_NET_ID + 4, "net\\_4");
                std::shared_ptr<Net> curly_bracket_net = nl->create_net(MIN_NET_ID + 5, "net[5]");
                std::shared_ptr<Net> angle_bracket_net = nl->create_net(MIN_NET_ID + 6, "net<6>");
                std::shared_ptr<Net> double_underscore_net = nl->create_net(MIN_NET_ID + 7, "net__7");
                std::shared_ptr<Net> edges_underscore_net = nl->create_net(MIN_NET_ID + 8, "_net_8_");
                std::shared_ptr<Net>
                    digit_only_net = nl->create_net(MIN_NET_ID + 9, "9"); // should be converted to NET_9

                // Connect the nets to dummy gates (nets must be connected, else they are removed)
                std::shared_ptr<Gate> dummy_gate_l_0 = nl->create_gate(MIN_GATE_ID+0, test_utils::get_gate_type_by_name("gate_8_to_8"), "l_0");
                std::shared_ptr<Gate> dummy_gate_l_1 = nl->create_gate(MIN_GATE_ID+1, test_utils::get_gate_type_by_name("gate_8_to_8"), "l_1");
                std::shared_ptr<Gate> dummy_gate_r_0 = nl->create_gate(MIN_GATE_ID+2, test_utils::get_gate_type_by_name("gate_8_to_8"), "r_0");
                std::shared_ptr<Gate> dummy_gate_r_1 = nl->create_gate(MIN_GATE_ID+3, test_utils::get_gate_type_by_name("gate_8_to_8"), "r_1");

                std::vector<std::shared_ptr<Net>> all_nets = {bracket_net,comma_net,comma_space_net,slash_net,backslash_net,curly_bracket_net,curly_bracket_net,
                                                              angle_bracket_net,double_underscore_net,edges_underscore_net,digit_only_net};
                std::vector<std::shared_ptr<Gate>> dummy_gates_l = {dummy_gate_l_0,dummy_gate_l_1};
                std::vector<std::shared_ptr<Gate>> dummy_gates_r = {dummy_gate_r_0,dummy_gate_r_1};
                // Connect the nets in a loop
                for(size_t i = 0; i<all_nets.size(); i++){
                    all_nets[i]->add_source(dummy_gates_l[i/8],"O"+std::to_string(i%8));
                    all_nets[i]->add_destination(dummy_gates_r[i/8],"I"+std::to_string(i%8));
                }

                // Write and parse the netlist now
                std::stringstream parser_input;
                HDLWriterVHDL vhdl_writer(parser_input);

                // Writes the netlist in the sstream
                bool writer_suc = vhdl_writer.write(nl);
                ASSERT_TRUE(writer_suc);

                HDLParserVHDL vhdl_parser(parser_input);
                // Parse the .vhdl file
                std::shared_ptr<Netlist> parsed_nl = vhdl_parser.parse_and_instantiate(m_gl);

                if (parsed_nl == nullptr) {
                    std::cout << test_def::get_captured_stdout() << std::endl;
                }
                ASSERT_NE(parsed_nl, nullptr);
                test_def::get_captured_stdout();

                // Check if the net_name is translated correctly
                EXPECT_FALSE(parsed_nl->get_nets(test_utils::net_name_filter("net_0")).empty());
                EXPECT_FALSE(parsed_nl->get_nets(test_utils::net_name_filter("net_1")).empty());
                EXPECT_FALSE(parsed_nl->get_nets(test_utils::net_name_filter("net_2")).empty());
                EXPECT_FALSE(parsed_nl->get_nets(test_utils::net_name_filter("net_3")).empty());
                EXPECT_FALSE(parsed_nl->get_nets(test_utils::net_name_filter("net_4")).empty());
                EXPECT_FALSE(parsed_nl->get_nets(test_utils::net_name_filter("net_5")).empty());
                EXPECT_FALSE(parsed_nl->get_nets(test_utils::net_name_filter("net_6")).empty());
                EXPECT_FALSE(parsed_nl->get_nets(test_utils::net_name_filter("net_7")).empty());
                EXPECT_FALSE(parsed_nl->get_nets(test_utils::net_name_filter("net_8")).empty());
                EXPECT_FALSE(parsed_nl->get_nets(test_utils::net_name_filter("NET_9")).empty());//
            }
            {
                // Testing the handling of special Gate names
                std::shared_ptr<Netlist> nl = test_utils::create_empty_netlist(0);

                // Create various gates with special Gate name characters
                std::shared_ptr<Gate>
                    bracket_gate = nl->create_gate(MIN_GATE_ID + 0, test_utils::get_gate_type_by_name("gate_1_to_1"), "gate(0)");
                std::shared_ptr<Gate>
                    comma_gate = nl->create_gate(MIN_GATE_ID + 1, test_utils::get_gate_type_by_name("gate_1_to_1"), "gate,1");
                std::shared_ptr<Gate>
                    comma_space_gate = nl->create_gate(MIN_GATE_ID + 2, test_utils::get_gate_type_by_name("gate_1_to_1"), "gate, 2");
                std::shared_ptr<Gate>
                    slash_gate = nl->create_gate(MIN_GATE_ID + 3, test_utils::get_gate_type_by_name("gate_1_to_1"), "gate/_3");
                std::shared_ptr<Gate>
                    backslash_gate = nl->create_gate(MIN_GATE_ID + 4, test_utils::get_gate_type_by_name("gate_1_to_1"), "gate\\_4");
                std::shared_ptr<Gate>
                    curly_bracket_gate = nl->create_gate(MIN_GATE_ID + 5, test_utils::get_gate_type_by_name("gate_1_to_1"), "gate[5]");
                std::shared_ptr<Gate>
                    angle_bracket_gate = nl->create_gate(MIN_GATE_ID + 6, test_utils::get_gate_type_by_name("gate_1_to_1"), "gate<6>");
                std::shared_ptr<Gate>
                    double_underscore_gate = nl->create_gate(MIN_GATE_ID + 7, test_utils::get_gate_type_by_name("gate_1_to_1"), "gate__7");
                std::shared_ptr<Gate>
                    edges_underscore_gate = nl->create_gate(MIN_GATE_ID + 8, test_utils::get_gate_type_by_name("gate_1_to_1"), "_gate_8_");
                std::shared_ptr<Gate> digit_only_gate = nl->create_gate(MIN_GATE_ID + 9,
                                                                        test_utils::get_gate_type_by_name("gate_1_to_1"),
                                                                        "9"); // should be converted to GATE_9

                // Create output nets for all gates to create a valid netlist
                unsigned int idx = 0;
                for (auto g : nl->get_gates()) {
                    std::shared_ptr<Net> out_net = nl->create_net("net_" + std::to_string(idx));
                    out_net->add_source(g, "O");
                    idx++;
                }

                // Write and parse the netlist now
                test_def::capture_stdout();
                std::stringstream parser_input;
                HDLWriterVHDL vhdl_writer(parser_input);

                // Writes the netlist in the sstream
                bool writer_suc = vhdl_writer.write(nl);
                if (!writer_suc) {
                    std::cout << test_def::get_captured_stdout() << std::endl;
                }
                ASSERT_TRUE(writer_suc);

                HDLParserVHDL vhdl_parser(parser_input);
                // Parse the .vhdl file
                std::shared_ptr<Netlist> parsed_nl = vhdl_parser.parse_and_instantiate(m_gl);

                if (parsed_nl == nullptr) {
                    std::cout << test_def::get_captured_stdout() << std::endl;
                }
                ASSERT_NE(parsed_nl, nullptr);
                test_def::get_captured_stdout();

                // Check if the gate_name is translated correctly
                EXPECT_FALSE(parsed_nl->get_gates(test_utils::gate_name_filter("gate_0" + m_gate_suffix)).empty());
                EXPECT_FALSE(parsed_nl->get_gates(test_utils::gate_name_filter("gate_1" + m_gate_suffix)).empty());
                EXPECT_FALSE(parsed_nl->get_gates(test_utils::gate_name_filter("gate_2" + m_gate_suffix)).empty());
                EXPECT_FALSE(parsed_nl->get_gates(test_utils::gate_name_filter("gate_3" + m_gate_suffix)).empty());
                EXPECT_FALSE(parsed_nl->get_gates(test_utils::gate_name_filter("gate_4" + m_gate_suffix)).empty());
                EXPECT_FALSE(parsed_nl->get_gates(test_utils::gate_name_filter("gate_5" + m_gate_suffix)).empty());
                EXPECT_FALSE(parsed_nl->get_gates(test_utils::gate_name_filter("gate_6" + m_gate_suffix)).empty());
                EXPECT_FALSE(parsed_nl->get_gates(test_utils::gate_name_filter("gate_7" + m_gate_suffix)).empty());
                EXPECT_FALSE(parsed_nl->get_gates(test_utils::gate_name_filter("gate_8" + m_gate_suffix)).empty());
                EXPECT_FALSE(parsed_nl->get_gates(test_utils::gate_name_filter("GATE_9" + m_gate_suffix)).empty());
            }
        TEST_END
    }

    /**
     * Testing the handling of collisions with Gate and Net names
     *
     * IMPORTANT: If an error occurs, first run the HDLParserVHDL test to check, that
     * the issue isn't within the parser, but in the writer...
     *
     * Functions: write, parse
     */
    TEST_F(HDLWriterVHDLTest, check_gate_net_name_collision) {
        TEST_START
            {
                // Testing the handling of two gates with the same name
                std::shared_ptr<Netlist> nl = test_utils::create_empty_netlist(0);

                std::shared_ptr<Net> test_net = nl->create_net(MIN_NET_ID + 0, "gate_net_name");
                std::shared_ptr<Gate>
                    test_gate = nl->create_gate(MIN_GATE_ID + 0, test_utils::get_gate_type_by_name("gate_1_to_1"), "gate_net_name");

                test_net->add_destination(test_gate, "I");

                // Write and parse the netlist now
                test_def::capture_stdout();
                std::stringstream parser_input;
                HDLWriterVHDL vhdl_writer(parser_input);

                // Writes the netlist in the sstream
                bool writer_suc = vhdl_writer.write(nl);
                if (!writer_suc) {
                    std::cout << test_def::get_captured_stdout() << std::endl;
                }
                ASSERT_TRUE(writer_suc);

                HDLParserVHDL vhdl_parser(parser_input);

                // Parse the .vhdl file
                std::shared_ptr<Netlist> parsed_nl = vhdl_parser.parse_and_instantiate(m_gl);

                if (parsed_nl == nullptr) {
                    std::cout << test_def::get_captured_stdout() << std::endl;
                }
                ASSERT_NE(parsed_nl, nullptr);
                test_def::get_captured_stdout();

                // Check if the Gate name was added a "_inst"
                EXPECT_NE(test_utils::get_net_by_subname(parsed_nl, "gate_net_name"), nullptr);
                EXPECT_NE(test_utils::get_gate_by_subname(parsed_nl, "gate_net_name"), nullptr);

            }
        TEST_END
    }

}// namespace hal
