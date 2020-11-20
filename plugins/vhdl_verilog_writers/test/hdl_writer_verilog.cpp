#include "gtest/gtest.h"
#include "netlist_test_utils.h"
#include "hal_core/netlist/netlist_factory.h"
#include "vhdl_verilog_parsers/hdl_parser_verilog.h"
#include "vhdl_verilog_writers/hdl_writer_verilog.h"

namespace hal {

    using test_utils::MIN_GATE_ID;
    using test_utils::MIN_NET_ID;

    class HDLWriterVerilogTest : public ::testing::Test {
    protected:
        const std::string m_gate_suffix = "_inst";
        GateLibrary* m_gl;

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
                std::unique_ptr<Netlist> nl = test_utils::create_example_parse_netlist(0);

                // Mark the global gates as such
                nl->mark_gnd_gate(nl->get_gate_by_id(test_utils::MIN_GATE_ID + 1));
                nl->mark_vcc_gate(nl->get_gate_by_id(test_utils::MIN_GATE_ID + 2));

                // Write and parse the netlist
                std::stringstream parser_input;
                HDLWriterVerilog verilog_writer;

                // Writes the netlist in the sstream
                bool writer_suc = verilog_writer.write(nl.get(), parser_input);

                ASSERT_TRUE(writer_suc);

                HDLParserVerilog verilog_parser;

                // Parse the .verilog file
                std::unique_ptr<Netlist> parsed_nl = verilog_parser.parse_and_instantiate(parser_input, m_gl);

                ASSERT_NE(parsed_nl, nullptr);

                // Since verilog does no provide a device name, we have to add it manually for comparison
                parsed_nl->set_device_name(nl->get_device_name());

                // Since the writer appends a suffix ("_inst") to all gate names, we have to add them manually to the
                // original netlist for comparison NOTE: (legacy?)
                for (auto g : nl->get_gates()) {
                    g->set_name(g->get_name() + m_gate_suffix);
                }

                // Since the writer does not consider modules (yet...), we have to set the name and the type of the top module
                // manually for comparison
                parsed_nl->get_top_module()->set_name(nl->get_top_module()->get_name());
                parsed_nl->get_top_module()->set_type(nl->get_top_module()->get_type());

                // Check if the original netlist and the parsed one are equal
                EXPECT_TRUE(test_utils::netlists_are_equal(nl.get(), parsed_nl.get(), true));
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
                std::unique_ptr<Netlist> nl = test_utils::create_empty_netlist(0);

                Net* global_in_0 = nl->create_net(test_utils::MIN_NET_ID + 0, "0_global_in");
                Net* global_in_1 = nl->create_net(test_utils::MIN_NET_ID + 1, "1_global_in");

                nl->mark_global_input_net(global_in_0);
                nl->mark_global_input_net(global_in_1);

                // Write and parse the netlist
                std::stringstream parser_input;
                HDLWriterVerilog verilog_writer;

                // Writes the netlist in the sstream
                bool writer_suc = verilog_writer.write(nl.get(), parser_input);
                ASSERT_TRUE(writer_suc);

                HDLParserVerilog verilog_parser;
                // Parse the .verilog file
                std::unique_ptr<Netlist> parsed_nl = verilog_parser.parse_and_instantiate(parser_input, m_gl);

                ASSERT_NE(parsed_nl, nullptr);

                // Check if the nets are written/parsed correctly
                Net* p_global_in_0 = test_utils::get_net_by_subname(parsed_nl.get(), "0_global_in");
                ASSERT_NE(p_global_in_0, nullptr);
                EXPECT_TRUE(parsed_nl->is_global_input_net(p_global_in_0));

