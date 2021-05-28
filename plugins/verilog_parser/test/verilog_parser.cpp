#include "verilog_parser/verilog_parser.h"

#include "netlist_test_utils.h"
#include "gate_library_test_utils.h"

#include <bitset>
#include <experimental/filesystem>

namespace hal {

    class VerilogParserTest : public ::testing::Test {
    protected:
        GateLibrary* m_gl;

        virtual void SetUp() {
            NO_COUT_BLOCK;
            test_utils::init_log_channels();
            test_utils::create_sandbox_directory();
            m_gl = test_utils::get_testing_gate_library();
        }

        virtual void TearDown() {
        }
    };

    /*                                    net_0
     *                  .--= INV (0) =------.
     *  net_global_in   |                   '-=               net_global_out
     *      ------------|                   .-= AND3 (2) = ----------
     *                  |                   | =
     *                  +--=                |
     *                  |    AND2 (1) =-----'
     *                  '--=              net_1
     */
    /**
     * Testing the correct usage of the verilog parser by parse a small verilog-format string, which describes the netlist
     * shown above.
     *
     * Functions: parse
     */
    TEST_F(VerilogParserTest, check_main_example) 
    {
        TEST_START
        {
            std::string netlist_input("module top ("
                                    "  net_global_in,"
                                    "  net_global_out "
                                    " ) ;"
                                    "  input net_global_in ;"
                                    "  output net_global_out ;"
                                    "  wire net_0 ;"
                                    "  wire net_1 ;"
                                    "gate_1_to_1 gate_0 ("
                                    "  .I (net_global_in ),"
                                    "  .O (net_0 )"
                                    " ) ;"
                                    "gate_2_to_1 gate_1 ("
                                    "  .I0 (net_global_in ),"
                                    "  .I1 (net_global_in ),"
                                    "  .O (net_1 )"
                                    " ) ;"
                                    "gate_3_to_1 gate_2 ("
                                    "  .I0 (net_0 ),"
                                    "  .I1 (net_1 ),"
                                    "  .O (net_global_out )"
                                    " ) ;"
                                    "endmodule");
            std::filesystem::path verilog_file = test_utils::create_sandbox_file("netlist.v", netlist_input);
            VerilogParser verilog_parser;
            std::unique_ptr<Netlist> nl = verilog_parser.parse_and_instantiate(verilog_file, m_gl);
            ASSERT_NE(nl, nullptr);

            // Check if the gates are parsed correctly
            ASSERT_EQ(nl->get_gates(test_utils::gate_type_filter("gate_1_to_1")).size(), 1);
            Gate* gate_0 = *(nl->get_gates(test_utils::gate_type_filter("gate_1_to_1")).begin());
            ASSERT_EQ(nl->get_gates(test_utils::gate_type_filter("gate_2_to_1")).size(), 1);
            Gate* gate_1 = *(nl->get_gates(test_utils::gate_type_filter("gate_2_to_1")).begin());
            ASSERT_EQ(nl->get_gates(test_utils::gate_type_filter("gate_3_to_1")).size(), 1);
            Gate* gate_2 = *(nl->get_gates(test_utils::gate_type_filter("gate_3_to_1")).begin());

            ASSERT_NE(gate_0, nullptr);
            EXPECT_EQ(gate_0->get_name(), "gate_0");

            ASSERT_NE(gate_1, nullptr);
            EXPECT_EQ(gate_1->get_name(), "gate_1");

            ASSERT_NE(gate_2, nullptr);
            EXPECT_EQ(gate_2->get_name(), "gate_2");

            // Check if the nets are parsed correctly
            Net* net_0 = *(nl->get_nets(test_utils::net_name_filter("net_0")).begin());
            Net* net_1 = *(nl->get_nets(test_utils::net_name_filter("net_1")).begin());
            Net*
                net_global_in = *(nl->get_nets(test_utils::net_name_filter("net_global_in")).begin());
            Net*
                net_global_out = *(nl->get_nets(test_utils::net_name_filter("net_global_out")).begin());

            ASSERT_NE(net_0, nullptr);
            EXPECT_EQ(net_0->get_name(), "net_0");
            ASSERT_EQ(net_0->get_sources().size(), 1);
            EXPECT_EQ(net_0->get_sources()[0], test_utils::get_endpoint(gate_0, "O"));
            std::vector<Endpoint*> exp_net_0_dsts = {test_utils::get_endpoint(gate_2, "I0")};
            EXPECT_TRUE(test_utils::vectors_have_same_content(net_0->get_destinations(),
                                                                std::vector<Endpoint*>({test_utils::get_endpoint(gate_2,
                                                                                                                "I0")})));

            ASSERT_NE(net_1, nullptr);
            EXPECT_EQ(net_1->get_name(), "net_1");
            ASSERT_EQ(net_1->get_sources().size(), 1);
            EXPECT_EQ(net_1->get_sources()[0], test_utils::get_endpoint(gate_1, "O"));
            EXPECT_TRUE(test_utils::vectors_have_same_content(net_1->get_destinations(),
                                                                std::vector<Endpoint*>({test_utils::get_endpoint(gate_2,
                                                                                                                "I1")})));

            ASSERT_NE(net_global_in, nullptr);
            EXPECT_EQ(net_global_in->get_name(), "net_global_in");
            EXPECT_EQ(net_global_in->get_sources().size(), 0);
            EXPECT_TRUE(test_utils::vectors_have_same_content(net_global_in->get_destinations(),
                                                                std::vector<Endpoint*>({test_utils::get_endpoint(gate_0,
                                                                                                                "I"),
                                                                                        test_utils::get_endpoint(gate_1,
                                                                                                                "I0"),
                                                                                        test_utils::get_endpoint(gate_1,
                                                                                                                "I1")})));
            EXPECT_TRUE(nl->is_global_input_net(net_global_in));

            ASSERT_NE(net_global_out, nullptr);
            EXPECT_EQ(net_global_out->get_name(), "net_global_out");
            ASSERT_EQ(net_global_out->get_sources().size(), 1);
            EXPECT_EQ(net_global_out->get_sources()[0], test_utils::get_endpoint(gate_2, "O"));
            EXPECT_TRUE(net_global_out->get_destinations().empty());
            EXPECT_TRUE(nl->is_global_output_net(net_global_out));

            EXPECT_EQ(nl->get_global_input_nets().size(), 1);
            EXPECT_EQ(nl->get_global_output_nets().size(), 1);
        }
        TEST_END
    }

    /**
     * The same test, as the main example, but use white spaces of different types (' ','\n','\t') in various locations (or remove some unnecessary ones)
     *
     * Functions: parse
     */
    TEST_F(VerilogParserTest, check_whitespace_chaos) 
    {
        TEST_START
        {
            std::string netlist_input("module top(net_global_in,net_global_out);input net_global_in;\n"
                                    "  output net_global_out;wire net_0;wire net_1;gate_1_to_1 gate_0 (\n"
                                    "  .I (net_global_in),\n"
                                    "\n"
                                    " \t.O (net_0 )\n"
                                    " );\n"
                                    "gate_2_to_1 gate_1 (.I0\n\t"
                                    "  (\n"
                                    "    net_global_in\n"
                                    "   \t)\n"
                                    "    ,\n"
                                    "  .I1 (net_global_in),.O (net_1));\n"
                                    "gate_3_to_1 gate_2 (.I0 (net_0 ),.I1 (net_1 ),.O (net_global_out ));endmodule");
            std::filesystem::path verilog_file = test_utils::create_sandbox_file("netlist.v", netlist_input);
            VerilogParser verilog_parser;
            std::unique_ptr<Netlist> nl = verilog_parser.parse_and_instantiate(verilog_file, m_gl);
            ASSERT_NE(nl, nullptr);

            // Check if the gates are parsed correctly
            ASSERT_EQ(nl->get_gates(test_utils::gate_type_filter("gate_1_to_1")).size(), 1);
            Gate* gate_0 = *(nl->get_gates(test_utils::gate_type_filter("gate_1_to_1")).begin());
            ASSERT_EQ(nl->get_gates(test_utils::gate_type_filter("gate_2_to_1")).size(), 1);
            Gate* gate_1 = *(nl->get_gates(test_utils::gate_type_filter("gate_2_to_1")).begin());
            ASSERT_EQ(nl->get_gates(test_utils::gate_type_filter("gate_3_to_1")).size(), 1);
            Gate* gate_2 = *(nl->get_gates(test_utils::gate_type_filter("gate_3_to_1")).begin());

            ASSERT_NE(gate_0, nullptr);
            EXPECT_EQ(gate_0->get_name(), "gate_0");

            ASSERT_NE(gate_1, nullptr);
            EXPECT_EQ(gate_1->get_name(), "gate_1");

            ASSERT_NE(gate_2, nullptr);
            EXPECT_EQ(gate_2->get_name(), "gate_2");

            // Check if the nets are parsed correctly
            Net* net_0 = *(nl->get_nets(test_utils::net_name_filter("net_0")).begin());
            Net* net_1 = *(nl->get_nets(test_utils::net_name_filter("net_1")).begin());
            Net* net_global_in = *(nl->get_nets(test_utils::net_name_filter("net_global_in")).begin());
            Net* net_global_out = *(nl->get_nets(test_utils::net_name_filter("net_global_out")).begin());

            ASSERT_NE(net_0, nullptr);
            EXPECT_EQ(net_0->get_name(), "net_0");
            ASSERT_EQ(net_0->get_sources().size(), 1);
            EXPECT_EQ(net_0->get_sources()[0], test_utils::get_endpoint(gate_0, "O"));
            std::vector<Endpoint*> exp_net_0_dsts = {test_utils::get_endpoint(gate_2, "I0")};
            EXPECT_TRUE(test_utils::vectors_have_same_content(net_0->get_destinations(), std::vector<Endpoint*>({test_utils::get_endpoint(gate_2, "I0")})));

            ASSERT_NE(net_1, nullptr);
            EXPECT_EQ(net_1->get_name(), "net_1");
            ASSERT_EQ(net_1->get_sources().size(), 1);
            EXPECT_EQ(net_1->get_sources()[0], test_utils::get_endpoint(gate_1, "O"));
            EXPECT_TRUE(test_utils::vectors_have_same_content(net_1->get_destinations(), std::vector<Endpoint*>({test_utils::get_endpoint(gate_2, "I1")})));

            ASSERT_NE(net_global_in, nullptr);
            EXPECT_EQ(net_global_in->get_name(), "net_global_in");
            EXPECT_EQ(net_global_in->get_sources().size(), 0);
            EXPECT_TRUE(test_utils::vectors_have_same_content(net_global_in->get_destinations(), std::vector<Endpoint*>({test_utils::get_endpoint(gate_0, "I"),
                                                                                                                            test_utils::get_endpoint(gate_1, "I0"),
                                                                                                                            test_utils::get_endpoint(gate_1, "I1")})));
            EXPECT_TRUE(nl->is_global_input_net(net_global_in));

            ASSERT_NE(net_global_out, nullptr);
            EXPECT_EQ(net_global_out->get_name(), "net_global_out");
            ASSERT_EQ(net_global_out->get_sources().size(), 1);
            EXPECT_EQ(net_global_out->get_sources()[0], test_utils::get_endpoint(gate_2, "O"));
            EXPECT_TRUE(net_global_out->get_destinations().empty());
            EXPECT_TRUE(nl->is_global_output_net(net_global_out));

            EXPECT_EQ(nl->get_global_input_nets().size(), 1);
            EXPECT_EQ(nl->get_global_output_nets().size(), 1);
        }
        TEST_END
    }

    /**
     * Testing the correct storage of data of the following data types:
     * integer, floating_point, string, bit_vector (hexadecimal, decimal, octal, binary)
     *
     * Functions: parse
     */
    TEST_F(VerilogParserTest, check_generic_map) 
    {
        TEST_START
        {
            // Store an instance of all possible data types in one Gate + some special cases
            std::string netlist_input("module top ("
                                    "  global_in,"
                                    "  global_out"
                                    " ) ;"
                                    "  input global_in ;"
                                    "  output global_out ;"
                                    "gate_1_to_1 #("
                                    ".key_integer(1234),"
                                    ".key_floating_point(1.234),"
                                    ".key_string(\"test_string\"),"
                                    ".key_bit_vector_hex('habc),"    // All values are 'ABC' in hex
                                    ".key_bit_vector_dec('d2748),"
                                    ".key_bit_vector_oct('o5274),"
                                    ".key_bit_vector_bin('b1010_1011_1100),"
                                    ".key_negative_comma_string(\"test,1,2,3\"),"
                                    ".key_negative_float_string(\"1.234\")) "
                                    "gate_0 ("
                                    "  .I (global_in ),"
                                    "  .O (global_out )"
                                    " ) ;"
                                    "endmodule");
            std::filesystem::path verilog_file = test_utils::create_sandbox_file("netlist.v", netlist_input);
            VerilogParser verilog_parser;
            std::unique_ptr<Netlist> nl = verilog_parser.parse_and_instantiate(verilog_file, m_gl);
            ASSERT_NE(nl, nullptr);

            ASSERT_EQ(nl->get_gates(test_utils::gate_filter("gate_1_to_1", "gate_0")).size(), 1);
            Gate* gate_0 = *nl->get_gates(test_utils::gate_filter("gate_1_to_1", "gate_0")).begin();

            // Integers are stored in their hex representation
            EXPECT_EQ(gate_0->get_data("generic", "key_integer"), std::make_tuple("integer", "1234"));
            EXPECT_EQ(gate_0->get_data("generic", "key_floating_point"), std::make_tuple("floating_point", "1.234"));
            EXPECT_EQ(gate_0->get_data("generic", "key_string"), std::make_tuple("string", "test_string"));
            EXPECT_EQ(gate_0->get_data("generic", "key_bit_vector_hex"), std::make_tuple("bit_vector", "ABC"));
            EXPECT_EQ(gate_0->get_data("generic", "key_bit_vector_dec"), std::make_tuple("bit_vector", "ABC"));
            EXPECT_EQ(gate_0->get_data("generic", "key_bit_vector_oct"), std::make_tuple("bit_vector", "ABC"));
            EXPECT_EQ(gate_0->get_data("generic", "key_bit_vector_bin"), std::make_tuple("bit_vector", "ABC"));

            // Special Characters
            EXPECT_EQ(gate_0->get_data("generic", "key_negative_comma_string"), std::make_tuple("string", "test,1,2,3"));
            EXPECT_EQ(gate_0->get_data("generic", "key_negative_float_string"), std::make_tuple("string", "1.234"));
        }
        TEST_END
    }

