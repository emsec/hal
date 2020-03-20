#include "netlist/gate.h"
#include "netlist/gate_library/gate_library_manager.h"
#include "netlist/netlist.h"
#include "netlist/netlist_factory.h"
#include "netlist/persistent/netlist_serializer.h"
#include "netlist_test_utils.h"
#include "gtest/gtest.h"
#include <core/log.h>
#include "netlist/hdl_parser/hdl_parser_verilog.h"
#include <iostream>
#include <sstream>
#include <experimental/filesystem>
#include <bitset>

using namespace test_utils;

class hdl_parser_verilog_test : public ::testing::Test
{
protected:

    virtual void SetUp()
    {
        NO_COUT_BLOCK;
        gate_library_manager::load_all();
    }

    virtual void TearDown()
    {
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
TEST_F(hdl_parser_verilog_test, check_main_example)
{
    TEST_START
        {
            std::stringstream input("module top ("
                                    "  net_global_in,"
                                    "  net_global_out "
                                    " ) ;"
                                    "  input net_global_in ;"
                                    "  output net_global_out ;"
                                    "  wire net_0 ;"
                                    "  wire net_1 ;"
                                    "INV gate_0 ("
                                    "  .\\I (net_global_in ),"
                                    "  .\\O (net_0 )"
                                    " ) ;"
                                    "AND2 gate_1 ("
                                    "  .\\I0 (net_global_in ),"
                                    "  .\\I1 (net_global_in ),"
                                    "  .\\O (net_1 )"
                                    " ) ;"
                                    "AND3 gate_2 ("
                                    "  .\\I0 (net_0 ),"
                                    "  .\\I1 (net_1 ),"
                                    "  .\\O (net_global_out )"
                                    " ) ;"
                                    "endmodule");
            hdl_parser_verilog verilog_parser(input);
            std::shared_ptr<netlist> nl = verilog_parser.parse(g_lib_name);

            ASSERT_NE(nl, nullptr);

            // Check if the gates are parsed correctly
            ASSERT_EQ(nl->get_gates(gate_type_filter("INV")).size(), 1);
            std::shared_ptr<gate> gate_0 = *(nl->get_gates(gate_type_filter("INV")).begin());
            ASSERT_EQ(nl->get_gates(gate_type_filter("AND2")).size(), 1);
            std::shared_ptr<gate> gate_1 = *(nl->get_gates(gate_type_filter("AND2")).begin());
            ASSERT_EQ(nl->get_gates(gate_type_filter("AND3")).size(), 1);
            std::shared_ptr<gate> gate_2 = *(nl->get_gates(gate_type_filter("AND3")).begin());

            ASSERT_NE(gate_0, nullptr);
            EXPECT_EQ(gate_0->get_name(), "gate_0");

            ASSERT_NE(gate_1, nullptr);
            EXPECT_EQ(gate_1->get_name(), "gate_1");

            ASSERT_NE(gate_2, nullptr);
            EXPECT_EQ(gate_2->get_name(), "gate_2");

            // Check if the nets are parsed correctly
            std::shared_ptr<net> net_0            = *(nl->get_nets(net_name_filter("net_0")).begin());
            std::shared_ptr<net> net_1        = *(nl->get_nets(net_name_filter("net_1")).begin());
            std::shared_ptr<net> net_global_in    = *(nl->get_nets(net_name_filter("net_global_in")).begin());
            std::shared_ptr<net> net_global_out   = *(nl->get_nets(net_name_filter("net_global_out")).begin());

            ASSERT_NE(net_0, nullptr);
            EXPECT_EQ(net_0->get_name(), "net_0");
            EXPECT_EQ(net_0->get_src(), get_endpoint(gate_0, "O"));
            std::vector<endpoint> exp_net_0_dsts = {get_endpoint(gate_2, "I0")};
            EXPECT_TRUE(vectors_have_same_content(net_0->get_dsts(), std::vector<endpoint>({get_endpoint(gate_2, "I0")})));

            ASSERT_NE(net_1, nullptr);
            EXPECT_EQ(net_1->get_name(), "net_1");
            EXPECT_EQ(net_1->get_src(), get_endpoint(gate_1, "O"));
            EXPECT_TRUE(vectors_have_same_content(net_1->get_dsts(), std::vector<endpoint>({get_endpoint(gate_2, "I1")})));

            ASSERT_NE(net_global_in, nullptr);
            EXPECT_EQ(net_global_in->get_name(), "net_global_in");
            EXPECT_EQ(net_global_in->get_src(), get_endpoint(nullptr, ""));
            EXPECT_TRUE(vectors_have_same_content(net_global_in->get_dsts(), std::vector<endpoint>({get_endpoint(gate_0, "I"), get_endpoint(gate_1, "I0"), get_endpoint(gate_1, "I1")})));
            EXPECT_TRUE(nl->is_global_input_net(net_global_in));

            ASSERT_NE(net_global_out, nullptr);
            EXPECT_EQ(net_global_out->get_name(), "net_global_out");
            EXPECT_EQ(net_global_out->get_src(), get_endpoint(gate_2, "O"));
            EXPECT_TRUE(net_global_out->get_dsts().empty());
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
TEST_F(hdl_parser_verilog_test, check_whitespace_chaos)
{
    TEST_START
         {  
             std::stringstream input("module top(net_global_in,net_global_out);input net_global_in;\n"
                                     "  output net_global_out;wire net_0;wire net_1;INV gate_0 (\n"
                                     "  .\\I (net_global_in),\n"
                                     "\n"
                                     " \t.\\O (net_0 )\n"
                                     " );\n"
                                     "AND2 gate_1 (.\\I0\n\t"
                                     "  (\n"
                                     "    net_global_in\n"
                                     "   \t)\n"
                                     "    ,\n"
                                     "  .\\I1 (net_global_in),.\\O (net_1));\n"
                                     "AND3 gate_2 (.\\I0 (net_0 ),.\\I1 (net_1 ),.\\O (net_global_out ));endmodule");
            hdl_parser_verilog verilog_parser(input);
            std::shared_ptr<netlist> nl = verilog_parser.parse(g_lib_name);

            ASSERT_NE(nl, nullptr);

            // Check if the gates are parsed correctly
            ASSERT_EQ(nl->get_gates(gate_type_filter("INV")).size(), 1);
            std::shared_ptr<gate> gate_0 = *(nl->get_gates(gate_type_filter("INV")).begin());
            ASSERT_EQ(nl->get_gates(gate_type_filter("AND2")).size(), 1);
            std::shared_ptr<gate> gate_1 = *(nl->get_gates(gate_type_filter("AND2")).begin());
            ASSERT_EQ(nl->get_gates(gate_type_filter("AND3")).size(), 1);
            std::shared_ptr<gate> gate_2 = *(nl->get_gates(gate_type_filter("AND3")).begin());

            ASSERT_NE(gate_0, nullptr);
            EXPECT_EQ(gate_0->get_name(), "gate_0");

            ASSERT_NE(gate_1, nullptr);
            EXPECT_EQ(gate_1->get_name(), "gate_1");

            ASSERT_NE(gate_2, nullptr);
            EXPECT_EQ(gate_2->get_name(), "gate_2");

            // Check if the nets are parsed correctly
            std::shared_ptr<net> net_0            = *(nl->get_nets(net_name_filter("net_0")).begin());
            std::shared_ptr<net> net_1        = *(nl->get_nets(net_name_filter("net_1")).begin());
            std::shared_ptr<net> net_global_in    = *(nl->get_nets(net_name_filter("net_global_in")).begin());
            std::shared_ptr<net> net_global_out   = *(nl->get_nets(net_name_filter("net_global_out")).begin());

            ASSERT_NE(net_0, nullptr);
            EXPECT_EQ(net_0->get_name(), "net_0");
            EXPECT_EQ(net_0->get_src(), get_endpoint(gate_0, "O"));
            std::vector<endpoint> exp_net_0_dsts = {get_endpoint(gate_2, "I0")};
            EXPECT_TRUE(vectors_have_same_content(net_0->get_dsts(), std::vector<endpoint>({get_endpoint(gate_2, "I0")})));

            ASSERT_NE(net_1, nullptr);
            EXPECT_EQ(net_1->get_name(), "net_1");
            EXPECT_EQ(net_1->get_src(), get_endpoint(gate_1, "O"));
            EXPECT_TRUE(vectors_have_same_content(net_1->get_dsts(), std::vector<endpoint>({get_endpoint(gate_2, "I1")})));

            ASSERT_NE(net_global_in, nullptr);
            EXPECT_EQ(net_global_in->get_name(), "net_global_in");
            EXPECT_EQ(net_global_in->get_src(), get_endpoint(nullptr, ""));
            EXPECT_TRUE(vectors_have_same_content(net_global_in->get_dsts(), std::vector<endpoint>({get_endpoint(gate_0, "I"), get_endpoint(gate_1, "I0"), get_endpoint(gate_1, "I1")})));
            EXPECT_TRUE(nl->is_global_input_net(net_global_in));

            ASSERT_NE(net_global_out, nullptr);
            EXPECT_EQ(net_global_out->get_name(), "net_global_out");
            EXPECT_EQ(net_global_out->get_src(), get_endpoint(gate_2, "O"));
            EXPECT_TRUE(net_global_out->get_dsts().empty());
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
TEST_F(hdl_parser_verilog_test, check_generic_map)
{
    TEST_START
        {
            // Store an instance of all possible data types in one gate
            std::stringstream input("module top ("
                                    "  global_in,"
                                    "  global_out"
                                    " ) ;"
                                    "  input global_in ;"
                                    "  output global_out ;"
                                    "INV #("
                                    ".key_integer(1234),"
                                    ".key_floating_point(1.234),"
                                    ".key_string(\"test_string\"),"
                                    ".key_bit_vector_hex('habc),"           // All values are 'ABC' in hex
                                    ".key_bit_vector_dec('d2748),"
                                    ".key_bit_vector_oct('o5274),"
                                    ".key_bit_vector_bin('b1010_1011_1100)) "
                                    "gate_0 ("
                                    "  .\\I (global_in ),"
                                    "  .\\O (global_out )"
                                    " ) ;"
                                    "endmodule");
            test_def::capture_stdout();
            hdl_parser_verilog verilog_parser(input);
            std::shared_ptr<netlist> nl = verilog_parser.parse(g_lib_name);
            if (nl == nullptr)
            {
                std::cout << test_def::get_captured_stdout();
            }
            else
            {
                test_def::get_captured_stdout();
            }

            ASSERT_NE(nl, nullptr);

            ASSERT_EQ(nl->get_gates(gate_filter("INV","gate_0")).size(), 1);
            std::shared_ptr<gate> gate_0 = *nl->get_gates(gate_filter("INV","gate_0")).begin();

            // Integers are stored in their hex representation
            EXPECT_EQ(gate_0->get_data_by_key("generic","key_integer"), std::make_tuple("integer", "1234"));
            //EXPECT_EQ(gate_0->get_data_by_key("generic","key_floating_point"), std::make_tuple("floating_point", "1.234")); // ISSUE: output: ("integer", "1")
            EXPECT_EQ(gate_0->get_data_by_key("generic","key_string"), std::make_tuple("string", "test_string"));
            EXPECT_EQ(gate_0->get_data_by_key("generic","key_bit_vector_hex"), std::make_tuple("bit_vector", "abc"));
            EXPECT_EQ(gate_0->get_data_by_key("generic","key_bit_vector_dec"), std::make_tuple("bit_vector", "abc"));
            EXPECT_EQ(gate_0->get_data_by_key("generic","key_bit_vector_oct"), std::make_tuple("bit_vector", "abc"));
            EXPECT_EQ(gate_0->get_data_by_key("generic","key_bit_vector_bin"), std::make_tuple("bit_vector", "abc"));
        }

    TEST_END
}

/**
 * Testing the handling of net-vectors in dimension 1-3
 *
 * Functions: parse
 */
TEST_F(hdl_parser_verilog_test, check_net_vectors)
{
    TEST_START
        create_temp_gate_lib();
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
            std::stringstream input("module top ( "
                                    "  global_in, "
                                    "  global_out "
                                    " ) ; "
                                    "  input global_in ; "
                                    "  output global_out ; "
                                    "  wire [0:3] n_vec_1 ; "
                                    "  wire [3:0] n_vec_2 ; "
                                    " GATE_1^0_IN_4^1_OUT gate_0 ( "
                                    "  .\\I (global_in ), "
                                    "  .\\O(0) (n_vec_1[0]), "
                                    "  .\\O(1) (n_vec_1[1]), "
                                    "  .\\O(2) (n_vec_1[2]), "
                                    "  .\\O(3) (n_vec_1[3]) "
                                    " ) ; "
                                    " GATE_4^1_IN_4^1_OUT gate_1 ( "
                                    "   .\\I(0) (n_vec_1[0]), "
                                    "   .\\I(1) (n_vec_1[1]), "
                                    "   .\\I(2) (n_vec_1[2]), "
                                    "   .\\I(3) (n_vec_1[3]), "
                                    "   .\\O(0) (n_vec_2[0]), "
                                    "   .\\O(1) (n_vec_2[1]), "
                                    "   .\\O(2) (n_vec_2[2]), "
                                    "   .\\O(3) (n_vec_2[3]) "
                                    "  ) ; "
                                    "  GATE_4^1_IN_1^0_OUT gate_2 ( "
                                    "    .\\I(0) (n_vec_2[0]), "
                                    "    .\\I(1) (n_vec_2[1]), "
                                    "    .\\I(2) (n_vec_2[2]), "
                                    "    .\\I(3) (n_vec_2[3]), "
                                    "    .\\O (global_out ) "
                                    "   ) ; "
                                    "endmodule");
            test_def::capture_stdout();
            hdl_parser_verilog verilog_parser(input);
            std::shared_ptr<netlist> nl = verilog_parser.parse(temp_lib_name);
            if (nl == nullptr)
            {
                std::cout << test_def::get_captured_stdout();
            }
            else
            {
                test_def::get_captured_stdout();
            }

            ASSERT_NE(nl, nullptr);

            // Check that all nets are created and connected correctly
            EXPECT_EQ(nl->get_nets().size(), 10); // net_global_in + net_global_out + 4 nets in n_vec_1 + 4 nets in n_vec_2
            for(auto net_name : std::set<std::string>({"n_vec_1(0)","n_vec_1(1)","n_vec_1(2)","n_vec_1(3)","n_vec_2(0)","n_vec_2(1)","n_vec_2(2)","n_vec_2(3)"})){
                ASSERT_EQ(nl->get_nets(net_name_filter(net_name)).size(), 1);
            }
            for(unsigned i = 0; i < 4; i++){
                std::string i_str = std::to_string(i);
                std::shared_ptr<net> n_vec_1_i = *nl->get_nets(net_name_filter("n_vec_1(" + i_str + ")")).begin();
                std::shared_ptr<net> n_vec_2_i = *nl->get_nets(net_name_filter("n_vec_2(" + i_str + ")")).begin();
                EXPECT_EQ(n_vec_1_i->get_src().get_pin_type(), "O(" + i_str + ")");
                EXPECT_EQ((*n_vec_1_i->get_dsts().begin()).get_pin_type(), "I(" + i_str + ")");
                EXPECT_EQ(n_vec_2_i->get_src().get_pin_type(), "O(" + i_str + ")");
                EXPECT_EQ((*n_vec_2_i->get_dsts().begin()).get_pin_type(), "I(" + i_str + ")");
            }
        }

        {
            // Use a logic vector of dimension two
            std::stringstream input("module top ( "
                                    "  global_in, "
                                    "  global_out "
                                    " ) ; "
                                    "  input global_in ; "
                                    "  output global_out ; "
                                    "  wire [0:1][2:3] n_vec ; "
                                    "GATE_1^0_IN_4^1_OUT gate_0 ( "
                                    "  .\\I (global_in ), "
                                    "  .\\O(0) (n_vec[0][2]), "
                                    "  .\\O(1) (n_vec[0][3]), "
                                    "  .\\O(2) (n_vec[1][2]), "
                                    "  .\\O(3) (n_vec[1][3]) "
                                    " ) ; "
                                    "GATE_4^1_IN_1^0_OUT gate_1 ( "
                                    "  .\\I(0) (n_vec[0][2]), "
                                    "  .\\I(1) (n_vec[0][3]), "
                                    "  .\\I(2) (n_vec[1][2]), "
                                    "  .\\I(3) (n_vec[1][3]), "
                                    "  .\\O (global_out ) "
                                    " ) ; "
                                    "endmodule");
            test_def::capture_stdout();
            hdl_parser_verilog verilog_parser(input);
            std::shared_ptr<netlist> nl = verilog_parser.parse(temp_lib_name);
            if (nl == nullptr)
            {
                std::cout << test_def::get_captured_stdout();
            }
            else
            {
                test_def::get_captured_stdout();
            }

            ASSERT_NE(nl, nullptr);

            // Check that all nets are created and connected correctly
            EXPECT_EQ(nl->get_nets().size(), 6); // net_global_in + global_out + 4 nets in n_vec
            unsigned pin = 0;
            for(auto n : std::vector<std::string>({"n_vec(0)(2)","n_vec(0)(3)","n_vec(1)(2)","n_vec(1)(3)"})){
                ASSERT_FALSE(nl->get_nets(net_name_filter(n)).empty());
                std::shared_ptr<net> n_vec_i_j = *nl->get_nets(net_name_filter(n)).begin();
                EXPECT_EQ(n_vec_i_j->get_src().get_pin_type(), "O(" + std::to_string(pin) + ")");
                EXPECT_EQ((*n_vec_i_j->get_dsts().begin()).get_pin_type(), "I(" + std::to_string(pin) + ")");
                pin++;
            }
        }
        {
            // Use a net vector of dimension three
            std::stringstream input("module top ( "
                                    "  global_in, "
                                    "  global_out "
                                    " ) ; "
                                    "  input global_in ; "
                                    "  output global_out ; "
                                    "  wire [0:1][1:0][0:1] n_vec ; "
                                    "GATE_1^0_IN_2^3_OUT gate_0 ( "
                                    "  .\\I (global_in ), "
                                    "  .\\O(0,0,0) (n_vec[0][0][0]), "
                                    "  .\\O(0,0,1) (n_vec[0][0][1]), "
                                    "  .\\O(0,1,0) (n_vec[0][1][0]), "
                                    "  .\\O(0,1,1) (n_vec[0][1][1]), "
                                    "  .\\O(1,0,0) (n_vec[1][0][0]), "
                                    "  .\\O(1,0,1) (n_vec[1][0][1]), "
                                    "  .\\O(1,1,0) (n_vec[1][1][0]), "
                                    "  .\\O(1,1,1) (n_vec[1][1][1]) "
                                    " ) ; "
                                    "GATE_2^3_IN_1^0_OUT gate_1 ( "
                                    "  .\\I(0,0,0) (n_vec[0][0][0]), "
                                    "  .\\I(0,0,1) (n_vec[0][0][1]), "
                                    "  .\\I(0,1,0) (n_vec[0][1][0]), "
                                    "  .\\I(0,1,1) (n_vec[0][1][1]), "
                                    "  .\\I(1,0,0) (n_vec[1][0][0]), "
                                    "  .\\I(1,0,1) (n_vec[1][0][1]), "
                                    "  .\\I(1,1,0) (n_vec[1][1][0]), "
                                    "  .\\I(1,1,1) (n_vec[1][1][1]), "
                                    "  .\\O (global_out ) "
                                    " ) ; "
                                    "endmodule");
            test_def::capture_stdout();
            hdl_parser_verilog verilog_parser(input);
            std::shared_ptr<netlist> nl = verilog_parser.parse(temp_lib_name);
            if (nl == nullptr)
            {
                std::cout << test_def::get_captured_stdout();
            }
            else
            {
                test_def::get_captured_stdout();
            }
            ASSERT_NE(nl, nullptr);

            // Check that all nets are created and connected correctly
            EXPECT_EQ(nl->get_nets().size(), 10); // net_global_in + net_global_out + 8 nets in n_vec
            std::vector<std::string> net_idx({"(0)(0)(0)","(0)(0)(1)","(0)(1)(0)","(0)(1)(1)",
                                              "(1)(0)(0)","(1)(0)(1)","(1)(1)(0)","(1)(1)(1)"});
            std::vector<std::string> pin_idx({"(0,0,0)","(0,0,1)","(0,1,0)","(0,1,1)",
                                      "(1,0,0)","(1,0,1)","(1,1,0)","(1,1,1)"});
            for(size_t idx = 0; idx < 8; idx++){
                ASSERT_FALSE(nl->get_nets(net_name_filter("n_vec"+net_idx[idx])).empty());
                std::shared_ptr<net> n_vec_i_j = *nl->get_nets(net_name_filter("n_vec"+net_idx[idx])).begin();
                EXPECT_EQ(n_vec_i_j->get_src().get_pin_type(), "O" + pin_idx[idx]);
                EXPECT_EQ((*n_vec_i_j->get_dsts().begin()).get_pin_type(), "I" + pin_idx[idx]);
            }

        }
        remove_temp_gate_lib();
    TEST_END
}

/**
 * Testing the addition of a gnd and vcc gates
 *
 * Functions: parse
 */
TEST_F(hdl_parser_verilog_test, check_gnd_vcc_gates)
{
    TEST_START
        {
            // Add a global_gnd
            std::stringstream input("module top ("
                                    "  global_out"
                                    " ) ;"
                                    "  output global_out ;"
                                    "GND g_gnd_gate ("
                                    "  .\\O (global_out )"
                                    " ) ;"
                                    "endmodule");
            test_def::capture_stdout();
            hdl_parser_verilog verilog_parser(input);
            std::shared_ptr<netlist> nl = verilog_parser.parse(g_lib_name);
            if (nl == nullptr)
            {
                std::cout << test_def::get_captured_stdout();
            }
            else
            {
                test_def::get_captured_stdout();
            }

            ASSERT_NE(nl, nullptr);

            ASSERT_NE(nl->get_gnd_gates().size(), 0);
            std::shared_ptr<gate> global_gnd = *nl->get_gnd_gates().begin();
            EXPECT_EQ(global_gnd->get_name(), "g_gnd_gate");
        }
        {
            // Add a global_vcc
            std::stringstream input("module top ("
                                    "  global_out"
                                    " ) ;"
                                    "  output global_out ;"
                                    "VCC g_vcc_gate ("
                                    "  .\\O (global_out )"
                                    " ) ;"
                                    "endmodule");
            test_def::capture_stdout();
            hdl_parser_verilog verilog_parser(input);
            std::shared_ptr<netlist> nl = verilog_parser.parse(g_lib_name);
            if (nl == nullptr)
            {
                std::cout << test_def::get_captured_stdout();
            }
            else
            {
                test_def::get_captured_stdout();
            }

            ASSERT_NE(nl, nullptr);

            ASSERT_NE(nl->get_vcc_gates().size(), 0);
            std::shared_ptr<gate> global_vcc = *nl->get_vcc_gates().begin();
            EXPECT_EQ(global_vcc->get_name(), "g_vcc_gate");
        }
    TEST_END
}

/**
 * Testing the port assigment of the nets '0' and '1' (by 'b0 and 'b1), that are connected to gnd/vcc gates.
 *
 * Functions: parse
 */
TEST_F(hdl_parser_verilog_test, check_zero_and_one_nets)
{
    TEST_START
        {
            // Use zero and one as inputs (via 'b0 and 'b1)
            std::stringstream input;
            input << "module top ("
                     "  global_out_0,"
                     "  global_out_1 "
                     " ) ;"
                     "  output global_out_0 ;"
                     "  output global_out_1 ;"
                     "INV gate_0 ("
                     "  .\\I ('b0 ),"
                     "  .\\O (global_out_0)"
                     " ) ;"
                     "INV gate_1 ("
                     "  .\\I ('b1 ),"
                     "  .\\O (global_out_1)"
                     " ) ;"
                     "endmodule";
            hdl_parser_verilog verilog_parser(input);
            std::shared_ptr<netlist> nl = verilog_parser.parse(g_lib_name);

            EXPECT_NE(nl, nullptr);
            ASSERT_EQ(nl->get_gates(gate_filter("INV", "gate_0")).size(), 1);
            ASSERT_EQ(nl->get_gates(gate_filter("INV", "gate_1")).size(), 1);
            std::shared_ptr<gate> gate_0 = *nl->get_gates(gate_filter("INV", "gate_0")).begin();
            std::shared_ptr<gate> gate_1 = *nl->get_gates(gate_filter("INV", "gate_1")).begin();

            // Test that the nets '0' and '1' are created and connected
            std::shared_ptr<net> net_gnd = gate_0->get_fan_in_net("I");
            std::shared_ptr<net> net_vcc = gate_1->get_fan_in_net("I");
            ASSERT_NE(net_gnd, nullptr);
            ASSERT_NE(net_vcc, nullptr);
            EXPECT_EQ(net_gnd->get_name(), "\'0\'");
            EXPECT_EQ(net_vcc->get_name(), "\'1\'");

            // Test that the nets '0' and '1' are connected to a created global gnd/vcc gate
            ASSERT_NE(net_gnd->get_src().get_gate(), nullptr);
            ASSERT_NE(net_vcc->get_src().get_gate(), nullptr);
            EXPECT_TRUE(net_gnd->get_src().get_gate()->is_gnd_gate());
            EXPECT_TRUE(net_vcc->get_src().get_gate()->is_vcc_gate());
        }
    TEST_END
}

/**
 * Testing the usage of additional entities. Entities that are used by the main entity (the last one) are recursively
 * split in gates that are part of the gate library, while the original entity hierarchy is represented by the module-
 * hierarchy of the netlist. Therefore, there can be multiple gates with the same name, so names that occur twice or
 * more will be extended by a unique suffix.
 *
 * Functions: parse
 */
TEST_F(hdl_parser_verilog_test, check_multiple_entities)
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
            std::stringstream input("module ENT_CHILD ("
                                    "  child_in,"
                                    "  child_out"
                                    " ) ;"
                                    "  input child_in ;"
                                    "  output child_out ;"
                                    "  wire net_0_child ;"
                                    "INV gate_0_child ("
                                    "  .\\I (child_in ),"
                                    "  .\\O (net_0_child )"
                                    " ) ;"
                                    "INV gate_1_child ("
                                    "  .\\I (net_0_child ),"
                                    "  .\\O (child_out )"
                                    " ) ;"
                                    "endmodule"
                                    "\n"
                                    "module ENT_TO ("
                                    "  net_global_in,"
                                    "  net_global_out"
                                    " ) ;"
                                    "  input net_global_in ;"
                                    "  output net_global_out ;"
                                    "  wire net_0 ;"
                                    "  wire net_1 ;"
                                    "INV gate_0 ("
                                    "  .\\I (net_global_in ),"
                                    "  .\\O (net_0 )"
                                    " ) ;"
                                    "ENT_CHILD child_mod ("
                                    "  .\\child_in (net_0 ),"
                                    "  .\\child_out (net_1 )"
                                    " ) ;"
                                    "INV gate_1 ("
                                    "  .\\I (net_1 ), "
                                    "  .\\O (net_global_out )"
                                    " ) ;"
                                    "endmodule");
            hdl_parser_verilog verilog_parser(input);
            std::shared_ptr<netlist> nl = verilog_parser.parse(g_lib_name);

            // Test that all gates are created
            ASSERT_NE(nl, nullptr);
            ASSERT_EQ(nl->get_gates(gate_filter("INV", "gate_0")).size(), 1);
            ASSERT_EQ(nl->get_gates(gate_filter("INV", "gate_1")).size(), 1);
            ASSERT_EQ(nl->get_gates(gate_filter("INV", "gate_0_child")).size(), 1);
            ASSERT_EQ(nl->get_gates(gate_filter("INV", "gate_1_child")).size(), 1);
            std::shared_ptr<gate> gate_0 = *nl->get_gates(gate_filter("INV", "gate_0")).begin();
            std::shared_ptr<gate> gate_1 = *nl->get_gates(gate_filter("INV", "gate_1")).begin();
            std::shared_ptr<gate> gate_0_child = *nl->get_gates(gate_filter("INV", "gate_0_child")).begin();
            std::shared_ptr<gate> gate_1_child = *nl->get_gates(gate_filter("INV", "gate_1_child")).begin();

            // Test that all nets are created
            ASSERT_EQ(nl->get_nets(net_name_filter("net_0")).size(), 1);
            ASSERT_EQ(nl->get_nets(net_name_filter("net_1")).size(), 1);
            ASSERT_EQ(nl->get_nets(net_name_filter("net_global_in")).size(), 1);
            ASSERT_EQ(nl->get_nets(net_name_filter("net_global_out")).size(), 1);
            ASSERT_EQ(nl->get_nets(net_name_filter("net_0_child")).size(), 1);
            std::shared_ptr<net> net_0 = *nl->get_nets(net_name_filter("net_0")).begin();
            std::shared_ptr<net> net_1 = *nl->get_nets(net_name_filter("net_1")).begin();
            std::shared_ptr<net> net_global_in= *nl->get_nets(net_name_filter("net_global_in")).begin();
            std::shared_ptr<net> net_global_out= *nl->get_nets(net_name_filter("net_global_out")).begin();
            std::shared_ptr<net> net_0_child= *nl->get_nets(net_name_filter("net_0_child")).begin();

            // Test that all nets are connected correctly
            EXPECT_EQ(gate_0->get_fan_in_net("I"), net_global_in);
            EXPECT_EQ(gate_0->get_fan_out_net("O"), net_0);
            EXPECT_EQ(gate_1->get_fan_in_net("I"), net_1);
            EXPECT_EQ(gate_1->get_fan_out_net("O"), net_global_out);
            EXPECT_EQ(gate_0_child->get_fan_in_net("I"), net_0);
            EXPECT_EQ(gate_0_child->get_fan_out_net("O"), net_0_child);
            EXPECT_EQ(gate_1_child->get_fan_in_net("I"), net_0_child);
            EXPECT_EQ(gate_1_child->get_fan_out_net("O"), net_1);

            // Test that the modules are created and assigned correctly
            std::shared_ptr<module> top_mod = nl->get_top_module();
            ASSERT_EQ(top_mod->get_submodules().size(), 1);
            std::shared_ptr<module> child_mod = *top_mod->get_submodules().begin();
            EXPECT_EQ(child_mod->get_name(), "ENT_CHILD");
            EXPECT_EQ(top_mod->get_gates(), std::set<std::shared_ptr<gate>>({gate_0, gate_1}));
            EXPECT_EQ(child_mod->get_gates(), std::set<std::shared_ptr<gate>>({gate_0_child, gate_1_child}));
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
            // Testing the correct build of the module hierarchy. Moreover the correct substitution of gate and net names,
            // which would be added twice (because an entity can be used multiple times) is tested as well.

            std::stringstream input("module ENT_CHILD_TWO ( "
                                    "  I_c2, "
                                    "  O_c2 "
                                    " ) ; "
                                    "  input I_c2 ; "
                                    "  output O_c2 ; "
                                    "INV gate_child_two ( "
                                    "  .\\I (I_c2 ), "
                                    "  .\\O (O_c2 ) "
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
                                    "INV gate_child_one ( "
                                    "  .\\I (net_child_1 ), "
                                    "  .\\O (O_c1 ) "
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
                                    "INV gate_top ("
                                    "  .\\I (net_1 ),"
                                    "  .\\O (net_global_out )"
                                    " ) ;"
                                    "endmodule");
            hdl_parser_verilog verilog_parser(input);
            std::shared_ptr<netlist> nl = verilog_parser.parse(g_lib_name);

            // Test if all modules are created and assigned correctly
            ASSERT_NE(nl, nullptr);
            EXPECT_EQ(nl->get_gates().size(), 5); // 3 * gate_child_two + gate_child_one + gate_top
            EXPECT_EQ(nl->get_modules().size(), 5); // 3 * ENT_CHILD_TWO + ENT_CHILD_ONE + ENT_TOP
            std::shared_ptr<module> top_module = nl->get_top_module();

            ASSERT_EQ(top_module->get_submodules().size(), 2);
            std::shared_ptr<module> top_child_one = *top_module->get_submodules().begin();
            std::shared_ptr<module> top_child_two = *(++top_module->get_submodules().begin());
            if (top_child_one->get_submodules().empty()){
                std::swap(top_child_one, top_child_two);
            }

            ASSERT_EQ(top_child_one->get_submodules().size(), 2);
            std::shared_ptr<module> one_child_0 = *(top_child_one->get_submodules().begin());
            std::shared_ptr<module> one_child_1 = *(++top_child_one->get_submodules().begin());

            // Test if all names that are used multiple times are substituted correctly
            std::string module_suffix = "_module_inst";

            EXPECT_EQ(top_child_one->get_name(), "ENT_CHILD_ONE");

            EXPECT_TRUE(core_utils::starts_with(top_child_two->get_name(), "ENT_CHILD_TWO" + module_suffix));
            EXPECT_TRUE(core_utils::starts_with(one_child_0->get_name(), "ENT_CHILD_TWO" + module_suffix));
            EXPECT_TRUE(core_utils::starts_with(one_child_1->get_name(), "ENT_CHILD_TWO" + module_suffix));
            // All 3 names should be unique
            EXPECT_EQ(std::set<std::string>({top_child_two->get_name(), one_child_0->get_name(), one_child_1->get_name()}).size(), 3);

            // Test if the gate names are substituted correctly as well (gate_child_two is used multiple times)
            std::string gate_suffix = "_module_inst";

            ASSERT_EQ(top_module->get_gates().size(), 1);
            EXPECT_EQ((*top_module->get_gates().begin())->get_name(), "gate_top");

            ASSERT_EQ(top_child_one->get_gates().size(), 1);
            EXPECT_EQ((*top_child_one->get_gates().begin())->get_name(), "gate_child_one");

            ASSERT_EQ( top_child_two->get_gates().size(), 1);
            ASSERT_EQ( one_child_0->get_gates().size(), 1);
            ASSERT_EQ( one_child_1->get_gates().size(), 1);
            std::shared_ptr<gate> gate_child_two_0 = *top_child_two->get_gates().begin();
            std::shared_ptr<gate> gate_child_two_1 = *one_child_0->get_gates().begin();
            std::shared_ptr<gate> gate_child_two_2 = *one_child_1->get_gates().begin();

            EXPECT_TRUE(core_utils::starts_with(gate_child_two_0->get_name(), "gate_child_two" + gate_suffix));
            EXPECT_TRUE(core_utils::starts_with(gate_child_two_1->get_name(), "gate_child_two" + gate_suffix));
            EXPECT_TRUE(core_utils::starts_with(gate_child_two_2->get_name(), "gate_child_two" + gate_suffix));
            // All 3 names should be unique
            EXPECT_EQ(std::set<std::string>({gate_child_two_0->get_name(), gate_child_two_1->get_name(), gate_child_two_2->get_name()}).size(), 3);

            // Test the creation on generic data of the module child_one_mod
            EXPECT_EQ(top_child_one->get_data_by_key("generic","child_one_mod_key"), std::make_tuple("integer", "1234"));
        }
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

            std::stringstream input("module ENT_MODULE ( "
                                    "  mod_in, "
                                    "  mod_out "
                                    " ) ; "
                                    "  input mod_in ; "
                                    "  output mod_out ; "
                                    "  wire mod_inner ; "
                                    "  assign mod_out = mod_inner ; "
                                    "INV gate_a ( "
                                    "  .\\I (mod_in ), "
                                    "  .\\O (mod_inner ) "
                                    " ) ; "
                                    "INV gate_b ( "
                                    "  .\\I (mod_inner ) "
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
                                    "INV gate_top ( "
                                    "  .\\I (net_0 ), "
                                    "  .\\O (net_global_out ) "
                                    " ) ; "
                                    "endmodule");
            hdl_parser_verilog verilog_parser(input);
            std::shared_ptr<netlist> nl = verilog_parser.parse(g_lib_name);

            // Test if all modules are created and assigned correctly
            ASSERT_NE(nl, nullptr);
            EXPECT_EQ(nl->get_gates().size(), 3); // 1 in top + 2 in mod
            EXPECT_EQ(nl->get_modules().size(), 2); // top + mod
            std::shared_ptr<module> top_module = nl->get_top_module();

            ASSERT_EQ(top_module->get_submodules().size(), 1);
            std::shared_ptr<module> mod = *top_module->get_submodules().begin();

            ASSERT_EQ(mod->get_gates(gate_name_filter("gate_a")).size(), 1);
            ASSERT_EQ(mod->get_gates(gate_name_filter("gate_b")).size(), 1);
            ASSERT_EQ(nl->get_gates(gate_name_filter("gate_top")).size(), 1);
            std::shared_ptr<gate> gate_a = *mod->get_gates(gate_name_filter("gate_a")).begin();
            std::shared_ptr<gate> gate_b = *mod->get_gates(gate_name_filter("gate_b")).begin();
            std::shared_ptr<gate> gate_top = *nl->get_gates(gate_name_filter("gate_top")).begin();

            std::shared_ptr<net> mod_out = gate_a->get_fan_out_net("O");
            ASSERT_NE(mod_out, nullptr);
            ASSERT_EQ(mod->get_output_nets().size(), 1);
            EXPECT_EQ(*mod->get_output_nets().begin(), mod_out);

            EXPECT_TRUE(vectors_have_same_content(mod_out->get_dsts(), std::vector<endpoint>({get_endpoint(gate_b, "I"), get_endpoint(gate_top, "I")})));
        }
    TEST_END
}

/**
 * Testing the correct handling of direct assignment (e.g. 'assign net_slave = net_master;'), where two
 * (wire-)identifiers address the same net. The net (wire) at the left side of the expression is mapped to the net
 * at the right side, so only the net with the right identifier will be created.
 *
 * In Verilog, assignemts of vectors (for example: assign sig_vec_1[0:3] = sig_vec_2[0:3]) and assignments of lists
 * of vectors and/or single nets (for example: "assign {sig_vec_1, signal_1} = {sig_vec_big[0:4]}", if |sig_vec_1|=4)
 * are also supported.
 *
 * However, logic expressions like 'assign A =  B ^ C' are NOT supported.
 *
 * Functions: parse
 */
TEST_F(hdl_parser_verilog_test, check_direct_assignment)
{
    TEST_START
        create_temp_gate_lib();
        {
            // (ISSUE: global input property is not propagated to net_master)
            // Build up a master-slave hierarchy as follows: (NOTE: Whats up with global inputs?)
            /*                                  .--- net_slave_1 (is global input)
             *   net_master <--- net_slave_0 <--+
             *                                  '--- net_slave_2
             */
            std::stringstream input("module top (\n"
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
                                    "INV gate_0 (\n"
                                    "  .\\I (net_global_in ),\n"
                                    "  .\\O (net_slave_0 )\n"
                                    " ) ;\n"
                                    "AND3 gate_1 (\n"
                                    " .\\I0 (net_master ),\n"
                                    " .\\I1 (net_slave_1 ),\n"
                                    " .\\I2 (net_slave_2 ),\n"
                                    " .\\O (net_global_out )\n"
                                    ") ;\n"
                                    "endmodule");
            hdl_parser_verilog verilog_parser(input);
            std::shared_ptr<netlist> nl = verilog_parser.parse(g_lib_name);

            ASSERT_NE(nl, nullptr);
            EXPECT_EQ(nl->get_nets().size(), 3); // global_in + global_out + net_master
            ASSERT_EQ(nl->get_nets(net_name_filter("net_master")).size(), 1);
            std::shared_ptr<net> net_master = *nl->get_nets(net_name_filter("net_master")).begin();

            ASSERT_EQ(nl->get_gates(gate_filter("INV","gate_0")).size(), 1);
            ASSERT_EQ(nl->get_gates(gate_filter("AND3","gate_1")).size(), 1);

            std::shared_ptr<gate> g_0 = *nl->get_gates(gate_filter("INV","gate_0")).begin();
            std::shared_ptr<gate> g_1 = *nl->get_gates(gate_filter("AND3","gate_1")).begin();

            // Check the connections
            EXPECT_EQ(g_0->get_fan_out_net("O"), net_master);
            EXPECT_EQ(g_1->get_fan_in_net("I0"), net_master);
            EXPECT_EQ(g_1->get_fan_in_net("I1"), net_master);
            EXPECT_EQ(g_1->get_fan_in_net("I2"), net_master);

            // Check that net_master becomes also a global input (ISSUE: global input property is not propagated to net_master)
            //EXPECT_TRUE(net_master->is_global_input_net());
        }
        // -- Verilog Specific Tests
        {
            // Verilog specific: Testing assignments with logic vectors (assign wires 0 and 1 of each dimension)
            // for example (for dim 2): wire [0:1][0:1] slave_vector; wire [0:3] master_vector; assign slave_vector = master_vector;

            // Will be tested for dimensions up to MAX_DIMENSION (runtime growth exponentially)
            const u8 MAX_DIMENSION = 3; // Can be turned up, if you are bored ;)

            for (u8 dim = 0; dim <= MAX_DIMENSION; dim++){

                std::stringstream global_out_list_module;
                std::stringstream global_out_list;
                std::stringstream gate_list;
                std::string dim_decl = "";

                dim_decl += "";
                for (u8 d = 0; d < dim; d++){
                    dim_decl += "[0:1]";
                }

                // 2^(dim) gates (with one pin) must be created to connect all assigned wires
                for (u64 i = 0; i < (1 << dim); i++){

                    global_out_list_module << "  global_out_" << i << ",";
                    global_out_list << "  output global_out_" << i << ";";


                    std::bitset<64> i_bs(i);
                    std::stringstream brackets("");
                    for (int j = dim-1; j>=0; j--){
                        brackets << "[" << (i_bs[j] ? "1":"0") << "]";
                    }

                    gate_list <<   "INV in_gate_"<< i <<" ("
                                                        "  .\\I (global_in ),"
                                                        "  .\\O ( net_slave_vector" << brackets.str() << ")"
                                                                                                         " ) ;";
                    gate_list <<   "INV out_gate_"<< i <<" ("
                                                         "  .\\I (net_slave_vector" << brackets.str() <<  "),"
                                                                                                          "  .\\O ( global_out_" << i << ")"
                                                                                                                                         " ) ;";
                }

                std::stringstream input;
                input << "module top ("
                         "  global_in,"
                      <<    global_out_list_module.str()
                      << "  );"
                         "  input global_in ;"
                      <<    global_out_list.str()
                      << "  wire " << dim_decl << " net_slave_vector;"
                      << "  wire [0:"<< ((1 << dim)-1) <<"] net_master_vector;"
                      << "  assign net_slave_vector = net_master_vector;" // <- !!!
                      << gate_list.str()
                      << "endmodule";


                test_def::capture_stdout();
                hdl_parser_verilog verilog_parser(input);
                std::shared_ptr<netlist> nl = verilog_parser.parse(g_lib_name);
                if (nl == nullptr)
                {
                    std::cout << test_def::get_captured_stdout();
                }
                else
                {
                    test_def::get_captured_stdout();
                }

                for (u64 i = 0; i < (1 << dim); i++){
                    ASSERT_NE(nl, nullptr);

                    ASSERT_EQ(nl->get_nets(net_name_filter("net_master_vector("+std::to_string(i)+")")).size(), 1);
                    std::shared_ptr<net> net_i = *nl->get_nets(net_name_filter("net_master_vector(" + std::to_string(i) + ")")).begin();

                    ASSERT_EQ(nl->get_gates(gate_filter("INV","in_gate_"+std::to_string(i))).size(), 1);
                    ASSERT_EQ(nl->get_gates(gate_filter("INV","out_gate_"+std::to_string(i))).size(), 1);
                    std::shared_ptr<gate> in_gate_i = *nl->get_gates(gate_filter("INV","in_gate_"+std::to_string(i))).begin();
                    std::shared_ptr<gate> out_gate_i = *nl->get_gates(gate_filter("INV","out_gate_"+std::to_string(i))).begin();

                    EXPECT_EQ(in_gate_i->get_fan_out_net("O"), net_i);
                    EXPECT_EQ(out_gate_i->get_fan_in_net("I"), net_i);

                }

            }
        }
        /*{
            // NOTE: currently not supported
            // Verilog specific: Assign constants ('b0 and 'b1)
            std::stringstream input("module top ("
                                    "  global_in,"
                                    "  global_out"
                                    " ) ;"
                                    "  input global_in ;"
                                    "  output global_out ;"
                                    "  wire [0:3] bit_vector ;"
                                    "  assign bit_vector = 4'hA ;"
                                    ""
                                    "  INV test_gate ("
                                    "  .\\I (bit_vector[0] ),"
                                    "  .\\O (global_out )"
                                    " ) ;"
                                    "endmodule");
            //test_def::capture_stdout();
            hdl_parser_verilog verilog_parser(input);
            std::shared_ptr<netlist> nl = verilog_parser.parse(g_lib_name);
            if (nl == nullptr)
            {
                //std::cout << test_def::get_captured_stdout();
            }
            else
            {
                //test_def::get_captured_stdout();
            }

            ASSERT_NE(nl, nullptr);

            ASSERT_EQ(nl->get_gates(gate_name_filter("test_gate")).size(), 1);
            std::shared_ptr<gate> test_gate = *nl->get_gates(gate_name_filter("test_gate")).begin();

        }*/

        {
            // Verilog specific: Assign a set of wires to a single vector
            std::stringstream input("module top ("
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
                                    "GATE_4^1_IN_1^0_OUT test_gate_0 ("
                                    "  .\\I(0) (single_net ),"
                                    "  .\\I(1) (big_vector[3] ),"
                                    "  .\\I(2) (big_vector[0] ),"
                                    "  .\\I(3) (big_vector[1] ),"
                                    "  .\\O (global_out_0 )"
                                    " ) ;"
                                    ""
                                    "GATE_4^1_IN_1^0_OUT test_gate_1 ("
                                    "  .\\I(0) (_2_d_vector_0[0][0] ),"
                                    "  .\\I(1) (_2_d_vector_0[0][1] ),"
                                    "  .\\I(2) (_2_d_vector_0[1][0] ),"
                                    "  .\\I(3) (_2_d_vector_0[1][1] ),"
                                    "  .\\O (global_out_1 )"
                                    " ) ;"
                                    "GATE_4^1_IN_1^0_OUT test_gate_2 ("
                                    "  .\\I(0) (_2_d_vector_1[1][0] ),"
                                    "  .\\I(1) (_2_d_vector_1[1][1] ),"
                                    "  .\\I(2) (_2_d_vector_1[0][0] ),"
                                    "  .\\I(3) (_2_d_vector_1[0][1] ),"
                                    "  .\\O (global_out_2 )"
                                    " ) ;"
                                    "endmodule");
            test_def::capture_stdout();
            hdl_parser_verilog verilog_parser(input);
            std::shared_ptr<netlist> nl = verilog_parser.parse(temp_lib_name);
            if (nl == nullptr)
            {
                std::cout << test_def::get_captured_stdout();
            }
            else
            {
                test_def::get_captured_stdout();
            }

            ASSERT_NE(nl, nullptr);
            std::vector<std::shared_ptr<net>> net_master_vector(12);
            for (int i = 0; i<12; i++){
                ASSERT_EQ(nl->get_nets(net_name_filter("net_vector_master(" + std::to_string(i) + ")")).size(), 1);
                net_master_vector[i] = *nl->get_nets(net_name_filter("net_vector_master(" + std::to_string(i) + ")")).begin();
            }
            for (int i = 0; i<12; i++){
                ASSERT_EQ(net_master_vector[i]->get_dsts().size(), 1);
                endpoint ep = *net_master_vector[i]->get_dsts().begin();
                EXPECT_EQ(ep.get_gate()->get_name(), "test_gate_" + std::to_string(i/4));
                EXPECT_EQ(ep.get_pin_type(), "I(" + std::to_string(i%4) + ")");
            }
        }
        {
            // Verilog specific: Assign a 2 bit vector to a set of 1 bit vectors

            std::stringstream input("module top ("
                                    "    global_out"
                                    ") ;"
                                    "    output global_out ;"
                                    "    wire net_master_0 ;"
                                    "    wire net_master_1 ;"
                                    "    wire [0:1] net_vector_slave;"
                                    "    assign net_vector_slave = { net_master_0, net_master_1 };"
                                    "    AND2 test_gate ("
                                    "        .\\I0 ( net_vector_slave[0] ),"
                                    "        .\\I1 ( net_vector_slave[1] ),"
                                    "        .\\O (global_out )"
                                    "    ) ;"
                                    "endmodule");

            hdl_parser_verilog verilog_parser(input);
            std::shared_ptr<netlist> nl = verilog_parser.parse(g_lib_name);

            ASSERT_NE(nl, nullptr);

            ASSERT_EQ(nl->get_nets(net_name_filter("net_master_0")).size(), 1);
            ASSERT_EQ(nl->get_nets(net_name_filter("net_master_1")).size(), 1);
            std::shared_ptr<net> net_vector_master_0 = *nl->get_nets(net_name_filter("net_master_0")).begin();
            std::shared_ptr<net> net_vector_master_1 = *nl->get_nets(net_name_filter("net_master_1")).begin();
            ASSERT_EQ(net_vector_master_0->get_dsts().size(), 1);
            ASSERT_EQ(net_vector_master_1->get_dsts().size(), 1);
            EXPECT_EQ((*net_vector_master_0->get_dsts().begin()).get_pin_type(), "I0");
            EXPECT_EQ((*net_vector_master_1->get_dsts().begin()).get_pin_type(), "I1");

        }
        {
            // Verilog specific: Testing assignments, where escaped identifiers are used (e.g.\net[1:3][2:3] stands for a net, literally named "net[1:3][2:3]")

            std::stringstream input("module top ("
                                    "    global_out"
                                    ") ;"
                                    "    output global_out ;"
                                    "    wire \\escaped_net_range[0:3] ;"
                                    "    wire [0:3] escaped_net_range ;"
                                    "    wire \\escaped_net[0] ;"
                                    "    wire [0:1] net_vector_master ;"
                                    "    assign { \\escaped_net_range[0:3] , \\escaped_net[0] } = net_vector_master;"
                                    "    AND2 test_gate ("
                                    "        .\\I0 ( \\escaped_net_range[0:3] ),"
                                    "        .\\I1 ( \\escaped_net[0] ),"
                                    "        .\\O (global_out )"
                                    "    ) ;"
                                    "endmodule");

            hdl_parser_verilog verilog_parser(input);
            std::shared_ptr<netlist> nl = verilog_parser.parse(g_lib_name);

            ASSERT_NE(nl, nullptr);

        }

        remove_temp_gate_lib();
    TEST_END
}

/*#########################################################################
   Verilog Specific Tests (Tests that can not be directly applied to VHDL)
  #########################################################################*/

/**
 * Testing the correct detection of single line comments (with '//') and comment blocks(with '/ *' and '* /').
 * Since verilog attribute (with  '(*' and '*)') are ignore as well, we can test them, like they were comment blocks.
 *
 * Functions: parse
 */
TEST_F(hdl_parser_verilog_test, check_comments){
    TEST_START
        {
            // Testing all comment types with attributes
            std::stringstream input;
            input << "/*here comes a module*/ module top (\n"
                     "  global_in,\n"
                     "  global_out\n"
                     " ) ;\n"
                     "  input global_in ;\n"
                     "  output global_out ;\n"
                     "\n"
                     "INV #(\n"
                     "  .no_comment_0(123), //.comment_0(123),\n"
                     "  //.comment_1(123),\n"
                     "  .no_comment_1(123), /*.comment_2(123),*/ .no_comment_2(123),\n"
                     "  /*.comment_3(123),\n"
                     "  .comment_4(123),\n"
                     "  .comment_5(123),*/\n"
                     "  .no_comment_3(123),\n"
                     "  .no_comment_4(123), (*.comment_6(123),*) .no_comment_5(123),\n"
                     "  (*.comment_7(123),\n"
                     "  .comment_8(123),\n"
                     "  .comment_9(123),*)\n"
                     "  .no_comment_6(123)\n"
                     ") \n"
                     "test_gate (\n"
                     "  .\\I (global_in ),\n"
                     "  .\\O (global_out )\n"
                     " ) ;\n"
                     "endmodule";
            test_def::capture_stdout();
            hdl_parser_verilog verilog_parser(input);
            std::shared_ptr<netlist> nl = verilog_parser.parse(g_lib_name);
            if (nl == nullptr)
            {
                std::cout << test_def::get_captured_stdout();
            }
            else
            {
                test_def::get_captured_stdout();
            }

            ASSERT_NE(nl, nullptr);
            ASSERT_EQ(nl->get_gates(gate_filter("INV", "test_gate")).size(), 1);
            std::shared_ptr<gate> test_gate = *nl->get_gates(gate_filter("INV", "test_gate")).begin();

            // Test that the comments did not removed other parts (all no_comment_n generics should be created)
            for (std::string key : std::set<std::string>({"no_comment_0", "no_comment_1", "no_comment_2",
                                                          "no_comment_3", "no_comment_4",
                                                          "no_comment_5", "no_comment_6"}))
            {
                EXPECT_NE(test_gate->get_data_by_key("generic", key), std::make_tuple("", ""));
                if (test_gate->get_data_by_key("generic", key) == std::make_tuple("", "")){
                    std::cout << "comment test failed for: " << key << std::endl;
                }
            }

            // Test that the comments are not interpreted (all comment_n generics shouldn't be created)
            for (std::string key : std::set<std::string>({"comment_0", "comment_1", "comment_2",
                                                          "comment_3", "comment_4", "comment_5",
                                                          "comment_6", "comment_7", "comment_8",
                                                          "comment_9"}))
            {
                EXPECT_EQ(test_gate->get_data_by_key("generic", key), std::make_tuple("", ""));
                if (test_gate->get_data_by_key("generic", key) != std::make_tuple("", "")){
                    std::cout << "comment failed for: " << key << std::endl;
                }
            }

        }

    TEST_END
}

/**
 * Testing the declaration of multiple wire vectors within one single line.
 *
 * Functions: parse
 */
TEST_F(hdl_parser_verilog_test, check_one_line_multiple_nets)
{
    TEST_START
        {
            // Declare multiple wire vectors in one line
            std::stringstream input("module top ("
                                    "  global_in,"
                                    "  global_out"
                                    " ) ;"
                                    "  input global_in ;"
                                    "  output global_out ;"
                                    "  wire [0:1] net_vec_0, net_vec_1 ;" // <- !!!
                                    "AND4 gate_0 ("
                                    "  .\\I0 (net_vec_0[0] ),"
                                    "  .\\I1 (net_vec_0[1] ),"
                                    "  .\\I2 (net_vec_1[0] ),"
                                    "  .\\I3 (net_vec_1[1] ),"
                                    "  .\\O (global_out )"
                                    " ) ;"
                                    "INV gate_1 ("
                                    "  .\\I (global_in ),"
                                    "  .\\O (net_vec_0[0] )"
                                    ") ;"
                                    "INV gate_2 ("
                                    "  .\\I (global_in ),"
                                    "  .\\O (net_vec_0[1] )"
                                    ") ;"
                                    "INV gate_3 ("
                                    "  .\\I (global_in ),"
                                    "  .\\O (net_vec_1[0] )"
                                    ") ;"
                                    "INV gate_4 ("
                                    "  .\\I (global_in ),"
                                    "  .\\O (net_vec_1[1] )"
                                    ") ;"
                                    "endmodule");
            test_def::capture_stdout();
            hdl_parser_verilog verilog_parser(input);
            std::shared_ptr<netlist> nl = verilog_parser.parse(g_lib_name);
            if (nl == nullptr)
            {
                std::cout << test_def::get_captured_stdout();
            }
            else
            {
                test_def::get_captured_stdout();
            }

            ASSERT_NE(nl, nullptr);
            EXPECT_EQ(nl->get_nets().size(), 6); // 3 of the net_vector + global_in + global_out
            ASSERT_EQ(nl->get_nets(net_name_filter("net_vec_0(0)")).size(), 1);
            ASSERT_EQ(nl->get_nets(net_name_filter("net_vec_0(1)")).size(), 1);
            ASSERT_EQ(nl->get_nets(net_name_filter("net_vec_0(0)")).size(), 1);
            ASSERT_EQ(nl->get_nets(net_name_filter("net_vec_0(1)")).size(), 1);
        }
    TEST_END
}

/**
 * Testing the correct handling of invalid input
 *
 * Functions: parse
 */
TEST_F(hdl_parser_verilog_test, check_invalid_input)
{
    TEST_START
        // ------ Tests that are present in booth parsers -----
        {
            // The passed gate library name is unknown
            NO_COUT_TEST_BLOCK;
            std::stringstream input;
            input << "module top ("
                     "  global_in,"
                     "  global_out"
                     " ) ;"
                     "  input global_in ;"
                     "  output global_out ;"
                     "INV gate_0 ("
                     "  .\\I (global_in ),"
                     "  .\\O (global_out )"
                     " ) ;"
                     "endmodule";
            hdl_parser_verilog verilog_parser(input);
            std::shared_ptr<netlist> nl = verilog_parser.parse("unknown_gate_lib");
            EXPECT_EQ(nl, nullptr);
        }
        {
            // Try to connect to a pin, that does not exist
            NO_COUT_TEST_BLOCK;
            std::stringstream input;
            input << "module top ("
                     "  global_in "
                     " ) ;"
                     "  input global_in ;"

                     "INV gate_0 ("
                     "  .\\NON_EXISTING_PIN (global_in)"
                     " ) ;"
                     "endmodule";
            hdl_parser_verilog verilog_parser(input);
            std::shared_ptr<netlist> nl = verilog_parser.parse(g_lib_name);
            EXPECT_EQ(nl, nullptr);
        }
        {
            // The passed gate library name is unknown
            NO_COUT_TEST_BLOCK;
            std::stringstream input;
            input << "module top ("
                     "  global_in,"
                     "  global_out"
                     " ) ;"
                     "  input global_in ;"
                     "  output global_out ;"
                     "UNKNOWN_GATE_TYPE gate_0 ("
                     "  .\\I (global_in ),"
                     "  .\\O (global_out )"
                     " ) ;"
                     "endmodule";
            hdl_parser_verilog verilog_parser(input);
            std::shared_ptr<netlist> nl = verilog_parser.parse(g_lib_name);
            EXPECT_EQ(nl, nullptr);
        }
        {
            // The input does not contain any module (is empty)
            NO_COUT_TEST_BLOCK;
            std::stringstream input;
            input << "";
            hdl_parser_verilog verilog_parser(input);
            std::shared_ptr<netlist> nl = verilog_parser.parse(g_lib_name);

            EXPECT_EQ(nl, nullptr);
        }
        {
            // Create a non-used entity (should not create any problems...)
            NO_COUT_TEST_BLOCK;
            std::stringstream input;
            input << "module ignore_me ("
                     "  min,"
                     "  mout"
                     " ) ;"
                     "  input min ;"
                     "  output mout ;"
                     "INV gate_0 ("
                     "  .\\I (min ),"
                     "  .\\O (mout )"
                     " ) ;"
                     "endmodule"
                     "\n"
                     "module top ("
                     "  global_in,"
                     "  global_out"
                     " ) ;"
                     "  input global_in ;"
                     "  output global_out ;"
                     "INV gate_0 ("
                     "  .\\I (global_in ),"
                     "  .\\O (global_out )"
                     " ) ;"
                     "endmodule";
            hdl_parser_verilog verilog_parser(input);
            std::shared_ptr<netlist> nl = verilog_parser.parse(g_lib_name);
            EXPECT_NE(nl, nullptr);
        }
        {
            // Having a cyclic master-slave net hierarchy
            NO_COUT_TEST_BLOCK;
            std::stringstream input;
            input << "module top ("
                     "  global_in,"
                     "  global_out "
                     " ) ;"
                     "  input global_in ;"
                     "  output global_out ;"
                     "  wire net_0;"
                     "  wire net_1;"
                     "  assign net_0 = net_1;"
                     "  assign net_1 = net_0;"
                     "INV gate_0 ("
                     "  .\\I (global_in ),"
                     "  .\\O (net_0 )"
                     " ) ;"
                     "INV gate_1 ("
                     "  .\\I (net_1 ),"
                     "  .\\O (global_out )"
                     " ) ;"
                     "endmodule";
            hdl_parser_verilog verilog_parser(input);
            std::shared_ptr<netlist> nl = verilog_parser.parse(g_lib_name);
            EXPECT_EQ(nl, nullptr);
        }
        /*{
            // Use non-numeric ranges (invalid) ISSUE: stoi Failure
            NO_COUT_TEST_BLOCK;
            std::stringstream input;
            input << "module top ("
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
                     "endmodule";
            hdl_parser_verilog verilog_parser(input);
            std::shared_ptr<netlist> nl = verilog_parser.parse(g_lib_name);
            EXPECT_EQ(nl, nullptr);
        }*/
        // ------ Verilog specific tests ------
        {
            // The module has no identifier
            NO_COUT_TEST_BLOCK;
            std::stringstream input;
            input << "module ("
                     "  global_in,"
                     "  global_out"
                     " ) ;"
                     "  input global_in ;"
                     "  output global_out ;"
                     "INV gate_0 ("
                     "  .\\I (global_in ),"
                     "  .\\O (global_out )"
                     " ) ;"
                     "endmodule";
            hdl_parser_verilog verilog_parser(input);
            std::shared_ptr<netlist> nl = verilog_parser.parse(g_lib_name);
            EXPECT_EQ(nl, nullptr);
        }
        /*{
            // one side of the direct assignment is empty (ISSUE: SIGSEGV l.1206)
            NO_COUT_TEST_BLOCK;
            std::stringstream input;
            input << "module top ("
                     "  global_in,"
                     "  global_out"
                     " ) ;"
                     "  input global_in ;"
                     "  output global_out ;"
                     "  wire signal_0 ;"
                     "  assign signal_0 = ;"
                     "endmodule";
            hdl_parser_verilog verilog_parser(input);
            std::shared_ptr<netlist> nl = verilog_parser.parse(g_lib_name);
            EXPECT_EQ(nl, nullptr);
        }*/
        {
            // The two sides of assignment do not have the same width
            NO_COUT_TEST_BLOCK;
            std::stringstream input;
            input << "module top ("
                     "  global_in,"
                     "  global_out"
                     " ) ;"
                     "  input global_in ;"
                     "  output global_out ;"
                     "  wire [0:4] signal_vec_0 ;"
                     "  wire [0:4] signal_vec_1 ;"
                     "  assign signal_vec_0[0:3] = signal_vec_0[0:4];"
                     "INV gate_0 ("
                     "  .\\I (global_in ),"
                     "  .\\O (global_out )"
                     " ) ;"
                     "endmodule";
            hdl_parser_verilog verilog_parser(input);
            std::shared_ptr<netlist> nl = verilog_parser.parse(g_lib_name);
            EXPECT_EQ(nl, nullptr);
        }
        {
            // Having a cyclic module hierarchy
            NO_COUT_TEST_BLOCK;
            std::stringstream input;
            input << "module ENT_0 ("
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
                     "endmodule";
            hdl_parser_verilog verilog_parser(input);
            std::shared_ptr<netlist> nl = verilog_parser.parse(g_lib_name);
            EXPECT_EQ(nl, nullptr);
        }
        /*{
            // Port map gets multiple nets
            NO_COUT_TEST_BLOCK;
            std::stringstream input;
            input << "module top ("
                     "  global_in,"
                     "  global_out "
                     " ) ;"
                     "  input global_in ;"
                     "  output global_out ;"
                     "  wire net_0;"
                     "INV gate_0 ("
                     "  .\\I ({global_in, net_0} )"
                     " ) ;"
                     "INV gate_1 ("
                     "  .\\I (net_0 ),"
                     "  .\\O (global_out )"
                     " ) ;"
                     "endmodule";
            hdl_parser_verilog verilog_parser(input);
            std::shared_ptr<netlist> nl = verilog_parser.parse(g_lib_name);
            EXPECT_EQ(nl, nullptr);
        }*/
        {
            // Store an unknown data type
            NO_COUT_TEST_BLOCK;
            std::stringstream input("module top ("
                                    "  global_in,"
                                    "  global_out"
                                    " ) ;"
                                    "  input global_in ;"
                                    "  output global_out ;"
                                    "INV #("
                                    ".key_unknown(#Unkn0wn!)) "
                                    "gate_0 ("
                                    "  .\\I (global_in ),"
                                    "  .\\O (global_out )"
                                    " ) ;"
                                    "endmodule");
            hdl_parser_verilog verilog_parser(input);
            std::shared_ptr<netlist> nl = verilog_parser.parse(g_lib_name);
            EXPECT_EQ(nl, nullptr);
        }
        {
            // Assign one net to another, but booth are connected to different source gates
            NO_COUT_TEST_BLOCK;
            std::stringstream input("module top ("
                                    "  global_in,"
                                    "  global_out"
                                    " ) ;"
                                    "  input global_in ;"
                                    "  output global_out ;"
                                    "  wire net_0 ;"
                                    "  wire net_1 ;"
                                    "  assign net_0 = net_1 ;"
                                    "INV gate_0 ("
                                    "  .\\I (global_in ),"
                                    "  .\\O (net_0 )"
                                    " ) ;"
                                    "INV gate_1 ("
                                    "  .\\I (global_in ),"
                                    "  .\\O (net_1 )"
                                    " ) ;"
                                    "AND2 gate_1 ("
                                    "  .\\I0 (net_0 ),"
                                    "  .\\I1 (net_1 ),"
                                    "  .\\O (global_out )"
                                    " ) ;"
                                    "endmodule");
            hdl_parser_verilog verilog_parser(input);
            std::shared_ptr<netlist> nl = verilog_parser.parse(g_lib_name);

            EXPECT_EQ(nl, nullptr);
        }
        {
            // Use an undeclared signal (should only throw a warning or return a nullptr)
            NO_COUT_TEST_BLOCK;
            std::stringstream input("module top ("
                                    "  global_in,"
                                    "  global_out"
                                    " ) ;"
                                    "  input global_in ;"
                                    "  output global_out ;"
                                    "INV gate_0 ("
                                    "  .\\I (global_in ),"
                                    "  .\\O (net_0 )" // <- undeclared
                                    " ) ;"
                                    "INV gate_1 ("
                                    "  .\\I (net_0 ),"
                                    "  .\\O (global_out )"
                                    " ) ;"
                                    "endmodule");
            hdl_parser_verilog verilog_parser(input);
            std::shared_ptr<netlist> nl = verilog_parser.parse(g_lib_name);

            EXPECT_TRUE(nl == nullptr || nl->get_nets(net_name_filter("net_0")).size() == 1);
        }
        /*{
            // Assign unknown signals ISSUE: map::at failure (l.790)
            NO_COUT_TEST_BLOCK;
            std::stringstream input;
            input << "module top ("
                     "  global_in,"
                     "  global_out"
                     " ) ;"
                     "  input global_in ;"
                     "  output global_out ;"
                     "  wire [0:4] signal_vec ;"
                     "  assign signal_unknown[0:4] = signal_vec[0:4];"
                     "INV gate_0 ("
                     "  .\\I (global_in ),"
                     "  .\\O (global_out )"
                     " ) ;"
                     "endmodule";
            hdl_parser_verilog verilog_parser(input);
            std::shared_ptr<netlist> nl = verilog_parser.parse(g_lib_name);
            EXPECT_EQ(nl, nullptr);
        }*/
        {
            // MAYBE SOME MORE TESTS HERE LATER...
        }
    TEST_END
}