                Net* p_global_in_1 = test_utils::get_net_by_subname(parsed_nl.get(), "1_global_in");
                ASSERT_NE(p_global_in_1, nullptr);
                EXPECT_TRUE(parsed_nl->is_global_input_net(p_global_in_1));

            }
            {
                // Add 2 global output nets to an empty netlist
                std::unique_ptr<Netlist> nl = test_utils::create_empty_netlist(0);

                Net* global_out_0 = nl->create_net(test_utils::MIN_NET_ID + 0, "0_global_out");
                Net* global_out_1 = nl->create_net(test_utils::MIN_NET_ID + 1, "1_global_out");

                nl->mark_global_output_net(global_out_0);
                nl->mark_global_output_net(global_out_1);

                // Write and parse the netlist
                std::stringstream parser_input;
                HDLWriterVerilog verilog_writer;

                // Writes the netlist in the sstream
                bool writer_suc = verilog_writer.write(nl.get(), parser_input);
                ASSERT_TRUE(writer_suc);

                HDLParserVerilog verilog_parser;
                // Parse the .verilog file
                std::unique_ptr<Netlist> parsed_nl = verilog_parser.parse_and_instantiate(parser_input, m_gl);

                ASSERT_NE(parsed_nl, nullptr);

                // Check if the nets are written/parsed correctly
                Net* p_global_out_0 = test_utils::get_net_by_subname(parsed_nl.get(), "0_global_out");
                ASSERT_NE(p_global_out_0, nullptr);
                EXPECT_TRUE(parsed_nl->is_global_output_net(p_global_out_0));

                Net* p_global_out_1 = test_utils::get_net_by_subname(parsed_nl.get(), "1_global_out");
                ASSERT_NE(p_global_out_1, nullptr);
                EXPECT_TRUE(parsed_nl->is_global_output_net(p_global_out_1));
            }
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
            {
                // Add a Gate to the netlist and store some data
                std::unique_ptr<Netlist> nl = test_utils::create_empty_netlist(0);

                Net* global_in = nl->create_net(MIN_NET_ID + 0, "global_in");
                nl->mark_global_input_net(global_in);

                Gate* test_gate_0 =
                    nl->create_gate(MIN_GATE_ID + 0, test_utils::get_gate_type_by_name("gate_1_to_1"), "test_gate_0");
                Gate* test_gate_1 =
                    nl->create_gate(MIN_GATE_ID + 1, test_utils::get_gate_type_by_name("gate_1_to_1"), "test_gate_1");
                Gate* test_gate_2 =
                    nl->create_gate(MIN_GATE_ID + 2, test_utils::get_gate_type_by_name("gate_1_to_1"), "test_gate_2");
                Gate* test_gate_3 =
                    nl->create_gate(MIN_GATE_ID + 3, test_utils::get_gate_type_by_name("gate_1_to_1"), "test_gate_3");
                Gate* test_gate_4 =
                    nl->create_gate(MIN_GATE_ID + 4, test_utils::get_gate_type_by_name("gate_1_to_1"), "test_gate_4");
                Gate* test_gate_5 =
                    nl->create_gate(MIN_GATE_ID + 5, test_utils::get_gate_type_by_name("gate_1_to_1"), "test_gate_5");
                Gate* test_gate_6 =
                    nl->create_gate(MIN_GATE_ID + 6, test_utils::get_gate_type_by_name("gate_1_to_1"), "test_gate_6");

                // Create output nets for all gates to create a valid netlist
                unsigned int idx = 0;
                for (auto g : nl->get_gates()) {
                    Net* out_net = nl->create_net("net_" + std::to_string(idx));
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
                test_gate_0->set_data("generic", "1_key_bit_vector", "bit_vector", "123abc");
                test_gate_0->set_data("generic", "2_key_bit_vector", "bit_vector", "456def");

                test_gate_1->set_data("generic", "0_key_bit_vector", "bit_vector", "123abc");
                test_gate_1->set_data("generic", "1_key_string", "string", "one_two_three");

                test_gate_2->set_data("generic", "0_key_string", "string", "one_two_three");
                test_gate_2->set_data("generic", "1_key_boolean", "boolean", "TRUE");

                test_gate_3->set_data("generic", "0_key_boolean", "boolean", "TRUE");
                test_gate_3->set_data("generic", "1_key_integer", "integer", "123");

                test_gate_4->set_data("generic", "0_key_integer", "integer", "123");

                test_gate_5->set_data("generic", "0_key_invalid", "invalid", "ignore_me"); // Should be ignored
                test_gate_5->set_data("generic", "1_key_bit_vector", "bit_value", "1");

                test_gate_6->set_data("generic", "0_key_bit_value", "bit_value", "10101100");
                test_gate_6->set_data("generic", "1_key_bit_value", "bit_value", "01010011");

                // Write and parse the netlist
                std::stringstream parser_input;
                HDLWriterVerilog verilog_writer;

                // Writes the netlist in the sstream
                bool writer_suc = verilog_writer.write(nl.get(), parser_input);

                ASSERT_TRUE(writer_suc);

                HDLParserVerilog verilog_parser;

                // Parse the .verilog file
                std::unique_ptr<Netlist> parsed_nl = verilog_parser.parse_and_instantiate(parser_input, m_gl);

                ASSERT_NE(parsed_nl, nullptr);

                // -- Verilog Specific --
                // Since bit_vectors and bit_values are stored in the same format and both parsed as bit_vectors,
                // the data must be changed in the original netlist for comparison
                test_gate_6->set_data("generic", "0_key_bit_value", "bit_vector", "ac");
                test_gate_6->set_data("generic", "1_key_bit_value", "bit_vector", "53");

                // Since boolean values are written and parsed as integervalues (0 and 1),
                // the data must be changed in the original netlist for comparison
                test_gate_2->set_data("generic", "1_key_boolean", "integer", "1");
                test_gate_3->set_data("generic", "0_key_boolean", "integer", "1");

                // Check if the data is written/parsed correctly
                Gate* p_test_gate_0 = test_utils::get_gate_by_subname(parsed_nl.get(), "test_gate_0");
                ASSERT_NE(p_test_gate_0, nullptr);
                EXPECT_EQ(p_test_gate_0->get_data_map(), test_gate_0->get_data_map());

                Gate* p_test_gate_1 = test_utils::get_gate_by_subname(parsed_nl.get(), "test_gate_1");
                ASSERT_NE(p_test_gate_1, nullptr);
                EXPECT_EQ(p_test_gate_1->get_data_map(), test_gate_1->get_data_map());

                Gate* p_test_gate_2 = test_utils::get_gate_by_subname(parsed_nl.get(), "test_gate_2");
                ASSERT_NE(p_test_gate_2, nullptr);
                EXPECT_EQ(p_test_gate_2->get_data_map(), test_gate_2->get_data_map());

                Gate* p_test_gate_3 = test_utils::get_gate_by_subname(parsed_nl.get(), "test_gate_3");
                ASSERT_NE(p_test_gate_3, nullptr);
                EXPECT_EQ(p_test_gate_3->get_data_map(), test_gate_3->get_data_map());

                Gate* p_test_gate_4 = test_utils::get_gate_by_subname(parsed_nl.get(), "test_gate_4");
                ASSERT_NE(p_test_gate_4, nullptr);
                EXPECT_EQ(p_test_gate_4->get_data_map(), test_gate_4->get_data_map());

                Gate* p_test_gate_5 = test_utils::get_gate_by_subname(parsed_nl.get(), "test_gate_5");
                ASSERT_NE(p_test_gate_5, nullptr);
                auto test_gate_5_data_without_invalid = test_gate_5->get_data_map();
                test_gate_5_data_without_invalid.erase(std::make_tuple("generic", "0_key_invalid"));
                EXPECT_EQ(p_test_gate_5->get_data_map(), test_gate_5_data_without_invalid);

                Gate* p_test_gate_6 = test_utils::get_gate_by_subname(parsed_nl.get(), "test_gate_6");
                ASSERT_NE(p_test_gate_6, nullptr);
                EXPECT_EQ(p_test_gate_6->get_data_map(), test_gate_6->get_data_map());
            }
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
                std::unique_ptr<Netlist> nl = test_utils::create_empty_netlist(0);

                Net* global_in = nl->create_net(test_utils::MIN_NET_ID + 0, "123");
                nl->mark_global_input_net(global_in);

                // Write and parse the netlist
                std::stringstream parser_input;
                HDLWriterVerilog verilog_writer;

                // Writes the netlist in the sstream
                bool writer_suc = verilog_writer.write(nl.get(), parser_input);

                ASSERT_TRUE(writer_suc);

                HDLParserVerilog verilog_parser;

                // Parse the .verilog file
                std::unique_ptr<Netlist> parsed_nl = verilog_parser.parse_and_instantiate(parser_input, m_gl);

                ASSERT_NE(parsed_nl, nullptr);

                // Check if the net_name is written/parsed correctly
                ASSERT_EQ(parsed_nl->get_nets().size(), (size_t) 1);
                Net* p_global_in = *parsed_nl->get_nets().begin();
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
                std::unique_ptr<Netlist> nl = test_utils::create_empty_netlist(0);

                Net* bracket_net = nl->create_net(test_utils::MIN_NET_ID + 0, "net(0)");
                Net* comma_net = nl->create_net(test_utils::MIN_NET_ID + 1, "net,1");
                Net* comma_space_net = nl->create_net(test_utils::MIN_NET_ID + 2, "net, 2");
                Net* slash_net = nl->create_net(test_utils::MIN_NET_ID + 3, "net/_3");
                Net* backslash_net = nl->create_net(test_utils::MIN_NET_ID + 4, "\\net\\_4");
                Net* curly_bracket_net = nl->create_net(test_utils::MIN_NET_ID + 5, "net[5]");
                Net* angle_bracket_net = nl->create_net(test_utils::MIN_NET_ID + 6, "net<6>");
                Net* double_underscore_net = nl->create_net(test_utils::MIN_NET_ID + 7, "net__7");
                Net* edges_underscore_net = nl->create_net(test_utils::MIN_NET_ID + 8, "_net_8_");
                Net*
                    digit_only_net = nl->create_net(test_utils::MIN_NET_ID + 9, "9"); // should be converted to NET_9

                // Connect the nets to dummy gates (nets must be connected, else they are removed)
                Gate* dummy_gate_l_0 =
                    nl->create_gate(MIN_GATE_ID + 0, test_utils::get_gate_type_by_name("gate_8_to_8"), "l_0");
                Gate* dummy_gate_l_1 =
                    nl->create_gate(MIN_GATE_ID + 1, test_utils::get_gate_type_by_name("gate_8_to_8"), "l_1");
                Gate* dummy_gate_r_0 =
                    nl->create_gate(MIN_GATE_ID + 2, test_utils::get_gate_type_by_name("gate_8_to_8"), "r_0");
                Gate* dummy_gate_r_1 =
                    nl->create_gate(MIN_GATE_ID + 3, test_utils::get_gate_type_by_name("gate_8_to_8"), "r_1");

                std::vector<Net*> all_nets =
                    {bracket_net, comma_net, comma_space_net, slash_net, backslash_net, curly_bracket_net,
                     curly_bracket_net,
                     angle_bracket_net, double_underscore_net, edges_underscore_net, digit_only_net};
                std::vector<Gate*> dummy_gates_l = {dummy_gate_l_0, dummy_gate_l_1};
                std::vector<Gate*> dummy_gates_r = {dummy_gate_r_0, dummy_gate_r_1};
                // Connect the nets in a loop
                for (size_t i = 0; i < all_nets.size(); i++) {
                    all_nets[i]->add_source(dummy_gates_l[i / 8], "O" + std::to_string(i % 8));
                    all_nets[i]->add_destination(dummy_gates_r[i / 8], "I" + std::to_string(i % 8));
                }

                // Write and parse the netlist
                std::stringstream parser_input;
                HDLWriterVerilog verilog_writer;

                // Writes the netlist in the sstream
                bool writer_suc = verilog_writer.write(nl.get(), parser_input);

                ASSERT_TRUE(writer_suc);

                HDLParserVerilog verilog_parser;
                // Parse the .verilog file
                std::unique_ptr<Netlist> parsed_nl = verilog_parser.parse_and_instantiate(parser_input, m_gl);

                ASSERT_NE(parsed_nl, nullptr);

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
                EXPECT_FALSE(parsed_nl->get_nets(test_utils::net_name_filter("NET_9")).empty());
            }
            {
                // Testing the handling of special Gate names
                std::unique_ptr<Netlist> nl = test_utils::create_empty_netlist(0);
                GateLibrary* gl = m_gl;

                // Create various gates with special Gate name characters
                Gate*
                    bracket_gate =
                    nl->create_gate(test_utils::MIN_GATE_ID + 0, gl->get_gate_types().at("gate_1_to_1"), "gate(0)");
                Gate*
                    comma_gate =
                    nl->create_gate(test_utils::MIN_GATE_ID + 1, gl->get_gate_types().at("gate_1_to_1"), "gate,1");
                Gate*
                    comma_space_gate =
                    nl->create_gate(test_utils::MIN_GATE_ID + 2, gl->get_gate_types().at("gate_1_to_1"), "gate, 2");
                Gate*
                    slash_gate =
                    nl->create_gate(test_utils::MIN_GATE_ID + 3, gl->get_gate_types().at("gate_1_to_1"), "gate/_3");
                Gate*
                    backslash_gate =
                    nl->create_gate(test_utils::MIN_GATE_ID + 4, gl->get_gate_types().at("gate_1_to_1"), "\\gate\\_4");
                Gate*
                    curly_bracket_gate =
                    nl->create_gate(test_utils::MIN_GATE_ID + 5, gl->get_gate_types().at("gate_1_to_1"), "gate[5]");
                Gate*
                    angle_bracket_gate =
                    nl->create_gate(test_utils::MIN_GATE_ID + 6, gl->get_gate_types().at("gate_1_to_1"), "gate<6>");
                Gate* double_underscore_gate =
                    nl->create_gate(test_utils::MIN_GATE_ID + 7, gl->get_gate_types().at("gate_1_to_1"), "gate__7");
                Gate* edges_underscore_gate =
                    nl->create_gate(test_utils::MIN_GATE_ID + 8, gl->get_gate_types().at("gate_1_to_1"), "_gate_8_");
                Gate* digit_only_gate = nl->create_gate(test_utils::MIN_GATE_ID + 9,
                                                        gl->get_gate_types().at("gate_1_to_1"),
                                                        "9"); // should be converted to GATE_9

                // Create output nets for all gates to create a valid netlist
                unsigned int idx = 0;
                for (auto g : nl->get_gates()) {
                    Net* out_net = nl->create_net("net_" + std::to_string(idx));
                    out_net->add_source(g, "O");
                    idx++;
                }

                // Write and parse the netlist
                std::stringstream parser_input;
                HDLWriterVerilog verilog_writer;

                // Writes the netlist in the sstream
                bool writer_suc = verilog_writer.write(nl.get(), parser_input);

                ASSERT_TRUE(writer_suc);

                HDLParserVerilog verilog_parser;
                // Parse the .verilog file
                std::unique_ptr<Netlist> parsed_nl = verilog_parser.parse_and_instantiate(parser_input, m_gl);

                ASSERT_NE(parsed_nl, nullptr);

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
                std::unique_ptr<Netlist> nl = test_utils::create_empty_netlist(0);

                Net* test_net = nl->create_net(test_utils::MIN_NET_ID + 0, "gate_net_name");
                Gate*
                    test_gate = nl->create_gate(test_utils::MIN_GATE_ID + 0,
                                                test_utils::get_gate_type_by_name("gate_1_to_1"),
                                                "gate_net_name");

                test_net->add_destination(test_gate, "I");

                // Write and parse the netlist now
                std::stringstream parser_input;
                HDLWriterVerilog verilog_writer;

                // Writes the netlist in the sstream
                bool writer_suc = verilog_writer.write(nl.get(), parser_input);

                ASSERT_TRUE(writer_suc);

                HDLParserVerilog verilog_parser;

                // Parse the .verilog file
                std::unique_ptr<Netlist> parsed_nl = verilog_parser.parse_and_instantiate(parser_input, m_gl);

                ASSERT_NE(parsed_nl, nullptr);

                // Check if the Gate name was added a "_inst"
                EXPECT_NE(test_utils::get_net_by_subname(parsed_nl.get(), "gate_net_name"), nullptr);
                EXPECT_NE(test_utils::get_gate_by_subname(parsed_nl.get(), "gate_net_name_inst"), nullptr);

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
            {
                // Two nets ('0'/'1')  are connected with a test_gate. For these net names, a global_gnd/global_vcc gate
                // should be created.
                std::unique_ptr<Netlist> nl = test_utils::create_empty_netlist(0);

                // Add the gate
                Gate* test_gate =
                    nl->create_gate(MIN_GATE_ID + 2, test_utils::get_gate_type_by_name("gate_2_to_1"), "test_gate");

                // Add and connect the nets
                Net* zero_net = nl->create_net(MIN_NET_ID + 0, "'0'");
                Net* one_net = nl->create_net(MIN_NET_ID + 1, "'1'");

                //zero_net->add_source(gnd_gate, "O");
                zero_net->add_destination(test_gate, "I0");

                //one_net->add_source(vcc_gate, "O");
                one_net->add_destination(test_gate, "I1");

                // Write and parse the netlist
                std::stringstream parser_input;
                HDLWriterVerilog verilog_writer;

                // Writes the netlist in the sstream
                bool writer_suc = verilog_writer.write(nl.get(), parser_input);
                ASSERT_TRUE(writer_suc);

                HDLParserVerilog verilog_parser;
                // Parse the .vhdl file
                std::unique_ptr<Netlist> parsed_nl = verilog_parser.parse_and_instantiate(parser_input, m_gl);

                ASSERT_NE(parsed_nl, nullptr);
                Gate* global_gnd = test_utils::get_gate_by_subname(parsed_nl.get(), "global_gnd");
                Gate* global_vcc = test_utils::get_gate_by_subname(parsed_nl.get(), "global_vcc");
                ASSERT_NE(global_gnd, nullptr);
                ASSERT_NE(global_vcc, nullptr);
                ASSERT_NE(global_gnd->get_fan_out_net("O"), nullptr);
                ASSERT_NE(global_vcc->get_fan_out_net("O"), nullptr);
                EXPECT_EQ(global_gnd->get_fan_out_net("O")->get_name(), "'0'");
                EXPECT_EQ(global_vcc->get_fan_out_net("O")->get_name(), "'1'");
                EXPECT_FALSE(global_gnd->get_fan_out_net("O")
                                 ->get_destinations(test_utils::endpoint_pin_type_filter("I0")).empty());
                EXPECT_FALSE(global_vcc->get_fan_out_net("O")
                                 ->get_destinations(test_utils::endpoint_pin_type_filter("I1")).empty());
            }
        TEST_END
    }