    /**
     * Testing the handling of Net-vectors in dimension 1-3
     *
     * Functions: parse
     */
    TEST_F(VerilogParserTest, check_net_vectors) {

        TEST_START
            {
                // Use two logic vectors with dimension 1. One is declared with ascending indices, the other with
                // descending indices. ([0:3] and [3:0])
                /*
                 *                           n_vec_1              n_vec_2
                 *                        =-----------=        =-----------=
                 *                        =-----------=        =-----------=
                 *  global_in ---= gate_0 =-----------= gate_1 =-----------= gate_2 =--- global_out
                 *                        =-----------=        =-----------=
                 *
                 */

                std::string netlist_input("module top ( "
                                        "  global_in, "
                                        "  global_out "
                                        " ) ; "
                                        "  input global_in ; "
                                        "  output global_out ; "
                                        "  wire [0:3] n_vec_1 ; "
                                        "  wire [3:0] n_vec_2 ; "
                                        " gate_1_to_4 gate_0 ( "
                                        "  .I (global_in ), "
                                        "  .O0 (n_vec_1[0]), "
                                        "  .O1 (n_vec_1[1]), "
                                        "  .O2 (n_vec_1[2]), "
                                        "  .O3 (n_vec_1[3]) "
                                        " ) ; "
                                        " gate_4_to_4 gate_1 ( "
                                        "   .I0 (n_vec_1[0]), "
                                        "   .I1 (n_vec_1[1]), "
                                        "   .I2 (n_vec_1[2]), "
                                        "   .I3 (n_vec_1[3]), "
                                        "   .O0 (n_vec_2[0]), "
                                        "   .O1 (n_vec_2[1]), "
                                        "   .O2 (n_vec_2[2]), "
                                        "   .O3 (n_vec_2[3]) "
                                        "  ) ; "
                                        "  gate_4_to_1 gate_2 ( "
                                        "    .I0 (n_vec_2[0]), "
                                        "    .I1 (n_vec_2[1]), "
                                        "    .I2 (n_vec_2[2]), "
                                        "    .I3 (n_vec_2[3]), "
                                        "    .O (global_out ) "
                                        "   ) ; "
                                        "endmodule");
                std::filesystem::path verilog_file = test_utils::create_sandbox_file("netlist.v", netlist_input);
                VerilogParser verilog_parser;
                std::unique_ptr<Netlist> nl = verilog_parser.parse_and_instantiate(verilog_file, m_gl);
                ASSERT_NE(nl, nullptr);

                // Check that all nets are created and connected correctly
                EXPECT_EQ(nl->get_nets().size(),
                          10);    // net_global_in + net_global_out + 4 nets in n_vec_1 + 4 nets in n_vec_2
                for (auto net_name : std::set<std::string>({"n_vec_1(0)", "n_vec_1(1)", "n_vec_1(2)", "n_vec_1(3)",
                                                            "n_vec_2(0)", "n_vec_2(1)", "n_vec_2(2)", "n_vec_2(3)"})) {
                    ASSERT_EQ(nl->get_nets(test_utils::net_name_filter(net_name)).size(), 1);
                }
                for (unsigned i = 0; i < 4; i++) {
                    std::string i_str = std::to_string(i);
                    Net* n_vec_1_i = *nl->get_nets(test_utils::net_name_filter("n_vec_1(" + i_str + ")")).begin();
                    Net* n_vec_2_i = *nl->get_nets(test_utils::net_name_filter("n_vec_2(" + i_str + ")")).begin();
                    ASSERT_EQ(n_vec_1_i->get_sources().size(), 1);
                    EXPECT_EQ(n_vec_1_i->get_sources()[0]->get_pin(), "O" + i_str);
                    EXPECT_EQ((*n_vec_1_i->get_destinations().begin())->get_pin(), "I" + i_str);
                    ASSERT_EQ(n_vec_2_i->get_sources().size(), 1);
                    EXPECT_EQ(n_vec_2_i->get_sources()[0]->get_pin(), "O" + i_str);
                    EXPECT_EQ((*n_vec_2_i->get_destinations().begin())->get_pin(), "I" + i_str);
                }
            }
            {
                // Use a logic vector of dimension two
                std::string netlist_input("module top ( "
                                        "  global_in, "
                                        "  global_out "
                                        " ) ; "
                                        "  input global_in ; "
                                        "  output global_out ; "
                                        "  wire [0:1][2:3] n_vec ; "
                                        "gate_1_to_4 gate_0 ( "
                                        "  .I (global_in ), "
                                        "  .O0 (n_vec[0][2]), "
                                        "  .O1 (n_vec[0][3]), "
                                        "  .O2 (n_vec[1][2]), "
                                        "  .O3 (n_vec[1][3]) "
                                        " ) ; "
                                        "gate_4_to_1 gate_1 ( "
                                        "  .I0 (n_vec[0][2]), "
                                        "  .I1 (n_vec[0][3]), "
                                        "  .I2 (n_vec[1][2]), "
                                        "  .I3 (n_vec[1][3]), "
                                        "  .O (global_out ) "
                                        " ) ; "
                                        "endmodule");
                std::filesystem::path verilog_file = test_utils::create_sandbox_file("netlist.v", netlist_input);
                VerilogParser verilog_parser;
                std::unique_ptr<Netlist> nl = verilog_parser.parse_and_instantiate(verilog_file, m_gl);
                ASSERT_NE(nl, nullptr);

                // Check that all nets are created and connected correctly
                EXPECT_EQ(nl->get_nets().size(), 6);    // net_global_in + global_out + 4 nets in n_vec
                unsigned pin = 0;
                for (auto n : std::vector<std::string>({"n_vec(0)(2)", "n_vec(0)(3)", "n_vec(1)(2)", "n_vec(1)(3)"})) {
                    ASSERT_FALSE(nl->get_nets(test_utils::net_name_filter(n)).empty());
                    Net* n_vec_i_j = *nl->get_nets(test_utils::net_name_filter(n)).begin();
                    ASSERT_EQ(n_vec_i_j->get_sources().size(), 1);
                    EXPECT_EQ(n_vec_i_j->get_sources()[0]->get_pin(), "O" + std::to_string(pin));
                    EXPECT_EQ((*n_vec_i_j->get_destinations().begin())->get_pin(), "I" + std::to_string(pin));
                    pin++;
                }
            }
            {
                // Use a Net vector of dimension three
                std::string netlist_input("module top ( "
                                        "  global_in, "
                                        "  global_out "
                                        " ) ; "
                                        "  input global_in ; "
                                        "  output global_out ; "
                                        "  wire [0:1][1:0][0:1] n_vec ; "
                                        "gate_1_to_8 gate_0 ( "
                                        "  .I (global_in ), "
                                        "  .O0 (n_vec[0][0][0]), "
                                        "  .O1 (n_vec[0][0][1]), "
                                        "  .O2 (n_vec[0][1][0]), "
                                        "  .O3 (n_vec[0][1][1]), "
                                        "  .O4 (n_vec[1][0][0]), "
                                        "  .O5 (n_vec[1][0][1]), "
                                        "  .O6 (n_vec[1][1][0]), "
                                        "  .O7 (n_vec[1][1][1]) "
                                        " ) ; "
                                        "gate_8_to_1 gate_1 ( "
                                        "  .I0 (n_vec[0][0][0]), "
                                        "  .I1 (n_vec[0][0][1]), "
                                        "  .I2 (n_vec[0][1][0]), "
                                        "  .I3 (n_vec[0][1][1]), "
                                        "  .I4 (n_vec[1][0][0]), "
                                        "  .I5 (n_vec[1][0][1]), "
                                        "  .I6 (n_vec[1][1][0]), "
                                        "  .I7 (n_vec[1][1][1]), "
                                        "  .O (global_out ) "
                                        " ) ; "
                                        "endmodule");
                std::filesystem::path verilog_file = test_utils::create_sandbox_file("netlist.v", netlist_input);
                VerilogParser verilog_parser;
                std::unique_ptr<Netlist> nl = verilog_parser.parse_and_instantiate(verilog_file, m_gl);
                ASSERT_NE(nl, nullptr);

                // Check that all nets are created and connected correctly
                EXPECT_EQ(nl->get_nets().size(), 10);    // net_global_in + net_global_out + 8 nets in n_vec
                std::vector<std::string> net_idx
                    ({"(0)(0)(0)", "(0)(0)(1)", "(0)(1)(0)", "(0)(1)(1)", "(1)(0)(0)", "(1)(0)(1)", "(1)(1)(0)",
                      "(1)(1)(1)"});
                for (size_t idx = 0; idx < 8; idx++) {
                    ASSERT_FALSE(nl->get_nets(test_utils::net_name_filter("n_vec" + net_idx[idx])).empty());
                    Net*
                        n_vec_i_j = *nl->get_nets(test_utils::net_name_filter("n_vec" + net_idx[idx])).begin();
                    ASSERT_EQ(n_vec_i_j->get_sources().size(), 1);
                    EXPECT_EQ(n_vec_i_j->get_sources()[0]->get_pin(), "O" + std::to_string(idx));
                    EXPECT_EQ((*n_vec_i_j->get_destinations().begin())->get_pin(), "I" + std::to_string(idx));
                }
            }
        TEST_END
    }

    /**
     * Test zero parsing and (implicit) zero padding.
     *
     * Functions: parse
     */
    TEST_F(VerilogParserTest, check_zero_padding) {

        TEST_START
            {
                const GateLibrary* gl = test_utils::get_gate_library();

                std::string netlist_input(
                                        "module top (); "
                                        "   wire [3:0] gate_0_in, gate_1_in;"
                                        "   RAM gate_0 ( "
                                        "       .DATA_IN (gate_0_in ), "
                                        "   ) ; "
                                        "   RAM gate_1 ( "
                                        "       .DATA_IN (gate_1_in ), "
                                        "   ) ; "
                                        "   assign gate_0_in = 4'd0;"
                                        "   assign gate_1_in = 1'd0;"
                                        "endmodule");
                
                std::filesystem::path verilog_file = test_utils::create_sandbox_file("netlist.v", netlist_input);
                VerilogParser verilog_parser;
                std::unique_ptr<Netlist> nl = verilog_parser.parse_and_instantiate(verilog_file, gl);
                ASSERT_NE(nl, nullptr);

                EXPECT_EQ(nl->get_gnd_gates().front()->get_successors().size(), 8);
                for (Gate* gate : nl->get_gates([](const Gate* gate) { return !gate->is_gnd_gate(); })) 
                {
                    EXPECT_EQ(gate->get_fan_in_endpoints().size(), 4);
                    for (Endpoint* ep : gate->get_fan_in_endpoints()) 
                    {
                        EXPECT_TRUE(ep->get_net()->is_gnd_net());
                    }
                }
            }
        TEST_END
    }

    /**
     * Testing assignment of '0', '1', 'Z', and 'X'.
     *
     * Functions: parse
     */
    TEST_F(VerilogParserTest, check_special_nets) {

        TEST_START
            {
                std::string netlist_input("module top ("
                                        "  global_out_0,"
                                        "  global_out_1,"
                                        "  global_out_2,"
                                        "  global_out_3 "
                                        " ) ;"
                                        "  output global_out_0 ;"
                                        "  output global_out_1 ;"
                                        "  output global_out_2 ;"
                                        "  output global_out_3 ;"
                                        "gate_1_to_1 gate_0 ("
                                        "  .I ('b0 ),"
                                        "  .O (global_out_0)"
                                        " ) ;"
                                        "gate_1_to_1 gate_1 ("
                                        "  .I ('b1 ),"
                                        "  .O (global_out_1)"
                                        " ) ;"
                                        "gate_1_to_1 gate_2 ("
                                        "  .I ('bZ ),"
                                        "  .O (global_out_2)"
                                        " ) ;"
                                        "gate_1_to_1 gate_3 ("
                                        "  .I ('bX ),"
                                        "  .O (global_out_3)"
                                        " ) ;"
                                        "endmodule");
                std::filesystem::path verilog_file = test_utils::create_sandbox_file("netlist.v", netlist_input);
                VerilogParser verilog_parser;
                std::unique_ptr<Netlist> nl = verilog_parser.parse_and_instantiate(verilog_file, m_gl);
                ASSERT_NE(nl, nullptr);

                Gate* gate_0 = *nl->get_gates(test_utils::gate_filter("gate_1_to_1", "gate_0")).begin();
                ASSERT_NE(gate_0, nullptr);
                Gate* gate_1 = *nl->get_gates(test_utils::gate_filter("gate_1_to_1", "gate_1")).begin();
                ASSERT_NE(gate_1, nullptr);
                Gate* gate_2 = *nl->get_gates(test_utils::gate_filter("gate_1_to_1", "gate_2")).begin();
                ASSERT_NE(gate_2, nullptr);
                Gate* gate_3 = *nl->get_gates(test_utils::gate_filter("gate_1_to_1", "gate_3")).begin();
                ASSERT_NE(gate_3, nullptr);

                // check whether net '0' was created and is connected to a GND gate through input pin "I"
                Net* net_gnd = gate_0->get_fan_in_net("I");
                ASSERT_NE(net_gnd, nullptr);
                EXPECT_EQ(net_gnd->get_name(), "'0'");
                ASSERT_EQ(net_gnd->get_sources().size(), 1);
                ASSERT_NE(net_gnd->get_sources()[0]->get_gate(), nullptr);
                EXPECT_TRUE(net_gnd->get_sources()[0]->get_gate()->is_gnd_gate());

                // check whether net '1' was created and is connected to a VCC gate through input pin "I"
                Net* net_vcc = gate_1->get_fan_in_net("I");
                ASSERT_NE(net_vcc, nullptr);
                EXPECT_EQ(net_vcc->get_name(), "'1'");
                ASSERT_EQ(net_vcc->get_sources().size(), 1);
                ASSERT_NE(net_vcc->get_sources()[0]->get_gate(), nullptr);
                EXPECT_TRUE(net_vcc->get_sources()[0]->get_gate()->is_vcc_gate());

                // check whether input pin "I" remains unconnected for 'Z' assignment
                ASSERT_EQ(gate_2->get_fan_in_net("I"), nullptr);

                // check whether input pin "I" remains unconnected for 'X' assignment
                ASSERT_EQ(gate_2->get_fan_in_net("I"), nullptr);
            }
        TEST_END
    }

