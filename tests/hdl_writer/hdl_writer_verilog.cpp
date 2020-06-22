#include "gtest/gtest.h"
#include "netlist_test_utils.h"
#include <core/log.h>
#include <core/utils.h>
#include <iostream>
#include <netlist/gate.h>
#include <netlist/net.h>
#include "netlist/gate_library/gate_library_manager.h"
#include "netlist/netlist_factory.h"
#include "netlist/netlist.h"
#include "netlist/hdl_parser/hdl_parser_verilog.h"
#include "netlist/hdl_writer/hdl_writer_verilog.h"

namespace hal {

    class HDLWriterVerilogTest : public ::testing::Test {
    protected:
        const std::string m_pseudo_simprim_lib_name = "PSEUDO_SIMPRIM_GATE_LIBRARY";
        const std::string m_gate_suffix = "_inst";
        std::filesystem::path m_pseudo_simprim_lib_path;
        std::shared_ptr<GateLibrary> m_gl;

        virtual void SetUp() {
            NO_COUT_BLOCK;
            m_pseudo_simprim_lib_path = core_utils::get_gate_library_directories()[0] / "pseudo_simprim_lib.json";
            m_gl = test_utils::get_testing_gate_library();
        }

        virtual void TearDown() {

        }
        /*
         * Gate library that only contains a very small set of gates of the xilinx simprim Gate library, for testing simprim exclusive behaviour
         */
        void create_pseudo_simprim_gate_lib() {
            // FIXME
            NO_COUT_BLOCK;
            std::ofstream test_lib(m_pseudo_simprim_lib_path.string());
            test_lib << "{\n"
                        "    \"library\": {\n"
                        "        \"library_name\": \"PSEUDO_SIMPRIM_GATE_LIBRARY\",\n"
                        "        \"elements\": {\n"
                        "            \"X_INV\" : [[\"I\"], [], [\"O\"]],\n"
                        "            \"X_AND4\" : [[\"I0\",\"I1\",\"I2\",\"I3\"], [], [\"O\"]],\n"
                        "            \"X_ZERO\" : [[], [], [\"O\"]],\n"
                        "            \"X_ONE\" : [[], [], [\"O\"]],\n"
                        "\n"
                        "            \"GLOBAL_GND\" : [[], [], [\"O\"]],\n"
                        "            \"GLOBAL_VCC\" : [[], [], [\"O\"]]\n"
                        "        },\n"
                        "        \"vhdl_includes\": [],\n"
                        "        \"global_gnd_nodes\": [\"GLOBAL_GND\"],\n"
                        "        \"global_vcc_nodes\": [\"GLOBAL_VCC\"]\n"
                        "    }\n"
                        "}";
            test_lib.close();

            gate_library_manager::load_all();
        }
    };

    /**
     * Testing to write a given netlist in a sstream and parses it after, with
     * the hdl_parse_verilog.
     * IMPORTANT: If an error occurs, first run the HDLParserVerilog test to check, that
     * the issue isn't within the parser, but in the writer...
     *
     * Functions: write, parse
     */
    TEST_F(HDLWriterVerilogTest, check_write_and_parse_main_example) {
        TEST_START
            {
                // Write and parse the example netlist (with some additions) and compare the result with the original netlist
                std::shared_ptr<Netlist> nl = test_utils::create_example_parse_netlist(0);

                // Mark the global gates as such
                nl->mark_gnd_gate(nl->get_gate_by_id(test_utils::MIN_GATE_ID + 1));
                nl->mark_vcc_gate(nl->get_gate_by_id(test_utils::MIN_GATE_ID + 2));

                // Write and parse the netlist now
                //test_def::capture_stdout();
                std::stringstream parser_input;
                HDLWriterVerilog verilog_writer(parser_input);

                // Writes the netlist in the sstream
                bool writer_suc = verilog_writer.write(nl);

                if (!writer_suc) {
                    std::cout << test_def::get_captured_stdout() << std::endl;
                }
                ASSERT_TRUE(writer_suc);

                HDLParserVerilog verilog_parser(parser_input);

                // Parse the .verilog file
                std::shared_ptr<Netlist> parsed_nl = verilog_parser.parse_and_instantiate(m_gl);

                if (parsed_nl == nullptr) {
                    std::cout << test_def::get_captured_stdout() << std::endl;
                }
                std::cout << parser_input.str() << std::endl;
                ASSERT_NE(parsed_nl, nullptr);

                // These are added by the parser in every netlist
                nl->create_gate(test_utils::MIN_GATE_ID + 101, test_utils::get_gate_type_by_name("gnd"), "global_gnd");
                nl->create_gate(test_utils::MIN_GATE_ID + 102, test_utils::get_gate_type_by_name("vcc"), "global_vcc");

                nl->mark_gnd_gate(nl->get_gate_by_id(test_utils::MIN_GATE_ID + 101));
                nl->mark_vcc_gate(nl->get_gate_by_id(test_utils::MIN_GATE_ID + 102));

                //test_def::get_captured_stdout();

                // Check if the original netlist and the parsed one are equal

                // -- Check if gates and nets are the same
                // FIXME
                /*
                EXPECT_EQ(nl->get_gates().size(), parsed_nl->get_gates().size());
                for(auto g_0 : nl->get_gates()){
                    EXPECT_TRUE(gates_are_equal(g_0, get_gate_by_subname(parsed_nl, g_0->get_name()),true,true));
                }

                EXPECT_EQ(nl->get_nets().size(), parsed_nl->get_nets().size());
                for(auto n_0 : nl->get_nets()){
                    EXPECT_TRUE(nets_are_equal(n_0, get_net_by_subname(parsed_nl, n_0->get_name()), true, true));
                }

                // -- Check if global gates are the same
                EXPECT_EQ(nl->get_gnd_gates().size(), parsed_nl->get_gnd_gates().size());
                for(auto gl_gnd_0 : nl->get_gnd_gates()){
                    EXPECT_TRUE(parsed_nl->is_gnd_gate(get_gate_by_subname(parsed_nl, gl_gnd_0->get_name())));
                }

                EXPECT_EQ(nl->get_vcc_gates().size(), parsed_nl->get_vcc_gates().size());
                for(auto gl_vcc_0 : nl->get_vcc_gates()){
                    EXPECT_TRUE(parsed_nl->is_vcc_gate(get_gate_by_subname(parsed_nl, gl_vcc_0->get_name())));
                }*/
            }
        TEST_END
    }