    /**
     * Testing the correct handling of pin groups (e.g. I(0), I(1), I(2), I(3)).
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

                std::unique_ptr<Netlist> nl(new Netlist(m_gl));
                nl->set_design_name("design_name");

                Gate*
                    gnd_gate = nl->create_gate(MIN_GATE_ID + 0, test_utils::get_gate_type_by_name("gnd"), "gnd_gate");
                Gate*
                    vcc_gate = nl->create_gate(MIN_GATE_ID + 1, test_utils::get_gate_type_by_name("vcc"), "vcc_gate");
                Net* global_out = nl->create_net(MIN_NET_ID + 0, "global_out");
                nl->mark_global_output_net(global_out);
                Gate* test_gate = nl->create_gate(MIN_GATE_ID + 2,
                                                  test_utils::get_gate_type_by_name(
                                                      "pin_group_gate_4_to_4"),
                                                  "test_gate");
                global_out->add_source(test_gate, "O(0)");

                Net* gnd_net = nl->create_net(MIN_NET_ID + 2, "test_gnd_net");
                Net* vcc_net = nl->create_net(MIN_NET_ID + 3, "test_vcc_net");

                gnd_net->add_source(gnd_gate, "O");
                vcc_net->add_source(vcc_gate, "O");

                gnd_net->add_destination(test_gate, "I(0)");
                vcc_net->add_destination(test_gate, "I(1)");
                gnd_net->add_destination(test_gate, "I(2)");
                vcc_net->add_destination(test_gate, "I(3)");


                // Write and parse the netlist
                std::stringstream parser_input;
                HDLWriterVerilog verilog_writer;

                // Writes the netlist in the sstream
                bool writer_suc = verilog_writer.write(nl.get(), parser_input);

                ASSERT_TRUE(writer_suc);

                HDLParserVerilog verilog_parser;

                // Parse the .verilog file
                std::unique_ptr<Netlist> parsed_nl = verilog_parser.parse_and_instantiate(parser_input, m_gl);

                ASSERT_NE(parsed_nl, nullptr);

                ASSERT_EQ(parsed_nl->get_nets(test_utils::net_name_filter("test_gnd_net")).size(), 1);
                ASSERT_EQ(parsed_nl->get_nets(test_utils::net_name_filter("test_vcc_net")).size(), 1);
                Net*
                    test_gnd_net_ref = *parsed_nl->get_nets(test_utils::net_name_filter("test_gnd_net")).begin();
                Net*
                    test_vcc_net_ref = *parsed_nl->get_nets(test_utils::net_name_filter("test_vcc_net")).begin();

                ASSERT_EQ(parsed_nl->get_gates(test_utils::gate_type_filter("pin_group_gate_4_to_4")).size(), 1);
                Gate* test_gate_ref =
                    *parsed_nl->get_gates(test_utils::gate_type_filter("pin_group_gate_4_to_4")).begin();
                //EXPECT_EQ(test_gate_ref->get_fan_in_net("I(0)"), test_gnd_net_ref);
                //EXPECT_EQ(test_gate_ref->get_fan_in_net("I(1)"), test_vcc_net_ref);
                //EXPECT_EQ(test_gate_ref->get_fan_in_net("I(2)"), test_gnd_net_ref);
                //EXPECT_EQ(test_gate_ref->get_fan_in_net("I(3)"), test_vcc_net_ref); // ISSUE: wrong order (verilog writer: l.557 useless?)

            }
        TEST_END
    }

} //namespace hal