    /**
     * Testing the usage of additional entities. Entities that are used by the main entity (the last one) are recursively
     * split in gates that are part of the Gate library, while the original entity hierarchy is represented by the Module-
     * hierarchy of the netlist. Therefore, there can be multiple gates with the same name, so names that occur twice or
     * more will be extended by a unique suffix.
     *
     * Functions: parse
     */
    TEST_F(VerilogParserTest, check_multiple_entities) 
    {
        TEST_START
        {
            // Create a new entity with an attribute that is used once by the main entity
            /*                               ---------------------------------------------.
            *                              | child_mod                                   |
            *                              |                                             |
            *  global_in ---=| gate_0 |=---=---=| gate_0_child |=---=| gate_1_child |=---=---=| gate_1 |=--- global_out
            *                              |                                             |
            *                              '---------------------------------------------'
            */
            std::string netlist_input("(* child_attribute = \"child_attribute_value\" *)"
                                    "module MODULE_CHILD ("
                                    "  child_in,"
                                    "  child_out"
                                    " ) ;"
                                    "  (* child_net_attribute = \"child_net_attribute_value\" *)"
                                    "  input child_in ;"
                                    "  output child_out ;"
                                    "  wire net_0_child ;"
                                    "gate_1_to_1 gate_0_child ("
                                    "  .I (child_in ),"
                                    "  .O (net_0_child )"
                                    " ) ;"
                                    "gate_1_to_1 gate_1_child ("
                                    "  .I (net_0_child ),"
                                    "  .O (child_out )"
                                    " ) ;"
                                    "endmodule"
                                    "\n"
                                    "module MODULE_TOP ("
                                    "  net_global_in,"
                                    "  net_global_out"
                                    " ) ;"
                                    "  input net_global_in ;"
                                    "  output net_global_out ;"
                                    "  wire net_0 ;"
                                    "  wire net_1 ;"
                                    "gate_1_to_1 gate_0 ("
                                    "  .I (net_global_in ),"
                                    "  .O (net_0 )"
                                    " ) ;"
                                    "MODULE_CHILD child_mod ("
                                    "  .\\child_in (net_0 ),"
                                    "  .\\child_out (net_1 )"
                                    " ) ;"
                                    "gate_1_to_1 gate_1 ("
                                    "  .I (net_1 ), "
                                    "  .O (net_global_out )"
                                    " ) ;"
                                    "endmodule");
            std::filesystem::path verilog_file = test_utils::create_sandbox_file("netlist.v", netlist_input);
            VerilogParser verilog_parser;
            std::unique_ptr<Netlist> nl = verilog_parser.parse_and_instantiate(verilog_file, m_gl);
            ASSERT_NE(nl, nullptr);

            // check gates
            ASSERT_EQ(nl->get_gates(test_utils::gate_filter("gate_1_to_1", "gate_0")).size(), 1);
            ASSERT_EQ(nl->get_gates(test_utils::gate_filter("gate_1_to_1", "gate_1")).size(), 1);
            ASSERT_EQ(nl->get_gates(test_utils::gate_filter("gate_1_to_1", "gate_0_child")).size(), 1);
            ASSERT_EQ(nl->get_gates(test_utils::gate_filter("gate_1_to_1", "gate_1_child")).size(), 1);
            Gate* gate_0 = *nl->get_gates(test_utils::gate_filter("gate_1_to_1", "gate_0")).begin();
            Gate* gate_1 = *nl->get_gates(test_utils::gate_filter("gate_1_to_1", "gate_1")).begin();
            Gate* gate_0_child = *nl->get_gates(test_utils::gate_filter("gate_1_to_1", "gate_0_child")).begin();
            Gate* gate_1_child = *nl->get_gates(test_utils::gate_filter("gate_1_to_1", "gate_1_child")).begin();

            // check nets
            ASSERT_EQ(nl->get_nets(test_utils::net_name_filter("net_0")).size(), 1);
            ASSERT_EQ(nl->get_nets(test_utils::net_name_filter("net_1")).size(), 1);
            ASSERT_EQ(nl->get_nets(test_utils::net_name_filter("net_global_in")).size(), 1);
            ASSERT_EQ(nl->get_nets(test_utils::net_name_filter("net_global_out")).size(), 1);
            ASSERT_EQ(nl->get_nets(test_utils::net_name_filter("net_0_child")).size(), 1);
            Net* net_0 = *nl->get_nets(test_utils::net_name_filter("net_0")).begin();
            Net* net_1 = *nl->get_nets(test_utils::net_name_filter("net_1")).begin();
            Net* net_global_in = *nl->get_nets(test_utils::net_name_filter("net_global_in")).begin();
            Net* net_global_out = *nl->get_nets(test_utils::net_name_filter("net_global_out")).begin();
            Net* net_0_child = *nl->get_nets(test_utils::net_name_filter("net_0_child")).begin();

            // check connections
            EXPECT_EQ(gate_0->get_fan_in_net("I"), net_global_in);
            EXPECT_EQ(gate_0->get_fan_out_net("O"), net_0);
            EXPECT_EQ(gate_1->get_fan_in_net("I"), net_1);
            EXPECT_EQ(gate_1->get_fan_out_net("O"), net_global_out);
            EXPECT_EQ(gate_0_child->get_fan_in_net("I"), net_0);
            EXPECT_EQ(gate_0_child->get_fan_out_net("O"), net_0_child);
            EXPECT_EQ(gate_1_child->get_fan_in_net("I"), net_0_child);
            EXPECT_EQ(gate_1_child->get_fan_out_net("O"), net_1);

            // check modules
            Module* top_mod = nl->get_top_module();
            ASSERT_NE(top_mod, nullptr);
            EXPECT_TRUE(top_mod->is_top_module());
            EXPECT_EQ(top_mod->get_name(), "top_module");
            EXPECT_EQ(top_mod->get_type(), "MODULE_TOP");
            ASSERT_EQ(top_mod->get_submodules().size(), 1);
            Module* child_mod = *top_mod->get_submodules().begin();
            ASSERT_NE(child_mod, nullptr);
            EXPECT_EQ(child_mod->get_name(), "child_mod");
            EXPECT_EQ(child_mod->get_type(), "MODULE_CHILD");
            EXPECT_EQ(top_mod->get_gates(), std::vector<Gate*>({gate_0, gate_1}));
            EXPECT_EQ(child_mod->get_gates(), std::vector<Gate*>({gate_0_child, gate_1_child}));

            // check attributes
            EXPECT_EQ(net_0->get_data("attribute", "child_net_attribute"), std::make_tuple("unknown", "child_net_attribute_value"));
            EXPECT_EQ(child_mod->get_data("attribute", "child_attribute"), std::make_tuple("unknown", "child_attribute_value"));
        }
        {
            // Create a netlist with the following MODULE hierarchy (assigned gates in '()'):
            /*
                        *                               .---- CHILD_TWO --- (gate_child_two)
                        *                               |
                        *              .----- CHILD_ONE-+
                        *              |                |
                        *  TOP_MODULE -+                +---- CHILD_TWO --- (gate_child_two)
                        *              |                |
                        *              |                '---- (gate_child_one)
                        *              |
                        *              +----- CHILD_TWO --- (gate_child_two)
                        *              |
                        *              '---- (gate_top)
                        *
                        */
            // Testing the correct build of the Module hierarchy. Moreover the correct substitution of Gate and Net names,
            // which would be added twice (because an entity can be used multiple times) is tested as well.

            std::string netlist_input("module ENT_CHILD_TWO ( "
                                    "  I_c2, "
                                    "  O_c2 "
                                    " ) ; "
                                    "  input I_c2 ; "
                                    "  output O_c2 ; "
                                    "gate_1_to_1 gate_child_two ( "
                                    "  .I (I_c2 ), "
                                    "  .O (O_c2 ) "
                                    " ) ; "
                                    "endmodule "
                                    " "
                                    "module ENT_CHILD_ONE ( "
                                    "  I_c1, "
                                    "  O_c1 "
                                    " ) ; "
                                    "  input I_c1 ; "
                                    "  output O_c1 ; "
                                    "  wire net_child_0 ; "
                                    "  wire net_child_1 ; "
                                    "ENT_CHILD_TWO gate_0_ent_two ( "
                                    "  .\\I_c2 (I_c1 ), "
                                    "  .\\O_c2 (net_child_0 ) "
                                    " ) ; "
                                    "ENT_CHILD_TWO gate_1_ent_two ( "
                                    "  .\\I_c2 (net_child_0 ), "
                                    "  .\\O_c2 (net_child_1 ) "
                                    " ) ; "
                                    "gate_1_to_1 gate_child_one ( "
                                    "  .I (net_child_1 ), "
                                    "  .O (O_c1 ) "
                                    " ) ; "
                                    "endmodule "
                                    " "
                                    "module ENT_TOP ("
                                    "  net_global_in,"
                                    "  net_global_out"
                                    " ) ;"
                                    "  input net_global_in ;"
                                    "  output net_global_out ;"
                                    "  wire net_0 ;"
                                    "  wire net_1 ;"
                                    "ENT_CHILD_ONE #("
                                    "  .child_one_mod_key(1234)"
                                    ") child_one_mod ("
                                    "  .\\I_c1 (net_global_in ),"
                                    "  .\\O_c1 (net_0 )"
                                    " ) ;"
                                    "ENT_CHILD_TWO child_two_mod ("
                                    "  .\\I_c2 (net_0 ),"
                                    "  .\\O_c2 (net_1 )"
                                    " ) ;"
                                    "gate_1_to_1 gate_top ("
                                    "  .I (net_1 ),"
                                    "  .O (net_global_out )"
                                    " ) ;"
                                    "endmodule");
            auto verilog_file = test_utils::create_sandbox_file("netlist.v", netlist_input);
            VerilogParser verilog_parser;
            std::unique_ptr<Netlist> nl = verilog_parser.parse_and_instantiate(verilog_file, m_gl);

            // Test if all modules are created and assigned correctly
            ASSERT_NE(nl, nullptr);
            EXPECT_EQ(nl->get_gates().size(), 5);      // 3 * gate_child_two + gate_child_one + gate_top
            EXPECT_EQ(nl->get_modules().size(), 5);    // 3 * ENT_CHILD_TWO + ENT_CHILD_ONE + ENT_TOP
            Module* top_module = nl->get_top_module();

            ASSERT_EQ(top_module->get_submodules().size(), 2);
            Module* top_child_one = *top_module->get_submodules().begin();
            Module* top_child_two = *(++top_module->get_submodules().begin());
            if (top_child_one->get_submodules().empty()) {
                std::swap(top_child_one, top_child_two);
            }

            ASSERT_EQ(top_child_one->get_submodules().size(), 2);
            Module* one_child_0 = *(top_child_one->get_submodules().begin());
            Module* one_child_1 = *(++top_child_one->get_submodules().begin());

            // Test if all names that are used multiple times are substituted correctly
            std::string module_suffix = "";

            EXPECT_EQ(top_child_one->get_name(), "child_one_mod");

            EXPECT_TRUE(utils::starts_with(top_child_two->get_name(), "child_two_mod" + module_suffix));
            EXPECT_TRUE(utils::starts_with(one_child_0->get_name(), "gate_0_ent_two" + module_suffix));
            EXPECT_TRUE(utils::starts_with(one_child_1->get_name(), "gate_1_ent_two" + module_suffix));
            // All 3 names should be unique
            EXPECT_EQ(std::set<std::string>({top_child_two->get_name(), one_child_0->get_name(),
                                                one_child_1->get_name()}).size(), 3);

            // Test if the Gate names are substituted correctly as well (gate_child_two is used multiple times)
            std::string gate_suffix = "";

            ASSERT_EQ(top_module->get_gates().size(), 1);
            EXPECT_EQ((*top_module->get_gates().begin())->get_name(), "gate_top");

            ASSERT_EQ(top_child_one->get_gates().size(), 1);
            EXPECT_EQ((*top_child_one->get_gates().begin())->get_name(), "gate_child_one");

            ASSERT_EQ(top_child_two->get_gates().size(), 1);
            ASSERT_EQ(one_child_0->get_gates().size(), 1);
            ASSERT_EQ(one_child_1->get_gates().size(), 1);
            Gate* gate_child_two_0 = *top_child_two->get_gates().begin();
            Gate* gate_child_two_1 = *one_child_0->get_gates().begin();
            Gate* gate_child_two_2 = *one_child_1->get_gates().begin();

            EXPECT_TRUE(utils::starts_with(gate_child_two_0->get_name(), "gate_child_two" + gate_suffix));
            EXPECT_TRUE(utils::starts_with(gate_child_two_1->get_name(), "gate_child_two" + gate_suffix));
            EXPECT_TRUE(utils::starts_with(gate_child_two_2->get_name(), "gate_child_two" + gate_suffix));
            // All 3 names should be unique
            EXPECT_EQ(std::set<std::string>({gate_child_two_0->get_name(), gate_child_two_1->get_name(),
                                                gate_child_two_2->get_name()}).size(), 3);

            // Test the creation on generic data of the Module child_one_mod
            EXPECT_EQ(top_child_one->get_data("generic", "child_one_mod_key"),
                        std::make_tuple("integer", "1234"));
        }
        if(test_utils::known_issue_tests_active())
        {
            // Create a netlist as follows and test its creation (due to request):
            /*                     - - - - - - - - - - - - - - - - - - - - - - .
                *                    ' mod                                        '
                *                    '                       mod_inner/mod_out    '
                *                    '                     .------------------.   '
                *                    'mod_in               |                  |   'net_0
                *  net_global_in ----=------=| gate_a |=---+---=| gate_b |=   '---=----=| gate_top |=---- net_global_out
                *                    '                                            '
                *                    '                                            '
                *                    '- - - - - - - - - - - - - - - - - - - - - - '
            */

            std::string netlist_input("module ENT_MODULE ( "
                                    "  mod_in, "
                                    "  mod_out "
                                    " ) ; "
                                    "  input mod_in ; "
                                    "  output mod_out ; "
                                    "  wire mod_inner ; "
                                    "  assign mod_out = mod_inner ; "
                                    "gate_1_to_1 gate_a ( "
                                    "  .I (mod_in ), "
                                    "  .O (mod_inner ) "
                                    " ) ; "
                                    "gate_1_to_1 gate_b ( "
                                    "  .I (mod_inner ) "
                                    " ) ; "
                                    "endmodule "
                                    " "
                                    " "
                                    "module ENT_TOP ( "
                                    "  net_global_in, "
                                    "  net_global_out "
                                    " ) ; "
                                    "  input net_global_in ; "
                                    "  output net_global_out ; "
                                    "  wire net_0 ; "
                                    "ENT_MODULE mod ( "
                                    "  .\\mod_in (net_global_in ), "
                                    "  .\\mod_out (net_0 ) "
                                    " ) ; "
                                    "gate_1_to_1 gate_top ( "
                                    "  .I (net_0 ), "
                                    "  .O (net_global_out ) "
                                    " ) ; "
                                    "endmodule");
            // ISSUE: mod_out/mod_inner is not connected to gate_top (assign statement is not applied)
            auto verilog_file = test_utils::create_sandbox_file("netlist.v", netlist_input);
            VerilogParser verilog_parser;
            std::unique_ptr<Netlist> nl = verilog_parser.parse_and_instantiate(verilog_file, m_gl);

            // Test if all modules are created and assigned correctly
            ASSERT_NE(nl, nullptr);
            EXPECT_EQ(nl->get_gates().size(), 3);      // 1 in top + 2 in mod
            EXPECT_EQ(nl->get_modules().size(), 2);    // top + mod
            Module* top_module = nl->get_top_module();

            ASSERT_EQ(top_module->get_submodules().size(), 1);
            Module* mod = *top_module->get_submodules().begin();

            ASSERT_EQ(mod->get_gates(test_utils::gate_name_filter("gate_a")).size(), 1);
            ASSERT_EQ(mod->get_gates(test_utils::gate_name_filter("gate_b")).size(), 1);
            ASSERT_EQ(nl->get_gates(test_utils::gate_name_filter("gate_top")).size(), 1);
            Gate* gate_a = *mod->get_gates(test_utils::gate_name_filter("gate_a")).begin();
            Gate* gate_b = *mod->get_gates(test_utils::gate_name_filter("gate_b")).begin();
            Gate* gate_top = *nl->get_gates(test_utils::gate_name_filter("gate_top")).begin();

            Net* mod_out = gate_a->get_fan_out_net("O");
            ASSERT_NE(mod_out, nullptr);
            ASSERT_EQ(mod->get_output_nets().size(), 1);
            EXPECT_EQ(*mod->get_output_nets().begin(), mod_out);

            EXPECT_TRUE(test_utils::vectors_have_same_content(mod_out->get_destinations(),
                                                                std::vector<Endpoint*>({test_utils::get_endpoint(gate_b,
                                                                                                                "I"),
                                                                                        test_utils::get_endpoint(
                                                                                            gate_top,
                                                                                            "I")})));

        }
        if(test_utils::known_issue_tests_active())
        {
            // Testing the correct naming of gates and nets that occur in multiple modules by
            // creating the following netlist:

            /*                        MODULE_B
                *                       . -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  .
                *                       '     .-----------------.  shared_net_name  .--------------.     '
                *                       '    |                  |=----------------=|               |     ' net_0
                *      net_global_in ---=---=| shared_gate_name |=----------------=|    gate_b     |=----=-- ...
                *                       '    '------------------'       net_b      '---------------'     '
                *                       ' -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  '
                *
                *                       MODULE_A
                *                       . -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  .
                *                       '     .-----------------.  shared_net_name  .--------------.     '
                *                net_0  '    |                  |=----------------=|               |     ' net_1
                *               ...  ---=---=| shared_gate_name |=----------------=|    gate_a     |=----=-- ...
                *                       '    '------------------'       net_a      '---------------'     '
                *                       ' -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  '
                *
                *                        MODULE_B
                *                net_1  . -  -  . net_2
                *               ... --- =  ...  =-------=| gate_top |=--- net_global_out
                *                       ' -  -  '
                */
            std::string netlist_input("module MODULE_A ( "
                                    "  I_A, "
                                    "  O_A "
                                    " ) ; "
                                    "  input I_A ; "
                                    "  output O_A ; "
                                    "  wire shared_net_name ; "
                                    "  wire net_a ;  "
                                    "gate_1_to_2 shared_gate_name ( "
                                    "  .\\I (I_A ), "
                                    "  .\\O0 (shared_net_name ), "
                                    "  .\\O1 (net_a ) "
                                    " ) ; "
                                    "gate_2_to_1 gate_a ( "
                                    "  .\\I0 (shared_net_name ), "
                                    "  .\\I1 (net_a ), "
                                    "  .\\O (O_A ) "
                                    " ) ; "
                                    "endmodule "
                                    " "
                                    "module MODULE_B ( "
                                    "  I_B, "
                                    "  O_B "
                                    " ) ; "
                                    "  input I_B ; "
                                    "  output O_B ; "
                                    "  wire shared_net_name ; "
                                    "  wire net_b ; "
                                    "gate_1_to_2 shared_gate_name ( "
                                    "  .\\I (I_B ), "
                                    "  .\\O0 (shared_net_name ), "
                                    "  .\\O1 (net_b ) "
                                    " ) ; "
                                    "gate_2_to_1 gate_b ( "
                                    "  .\\I0 (shared_net_name ), "
                                    "  .\\I1 (net_b ), "
                                    "  .\\O (O_B ) "
                                    " ) ; "
                                    "endmodule "
                                    " "
                                    "module ENT_TOP ( "
                                    "  net_global_in, "
                                    "  net_global_out "
                                    " ) ; "
                                    "  input net_global_in ; "
                                    "  output net_global_out ; "
                                    "  wire net_0 ; "
                                    "  wire net_1; "
                                    "  wire net_2; "
                                    "MODULE_B mod_b_0 ( "
                                    "  .\\I_B (net_global_in ), "
                                    "  .\\O_B (net_0 ) "
                                    " ) ; "
                                    "MODULE_A mod_a_0 ( "
                                    "  .\\I_A (net_0 ), "
                                    "  .\\O_A (net_1 ) "
                                    " ) ; "
                                    "MODULE_B mod_b_1 ( "
                                    "  .\\I_B (net_1 ), "
                                    "  .\\O_B (net_2 ) "
                                    " ) ; "
                                    "gate_1_to_1 gate_top ( "
                                    "  .\\I (net_2 ), "
                                    "  .\\O (net_global_out ) "
                                    " ) ; "
                                    "endmodule");
            auto verilog_file = test_utils::create_sandbox_file("netlist.v", netlist_input);
            VerilogParser verilog_parser;
            std::unique_ptr<Netlist> nl = verilog_parser.parse_and_instantiate(verilog_file, m_gl);

            // Test if all modules are created and assigned correctly
            ASSERT_NE(nl, nullptr);

            // ISSUE: Seems not to be correct. For example net_b occurs two times, but is named net_b__[3]__ and net_b__[4]__
            //  or shared_net_name occurs 3 times and is labeled with 4,5,6

            Net* glob_in = *nl->get_global_input_nets().begin();
            ASSERT_NE(glob_in, nullptr);
            ASSERT_EQ(glob_in->get_destinations().size(), 1);

            Gate* shared_gate_0 = (*glob_in->get_destinations().begin())->get_gate();
            ASSERT_NE(shared_gate_0, nullptr);

            // Get all gates from left to right
            std::vector<Gate*> nl_gates = {shared_gate_0};
            std::vector<std::string> suc_pin = {"O0","O","O0","O","O0","O"};
            for(size_t idx = 0; idx < suc_pin.size(); idx++){
                ASSERT_NE(nl_gates[idx]->get_successor(suc_pin[idx]), nullptr);
                Gate* next_gate = nl_gates[idx]->get_successor(suc_pin[idx])->get_gate();
                ASSERT_NE(next_gate, nullptr);
                nl_gates.push_back(next_gate);
            }

            // Get all nets from left to right (and from top to bottom)
            std::vector<Net*> nl_nets = {glob_in};
            std::vector<std::pair<Gate*, std::string>> net_out_gate_and_pin = {{nl_gates[0], "O0"}, {nl_gates[0], "O1"}, {nl_gates[1], "O"}, {nl_gates[2], "O0"},
                    {nl_gates[2], "O1"}, {nl_gates[3], "O"}, {nl_gates[4], "O0"}, {nl_gates[4], "O1"}, {nl_gates[5], "O"}, {nl_gates[6], "O"}};
            for(size_t idx = 0; idx < net_out_gate_and_pin.size(); idx++){
                Net* next_net = (net_out_gate_and_pin[idx].first)->get_fan_out_net(net_out_gate_and_pin[idx].second);
                ASSERT_NE(next_net, nullptr);
                nl_nets.push_back(next_net);
            }

            // Check that the gate names are correct
            std::vector<std::string> nl_gate_names;
            for(Gate* g : nl_gates) nl_gate_names.push_back(g->get_name());
            std::vector<std::string> expected_gate_names = {"shared_gate_name__[0]__", "gate_b__[0]__",
                    "shared_gate_name__[1]__", "gate_a", "shared_gate_name__[2]__", "gate_b__[1]__", "gate_top"};
            EXPECT_EQ(nl_gate_names, expected_gate_names);

            // Check that the net names are correct
            std::vector<std::string> nl_net_names;
            for(Net* n : nl_nets) nl_net_names.push_back(n->get_name());
            std::vector<std::string> expected_net_names = {"net_global_in", "shared_net_name__[0]__", "net_b__[0]__", "net_0", "shared_net_name__[1]__", "net_a",
                        "net_1", "shared_net_name__[2]__", "net_b__[1]__", "net_2", "net_global_out"};
            EXPECT_EQ(nl_net_names, expected_net_names);

        }
        TEST_END
    }