    /**
     * Testing the writing of global input/output/inout nets
     *
     * IMPORTANT: If an error occurs, first run the HDLParserVerilog test to check, that
     * the issue isn't within the parser, but in the writer...
     *
     * Functions: write, parse
     */
    TEST_F(HDLWriterVerilogTest, check_global_nets) {
        TEST_START
            {
                // Add 2 global input nets to an empty netlist
                std::shared_ptr<Netlist> nl = test_utils::create_empty_netlist(0);

                std::shared_ptr<Net> global_in_0 = nl->create_net(test_utils::MIN_NET_ID + 0, "0_global_in");
                std::shared_ptr<Net> global_in_1 = nl->create_net(test_utils::MIN_NET_ID + 1, "1_global_in");

                nl->mark_global_input_net(global_in_0);
                nl->mark_global_input_net(global_in_1);

                // Write and parse the netlist now
                test_def::capture_stdout();
                std::stringstream parser_input;
                HDLWriterVerilog verilog_writer(parser_input);

                // Writes the netlist in the sstream
                bool writer_suc = verilog_writer.write(nl);
                if (!writer_suc) {
                    std::cout << test_def::get_captured_stdout() << std::endl;
                }
                ASSERT_TRUE(writer_suc);

                HDLParserVerilog verilog_parser(parser_input);
                // Parse the .verilog file
                std::shared_ptr<Netlist> parsed_nl = verilog_parser.parse_and_instantiate(m_gl);

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

                std::shared_ptr<Net> global_out_0 = nl->create_net(test_utils::MIN_NET_ID + 0, "0_global_out");
                std::shared_ptr<Net> global_out_1 = nl->create_net(test_utils::MIN_NET_ID + 1, "1_global_out");

                nl->mark_global_output_net(global_out_0);
                nl->mark_global_output_net(global_out_1);

                // Write and parse the netlist now
                test_def::capture_stdout();
                std::stringstream parser_input;
                HDLWriterVerilog verilog_writer(parser_input);

                // Writes the netlist in the sstream
                bool writer_suc = verilog_writer.write(nl);
                if (!writer_suc) {
                    std::cout << test_def::get_captured_stdout() << std::endl;
                }
                ASSERT_TRUE(writer_suc);

                HDLParserVerilog verilog_parser(parser_input);
                // Parse the .verilog file
                std::shared_ptr<Netlist> parsed_nl = verilog_parser.parse_and_instantiate(m_gl);

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
            /*{ // NOTE: Inout nets are not handled by the parser currently
                // Add 2 global inout nets to an empty netlist
                std::shared_ptr<Netlist> nl = create_empty_netlist(0);

                std::shared_ptr<Net> global_inout_0 = nl->create_net( test_utils::MIN_NET_ID+0, "0_global_inout");
                std::shared_ptr<Net> global_inout_1 = nl->create_net( test_utils::MIN_NET_ID+1, "1_global_inout");

                nl->mark_global_inout_net(global_inout_0);
                nl->mark_global_inout_net(global_inout_1);

                // Write and parse the netlist now
                test_def::capture_stdout();
                std::stringstream parser_input;
                HDLWriterVerilog verilog_writer(parser_input);

                // Writes the netlist in the sstream
                bool writer_suc = verilog_writer.write(nl);
                if (!writer_suc) {
                    std::cout << test_def::get_captured_stdout() << std::endl;
                }
                ASSERT_TRUE(writer_suc);

                HDLParserVerilog verilog_parser(parser_input);
                // Parse the .verilog file
                std::shared_ptr<Netlist> parsed_nl = verilog_parser.parse_and_instantiate(m_gl);

                if (parsed_nl == nullptr) {
                    std::cout << test_def::get_captured_stdout() << std::endl;
                }
                ASSERT_NE(parsed_nl, nullptr);
                test_def::get_captured_stdout();

                // Check if the nets are written/parsed correctly
                std::shared_ptr<Net> p_global_inout_0 = get_net_by_subname(parsed_nl, "0_global_inout");
                ASSERT_NE(p_global_inout_0, nullptr);
                EXPECT_TRUE(parsed_nl->is_global_inout_net(p_global_inout_0));

                std::shared_ptr<Net> p_global_inout_1 = get_net_by_subname(parsed_nl, "1_global_inout");
                ASSERT_NE(p_global_inout_1, nullptr);
                EXPECT_TRUE(parsed_nl->is_global_inout_net(p_global_inout_1));

            }*/
        TEST_END
    }

    /**
     * Testing the storage of generic data within gates
     *
     * IMPORTANT: If an error occurs, first run the HDLParserVerilog test to check, that
     * the issue isn't within the parser, but in the writer...
     *
     * Functions: write, parse
     */
    TEST_F(HDLWriterVerilogTest, check_generic_data_storage) {
        TEST_START
            /*{ //NOTE: generic data isn't handled correctly in this version (parser issue)
                // Add a Gate to the netlist and store some data
                std::shared_ptr<Netlist> nl = create_empty_netlist(0);

                std::shared_ptr<Net> global_in = nl->create_net( MIN_NET_ID+0, "global_in");
                nl->mark_global_input_net(global_in);

                std::shared_ptr<Gate> test_gate_0 = nl->create_gate( MIN_GATE_ID+0, "gate_1_to_1", "test_gate_0");
                std::shared_ptr<Gate> test_gate_1 = nl->create_gate( MIN_GATE_ID+1, "gate_1_to_1", "test_gate_1");
                std::shared_ptr<Gate> test_gate_2 = nl->create_gate( MIN_GATE_ID+2, "gate_1_to_1", "test_gate_2");
                std::shared_ptr<Gate> test_gate_3 = nl->create_gate( MIN_GATE_ID+3, "gate_1_to_1", "test_gate_3");
                std::shared_ptr<Gate> test_gate_4 = nl->create_gate( MIN_GATE_ID+4, "gate_1_to_1", "test_gate_4");
                std::shared_ptr<Gate> test_gate_5 = nl->create_gate( MIN_GATE_ID+5, "gate_1_to_1", "test_gate_5");
                std::shared_ptr<Gate> test_gate_6 = nl->create_gate( MIN_GATE_ID+6, "gate_1_to_1", "test_gate_6");

                // Create output nets for all gates to create a valid netlist
                unsigned int idx = 0;
                for (auto g : nl->get_gates()){
                    std::shared_ptr<Net> out_net = nl->create_net("net_" + std::to_string(idx));
                    out_net->add_source(g,"O");
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

                test_gate_5->set_data("generic", "0_key_invalid", "invalid", "ignore_me" ); // Should be ignored
                test_gate_5->set_data("generic", "1_key_bit_vector", "bit_vector", "1");

                test_gate_6->set_data("generic", "0_key_bit_value", "bit_value", "10101100");
                test_gate_6->set_data("generic", "1_key_bit_value", "bit_value", "01010011");

                // Write and parse the netlist now
                test_def::capture_stdout();
                std::stringstream parser_input;
                HDLWriterVerilog verilog_writer(parser_input);

                // Writes the netlist in the sstream
                bool writer_suc = verilog_writer.write(nl);
                if (!writer_suc) {
                    std::cout << test_def::get_captured_stdout() << std::endl;
                }
                ASSERT_TRUE(writer_suc);

                std::cout << parser_input.str() << std::endl;
                HDLParserVerilog verilog_parser(parser_input);

                // Parse the .verilog file
                std::shared_ptr<Netlist> parsed_nl = verilog_parser.parse_and_instantiate(m_gl);

                if (parsed_nl == nullptr) {
                    std::cout << test_def::get_captured_stdout() << std::endl;
                }
                ASSERT_NE(parsed_nl, nullptr);
                test_def::get_captured_stdout();

                // Check if the data is written/parsed correctly
                std::shared_ptr<Gate> p_test_gate_0 = get_gate_by_subname(parsed_nl, "test_gate_0");
                ASSERT_NE(p_test_gate_0, nullptr);
                EXPECT_EQ(p_test_gate_0->get_data(), test_gate_0->get_data());

                std::shared_ptr<Gate> p_test_gate_1 = get_gate_by_subname(parsed_nl, "test_gate_1");
                ASSERT_NE(p_test_gate_1, nullptr);
                EXPECT_EQ(p_test_gate_1->get_data(), test_gate_1->get_data());

                std::shared_ptr<Gate> p_test_gate_2 = get_gate_by_subname(parsed_nl, "test_gate_2");
                ASSERT_NE(p_test_gate_2, nullptr);
                EXPECT_EQ(p_test_gate_2->get_data(), test_gate_2->get_data());

                std::shared_ptr<Gate> p_test_gate_3 = get_gate_by_subname(parsed_nl, "test_gate_3");
                ASSERT_NE(p_test_gate_3, nullptr);
                EXPECT_EQ(p_test_gate_3->get_data(), test_gate_3->get_data());

                std::shared_ptr<Gate> p_test_gate_4 = get_gate_by_subname(parsed_nl, "test_gate_4");
                ASSERT_NE(p_test_gate_4, nullptr);
                EXPECT_EQ(p_test_gate_4->get_data(), test_gate_4->get_data());

                std::shared_ptr<Gate> p_test_gate_5 = get_gate_by_subname(parsed_nl, "test_gate_5");
                ASSERT_NE(p_test_gate_5, nullptr);
                auto test_gate_5_data_without_invalid = test_gate_5->get_data();
                test_gate_5_data_without_invalid.erase(std::make_tuple("generic", "0_key_invalid"));
                EXPECT_EQ(p_test_gate_5->get_data(), test_gate_5_data_without_invalid);

                std::shared_ptr<Gate> p_test_gate_6 = get_gate_by_subname(parsed_nl, "test_gate_6");
                ASSERT_NE(p_test_gate_6, nullptr);
                EXPECT_EQ(p_test_gate_6->get_data(), test_gate_6->get_data());
            }*/
        TEST_END
    }

    /**
     * Testing the handling of Net names which contains only digits (i.e. 123 should become NET_123)
     *
     * IMPORTANT: If an error occurs, first run the HDLParserVerilog test to check, that
     * the issue isn't within the parser, but in the writer...
     *
     * Functions: write, parse
     */
    TEST_F(HDLWriterVerilogTest, check_digit_net_name) {
        TEST_START
            {
                // Add a Gate to the netlist and store some data
                std::shared_ptr<Netlist> nl = test_utils::create_empty_netlist(0);

                std::shared_ptr<Net> global_in = nl->create_net(test_utils::MIN_NET_ID + 0, "123");
                nl->mark_global_input_net(global_in);

                // Write and parse the netlist now
                test_def::capture_stdout();
                std::stringstream parser_input;
                HDLWriterVerilog verilog_writer(parser_input);

                // Writes the netlist in the sstream
                bool writer_suc = verilog_writer.write(nl);
                if (!writer_suc) {
                    std::cout << test_def::get_captured_stdout() << std::endl;
                }
                ASSERT_TRUE(writer_suc);

                HDLParserVerilog verilog_parser(parser_input);

                // Parse the .verilog file
                std::shared_ptr<Netlist> parsed_nl = verilog_parser.parse_and_instantiate(m_gl);

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
     * Testing the handling of Net/Gate names with special characters and their translation.
     * Special characters: '(', ')', ',', ', ', '/', '\', '[', ']', '<', '>', '__', '_'
     * Other special cases: only digits, '_' at the beginning or at the end
     *
     * IMPORTANT: If an error occurs, first run the HDLParserVerilog test to check, that
     * the issue isn't within the parser, but in the writer...
     *
     * Functions: write, parse
     */
    TEST_F(HDLWriterVerilogTest, check_special_net_names) {
        TEST_START
            {
                // Testing the handling of special Net names
                std::shared_ptr<Netlist> nl = test_utils::create_empty_netlist(0);

                std::shared_ptr<Net> bracket_net = nl->create_net(test_utils::MIN_NET_ID + 0, "Net(0)");
                std::shared_ptr<Net> comma_net = nl->create_net(test_utils::MIN_NET_ID + 1, "Net,1");
                std::shared_ptr<Net> comma_space_net = nl->create_net(test_utils::MIN_NET_ID + 2, "Net, 2");
                std::shared_ptr<Net> slash_net = nl->create_net(test_utils::MIN_NET_ID + 3, "Net/_3");
                std::shared_ptr<Net> backslash_net = nl->create_net(test_utils::MIN_NET_ID + 4, "Net\\_4");
                std::shared_ptr<Net> curly_bracket_net = nl->create_net(test_utils::MIN_NET_ID + 5, "Net[5]");
                std::shared_ptr<Net> angle_bracket_net = nl->create_net(test_utils::MIN_NET_ID + 6, "Net<6>");
                std::shared_ptr<Net> double_underscore_net = nl->create_net(test_utils::MIN_NET_ID + 7, "net__7");
                std::shared_ptr<Net> edges_underscore_net = nl->create_net(test_utils::MIN_NET_ID + 8, "_net_8_");
                std::shared_ptr<Net>
                    digit_only_net = nl->create_net(test_utils::MIN_NET_ID + 9, "9"); // should be converted to NET_9

                // Write and parse the netlist now
                test_def::capture_stdout();
                std::stringstream parser_input;
                HDLWriterVerilog verilog_writer(parser_input);

                // Writes the netlist in the sstream
                bool writer_suc = verilog_writer.write(nl);
                if (!writer_suc) {
                    std::cout << test_def::get_captured_stdout() << std::endl;
                }
                ASSERT_TRUE(writer_suc);

                HDLParserVerilog verilog_parser(parser_input);
                // Parse the .verilog file
                std::shared_ptr<Netlist> parsed_nl = verilog_parser.parse_and_instantiate(m_gl);

                if (parsed_nl == nullptr) {
                    std::cout << test_def::get_captured_stdout() << std::endl;
                }
                ASSERT_NE(parsed_nl, nullptr);
                test_def::get_captured_stdout();

                // FIXME: Nets must be connected. Otherwise they are removed...

                // Check if the net_name is translated correctly
                /*EXPECT_FALSE(parsed_nl->get_nets("net_0").empty());
                EXPECT_FALSE(parsed_nl->get_nets("net_1").empty());
                EXPECT_FALSE(parsed_nl->get_nets("net_2").empty());
                EXPECT_FALSE(parsed_nl->get_nets("net_3").empty());
                EXPECT_FALSE(parsed_nl->get_nets("net_4").empty());
                EXPECT_FALSE(parsed_nl->get_nets("net_5").empty());
                EXPECT_FALSE(parsed_nl->get_nets("net_6").empty());
                EXPECT_FALSE(parsed_nl->get_nets("net_7").empty());
                EXPECT_FALSE(parsed_nl->get_nets("net_8").empty());
                EXPECT_FALSE(parsed_nl->get_nets("NET_9").empty());*/
            }
            {
                // Testing the handling of special Gate names
                std::shared_ptr<Netlist> nl = test_utils::create_empty_netlist(0);
                std::shared_ptr<GateLibrary> gl = m_gl;

                // Create various gates with special Gate name characters
                std::shared_ptr<Gate>
                    bracket_gate = nl->create_gate(test_utils::MIN_GATE_ID + 0, gl->get_gate_types().at("gate_1_to_1"), "gate(0)");
                std::shared_ptr<Gate>
                    comma_gate = nl->create_gate(test_utils::MIN_GATE_ID + 1, gl->get_gate_types().at("gate_1_to_1"), "gate,1");
                std::shared_ptr<Gate>
                    comma_space_gate = nl->create_gate(test_utils::MIN_GATE_ID + 2, gl->get_gate_types().at("gate_1_to_1"), "gate, 2");
                std::shared_ptr<Gate>
                    slash_gate = nl->create_gate(test_utils::MIN_GATE_ID + 3, gl->get_gate_types().at("gate_1_to_1"), "gate/_3");
                std::shared_ptr<Gate>
                    backslash_gate = nl->create_gate(test_utils::MIN_GATE_ID + 4, gl->get_gate_types().at("gate_1_to_1"), "gate\\_4");
                std::shared_ptr<Gate>
                    curly_bracket_gate = nl->create_gate(test_utils::MIN_GATE_ID + 5, gl->get_gate_types().at("gate_1_to_1"), "gate[5]");
                std::shared_ptr<Gate>
                    angle_bracket_gate = nl->create_gate(test_utils::MIN_GATE_ID + 6, gl->get_gate_types().at("gate_1_to_1"), "gate<6>");
                std::shared_ptr<Gate> double_underscore_gate =
                    nl->create_gate(test_utils::MIN_GATE_ID + 7, gl->get_gate_types().at("gate_1_to_1"), "gate__7");
                std::shared_ptr<Gate> edges_underscore_gate =
                    nl->create_gate(test_utils::MIN_GATE_ID + 8, gl->get_gate_types().at("gate_1_to_1"), "_gate_8_");
                std::shared_ptr<Gate> digit_only_gate = nl->create_gate(test_utils::MIN_GATE_ID + 9,
                                                                        gl->get_gate_types().at("gate_1_to_1"),
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
                HDLWriterVerilog verilog_writer(parser_input);

                // Writes the netlist in the sstream
                bool writer_suc = verilog_writer.write(nl);
                if (!writer_suc) {
                    std::cout << test_def::get_captured_stdout() << std::endl;
                }
                ASSERT_TRUE(writer_suc);

                HDLParserVerilog verilog_parser(parser_input);
                // Parse the .verilog file
                std::shared_ptr<Netlist> parsed_nl = verilog_parser.parse_and_instantiate(m_gl);

                if (parsed_nl == nullptr) {
                    std::cout << test_def::get_captured_stdout() << std::endl;
                }
                ASSERT_NE(parsed_nl, nullptr);
                test_def::get_captured_stdout();


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
     * IMPORTANT: If an error occurs, first run the HDLParserVerilog test to check, that
     * the issue isn't within the parser, but in the writer...
     *
     * Functions: write, parse
     */
    TEST_F(HDLWriterVerilogTest, check_gate_net_name_collision) {
        TEST_START
            {
                // Testing the handling of two gates with the same name
                std::shared_ptr<Netlist> nl = test_utils::create_empty_netlist(0);

                std::shared_ptr<Net> test_net = nl->create_net(test_utils::MIN_NET_ID + 0, "gate_net_name");
                std::shared_ptr<Gate>
                    test_gate = nl->create_gate(test_utils::MIN_GATE_ID + 0, test_utils::get_gate_type_by_name("gate_1_to_1"), "gate_net_name");

                test_net->add_destination(test_gate, "I");

                // Write and parse the netlist now
                test_def::capture_stdout();
                std::stringstream parser_input;
                HDLWriterVerilog verilog_writer(parser_input);

                // Writes the netlist in the sstream
                bool writer_suc = verilog_writer.write(nl);
                if (!writer_suc) {
                    std::cout << test_def::get_captured_stdout() << std::endl;
                }
                ASSERT_TRUE(writer_suc);

                HDLParserVerilog verilog_parser(parser_input);

                // Parse the .verilog file
                std::shared_ptr<Netlist> parsed_nl = verilog_parser.parse_and_instantiate(m_gl);

                if (parsed_nl == nullptr) {
                    std::cout << test_def::get_captured_stdout() << std::endl;
                }
                ASSERT_NE(parsed_nl, nullptr);
                test_def::get_captured_stdout();

                // Check if the Gate name was added a "_inst"
                EXPECT_NE(test_utils::get_net_by_subname(parsed_nl, "gate_net_name"), nullptr);
                EXPECT_NE(test_utils::get_gate_by_subname(parsed_nl, "gate_net_name_inst"), nullptr);

            }
        TEST_END
    }

    /**
     * Testing translation of '0' and '1' Net names in the verilog standard (1'b0 and 1'b1).
     *
     * IMPORTANT: If an error occurs, first run the HDLParserVerilog test to check, that
     * the issue isn't within the parser, but in the writer...
     *
     * Functions: write, parse
     */
    TEST_F(HDLWriterVerilogTest, check_constant_nets) {
        TEST_START
            /*{ FIXME, NOTE: GND/VCC gates are replaced (but not deleted) by global_gnd/global_vcc
                // Testing the Net name translation of a '0' Net
                std::shared_ptr<Netlist> nl = create_empty_netlist(0);

                std::shared_ptr<Gate> gnd_gate = nl->create_gate(test_utils::MIN_GATE_ID+0, test_utils::get_gate_type_by_name("GND"), "gnd_gate");
                std::shared_ptr<Net> global_out = nl->create_net(test_utils::MIN_NET_ID+0, "global_out");
                nl->mark_global_output_net(global_out);
                std::shared_ptr<Gate> test_gate = nl->create_gate( test_utils::MIN_GATE_ID+1, test_utils::get_gate_type_by_name("gate_1_to_1"), "test_gate");
                global_out->add_source(test_gate, "O");

                std::shared_ptr<Net> gnd_net = nl->create_net(test_utils::MIN_NET_ID+1, "'0'");
                gnd_net->add_source(gnd_gate, "O");
                gnd_net->add_destination(test_gate, "I");

                // Write and parse the netlist now
                test_def::capture_stdout();
                std::stringstream parser_input;
                HDLWriterVerilog verilog_writer(parser_input);

                // Writes the netlist in the sstream
                bool writer_suc = verilog_writer.write(nl);
                if (!writer_suc) {
                    std::cout << test_def::get_captured_stdout() << std::endl;
                }
                ASSERT_TRUE(writer_suc);

                HDLParserVerilog verilog_parser(parser_input);

                // Parse the .verilog file
                std::shared_ptr<Netlist> parsed_nl = verilog_parser.parse_and_instantiate(m_gl);

                if (parsed_nl == nullptr) {
                    std::cout << test_def::get_captured_stdout() << std::endl;
                }
                ASSERT_NE(parsed_nl, nullptr);
                test_def::get_captured_stdout();

                // Check if a Net name is correctly translated
                ASSERT_EQ(parsed_nl->get_nets("1'b0").size(), 1);
                std::shared_ptr<Net> gnd_net_translated = *parsed_nl->get_nets("1'b0").begin();
                ASSERT_NE(gnd_net_translated->get_source().get_gate(), nullptr);
                EXPECT_EQ(gnd_net_translated->get_source().get_gate()->get_type()->get_name(), "GND");
                ASSERT_EQ(gnd_net_translated->get_destinations().size(), 1);
                EXPECT_EQ((*gnd_net_translated->get_destinations().begin()).get_gate()->get_name(), "test_gate" + m_gate_suffix);
            }
            {
                // Testing the Net name translation of a '0' Net
                std::shared_ptr<Netlist> nl = create_empty_netlist(0);

                std::shared_ptr<Gate> vcc_gate = nl->create_gate(test_utils::MIN_GATE_ID+0, test_utils::get_gate_type_by_name("vcc"), "vcc_gate");
                std::shared_ptr<Net> global_out = nl->create_net(test_utils::MIN_NET_ID+0, "global_out");
                nl->mark_global_output_net(global_out);
                std::shared_ptr<Gate> test_gate = nl->create_gate( test_utils::MIN_GATE_ID+1, get_gate_type_by_name("gate_1_to_1"), "test_gate");
                global_out->add_source(test_gate, "O");

                std::shared_ptr<Net> vcc_net = nl->create_net(test_utils::MIN_NET_ID+1, "'1'");
                vcc_net->add_source(vcc_gate, "O");
                vcc_net->add_destination(test_gate, "I");

                // Write and parse the netlist now
                test_def::capture_stdout();
                std::stringstream parser_input;
                HDLWriterVerilog verilog_writer(parser_input);

                // Writes the netlist in the sstream
                bool writer_suc = verilog_writer.write(nl);
                if (!writer_suc) {
                    std::cout << test_def::get_captured_stdout() << std::endl;
                }
                ASSERT_TRUE(writer_suc);

                HDLParserVerilog verilog_parser(parser_input);

                // Parse the .verilog file
                std::shared_ptr<Netlist> parsed_nl = verilog_parser.parse_and_instantiate(m_gl);

                if (parsed_nl == nullptr) {
                    std::cout << test_def::get_captured_stdout() << std::endl;
                }
                ASSERT_NE(parsed_nl, nullptr);
                test_def::get_captured_stdout();

                // Check if a Net name is correctly translated
                ASSERT_EQ(parsed_nl->get_nets("1'b1").size(), 1);
                std::shared_ptr<Net> vcc_net_translated = *parsed_nl->get_nets("1'b1").begin();
                ASSERT_NE(vcc_net_translated->get_source().get_gate(), nullptr);
                EXPECT_EQ(vcc_net_translated->get_source().get_gate()->get_type()->get_name(), "VCC");
                ASSERT_EQ(vcc_net_translated->get_destinations().size(), 1);
                EXPECT_EQ((*vcc_net_translated->get_destinations().begin()).get_gate()->get_name(), "test_gate" + m_gate_suffix);
            }*/
        TEST_END
    }

    /**
     * Testing the correct handling of pin vectors (e.g. I(0), I(1), I(2), I(3)).
     *
     * IMPORTANT: If an error occurs, first run the HDLParserVerilog test to check, that
     * the issue isn't within the parser, but in the writer...
     *
     * Functions: write, parse
     */
    TEST_F(HDLWriterVerilogTest, check_pin_vector) {
        TEST_START
            {
                /*                      .-------------------.
                 *    test_gnd_net ----=|I(0)               |
                 *    test_vcc_net ----=|I(1)  test_gate   O|=---- global_out
                 *    test_gnd_net ----=|I(2)               |
                 *    test_vcc_net ----=|I(3)               |
                 *                      '-------------------'
                 */
                /*
                 * ISSUE: reversed order of input nets (parser or writer issue?)
                 * create_temp_gate_lib();
                // Testing the usage of a pin vector using the temp Gate library
                std::shared_ptr<GateLibrary> gl = gate_library_manager::get_gate_library(temp_lib_name);
                std::shared_ptr<Netlist> nl(new Netlist(gl));

                std::shared_ptr<Gate> gnd_gate = nl->create_gate(MIN_GATE_ID + 0, "GND", "gnd_gate");
                std::shared_ptr<Gate> vcc_gate = nl->create_gate(MIN_GATE_ID + 1, "VCC", "vcc_gate");
                std::shared_ptr<Net> global_out = nl->create_net(MIN_NET_ID + 0, "global_out");
                nl->mark_global_output_net(global_out);
                std::shared_ptr<Gate> test_gate = nl->create_gate(MIN_GATE_ID + 2, "GATE_4^1_IN_1^0_OUT", "test_gate");
                global_out->add_source(test_gate, "O");

                std::shared_ptr<Net> gnd_net = nl->create_net(MIN_NET_ID + 2, "test_gnd_net");
                std::shared_ptr<Net> vcc_net = nl->create_net(MIN_NET_ID + 3, "test_vcc_net");

                gnd_net->add_source(gnd_gate, "O");
                vcc_net->add_source(vcc_gate, "O");

                gnd_net->add_destination(test_gate, "I(0)");
                vcc_net->add_destination(test_gate, "I(1)");
                gnd_net->add_destination(test_gate, "I(2)");
                vcc_net->add_destination(test_gate, "I(3)");

                // Write and parse the netlist now
                test_def::capture_stdout();
                std::stringstream parser_input;
                HDLWriterVerilog verilog_writer(parser_input);

                // Writes the netlist in the sstream
                bool writer_suc = verilog_writer.write(nl);
                if (!writer_suc) {
                    std::cout << test_def::get_captured_stdout() << std::endl;
                }
                ASSERT_TRUE(writer_suc);

                HDLParserVerilog verilog_parser(parser_input);

                // Parse the .verilog file
                std::shared_ptr<Netlist> parsed_nl = verilog_parser.parse_and_instantiate(m_gl);

                if (parsed_nl == nullptr) {
                    std::cout << test_def::get_captured_stdout() << std::endl;
                }
                ASSERT_NE(parsed_nl, nullptr);
                test_def::get_captured_stdout();

                ASSERT_EQ(parsed_nl->get_nets("test_gnd_net").size(), 1);
                ASSERT_EQ(parsed_nl->get_nets("test_vcc_net").size(), 1);
                std::shared_ptr<Net> test_gnd_net_ref = *parsed_nl->get_nets("test_gnd_net").begin();
                std::shared_ptr<Net> test_vcc_net_ref = *parsed_nl->get_nets("test_vcc_net").begin();

                ASSERT_EQ(parsed_nl->get_gates("GATE_4^1_IN_1^0_OUT").size(), 1);
                std::shared_ptr<Gate> test_gate_ref = *parsed_nl->get_gates("GATE_4^1_IN_1^0_OUT").begin();
                EXPECT_EQ(test_gate_ref->get_fan_in_net("I(0)"), test_gnd_net_ref);
                EXPECT_EQ(test_gate_ref->get_fan_in_net("I(1)"), test_vcc_net_ref);
                EXPECT_EQ(test_gate_ref->get_fan_in_net("I(2)"), test_gnd_net_ref);
                EXPECT_EQ(test_gate_ref->get_fan_in_net("I(3)"), test_vcc_net_ref);
                 */
            }
        TEST_END
    }

    /**
     * Testing the correct handling of the simprim exclusive X_ZERO and X_ONE gates, as well as the usage of GLOBAL_GND
     * and GLOBAL_VCC gates.
     *
     * IMPORTANT: If an error occurs, first run the HDLParserVerilog test to check, that
     * the issue isn't within the parser, but in the writer...
     *
     * Functions: write, parse
     */
    TEST_F(HDLWriterVerilogTest, check_simprim_exclusive_behaviour) {
        TEST_START
            //create_pseudo_simprim_gate_lib();
            { // ISSUE: Net definition: "wire net_zero_gate_0 = 1'h0" is created, but can't be interpreted by the parser (stoi failure) (parser or writer issue?)
                // NOTE: GLOBAL_GND / GLOBAL_VCC gates are removed. Why?
                /*
                            // Testing the usage of nets connected to a X_ZERO Gate
                            std::shared_ptr<GateLibrary> gl = gate_library_manager::get_gate_library(m_pseudo_simprim_lib_name);
                            std::shared_ptr<Netlist> nl(new Netlist(gl));

                            std::shared_ptr<Gate> x_zero_gate_0 = nl->create_gate("X_ZERO", "x_zero_gate_0");
                            std::shared_ptr<Gate> x_zero_gate_1 = nl->create_gate("X_ZERO", "x_zero_gate_1");
                            std::shared_ptr<Gate> test_gate = nl->create_gate("X_AND4", "test_gate");

                            std::shared_ptr<Net> global_out_net = nl->create_net("global_out");
                            global_out_net->add_source(test_gate, "O");
                            nl->mark_global_output_net(global_out_net);

                            std::shared_ptr<Net> x_zero_net_0 = nl->create_net("x_zero_net_0");
                            std::shared_ptr<Net> x_zero_net_1 = nl->create_net("x_zero_net_1");

                            x_zero_net_0->add_source(x_zero_gate_0, "O");
                            x_zero_net_1->add_source(x_zero_gate_1, "O");

                            x_zero_net_0->add_destination(test_gate, "I0");
                            x_zero_net_1->add_destination(test_gate, "I1");

                            // Write and parse the netlist now
                            //test_def::capture_stdout();
                            std::stringstream parser_input;
                            HDLWriterVerilog verilog_writer(parser_input);

                            // Writes the netlist in the sstream
                            bool writer_suc = verilog_writer.write(nl);
                            if (!writer_suc) {
                                //std::cout << test_def::get_captured_stdout() << std::endl;
                            }
                            ASSERT_TRUE(writer_suc);
a
                            HDLParserVerilog verilog_parser(parser_input);

                            // Parse the .verilog file
                            std::shared_ptr<Netlist> parsed_nl = verilog_parser.parse_and_instantiate(m_pseudo_simprim_lib_name);

                            if (parsed_nl == nullptr) {
                                //std::cout << test_def::get_captured_stdout() << std::endl;
                            }
                            ASSERT_NE(parsed_nl, nullptr);
                            //test_def::get_captured_stdout();

                */
            }
        TEST_END
    }
} //namespace hal