    /**
     * Testing the correct handling of direct assignment (e.g. 'assign net_slave = net_master;'), where two
     * (wire-)identifiers address the same Net. The Net (wire) at the left side of the expression is mapped to the Net
     * at the right side, so only the Net with the right identifier will be created.
     *
     * In Verilog, assignemts of vectors (for example: assign sig_vec_1[0:3] = sig_vec_2[0:3]) and assignments of lists
     * of vectors and/or single nets (for example: "assign {sig_vec_1, signal_1} = {sig_vec_big[0:4]}", if |sig_vec_1|=4)
     * are also supported.
     *
     * However, logic expressions like 'assign A =  B ^ C' are NOT supported.
     *
     * Functions: parse
     */
    TEST_F(VerilogParserTest, check_direct_assignment) {

        TEST_START
            {
                // Build up a master-slave hierarchy as follows: (NOTE: Whats up with global inputs?)
                /*                                  .--- net_slave_1 (is global input)
                 *   net_master <--- net_slave_0 <--+
                 *                                  '--- net_slave_2
                 */
                std::string netlist_input("module top (\n"
                                        "  net_global_in,\n"
                                        "  net_global_out,\n"
                                        "  net_slave_1\n"
                                        " ) ;\n"
                                        "  input net_global_in ;\n"
                                        "  output net_global_out ;\n"
                                        "  wire net_master ;\n"
                                        "  wire net_slave_0 ;\n"
                                        "  input net_slave_1 ;\n"
                                        "  wire net_slave_2 ;\n"
                                        "  assign net_slave_1 = net_slave_0;\n"
                                        "  assign net_slave_0 = net_master;\n"
                                        "  assign net_slave_2 = net_slave_0;\n"
                                        "gate_1_to_1 gate_0 (\n"
                                        "  .I (net_global_in ),\n"
                                        "  .O (net_slave_0 )\n"
                                        " ) ;\n"
                                        "gate_3_to_1 gate_1 (\n"
                                        " .I0 (net_master ),\n"
                                        " .I1 (net_slave_1 ),\n"
                                        " .I2 (net_slave_2 ),\n"
                                        " .O (net_global_out )\n"
                                        ") ;\n"
                                        "endmodule");

                auto verilog_file = test_utils::create_sandbox_file("netlist.v", netlist_input);
                VerilogParser verilog_parser;
                std::unique_ptr<Netlist> nl = verilog_parser.parse_and_instantiate(verilog_file, m_gl);

                ASSERT_NE(nl, nullptr);
                EXPECT_EQ(nl->get_nets().size(), 3);    // global_in + global_out + net_master
                ASSERT_EQ(nl->get_nets(test_utils::net_name_filter("net_master")).size(), 1);
                Net* net_master = *nl->get_nets(test_utils::net_name_filter("net_master")).begin();

                ASSERT_EQ(nl->get_gates(test_utils::gate_filter("gate_1_to_1", "gate_0")).size(), 1);
                ASSERT_EQ(nl->get_gates(test_utils::gate_filter("gate_3_to_1", "gate_1")).size(), 1);

                Gate* g_0 = *nl->get_gates(test_utils::gate_filter("gate_1_to_1", "gate_0")).begin();
                Gate* g_1 = *nl->get_gates(test_utils::gate_filter("gate_3_to_1", "gate_1")).begin();

                // Check the connections
                EXPECT_EQ(g_0->get_fan_out_net("O"), net_master);
                EXPECT_EQ(g_1->get_fan_in_net("I0"), net_master);
                EXPECT_EQ(g_1->get_fan_in_net("I1"), net_master);
                EXPECT_EQ(g_1->get_fan_in_net("I2"), net_master);

                // Check that net_master becomes also a global input
                EXPECT_TRUE(net_master->is_global_input_net());

            }
            // -- Verilog Specific Tests
            {
                // Verilog specific: Testing assignments with logic vectors (assign wires 0 and 1 of each dimension)
                // for example (for dim 2): wire [0:1][0:1] slave_vector; wire [0:3] master_vector; assign slave_vector = master_vector;

                // Will be tested for dimensions up to MAX_DIMENSION (runtime growth exponentially)
                const u8 MAX_DIMENSION = 3;    // Can be turned up, if you are bored ;)

                for (u8 dim = 0; dim <= MAX_DIMENSION; dim++) {
                    std::stringstream global_out_list_module;
                    std::stringstream global_out_list;
                    std::stringstream gate_list;
                    std::string dim_decl = "";

                    dim_decl += "";
                    for (u8 d = 0; d < dim; d++) {
                        dim_decl += "[0:1]";
                    }

                    // 2^(dim) gates (with one pin) must be created to connect all assigned wires
                    for (u64 i = 0; i < (1 << dim); i++) {
                        global_out_list_module << "  global_out_" << i << ",";
                        global_out_list << "  output global_out_" << i << ";";

                        std::bitset<64> i_bs(i);
                        std::stringstream brackets("");
                        for (int j = dim - 1; j >= 0; j--) {
                            brackets << "[" << (i_bs[j] ? "1" : "0") << "]";
                        }

                        gate_list << "gate_1_to_1 in_gate_" << i
                                  << " ("
                                     "  .I (global_in ),"
                                     "  .O ( net_slave_vector"
                                  << brackets.str()
                                  << ")"
                                     " ) ;";
                        gate_list << "gate_1_to_1 out_gate_" << i
                                  << " ("
                                     "  .I (net_slave_vector"
                                  << brackets.str()
                                  << "),"
                                     "  .O ( global_out_"
                                  << i
                                  << ")"
                                     " ) ;";
                    }

                    std::stringstream netlist_input;
                    netlist_input << "module top ("
                             "  global_in,"
                          << global_out_list_module.str()
                          << "  );"
                             "  input global_in ;"
                          << global_out_list.str() << "  wire " << dim_decl << " net_slave_vector;"
                          << "  wire [0:" << ((1 << dim) - 1) << "] net_master_vector;"
                          << "  assign net_slave_vector = net_master_vector;"    // <- !!!
                          << gate_list.str() << "endmodule";

                    test_def::capture_stdout();
                    auto verilog_file = test_utils::create_sandbox_file("netlist.v", netlist_input.str());
                    VerilogParser verilog_parser;
                    std::unique_ptr<Netlist> nl = verilog_parser.parse_and_instantiate(verilog_file, m_gl);
                    if (nl == nullptr) {
                        std::cout << test_def::get_captured_stdout();
                    } else {
                        test_def::get_captured_stdout();
                    }

                    for (u64 i = 0; i < (1 << dim); i++) {
                        ASSERT_NE(nl, nullptr);

                        ASSERT_EQ(nl->get_nets(test_utils::net_name_filter(
                            "net_master_vector(" + std::to_string(i) + ")")).size(),
                                  1);
                        Net* net_i =
                            *nl->get_nets(test_utils::net_name_filter("net_master_vector(" + std::to_string(i) + ")"))
                                .begin();

                        ASSERT_EQ(nl->get_gates(test_utils::gate_filter("gate_1_to_1", "in_gate_" + std::to_string(i)))
                                      .size(), 1);
                        ASSERT_EQ(nl->get_gates(test_utils::gate_filter("gate_1_to_1", "out_gate_" + std::to_string(i)))
                                      .size(), 1);
                        Gate* in_gate_i =
                            *nl->get_gates(test_utils::gate_filter("gate_1_to_1", "in_gate_" + std::to_string(i)))
                                .begin();
                        Gate* out_gate_i =
                            *nl->get_gates(test_utils::gate_filter("gate_1_to_1", "out_gate_" + std::to_string(i)))
                                .begin();

                        EXPECT_EQ(in_gate_i->get_fan_out_net("O"), net_i);
                        EXPECT_EQ(out_gate_i->get_fan_in_net("I"), net_i);
                    }
                }
            }
            {
                // Verilog specific: Assign constants ('b0 and 'b1)
                std::string netlist_input("module top ("
                                        "  global_out"
                                        " ) ;"
                                        "  output global_out ;"
                                        "  wire [0:3] bit_vector ;"
                                        "  assign bit_vector = 4'hA ;"
                                        ""
                                        "  gate_1_to_1 test_gate ("
                                        "  .I (bit_vector[0] ),"
                                        "  .O (global_out )"
                                        " ) ;"
                                        "endmodule");
                //test_def::capture_stdout();
                auto verilog_file = test_utils::create_sandbox_file("netlist.v", netlist_input);
                VerilogParser verilog_parser;
                std::unique_ptr<Netlist> nl = verilog_parser.parse_and_instantiate(verilog_file, m_gl);
                if (nl == nullptr) {
                    std::cout << test_def::get_captured_stdout();
                } else {
                    test_def::get_captured_stdout();
                }

                ASSERT_NE(nl, nullptr);

                ASSERT_EQ(nl->get_gates(test_utils::gate_name_filter("test_gate")).size(), 1);
                Gate* test_gate = *nl->get_gates(test_utils::gate_name_filter("test_gate")).begin();
            }
            {
                // Verilog specific: Assign a set of wires to a single vector
                std::string netlist_input("module top ("
                                        "  global_out_0,"
                                        "  global_out_1,"
                                        "  global_out_2"
                                        " ) ;"
                                        "  output global_out_0 ;"
                                        "  output global_out_1 ;"
                                        "  output global_out_2 ;"
                                        ""
                                        "  wire single_net ;"
                                        "  wire [0:2][0:2] _2_d_vector_0;"
                                        "  wire [0:2][0:1] _2_d_vector_1;"
                                        "  wire [0:15] big_vector;"
                                        "  wire [0:11] net_vector_master;"
                                        "  assign {single_net, big_vector[3], big_vector[0:1], _2_d_vector_0[0:1][0:1], _2_d_vector_1[1:0][0:1]} = net_vector_master;"
                                        ""
                                        "gate_4_to_1 test_gate_0 ("
                                        "  .I0 (single_net ),"
                                        "  .I1 (big_vector[3] ),"
                                        "  .I2 (big_vector[0] ),"
                                        "  .I3 (big_vector[1] ),"
                                        "  .O (global_out_0 )"
                                        " ) ;"
                                        ""
                                        "gate_4_to_1 test_gate_1 ("
                                        "  .I0 (_2_d_vector_0[0][0] ),"
                                        "  .I1 (_2_d_vector_0[0][1] ),"
                                        "  .I2 (_2_d_vector_0[1][0] ),"
                                        "  .I3 (_2_d_vector_0[1][1] ),"
                                        "  .O (global_out_1 )"
                                        " ) ;"
                                        "gate_4_to_1 test_gate_2 ("
                                        "  .I0 (_2_d_vector_1[1][0] ),"
                                        "  .I1 (_2_d_vector_1[1][1] ),"
                                        "  .I2 (_2_d_vector_1[0][0] ),"
                                        "  .I3 (_2_d_vector_1[0][1] ),"
                                        "  .O (global_out_2 )"
                                        " ) ;"
                                        "endmodule");
                test_def::capture_stdout();
                auto verilog_file = test_utils::create_sandbox_file("netlist.v", netlist_input);
                VerilogParser verilog_parser;
                std::unique_ptr<Netlist> nl = verilog_parser.parse_and_instantiate(verilog_file, m_gl);
                if (nl == nullptr) {
                    std::cout << test_def::get_captured_stdout();
                } else {
                    test_def::get_captured_stdout();
                }

                ASSERT_NE(nl, nullptr);
                std::vector<Net *> net_master_vector(12);
                for (int i = 0; i < 12; i++) {
                    ASSERT_EQ(nl->get_nets(test_utils::net_name_filter("net_vector_master(" + std::to_string(i) + ")"))
                                  .size(), 1);
                    net_master_vector[i] =
                        *nl->get_nets(test_utils::net_name_filter("net_vector_master(" + std::to_string(i) + ")"))
                            .begin();
                }
                for (int i = 0; i < 12; i++) {
                    ASSERT_EQ(net_master_vector[i]->get_destinations().size(), 1);
                    Endpoint *ep = *net_master_vector[i]->get_destinations().begin();
                    EXPECT_EQ(ep->get_gate()->get_name(), "test_gate_" + std::to_string(i / 4));
                    EXPECT_EQ(ep->get_pin(), "I" + std::to_string(i % 4));
                }
            }
            {
                // Verilog specific: Assign a 2 bit vector to a set of 1 bit vectors

                std::string netlist_input("module top ("
                                        "    global_out"
                                        ") ;"
                                        "    output global_out ;"
                                        "    wire net_master_0 ;"
                                        "    wire net_master_1 ;"
                                        "    wire [0:1] net_vector_slave;"
                                        "    assign net_vector_slave = { net_master_0, net_master_1 };"
                                        "    gate_2_to_1 test_gate ("
                                        "        .I0 ( net_vector_slave[0] ),"
                                        "        .I1 ( net_vector_slave[1] ),"
                                        "        .O (global_out )"
                                        "    ) ;"
                                        "endmodule");

                test_def::capture_stdout();
                auto verilog_file = test_utils::create_sandbox_file("netlist.v", netlist_input);
                VerilogParser verilog_parser;
                std::unique_ptr<Netlist> nl = verilog_parser.parse_and_instantiate(verilog_file, m_gl);
                if (nl == nullptr) {
                    std::cout << test_def::get_captured_stdout();
                } else {
                    test_def::get_captured_stdout();
                }

                ASSERT_NE(nl, nullptr);

                ASSERT_EQ(nl->get_nets(test_utils::net_name_filter("net_master_0")).size(), 1);
                ASSERT_EQ(nl->get_nets(test_utils::net_name_filter("net_master_1")).size(), 1);
                Net*
                    net_vector_master_0 = *nl->get_nets(test_utils::net_name_filter("net_master_0")).begin();
                Net*
                    net_vector_master_1 = *nl->get_nets(test_utils::net_name_filter("net_master_1")).begin();
                ASSERT_EQ(net_vector_master_0->get_destinations().size(), 1);
                ASSERT_EQ(net_vector_master_1->get_destinations().size(), 1);
                EXPECT_EQ((*net_vector_master_0->get_destinations().begin())->get_pin(), "I0");
                EXPECT_EQ((*net_vector_master_1->get_destinations().begin())->get_pin(), "I1");
            }
            {
                // Verilog specific: Testing assignments, where escaped identifiers are used (e.g.\Net[1:3][2:3] stands for a Net, literally named "Net[1:3][2:3]")

                std::string netlist_input("module top ("
                                        "    global_out"
                                        ") ;"
                                        "    output global_out ;"
                                        "    wire \\escaped_net_range[0:3] ;"
                                        "    wire [0:3] escaped_net_range ;"
                                        "    wire \\escaped_net[0] ;"
                                        "    wire [0:1] net_vector_master ;"
                                        "    assign { \\escaped_net_range[0:3] , \\escaped_net[0] } = net_vector_master;"
                                        "    gate_2_to_1 test_gate ("
                                        "        .I0 ( \\escaped_net_range[0:3] ),"
                                        "        .I1 ( \\escaped_net[0] ),"
                                        "        .O (global_out )"
                                        "    ) ;"
                                        "endmodule");

                test_def::capture_stdout();
                auto verilog_file = test_utils::create_sandbox_file("netlist.v", netlist_input);
                VerilogParser verilog_parser;
                std::unique_ptr<Netlist> nl = verilog_parser.parse_and_instantiate(verilog_file, m_gl);
                if (nl == nullptr) {
                    std::cout << test_def::get_captured_stdout();
                } else {
                    test_def::get_captured_stdout();
                }

                ASSERT_NE(nl, nullptr);
            }
        TEST_END
    }

    /**
     * Testing the port assignment of multiple pins and nets using pin groups and signal vectors
     *
     * Functions: parse
     */
    TEST_F(VerilogParserTest, check_pin_group_port_assignment) {

        TEST_START
            {
                // Connect an entire output pin group with global input nets by using a binary string ('b0101)
                std::string netlist_input("module top (\n"
                                        " ) ;\n"
                                        "  wire [0:3] l_vec;\n"
                                        "  wire net_1 ;\n"
                                        "pin_group_gate_4_to_4 gate_0 (\n"
                                        "  .I ('b0101)\n"
                                        " ) ;\n"
                                        "endmodule");
                test_def::capture_stdout();
                auto verilog_file = test_utils::create_sandbox_file("netlist.v", netlist_input);
                VerilogParser verilog_parser;
                std::unique_ptr<Netlist> nl = verilog_parser.parse_and_instantiate(verilog_file, m_gl);
                if (nl == nullptr) {
                    std::cout << test_def::get_captured_stdout();
                } else {
                    test_def::get_captured_stdout();
                }

                ASSERT_NE(nl, nullptr);
                ASSERT_FALSE(nl->get_gates(test_utils::gate_filter("pin_group_gate_4_to_4", "gate_0")).empty());
                Gate* gate_0 = *(nl->get_gates(test_utils::gate_filter("pin_group_gate_4_to_4", "gate_0")).begin());
                Net* net_0 = gate_0->get_fan_in_net("I(0)");
                ASSERT_NE(net_0, nullptr);
                EXPECT_EQ(net_0->get_name(), "'1'");

                Net* net_1 = gate_0->get_fan_in_net("I(1)");
                ASSERT_NE(net_1, nullptr);
                EXPECT_EQ(net_1->get_name(), "'0'");

                Net* net_2 = gate_0->get_fan_in_net("I(2)");
                ASSERT_NE(net_2, nullptr);
                EXPECT_EQ(net_2->get_name(), "'1'");

                Net* net_3 = gate_0->get_fan_in_net("I(3)");
                ASSERT_NE(net_3, nullptr);
                EXPECT_EQ(net_3->get_name(), "'0'");

            }
            {
                // Connect a vector of output pins with a list of nets using '{ net_0, net_1, ... }'
                std::string netlist_input("module top (\n"
                                        " ) ;\n"
                                        "  wire net_0;\n"
                                        "  wire net_1;\n"
                                        "  wire[0:1] l_vec;\n"
                                        "pin_group_gate_4_to_4 gate_0 (\n"
                                        "  .O ({net_0, net_1, l_vec[0], l_vec[1]})\n"
                                        " ) ;\n"
                                        "endmodule");
                auto verilog_file = test_utils::create_sandbox_file("netlist.v", netlist_input);
                VerilogParser verilog_parser;
                std::unique_ptr<Netlist> nl = verilog_parser.parse_and_instantiate(verilog_file, m_gl);

                ASSERT_NE(nl, nullptr);
                ASSERT_FALSE(nl->get_gates(test_utils::gate_filter("pin_group_gate_4_to_4", "gate_0")).empty());
                Gate* gate_0 = *(nl->get_gates(test_utils::gate_filter("pin_group_gate_4_to_4", "gate_0")).begin());

                EXPECT_EQ(gate_0->get_fan_out_nets().size(), 4);

                Net* net_0 = gate_0->get_fan_out_net("O(0)");
                ASSERT_NE(net_0, nullptr);
                EXPECT_EQ(net_0->get_name(), "l_vec(1)");

                Net* net_1 = gate_0->get_fan_out_net("O(1)");
                ASSERT_NE(net_1, nullptr);
                EXPECT_EQ(net_1->get_name(), "l_vec(0)");

                Net* net_2 = gate_0->get_fan_out_net("O(2)");
                ASSERT_NE(net_2, nullptr);
                EXPECT_EQ(net_2->get_name(), "net_1");

                Net* net_3 = gate_0->get_fan_out_net("O(3)");
                ASSERT_NE(net_3, nullptr);
                EXPECT_EQ(net_3->get_name(), "net_0");
            }
            {
                // Connect a vector of output pins with a vector of nets (O(0) with l_vec(0),...,O(4) with l_vec(4))
                std::string netlist_input("module top (\n"
                                        " ) ;\n"
                                        "  wire [3:0] l_vec;\n"
                                        "  wire net_1 ;\n"
                                        "pin_group_gate_4_to_4 gate_0 (\n"
                                        "  .O (l_vec)\n"
                                        " ) ;\n"
                                        "endmodule");
                auto verilog_file = test_utils::create_sandbox_file("netlist.v", netlist_input);
                VerilogParser verilog_parser;
                std::unique_ptr<Netlist> nl = verilog_parser.parse_and_instantiate(verilog_file, m_gl);

                ASSERT_NE(nl, nullptr);
                ASSERT_FALSE(nl->get_gates(test_utils::gate_filter("pin_group_gate_4_to_4", "gate_0")).empty());
                Gate* gate_0 = *(nl->get_gates(test_utils::gate_filter("pin_group_gate_4_to_4", "gate_0")).begin());

                EXPECT_EQ(gate_0->get_fan_out_nets().size(), 4);
                Net* net_0 = gate_0->get_fan_out_net("O(0)");
                ASSERT_NE(net_0, nullptr);
                EXPECT_EQ(net_0->get_name(), "l_vec(0)");

                Net* net_1 = gate_0->get_fan_out_net("O(1)");
                ASSERT_NE(net_1, nullptr);
                EXPECT_EQ(net_1->get_name(), "l_vec(1)");

                Net* net_2 = gate_0->get_fan_out_net("O(2)");
                ASSERT_NE(net_2, nullptr);
                EXPECT_EQ(net_2->get_name(), "l_vec(2)");

                Net* net_3 = gate_0->get_fan_out_net("O(3)");
                ASSERT_NE(net_3, nullptr);
                EXPECT_EQ(net_3->get_name(), "l_vec(3)");

            }
            {
                // Connect a vector of output pins with a vector of nets (O(0) with l_vec(0),...,O(3) with l_vec(3))
                // but the vector has a smaller size
                std::string netlist_input("module top (\n"
                                        " ) ;\n"
                                        "  wire [2:0] l_vec;\n"
                                        "pin_group_gate_4_to_4 gate_0 (\n"
                                        "  .O (l_vec)\n"
                                        " ) ;\n"
                                        "endmodule");
                auto verilog_file = test_utils::create_sandbox_file("netlist.v", netlist_input);
                VerilogParser verilog_parser;
                std::unique_ptr<Netlist> nl = verilog_parser.parse_and_instantiate(verilog_file, m_gl);


                ASSERT_NE(nl, nullptr);
                ASSERT_FALSE(nl->get_gates(test_utils::gate_filter("pin_group_gate_4_to_4", "gate_0")).empty());
                Gate* gate_0 = *(nl->get_gates(test_utils::gate_filter("pin_group_gate_4_to_4", "gate_0")).begin());

                EXPECT_EQ(gate_0->get_fan_out_nets().size(), 3);
                Net* net_0 = gate_0->get_fan_out_net("O(0)");
                ASSERT_NE(net_0, nullptr);
                EXPECT_EQ(net_0->get_name(), "l_vec(0)");

                Net* net_1 = gate_0->get_fan_out_net("O(1)");
                ASSERT_NE(net_1, nullptr);
                EXPECT_EQ(net_1->get_name(), "l_vec(1)");

                Net* net_2 = gate_0->get_fan_out_net("O(2)");
                ASSERT_NE(net_2, nullptr);
                EXPECT_EQ(net_2->get_name(), "l_vec(2)");

                EXPECT_EQ(gate_0->get_fan_out_net("O(3)"), nullptr);

            }
            {
                // Test assigning 2-bit input and 3-bit output wires to 4-bit ports
                std::string netlist_input("module top (in, out) ;\n"
                                        "  input [1:0] in;\n"
                                        "  output [2:0] out;\n"
                                        "pin_group_gate_4_to_4 gate_0 (\n"
                                        "  .I (in),\n"
                                        "  .O (out)\n"
                                        " ) ;\n"
                                        "endmodule");
                auto verilog_file = test_utils::create_sandbox_file("netlist.v", netlist_input);
                VerilogParser verilog_parser;
                std::unique_ptr<Netlist> nl = verilog_parser.parse_and_instantiate(verilog_file, m_gl);

                Net* net;
                Gate* gate;

                ASSERT_NE(nl, nullptr);
                ASSERT_FALSE(nl->get_gates(test_utils::gate_filter("pin_group_gate_4_to_4", "gate_0")).empty());
                gate = *(nl->get_gates(test_utils::gate_filter("pin_group_gate_4_to_4", "gate_0")).begin());

                EXPECT_EQ(gate->get_fan_in_nets().size(), 2);
                net = gate->get_fan_in_net("I(0)");
                ASSERT_NE(net, nullptr);
                EXPECT_EQ(net->get_name(), "in(0)");
                net = gate->get_fan_in_net("I(1)");
                ASSERT_NE(net, nullptr);
                EXPECT_EQ(net->get_name(), "in(1)");

                EXPECT_EQ(gate->get_fan_out_nets().size(), 3);
                net = gate->get_fan_out_net("O(0)");
                ASSERT_NE(net, nullptr);
                EXPECT_EQ(net->get_name(), "out(0)");
                net = gate->get_fan_out_net("O(1)");
                ASSERT_NE(net, nullptr);
                EXPECT_EQ(net->get_name(), "out(1)");
                net = gate->get_fan_out_net("O(2)");
                ASSERT_NE(net, nullptr);
                EXPECT_EQ(net->get_name(), "out(2)");
            }
        TEST_END
    }

    /*#########################################################################
       Verilog Specific Tests (Tests that can not be directly applied to VHDL)
      #########################################################################*/

    /**
     * Testing the correct detection of single line comments (with '//') and comment blocks(with '/ *' and '* /').
     *
     * Functions: parse
     */
    TEST_F(VerilogParserTest, check_comments) {

        TEST_START
            {
                // Testing all comment types with attributes
                std::string netlist_input("/*here comes a module*/ module top (\n"
                                        "  global_in,\n"
                                        "  global_out\n"
                                        " ) ;\n"
                                        "  input global_in ;\n"
                                        "  output global_out ;\n"
                                        "\n"
                                        "gate_1_to_1 #(\n"
                                        "  .no_comment_0(123), //.comment_0(123),\n"
                                        "  //.comment_1(123),\n"
                                        "  .no_comment_1(123), /*.comment_2(123),*/ .no_comment_2(123),\n"
                                        "  /*.comment_3(123),\n"
                                        "  .comment_4(123),\n"
                                        "  .comment_5(123),*/\n"
                                        "  .no_comment_3(123),\n"
                                        "  .no_comment_4(123), /*.comment_6(123),*/ .no_comment_5(123),\n"
                                        "  /*.comment_7(123),\n"
                                        "  .comment_8(123),\n"
                                        "  .comment_9(123),*/\n"
                                        "  .no_comment_6(123)\n"
                                        ") \n"
                                        "test_gate (\n"
                                        "  .I (global_in ),\n"
                                        "  .O (global_out )\n"
                                        " ) ;\n"
                                        "endmodule");
                test_def::capture_stdout();
                auto verilog_file = test_utils::create_sandbox_file("netlist.v", netlist_input);
                VerilogParser verilog_parser;
                std::unique_ptr<Netlist> nl = verilog_parser.parse_and_instantiate(verilog_file, m_gl);
                if (nl == nullptr) {
                    std::cout << test_def::get_captured_stdout();
                } else {
                    test_def::get_captured_stdout();
                }

                ASSERT_NE(nl, nullptr);
                ASSERT_EQ(nl->get_gates(test_utils::gate_filter("gate_1_to_1", "test_gate")).size(), 1);
                Gate*
                    test_gate = *nl->get_gates(test_utils::gate_filter("gate_1_to_1", "test_gate")).begin();

                // Test that the comments did not removed other parts (all no_comment_n generics should be created)
                for (std::string key : std::set<std::string>({"no_comment_0", "no_comment_1", "no_comment_2",
                                                              "no_comment_3", "no_comment_4", "no_comment_5",
                                                              "no_comment_6"})) {
                    EXPECT_NE(test_gate->get_data("generic", key), std::make_tuple("", ""));
                    if (test_gate->get_data("generic", key) == std::make_tuple("", "")) {
                        std::cout << "comment test failed for: " << key << std::endl;
                    }
                }

                // Test that the comments are not interpreted (all comment_n generics shouldn't be created)
                for (std::string key : std::set<std::string>({"comment_0", "comment_1", "comment_2", "comment_3",
                                                              "comment_4", "comment_5", "comment_6", "comment_7",
                                                              "comment_8", "comment_9"})) {
                    EXPECT_EQ(test_gate->get_data("generic", key), std::make_tuple("", ""));
                    if (test_gate->get_data("generic", key) != std::make_tuple("", "")) {
                        std::cout << "comment failed for: " << key << std::endl;
                    }
                }
            }
        TEST_END
    }

    /**
     * Testing the usage of attributes for gates nets and modules
     *
     * Functions: parse
     */
    TEST_F(VerilogParserTest, check_attributes) {
        TEST_START
            {
                // Add attributes for a  module, a gate and a net.
                std::string netlist_input("(* ATTRIBUTE_MODULE=attri_module, FLAG_MODULE *)\n"
                                        "module top (\n"
                                        "  net_global_in,\n"
                                        "  net_global_out\n"
                                        " ) ;\n"
                                        "(* ATTRIBUTE_NET=attri_net, FLAG_NET *)\n"
                                        "  input net_global_in ;\n"
                                        "  output net_global_out ;\n"
                                        "(* ATTRIBUTE_GATE=attri_gate, FLAG_GATE *)\n"
                                        "gate_1_to_1 gate_0 (\n"
                                        "  .\\I (net_global_in ),\n"
                                        "  .\\O (net_global_out )\n"
                                        " ) ;\n"
                                        "endmodule");
                auto verilog_file = test_utils::create_sandbox_file("netlist.v", netlist_input);
                VerilogParser verilog_parser;
                std::unique_ptr<Netlist> nl = verilog_parser.parse_and_instantiate(verilog_file, m_gl);

                ASSERT_NE(nl, nullptr);

                // Access the Elements with attributes
                Module* attri_module = nl->get_top_module();

                ASSERT_EQ(nl->get_gates(test_utils::gate_type_filter("gate_1_to_1")).size(), 1);
                Gate* attri_gate = *nl->get_gates(test_utils::gate_type_filter("gate_1_to_1")).begin();

                ASSERT_EQ(nl->get_global_input_nets().size(), 1);
                Net* attri_net = *nl->get_global_input_nets().begin();

                // Check their attributes
                // -- Module
                EXPECT_EQ(attri_module->get_data("attribute", "ATTRIBUTE_MODULE"),
                          std::make_tuple("unknown", "attri_module"));
                EXPECT_EQ(attri_module->get_data("attribute", "FLAG_MODULE"), std::make_tuple("unknown", ""));
                // -- Gate
                EXPECT_EQ(attri_gate->get_data("attribute", "ATTRIBUTE_GATE"),
                          std::make_tuple("unknown", "attri_gate"));
                EXPECT_EQ(attri_gate->get_data("attribute", "FLAG_GATE"), std::make_tuple("unknown", ""));
                // -- Net
                EXPECT_EQ(attri_net->get_data("attribute", "ATTRIBUTE_NET"),
                          std::make_tuple("unknown", "attri_net"));
                EXPECT_EQ(attri_net->get_data("attribute", "FLAG_NET"), std::make_tuple("unknown", ""));
            }
            {
                // Use atrribute strings with special characters (',','.')
                std::string netlist_input("module top (\n"
                                        "  net_global_in,\n"
                                        "  net_global_out\n"
                                        " ) ;\n"
                                        "  input net_global_in ;\n"
                                        "  output net_global_out ;\n"
                                        "  (* ATTRI_COMMA_STRING=\"test, 1, 2, 3\", ATTRI_FLOAT_STRING=\"1.234\" *)\n"
                                        "gate_1_to_1 gate_0 (\n"
                                        "  .\\I (net_global_in ),\n"
                                        "  .\\O (net_global_out )\n"
                                        " ) ;\n"
                                        "endmodule");
                auto verilog_file = test_utils::create_sandbox_file("netlist.v", netlist_input);
                VerilogParser verilog_parser;
                std::unique_ptr<Netlist> nl = verilog_parser.parse_and_instantiate(verilog_file, m_gl);

                ASSERT_NE(nl, nullptr);
                ASSERT_EQ(nl->get_gates(test_utils::gate_type_filter("gate_1_to_1")).size(), 1);
                Gate* attri_gate = *nl->get_gates(test_utils::gate_type_filter("gate_1_to_1")).begin();
                EXPECT_EQ(attri_gate->get_data("attribute", "ATTRI_COMMA_STRING"),
                          std::make_tuple("unknown", "test, 1, 2, 3"));
                EXPECT_EQ(attri_gate->get_data("attribute", "ATTRI_FLOAT_STRING"),
                          std::make_tuple("unknown", "1.234"));
            }
        TEST_END
    }

    /**
     * Testing the declaration of multiple wire vectors within one single line.
     *
     * Functions: parse
     */
    TEST_F(VerilogParserTest, check_one_line_multiple_nets) {

        TEST_START
            {
                // Declare multiple wire vectors in one line
                std::string netlist_input("module top ("
                                        "  global_in,"
                                        "  global_out"
                                        " ) ;"
                                        "  input global_in ;"
                                        "  output global_out ;"
                                        "  wire [0:1] net_vec_0, net_vec_1 ;"    // <- !!!
                                        "gate_4_to_1 gate_0 ("
                                        "  .I0 (net_vec_0[0] ),"
                                        "  .I1 (net_vec_0[1] ),"
                                        "  .I2 (net_vec_1[0] ),"
                                        "  .I3 (net_vec_1[1] ),"
                                        "  .O (global_out )"
                                        " ) ;"
                                        "gate_1_to_1 gate_1 ("
                                        "  .I (global_in ),"
                                        "  .O (net_vec_0[0] )"
                                        ") ;"
                                        "gate_1_to_1 gate_2 ("
                                        "  .I (global_in ),"
                                        "  .O (net_vec_0[1] )"
                                        ") ;"
                                        "gate_1_to_1 gate_3 ("
                                        "  .I (global_in ),"
                                        "  .O (net_vec_1[0] )"
                                        ") ;"
                                        "gate_1_to_1 gate_4 ("
                                        "  .I (global_in ),"
                                        "  .O (net_vec_1[1] )"
                                        ") ;"
                                        "endmodule");
                test_def::capture_stdout();
                auto verilog_file = test_utils::create_sandbox_file("netlist.v", netlist_input);
                VerilogParser verilog_parser;
                std::unique_ptr<Netlist> nl = verilog_parser.parse_and_instantiate(verilog_file, m_gl);
                if (nl == nullptr) {
                    std::cout << test_def::get_captured_stdout();
                } else {
                    test_def::get_captured_stdout();
                }

                ASSERT_NE(nl, nullptr);
                EXPECT_EQ(nl->get_nets().size(), 6);    // 3 of the net_vector + global_in + global_out
                ASSERT_EQ(nl->get_nets(test_utils::net_name_filter("net_vec_0(0)")).size(), 1);
                ASSERT_EQ(nl->get_nets(test_utils::net_name_filter("net_vec_0(1)")).size(), 1);
                ASSERT_EQ(nl->get_nets(test_utils::net_name_filter("net_vec_0(0)")).size(), 1);
                ASSERT_EQ(nl->get_nets(test_utils::net_name_filter("net_vec_0(1)")).size(), 1);
            }
        TEST_END
    }

    /**
     * Testing the port declaration within the list of ports.
     *
     * Functions: parse
     */
    TEST_F(VerilogParserTest, check_port_dekl_in_list_of_ports) {
        TEST_START
            {
                // Testing the declaration of single ports within the list of ports.
                std::string netlist_input("module top ( \n"
                                        "  input global_in_0, global_in_1, global_in_2,\n"
                                        "  output global_out_0, global_out_1, global_out_2,\n"
                                        "  input global_in_3,"
                                        "  output global_out_3"
                                        " ) ; \n"
                                        "gate_4_to_4 gate_0 ( \n"
                                        "  .I0 (global_in_0 ), \n"
                                        "  .I1 (global_in_1 ), \n"
                                        "  .I2 (global_in_2 ), \n"
                                        "  .I3 (global_in_3 ), \n"
                                        "  .O0 (global_out_0 ), \n"
                                        "  .O1 (global_out_1 ), \n"
                                        "  .O2 (global_out_2 ), \n"
                                        "  .O3 (global_out_3 ) \n"
                                        " ) ; \n"
                                        "endmodule \n");
                auto verilog_file = test_utils::create_sandbox_file("netlist.v", netlist_input);
                VerilogParser verilog_parser;
                std::unique_ptr<Netlist> nl = verilog_parser.parse_and_instantiate(verilog_file, m_gl);

                ASSERT_NE(nl, nullptr);

                // Check all input nets
                std::set<std::string> input_net_names;
                for(Net* n : nl->get_global_input_nets())
                    input_net_names.insert(n->get_name());
                EXPECT_EQ(input_net_names, std::set<std::string>({"global_in_0", "global_in_1","global_in_2","global_in_3"}));

                // Check all output nets
                std::set<std::string> output_net_names;
                for(Net* n : nl->get_global_output_nets())
                    output_net_names.insert(n->get_name());
                EXPECT_EQ(output_net_names, std::set<std::string>({"global_out_0", "global_out_1","global_out_2","global_out_3"}));
            }
            {
                // Testing the declaration of port vectors within the list of ports.
                std::string netlist_input("module top ( \n"
                                        "  input [0:1] global_in_asc,\n"
                                        "  output [0:1] global_out_asc,\n"
                                        "  input [3:2] global_in_desc,"
                                        "  output [3:2] global_out_desc"
                                        " ) ; \n"
                                        "gate_4_to_4 gate_0 ( \n"
                                        "  .I0 (global_in_asc[0] ), \n"
                                        "  .I1 (global_in_asc[1] ), \n"
                                        "  .I2 (global_in_desc[2] ), \n"
                                        "  .I3 (global_in_desc[3] ), \n"
                                        "  .O0 (global_out_asc[0] ), \n"
                                        "  .O1 (global_out_asc[1] ), \n"
                                        "  .O2 (global_out_desc[2] ), \n"
                                        "  .O3 (global_out_desc[3] ) \n"
                                        " ) ; \n"
                                        "endmodule \n");
                auto verilog_file = test_utils::create_sandbox_file("netlist.v", netlist_input);
                VerilogParser verilog_parser;
                std::unique_ptr<Netlist> nl = verilog_parser.parse_and_instantiate(verilog_file, m_gl);

                ASSERT_NE(nl, nullptr);

                // Check all input nets
                std::set<std::string> input_net_names;
                for(Net* n : nl->get_global_input_nets())
                    input_net_names.insert(n->get_name());
                EXPECT_EQ(input_net_names, std::set<std::string>({"global_in_asc(0)", "global_in_asc(1)","global_in_desc(2)","global_in_desc(3)"}));

                // Check all output nets
                std::set<std::string> output_net_names;
                for(Net* n : nl->get_global_output_nets())
                    output_net_names.insert(n->get_name());
                EXPECT_EQ(output_net_names, std::set<std::string>({"global_out_asc(0)", "global_out_asc(1)","global_out_desc(2)","global_out_desc(3)"}));
            }
            {
                // Testing the declaration of 2-dim port vectors within the list of ports.
                std::string netlist_input("module top ( \n"
                                        "  input [0:1][1:0] global_in,\n"
                                        "  output [1:0][0:1] global_out"
                                        " ) ; \n"
                                        "gate_4_to_4 gate_0 ( \n"
                                        "  .I0 (global_in[0][0] ), \n"
                                        "  .I1 (global_in[0][1] ), \n"
                                        "  .I2 (global_in[1][0] ), \n"
                                        "  .I3 (global_in[1][1] ), \n"
                                        "  .O0 (global_out[0][0] ), \n"
                                        "  .O1 (global_out[0][1] ), \n"
                                        "  .O2 (global_out[1][0] ), \n"
                                        "  .O3 (global_out[1][1] ) \n"
                                        " ) ; \n"
                                        "endmodule \n");
                auto verilog_file = test_utils::create_sandbox_file("netlist.v", netlist_input);
                VerilogParser verilog_parser;
                std::unique_ptr<Netlist> nl = verilog_parser.parse_and_instantiate(verilog_file, m_gl);

                ASSERT_NE(nl, nullptr);

                // Check all input nets
                std::set<std::string> input_net_names;
                for(Net* n : nl->get_global_input_nets())
                    input_net_names.insert(n->get_name());
                EXPECT_EQ(input_net_names, std::set<std::string>({"global_in(0)(0)", "global_in(0)(1)","global_in(1)(0)","global_in(1)(1)"}));

                // Check all output nets
                std::set<std::string> output_net_names;
                for(Net* n : nl->get_global_output_nets())
                    output_net_names.insert(n->get_name());
                EXPECT_EQ(output_net_names, std::set<std::string>({"global_out(0)(0)", "global_out(0)(1)","global_out(1)(0)","global_out(1)(1)"}));
            }
        TEST_END
    }
    /**
     * Testing the correct handling of invalid input
     *
     * Functions: parse
     */
    TEST_F(VerilogParserTest, check_invalid_input) {
        TEST_START
            {
                // Try to connect to a pin, that does not exist
                NO_COUT_TEST_BLOCK;
                std::string netlist_input("module top ("
                                        "  global_in "
                                        " ) ;"
                                        "  input global_in ;"

                                        "gate_1_to_1 gate_0 ("
                                        "  .\\NON_EXISTING_PIN (global_in)"
                                        " ) ;"
                                        "endmodule");
                auto verilog_file = test_utils::create_sandbox_file("netlist.v", netlist_input);
                VerilogParser verilog_parser;
                std::unique_ptr<Netlist> nl = verilog_parser.parse_and_instantiate(verilog_file, m_gl);
                EXPECT_EQ(nl, nullptr);
            }
            {
                // The passed Gate library name is unknown
                NO_COUT_TEST_BLOCK;
                std::string netlist_input("module top ("
                                        "  global_in,"
                                        "  global_out"
                                        " ) ;"
                                        "  input global_in ;"
                                        "  output global_out ;"
                                        "UNKNOWN_GATE_TYPE gate_0 ("
                                        "  .I (global_in ),"
                                        "  .O (global_out )"
                                        " ) ;"
                                        "endmodule");
                auto verilog_file = test_utils::create_sandbox_file("netlist.v", netlist_input);
                VerilogParser verilog_parser;
                std::unique_ptr<Netlist> nl = verilog_parser.parse_and_instantiate(verilog_file, m_gl);
                EXPECT_EQ(nl, nullptr);
            }
            {
                // The input does not contain any Module (is empty)
                NO_COUT_TEST_BLOCK;
                std::string netlist_input("");
                auto verilog_file = test_utils::create_sandbox_file("netlist.v", netlist_input);
                VerilogParser verilog_parser;
                std::unique_ptr<Netlist> nl = verilog_parser.parse_and_instantiate(verilog_file, m_gl);

                EXPECT_EQ(nl, nullptr);
            }
            {
                // Create a non-used entity (should not create any problems...)
                NO_COUT_TEST_BLOCK;
                std::string netlist_input("module ignore_me ("
                                        "  min,"
                                        "  mout"
                                        " ) ;"
                                        "  input min ;"
                                        "  output mout ;"
                                        "gate_1_to_1 gate_0 ("
                                        "  .I (min ),"
                                        "  .O (mout )"
                                        " ) ;"
                                        "endmodule"
                                        "\n"
                                        "module top ("
                                        "  global_in,"
                                        "  global_out"
                                        " ) ;"
                                        "  input global_in ;"
                                        "  output global_out ;"
                                        "gate_1_to_1 gate_0 ("
                                        "  .I (global_in ),"
                                        "  .O (global_out )"
                                        " ) ;"
                                        "endmodule");
                auto verilog_file = test_utils::create_sandbox_file("netlist.v", netlist_input);
                VerilogParser verilog_parser;
                std::unique_ptr<Netlist> nl = verilog_parser.parse_and_instantiate(verilog_file, m_gl);
                EXPECT_NE(nl, nullptr);
            }
            {
                // Having a cyclic master-slave Net hierarchy
                NO_COUT_TEST_BLOCK;
                std::string netlist_input("module top ("
                                        "  global_in,"
                                        "  global_out "
                                        " ) ;"
                                        "  input global_in ;"
                                        "  output global_out ;"
                                        "  wire net_0;"
                                        "  wire net_1;"
                                        "  assign net_0 = net_1;"
                                        "  assign net_1 = net_0;"
                                        "gate_1_to_1 gate_0 ("
                                        "  .I (global_in ),"
                                        "  .O (net_0 )"
                                        " ) ;"
                                        "gate_1_to_1 gate_1 ("
                                        "  .I (net_1 ),"
                                        "  .O (global_out )"
                                        " ) ;"
                                        "endmodule");
                auto verilog_file = test_utils::create_sandbox_file("netlist.v", netlist_input);
                VerilogParser verilog_parser;
                std::unique_ptr<Netlist> nl = verilog_parser.parse_and_instantiate(verilog_file, m_gl);
                EXPECT_EQ(nl, nullptr);
            }
            if(test_utils::known_issue_tests_active())
            {
                    // Use non-numeric ranges (invalid) ISSUE: stoi Failure
                    NO_COUT_TEST_BLOCK;
                    std::string netlist_input("module top ("
                             "  global_in,"
                             "  global_out"
                             " ) ;"
                             "  input global_in ;"
                             "  output global_out ;"
                             "  wire [0:4] signal_vec_0 ;"
                             "  wire [0:4] signal_vec_1 ;"
                             "  assign signal_vec_0[p:q] = signal_vec_1[p:q];"
                             "INV gate_0 ("
                             "  .\\I (global_in ),"
                             "  .\\O (global_out )"
                             " ) ;"
                             "endmodule");
                    auto verilog_file = test_utils::create_sandbox_file("netlist.v", netlist_input);
                VerilogParser verilog_parser;
                std::unique_ptr<Netlist> nl = verilog_parser.parse_and_instantiate(verilog_file, m_gl);
                    EXPECT_EQ(nl, nullptr);
            }
            // ------ Verilog specific tests ------
            {
                // The Module has no identifier
                NO_COUT_TEST_BLOCK;
                std::string netlist_input("module ("
                                        "  global_in,"
                                        "  global_out"
                                        " ) ;"
                                        "  input global_in ;"
                                        "  output global_out ;"
                                        "gate_1_to_1 gate_0 ("
                                        "  .I (global_in ),"
                                        "  .O (global_out )"
                                        " ) ;"
                                        "endmodule");
                auto verilog_file = test_utils::create_sandbox_file("netlist.v", netlist_input);
                VerilogParser verilog_parser;
                std::unique_ptr<Netlist> nl = verilog_parser.parse_and_instantiate(verilog_file, m_gl);
                EXPECT_EQ(nl, nullptr);
            }
            if(test_utils::known_issue_tests_active())
            {
                // one side of the direct assignment is empty (ISSUE: SIGSEGV l.1206)
                NO_COUT_TEST_BLOCK;
                std::string netlist_input("module top ("
                         "  global_in,"
                         "  global_out"
                         " ) ;"
                         "  input global_in ;"
                         "  output global_out ;"
                         "  wire signal_0 ;"
                         "  assign signal_0 = ;"
                         "endmodule");
                auto verilog_file = test_utils::create_sandbox_file("netlist.v", netlist_input);
                VerilogParser verilog_parser;
                std::unique_ptr<Netlist> nl = verilog_parser.parse_and_instantiate(verilog_file, m_gl);
                EXPECT_EQ(nl, nullptr);
            }
            {
                // Having a cyclic Module hierarchy
                NO_COUT_TEST_BLOCK;
                std::string netlist_input("module ENT_0 ("
                                        "  IE0,"
                                        "  OE0 "
                                        " ) ;"
                                        "  input IE0 ;"
                                        "  output OE0 ;"
                                        "ENT_1 gate_0 ("
                                        "  .\\IE1 (IE0 ),"
                                        "  .\\OE1 (OE0 )"
                                        " ) ;"
                                        "endmodule"
                                        "module ENT_1 ("
                                        "  IE1,"
                                        "  OE1 "
                                        " ) ;"
                                        "  input IE1 ;"
                                        "  output OE1 ;"
                                        "ENT_0 gate_0 ("
                                        "  .\\IE0 (IE1 ),"
                                        "  .\\OE0 (OE1 )"
                                        " ) ;"
                                        "endmodule");
                auto verilog_file = test_utils::create_sandbox_file("netlist.v", netlist_input);
                VerilogParser verilog_parser;
                std::unique_ptr<Netlist> nl = verilog_parser.parse_and_instantiate(verilog_file, m_gl);
                EXPECT_EQ(nl, nullptr);
            }
            {
                // Port map gets multiple nets
                NO_COUT_TEST_BLOCK;
                std::string netlist_input("module top ("
                                        "  global_in,"
                                        "  global_out "
                                        " ) ;"
                                        "  input global_in ;"
                                        "  output global_out ;"
                                        "  wire net_0;"
                                        "gate_1_to_1 gate_0 ("
                                        "  .I ({global_in, net_0} )"
                                        " ) ;"
                                        "gate_1_to_1 gate_1 ("
                                        "  .I (net_0 ),"
                                        "  .O (global_out )"
                                        " ) ;"
                                        "endmodule");
                auto verilog_file = test_utils::create_sandbox_file("netlist.v", netlist_input);
                VerilogParser verilog_parser;
                std::unique_ptr<Netlist> nl = verilog_parser.parse_and_instantiate(verilog_file, m_gl);
                ASSERT_NE(nl, nullptr);
                Gate* gate_0 = *nl->get_gates(test_utils::gate_name_filter("gate_0")).begin();
                ASSERT_NE(gate_0, nullptr);
                EXPECT_EQ(gate_0->get_fan_in_net("I")->get_name(), "net_0");
            }
            {
                // Connect a vector of output pins with a list of nets using '{ net_0, net_1, ... }' that is wider than
                // the input port size (only the last elements of the list should be assigned)
                std::string netlist_input("module top (\n"
                                        " ) ;\n"
                                        "  wire net_0;\n"
                                        "  wire net_1;\n"
                                        "  wire[5:0] l_vec;\n"
                                        "pin_group_gate_4_to_4 gate_0 (\n"
                                        "  .O (l_vec)\n"
                                        " ) ;\n"
                                        "endmodule");
                auto verilog_file = test_utils::create_sandbox_file("netlist.v", netlist_input);
                VerilogParser verilog_parser;
                std::unique_ptr<Netlist> nl = verilog_parser.parse_and_instantiate(verilog_file, m_gl);
                ASSERT_NE(nl, nullptr);
                ASSERT_FALSE(nl->get_gates(test_utils::gate_filter("pin_group_gate_4_to_4", "gate_0")).empty());
                Gate* gate_0 = *(nl->get_gates(test_utils::gate_filter("pin_group_gate_4_to_4", "gate_0")).begin());
                EXPECT_EQ(gate_0->get_fan_out_nets().size(), 4);
                Net* net_0 = gate_0->get_fan_out_net("O(0)");
                ASSERT_NE(net_0, nullptr);
                EXPECT_EQ(net_0->get_name(), "l_vec(0)");
                Net* net_1 = gate_0->get_fan_out_net("O(1)");
                ASSERT_NE(net_1, nullptr);
                EXPECT_EQ(net_1->get_name(), "l_vec(1)");
                Net* net_2 = gate_0->get_fan_out_net("O(2)");
                ASSERT_NE(net_2, nullptr);
                EXPECT_EQ(net_2->get_name(), "l_vec(2)");
                Net* net_3 = gate_0->get_fan_out_net("O(3)");
                ASSERT_NE(net_3, nullptr);
                EXPECT_EQ(net_3->get_name(), "l_vec(3)");

            }
            {
                // Store an unknown data type
                NO_COUT_TEST_BLOCK;
                std::string netlist_input("module top ("
                                        "  global_in,"
                                        "  global_out"
                                        " ) ;"
                                        "  input global_in ;"
                                        "  output global_out ;"
                                        "gate_1_to_1 #("
                                        ".key_unknown(#Unkn0wn!)) "
                                        "gate_0 ("
                                        "  .I (global_in ),"
                                        "  .O (global_out )"
                                        " ) ;"
                                        "endmodule");
                auto verilog_file = test_utils::create_sandbox_file("netlist.v", netlist_input);
                VerilogParser verilog_parser;
                std::unique_ptr<Netlist> nl = verilog_parser.parse_and_instantiate(verilog_file, m_gl);
                EXPECT_NE(nl, nullptr);
            }
            {
                // Assign one Net to another, but booth are connected to different source gates
                NO_COUT_TEST_BLOCK;
                std::string netlist_input("module top ("
                                        "  global_in,"
                                        "  global_out"
                                        " ) ;"
                                        "  input global_in ;"
                                        "  output global_out ;"
                                        "  wire net_0 ;"
                                        "  wire net_1 ;"
                                        "  assign net_0 = net_1 ;"
                                        "gate_1_to_1 gate_0 ("
                                        "  .I (global_in ),"
                                        "  .O (net_0 )"
                                        " ) ;"
                                        "gate_1_to_1 gate_1 ("
                                        "  .I (global_in ),"
                                        "  .O (net_1 )"
                                        " ) ;"
                                        "gate_2_to_1 gate_1 ("
                                        "  .I0 (net_0 ),"
                                        "  .I1 (net_1 ),"
                                        "  .O (global_out )"
                                        " ) ;"
                                        "endmodule");
                auto verilog_file = test_utils::create_sandbox_file("netlist.v", netlist_input);
                VerilogParser verilog_parser;
                std::unique_ptr<Netlist> nl = verilog_parser.parse_and_instantiate(verilog_file, m_gl);

                EXPECT_EQ(nl, nullptr);
            }
            {
                // Use an undeclared signal (should only throw a warning or return a nullptr)
                NO_COUT_TEST_BLOCK;
                std::string netlist_input("module top ("
                                        "  global_in,"
                                        "  global_out"
                                        " ) ;"
                                        "  input global_in ;"
                                        "  output global_out ;"
                                        "gate_1_to_1 gate_0 ("
                                        "  .I (global_in ),"
                                        "  .O (net_0 )"    // <- undeclared
                                        " ) ;"
                                        "gate_1_to_1 gate_1 ("
                                        "  .I (net_0 ),"
                                        "  .O (global_out )"
                                        " ) ;"
                                        "endmodule");
                auto verilog_file = test_utils::create_sandbox_file("netlist.v", netlist_input);
                VerilogParser verilog_parser;
                std::unique_ptr<Netlist> nl = verilog_parser.parse_and_instantiate(verilog_file, m_gl);

                EXPECT_TRUE(nl == nullptr || nl->get_nets(test_utils::net_name_filter("net_0")).size() == 1);
            }
            {
                // Assign unknown signals
                NO_COUT_TEST_BLOCK;
                std::string netlist_input("module top ("
                         "  global_in,"
                         "  global_out"
                         " ) ;"
                         "  input global_in ;"
                         "  output global_out ;"
                         "  wire [0:4] signal_vec ;"
                         "  assign signal_unknown[0:4] = signal_vec[0:4];"
                         "gate_1_to_1 gate_0 ("
                         "  .I (global_in ),"
                         "  .O (global_out )"
                         " ) ;"
                         "endmodule");
                auto verilog_file = test_utils::create_sandbox_file("netlist.v", netlist_input);
                VerilogParser verilog_parser;
                std::unique_ptr<Netlist> nl = verilog_parser.parse_and_instantiate(verilog_file, m_gl);
                EXPECT_EQ(nl, nullptr);
            }
        TEST_END
    }
} // namespace hal
