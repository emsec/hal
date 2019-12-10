#include "netlist/gate.h"
#include "netlist/gate_library/gate_library_manager.h"
#include "netlist/netlist.h"
#include "netlist/netlist_factory.h"
#include "netlist/persistent/netlist_serializer.h"
#include "netlist_test_utils.h"
#include "gtest/gtest.h"
#include <core/log.h>
#include <hdl_parser/hdl_parser_verilog.h>
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

static std::function<bool(const std::shared_ptr<net>&)> net_name_filter(const std::string& name){
    return [name](auto& n){return n->get_name() == name;};
}

static std::function<bool(const std::shared_ptr<gate>&)> gate_name_filter(const std::string& name){
    return [name](auto& g){return g->get_name() == name;};
}

static std::function<bool(const std::shared_ptr<gate>&)> gate_type_filter(const std::string& type){
    return [type](auto& g){return g->get_type()->get_name() == type;};
}

static std::function<bool(const std::shared_ptr<gate>&)> gate_filter(const std::string& type, const std::string& name){
    return [name, type](auto& g){return g->get_name() == name && g->get_type()->get_name() == type;};
}

static std::function<bool(const endpoint&)> endpoint_type_filter(const std::string& type){
    return [type](auto& ep){return ep.gate->get_type()->get_name() == type;};
}


/*                                    net_0
 *                  .--= INV (0) =------.
 *  global_in       |                   '-=                     global_out
 *      ------------|                   .-= AND3 (2) = ----------
 *                  |                   | =
 *                  '--=                |
 *                       AND2 (1) =-----'
 *               .-----=              net_1
 *               |                                              global_inout
 *      ---------+-------------------------------------------------
 *
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
            std::stringstream input("module top (\n"
                                    "  global_in,\n"
                                    "  global_out, \n"
                                    "  global_inout\n"
                                    " ) ;\n"
                                    "  input global_in ;\n"
                                    "  output global_out ;\n"
                                    "  input global_inout ;\n"
                                    "  wire net_0 ;\n"
                                    "  wire net_1 ;\n"
                                    "INV gate_0 (\n"
                                    "  .\\I (global_in ),\n"
                                    "  .\\O (net_0 )\n"
                                    " ) ;\n"
                                    "AND2 gate_1 (\n"
                                    "  .\\I0 (global_in ),\n"
                                    "  .\\I1 (global_inout ),\n"
                                    "  .\\O (net_1 )\n"
                                    " ) ;\n"
                                    "AND3 gate_2 (\n"
                                    "  .\\I0 (net_0 ),\n"
                                    "  .\\I1 (net_1 ),\n"
                                    "  .\\O (global_out )\n"
                                    " ) ;\n"
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
            ASSERT_FALSE(nl->get_nets(net_name_filter("net_0")).empty());
            std::shared_ptr<net> net_0            = *(nl->get_nets(net_name_filter("net_0")).begin());
            ASSERT_FALSE(nl->get_nets(net_name_filter("net_1")).empty());
            std::shared_ptr<net> net_1            = *(nl->get_nets(net_name_filter("net_1")).begin());
            ASSERT_FALSE(nl->get_nets(net_name_filter("global_in")).empty());
            std::shared_ptr<net> net_global_in    = *(nl->get_nets(net_name_filter("global_in")).begin());
            ASSERT_FALSE(nl->get_nets(net_name_filter("global_out")).empty());
            std::shared_ptr<net> net_global_out   = *(nl->get_nets(net_name_filter("global_out")).begin());
            ASSERT_FALSE(nl->get_nets(net_name_filter("global_inout")).empty());
            std::shared_ptr<net> net_global_inout = *(nl->get_nets(net_name_filter("global_inout")).begin());

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
            EXPECT_EQ(net_global_in->get_name(), "global_in");
            EXPECT_EQ(net_global_in->get_src(), get_endpoint(nullptr, ""));
            EXPECT_TRUE(vectors_have_same_content(net_global_in->get_dsts(), std::vector<endpoint>({get_endpoint(gate_0, "I"), get_endpoint(gate_1, "I0")})));
            EXPECT_TRUE(nl->is_global_input_net(net_global_in));

            ASSERT_NE(net_global_out, nullptr);
            EXPECT_EQ(net_global_out->get_name(), "global_out");
            EXPECT_EQ(net_global_out->get_src(), get_endpoint(gate_2, "O"));
            EXPECT_TRUE(net_global_out->get_dsts().empty());
            EXPECT_TRUE(nl->is_global_output_net(net_global_out));

            EXPECT_EQ(nl->get_global_input_nets().size(), 2);
            EXPECT_EQ(nl->get_global_output_nets().size(), 1);
            //EXPECT_EQ(nl->get_global_inout_nets().size(), 1);
        }
    TEST_END
}

/**
 * The same test, as the main example, but use white spaces of different types (' ','\n','\t') in various locations (or remove some unnecessary ones)
 *
 * Functions: parse
 */
// ISSUE ?: multiple lines in one row (separated by ';')
TEST_F(hdl_parser_verilog_test, check_whitespace_chaos)
{
    TEST_START
         {
             std::stringstream input("module top (\n"
                                     "  global_in, \n"
                                     " global_out, "
                                     "global_inout\n"
                                     " ) ;\n"
                                     "  input global_in ;\n"
                                     "  output global_out ;\n"
                                     "  input global_inout ;\n"
                                     "  wire net_0;   wire net_1 ;\n"
                                     "\n"
                                     "INV gate_0 (\n"
                                     "  .\\I \t (global_in),\n"
                                     "  .\\O \n"
                                     "(net_0)\n"
                                     " ) ;\n"
                                     "AND2 gate_1 (\n"
                                     "  .\\I0 (global_in ),\n"
                                     ".\\I1     (global_inout ), .\\O (net_1 )\n"
                                     " ) ;\n"
                                     "AND3 gate_2 (.\\I0 ( net_0 ), .\\I1 (net_1 ),.\\O (global_out));\n"
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
            ASSERT_FALSE(nl->get_nets(net_name_filter("net_0")).empty());
            std::shared_ptr<net> net_0            = *(nl->get_nets(net_name_filter("net_0")).begin());
            ASSERT_FALSE(nl->get_nets(net_name_filter("net_1")).empty());
            std::shared_ptr<net> net_1            = *(nl->get_nets(net_name_filter("net_1")).begin());
            ASSERT_FALSE(nl->get_nets(net_name_filter("global_in")).empty());
            std::shared_ptr<net> net_global_in    = *(nl->get_nets(net_name_filter("global_in")).begin());
            ASSERT_FALSE(nl->get_nets(net_name_filter("global_out")).empty());
            std::shared_ptr<net> net_global_out   = *(nl->get_nets(net_name_filter("global_out")).begin());
            ASSERT_FALSE(nl->get_nets(net_name_filter("global_inout")).empty());
            std::shared_ptr<net> net_global_inout = *(nl->get_nets(net_name_filter("global_inout")).begin());

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
            EXPECT_EQ(net_global_in->get_name(), "global_in");
            EXPECT_EQ(net_global_in->get_src(), get_endpoint(nullptr, ""));
            EXPECT_TRUE(vectors_have_same_content(net_global_in->get_dsts(), std::vector<endpoint>({get_endpoint(gate_0, "I"), get_endpoint(gate_1, "I0")})));
            EXPECT_TRUE(nl->is_global_input_net(net_global_in));

            ASSERT_NE(net_global_out, nullptr);
            EXPECT_EQ(net_global_out->get_name(), "global_out");
            EXPECT_EQ(net_global_out->get_src(), get_endpoint(gate_2, "O"));
            EXPECT_TRUE(net_global_out->get_dsts().empty());
            EXPECT_TRUE(nl->is_global_output_net(net_global_out));

            EXPECT_EQ(nl->get_global_input_nets().size(), 2);
            EXPECT_EQ(nl->get_global_output_nets().size(), 1);
            //EXPECT_EQ(nl->get_global_inout_nets().size(), 1);
         }
    TEST_END
}

/**
 * Testing the correct detection of single line comments (with '//') and comment blocks(with '/ *' and '* /'). Therefore
 * we comment out a verilog statement, which should be ignored. Since verilog attribute (with  '(*' and '*)') are ignored
 * as well, we can test them, like they were comment blocks
 *
 * Functions: parse
 */
TEST_F(hdl_parser_verilog_test, check_comment_detection){
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
 * Testing the correct storage of data of the following data types:
 * integer, floating_point, string, bit_vector (hexadecimal, decimal, octal, binary)
 *
 * Functions: parse
 */
TEST_F(hdl_parser_verilog_test, check_generic_map){
    TEST_START
        {
            // Store an instance of all possible data types in one gate
            std::stringstream input("module top (\n"
                                    "  global_in,\n"
                                    "  global_out\n"
                                    " ) ;\n"
                                    "  input global_in ;\n"
                                    "  output global_out ;\n"
                                    "INV #(\n"
                                    ".key_integer(1234),\n"                   // no prefix => integer (stored in hex)
                                    ".key_floating_point(1.234),\n"
                                    ".key_string(\"test_string\"),\n"
                                    ".key_bit_vector_hex('habc),\n"           // All values are 'ABC' in hex
                                    ".key_bit_vector_dec('d2748),\n"
                                    ".key_bit_vector_oct('o5274),\n"
                                    ".key_bit_vector_bin('b101010111100)) \n"
                                    "gate_0 (\n"
                                    "  .\\I (global_in ),\n"
                                    "  .\\O (global_out )\n"
                                    " ) ;\n"
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
            //EXPECT_EQ(gate_0->get_data_by_key("generic","key_floating_point"), std::make_tuple("floating_point", "1.234"));
            EXPECT_EQ(gate_0->get_data_by_key("generic","key_string"), std::make_tuple("string", "test_string"));
            EXPECT_EQ(gate_0->get_data_by_key("generic","key_bit_vector_hex"), std::make_tuple("bit_vector", "abc"));
            EXPECT_EQ(gate_0->get_data_by_key("generic","key_bit_vector_dec"), std::make_tuple("bit_vector", "abc"));
            EXPECT_EQ(gate_0->get_data_by_key("generic","key_bit_vector_oct"), std::make_tuple("bit_vector", "abc"));
            EXPECT_EQ(gate_0->get_data_by_key("generic","key_bit_vector_bin"), std::make_tuple("bit_vector", "abc"));
        }

    TEST_END
}

/**
 * Testing the correct usage of vector bounds (for example: "wire [1:3] net_0")
 *
 * Functions: parse
 */
TEST_F(hdl_parser_verilog_test, check_vector_bounds){
    TEST_START
        {
            // Use a net vector of size 3
            std::stringstream input("module top (\n"
                                    "  global_in,\n"
                                    "  global_out\n"
                                    " ) ;\n"
                                    "  input global_in ;\n"
                                    "  output global_out ;\n"
                                    "  wire [1:3] net_vec ;\n"
                                    "AND3 gate_0 (\n"
                                    "  .\\I0 (net_vec[1] ),\n"
                                    "  .\\I1 (net_vec[2] ),\n"
                                    "  .\\I2 (net_vec[3] ),\n"
                                    "  .\\O (global_out )\n"
                                    " ) ;\n"
                                    "INV gate_1 (\n"
                                    "  .\\I (global_in ),\n"
                                    "  .\\O (net_vec[1] )\n"
                                    ") ;\n"
                                    "INV gate_2 (\n"
                                    "  .\\I (global_in ),\n"
                                    "  .\\O (net_vec[2] )\n"
                                    ") ;\n"
                                    "INV gate_3 (\n"
                                    "  .\\I (global_in ),\n"
                                    "  .\\O (net_vec[3] )\n"
                                    ") ;\n"
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
            EXPECT_EQ(nl->get_nets().size(), 5); // 3 of the net_vector + global_in + global_out
            ASSERT_EQ(nl->get_nets(net_name_filter("net_vec(1)")).size(), 1);
            ASSERT_EQ(nl->get_nets(net_name_filter("net_vec(2)")).size(), 1);
            ASSERT_EQ(nl->get_nets(net_name_filter("net_vec(3)")).size(), 1);

            std::shared_ptr<net> net_vec_1 = *nl->get_nets(net_name_filter("net_vec(1)")).begin();
            std::shared_ptr<net> net_vec_2 = *nl->get_nets(net_name_filter("net_vec(2)")).begin();
            std::shared_ptr<net> net_vec_3 = *nl->get_nets(net_name_filter("net_vec(3)")).begin();

            // Check if all gates are created
            for (std::string g_name : std::set<std::string>({"gate_0", "gate_1", "gate_2", "gate_3"})){
                ASSERT_EQ(nl->get_gates(gate_name_filter(g_name)).size(), 1);
            }

            EXPECT_EQ(net_vec_1->get_src().get_gate(), *nl->get_gates(gate_name_filter("gate_1")).begin());
            ASSERT_EQ(net_vec_1->get_dsts().size(), 1);
            EXPECT_EQ((*net_vec_1->get_dsts().begin()).get_gate(), *nl->get_gates(gate_name_filter("gate_0")).begin());
        }
        {
            // Declare multiple wire vectors in one line
            std::stringstream input("module top (\n"
                                    "  global_in,\n"
                                    "  global_out\n"
                                    " ) ;\n"
                                    "  input global_in ;\n"
                                    "  output global_out ;\n"
                                    "  wire [0:1] net_vec_0, net_vec_1 ;\n"
                                    "AND4 gate_0 (\n"
                                    "  .\\I0 (net_vec_0[0] ),\n"
                                    "  .\\I1 (net_vec_0[1] ),\n"
                                    "  .\\I2 (net_vec_1[0] ),\n"
                                    "  .\\I3 (net_vec_1[1] ),\n"
                                    "  .\\O (global_out )\n"
                                    " ) ;\n"
                                    "INV gate_1 (\n"
                                    "  .\\I (global_in ),\n"
                                    "  .\\O (net_vec_0[0] )\n"
                                    ") ;\n"
                                    "INV gate_2 (\n"
                                    "  .\\I (global_in ),\n"
                                    "  .\\O (net_vec_0[1] )\n"
                                    ") ;\n"
                                    "INV gate_3 (\n"
                                    "  .\\I (global_in ),\n"
                                    "  .\\O (net_vec_1[0] )\n"
                                    ") ;\n"
                                    "INV gate_4 (\n"
                                    "  .\\I (global_in ),\n"
                                    "  .\\O (net_vec_1[1] )\n"
                                    ") ;\n"
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
 * Testing the correct handling of the 'assign' keyword. The assign-keyword is only used, to assign n nets to other n nets
 * (so both identifiers can be used as the same net)
 * Therefore only statements like
 * "assing signal_1 = signal_0" are valid, which would only create one net named "signal_0" that is also connected to all
 * gates signal_1 is connected to. Assigning multiple nets of a vector at once is also supported (for example: assign sig_vec_1[0:3] = sig_vec_2[0:3])
 * as well as lists of vectors and/or single nets (for example: "assign {sig_vec_1, signal_1} = {sig_vec_big[0:4]}", if |sig_vec_1|=4)
 *
 *  Note: logic assignments like "assign net_0 = net_1 ^ net_2" are not supported
 *
 * Functions: parse
 */
TEST_F(hdl_parser_verilog_test, check_assign)
{
    TEST_START
        create_temp_gate_lib();
        {
            // Use the assign statement where one wire is assigned to another. The netlist should be created like this:
            /*
             *
             *     ---<global_in>---=| gate_0 |=--<net_0>---+---=| gate_1 |=---<global_out_0>---
             *                                              |
             *                                              '---=| gate_2 |=---<global_out_1>---
             */
            std::stringstream input("module top (\n"
                                    "  global_in,\n"
                                    "  global_out_0,\n"
                                    "  global_out_1\n"
                                    " ) ;\n"
                                    "  input global_in ;\n"
                                    "  output global_out_0 ;\n"
                                    "  output global_out_1 ;\n"
                                    "  wire net_0;\n"
                                    "  wire net_1;\n"
                                    "  assign net_1 = net_0 ;\n"
                                    "INV gate_0 (\n"
                                    "  .\\I (global_in ),\n"
                                    "  .\\O ( net_0 )\n"
                                    " ) ;\n"
                                    "INV gate_1 (\n"
                                    "  .\\I ( net_0 ),\n"
                                    "  .\\O ( global_out_0 )\n"
                                    " ) ;\n"
                                    "INV gate_2 (\n"
                                    "  .\\I ( net_1 ),\n"
                                    "  .\\O ( global_out_1 )\n"
                                    " ) ;\n"
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

            ASSERT_EQ(nl->get_gates(gate_name_filter("gate_0")).size(), 1);
            ASSERT_EQ(nl->get_gates(gate_name_filter("gate_1")).size(), 1);
            ASSERT_EQ(nl->get_gates(gate_name_filter("gate_2")).size(), 1);
            std::shared_ptr<gate> gate_0 = *nl->get_gates(gate_name_filter("gate_0")).begin();
            std::shared_ptr<gate> gate_1 = *nl->get_gates(gate_name_filter("gate_1")).begin();
            std::shared_ptr<gate> gate_2 = *nl->get_gates(gate_name_filter("gate_2")).begin();

            ASSERT_EQ(nl->get_nets(net_name_filter("net_0")).size(), 1);
            std::shared_ptr<net> net_0 = *nl->get_nets(net_name_filter("net_0")).begin();

            EXPECT_EQ(net_0->get_src().get_gate(), gate_0);
            ASSERT_EQ(net_0->get_dsts(endpoint_type_filter("INV")).size(), 2);
            std::vector<std::shared_ptr<gate>> net_exp_dsts = {gate_1, gate_2};
            std::vector<std::shared_ptr<gate>> net_0_dsts = {net_0->get_dsts(endpoint_type_filter("INV"))[0].get_gate(), net_0->get_dsts(endpoint_type_filter("INV"))[1].get_gate()};
            EXPECT_TRUE(vectors_have_same_content(net_0_dsts, net_exp_dsts));
        }
        {
            // Testing assignments with logic vectors (assign wires 0 and 1 of each dimension)
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

                    global_out_list_module << "  global_out_" << i << ",\n";
                    global_out_list << "  output global_out_" << i << ";\n";


                    std::bitset<64> i_bs(i);
                    std::stringstream brackets("");
                    for (int j = dim-1; j>=0; j--){
                        brackets << "[" << (i_bs[j] ? "1":"0") << "]";
                    }

                    gate_list <<  "INV in_gate_"<< i <<" (\n"
                                                       "  .\\I (global_in ),\n"
                                                       "  .\\O ( net_slave_vector" << brackets.str() << ")\n"
                                                                                                        " ) ;\n";
                    gate_list <<  "INV out_gate_"<< i <<" (\n"
                                                        "  .\\I (net_slave_vector" << brackets.str() <<  "),\n"
                                                                                                         "  .\\O ( global_out_" << i << ")\n"
                                                                                                                                        " ) ;\n";
                }

                std::stringstream input;
                input << "module top (\n"
                         "  global_in,\n"
                      <<    global_out_list_module.str()
                      << "  );\n"
                         "  input global_in ;\n"
                      <<    global_out_list.str()
                      << "  wire " << dim_decl << " net_slave_vector;\n"
                      << "  wire [0:"<< ((1 << dim)-1) <<"] net_master_vector;\n"
                      << "  assign net_slave_vector = net_master_vector;\n" // <- !!!
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
            // NOTE: fails with map::at (l. 635)
            // Assign constants ('b0 and 'b1)
            std::stringstream input("module top (\n"
                                    "  global_in,\n"
                                    "  global_out\n"
                                    " ) ;\n"
                                    "  input global_in ;\n"
                                    "  output global_out ;\n"
                                    "  wire [0:3] bit_vector ;\n"
                                    "  assign bit_vector = 4'hA ;\n"
                                    "\n"
                                    "  INV test_gate (\n"
                                    "  .\\I (bit_vector[0] ),\n"
                                    "  .\\O (global_out )\n"
                                    " ) ;\n"
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
        /*{
            // NOTE: fails currently (bc for example .\I(0) (single_net) is interpreted as "I" => "0")(wrong escaped?)
            // Assign constants a set of wires (single AND vectors) to a vector
            std::stringstream input("module top (\n"
                                    "  global_out_0,\n"
                                    "  global_out_1,\n"
                                    "  global_out_2\n"
                                    " ) ;\n"
                                    "  output global_out_0 ;\n"
                                    "  output global_out_1 ;\n"
                                    "  output global_out_2 ;\n"
                                    "\n"
                                    "  wire single_net ;\n"
                                    "  wire [0:2][0:2] 2_d_vector_0;\n"
                                    "  wire [0:2][0:1] 2_d_vector_1;\n"
                                    "  wire [0:15] big_vector;\n"
                                    "  wire [0:11] net_vector_master;\n"
                                    "  assign {single_net, big_vector[3], big_vector[0:1], 2_d_vector_0[0:1][0:1], 2_d_vector_1[1:0][0:1]} = net_vector_master;\n"
                                    "\n"
                                    "GATE_4^1_IN_1^0_OUT test_gate_0 (\n"
                                    "  .\\I(0) (single_net ),\n"
                                    "  .\\I(1) (big_vector[3] ),\n"
                                    "  .\\I(2) (big_vector[0] ),\n"
                                    "  .\\I(3) (big_vector[1] ),\n"
                                    "  .\\O (global_out_0 )\n"
                                    " ) ;\n"
                                    "\n"
                                    "GATE_4^1_IN_1^0_OUT test_gate_1 (\n"
                                    "  .\\I(0) (2_d_vector_0[0][0] ),\n"
                                    "  .\\I(1) (2_d_vector_0[0][1] ),\n"
                                    "  .\\I(2) (2_d_vector_0[1][0] ),\n"
                                    "  .\\I(3) (2_d_vector_0[1][1] ),\n"
                                    "  .\\O (global_out_1 )\n"
                                    " ) ;\n"
                                    "// Not sure...\n"
                                    "GATE_4^1_IN_1^0_OUT test_gate_2 (\n"
                                    "  .\\I(0) (2_d_vector_1[1][0] ),\n"
                                    "  .\\I(1) (2_d_vector_1[1][0] ),\n"
                                    "  .\\I(2) (2_d_vector_1[0][1] ),\n"
                                    "  .\\I(3) (2_d_vector_1[0][1] ),\n"
                                    "  .\\O (global_out_2 )\n"
                                    " ) ;\n"
                                    "endmodule");
            //test_def::capture_stdout();
            hdl_parser_verilog verilog_parser(input);
            std::shared_ptr<netlist> nl = verilog_parser.parse(temp_lib_name);
            if (nl == nullptr)
            {
                //std::cout << test_def::get_captured_stdout();
            }
            else
            {
                //test_def::get_captured_stdout();
            }

            ASSERT_NE(nl, nullptr);

            // NOTE: Test me, pls :)
        }*/
        {
             // NOTE: stoi failure (l.978)
             // Testing assignments, where escaped identifiers are used (e.g. \net[1:3][2:3] stands for a net, literally named "net[1:3][2:3]")

             std::stringstream input("module top (\n"
                                    "    global_out\n"
                                    ") ;\n"
                                    "    output global_out ;\n"
                                    "    wire \\escaped_net_range[0:3] ;\n"
                                    "    wire [0:3] escaped_net_range ;\n"
                                    "    wire \\escaped_net[0] ;\n"
                                    "    wire [0:1] net_vector_master ;\n"
                                    "    assign { \\escaped_net_range[0:3] , \\escaped_net[0] } = net_vector_master;\n"
                                    "    AND2 test_gate (\n"
                                    "        .\\I0 ( \\escaped_net_range[0:3] ),\n"
                                    "        .\\I1 ( \\escaped_net[0] ),\n"
                                    "        .\\O (global_out )\n"
                                    "    ) ;\n"
                                    "endmodule");

             hdl_parser_verilog verilog_parser(input);
             std::shared_ptr<netlist> nl = verilog_parser.parse(g_lib_name);

             ASSERT_NE(nl, nullptr);

         }
        remove_temp_gate_lib();
    TEST_END
}


/**
 * Testing the usage of global gnd/vcc gates
 *
 * Functions: parse
 */
TEST_F(hdl_parser_verilog_test, check_global_gnd_vcc_gates)
{
    TEST_START
        {
            // Testing the usage of a global VCC gate (gate type 'VCC' is global VCC gate in our test gate library)
            std::stringstream input("module top (\n"
                                    "  global_out\n"
                                    " ) ;\n"
                                    "  output global_out ;\n"
                                    "VCC gate_0 (\n"
                                    "  .\\O (global_out )\n"
                                    " ) ;\n"
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
            ASSERT_EQ(nl->get_gates(gate_filter("VCC","gate_0")).size(), 1);
            std::shared_ptr<gate> gate_0 = *nl->get_gates(gate_filter("VCC","gate_0")).begin();
            /* FIXME
            EXPECT_FALSE(gate_0->is_global_gnd_gate());
            EXPECT_TRUE(gate_0->is_global_vcc_gate());*/
        }
        {
            // Testing the usage of a global GND gate (gate type 'GND' is global GND gate in our test gate library)
            std::stringstream input("module top (\n"
                                    "  global_out\n"
                                    " ) ;\n"
                                    "  output global_out ;\n"
                                    "GND gate_0 (\n"
                                    "  .\\O (global_out )\n"
                                    " ) ;\n"
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
            ASSERT_EQ(nl->get_gates(gate_filter("GND","gate_0")).size(), 1);
            std::shared_ptr<gate> gate_0 = *nl->get_gates(gate_filter("GND","gate_0")).begin();
            /* FIXME
            EXPECT_TRUE(gate_0->is_global_gnd_gate());
            EXPECT_FALSE(gate_0->is_global_vcc_gate()); */
        }
    TEST_END
}

/** (NOTE: old Description)
 * Testing the usage of additional entities. Entities that are used by the main entity (the last one) are recursively
 * split in gates that are part of the gate library, while the original entity hierarchy is represented by the module-
 * hierarchy of the netlist. Therefore, there can be multiple gates with the same name, so names that occur twice or
 * more will be given a unique suffix.
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
            // NOTE: Data propagation?
            std::stringstream input("module ENT_CHILD (\n"
                                    "  child_in,\n"
                                    "  child_out\n"
                                    " ) ;\n"
                                    "  input child_in ;\n"
                                    "  output child_out ;\n"
                                    "  wire net_0_child ;\n"
                                    "INV gate_0_child (\n"
                                    "  .\\I (child_in ),\n"
                                    "  .\\O (net_0_child )\n"
                                    " ) ;\n"
                                    "INV gate_1_child (\n"
                                    "  .\\I (net_0_child ),\n"
                                    "  .\\O (child_out )\n"
                                    " ) ;\n"
                                    "endmodule\n"
                                    "\n"
                                    "module ENT_TO (\n"
                                    "  net_global_in,\n"
                                    "  net_global_out\n"
                                    " ) ;\n"
                                    "  input net_global_in ;\n"
                                    "  output net_global_out ;\n"
                                    "  wire net_0 ;\n"
                                    "  wire net_1 ;\n"
                                    "INV gate_0 (\n"
                                    "  .\\I (net_global_in ),\n"
                                    "  .\\O (net_0 )\n"
                                    " ) ;\n"
                                    "ENT_CHILD child_mod (\n"
                                    "  .\\child_in (net_0 ),\n"
                                    "  .\\child_out (net_1 )\n"
                                    " ) ;\n"
                                    "INV gate_1 (\n"
                                    "  .\\I (net_1 ), \n"
                                    "  .\\O (net_global_out )\n"
                                    " ) ;\n"
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

            std::stringstream input("module ENT_CHILD_TWO (\n"
                                    "  I_c2,\n"
                                    "  O_c2\n"
                                    " ) ;\n"
                                    "  input I_c2 ;\n"
                                    "  output O_c2 ;\n"
                                    "INV gate_child_two (\n"
                                    "  .\\I (I_c2 ),\n"
                                    "  .\\O (O_c2 )\n"
                                    " ) ;\n"
                                    "endmodule\n"
                                    "\n"
                                    "module ENT_CHILD_ONE (\n"
                                    "  I_c1,\n"
                                    "  O_c1\n"
                                    " ) ;\n"
                                    "  input I_c1 ;\n"
                                    "  output O_c1 ;\n"
                                    "  wire net_child_0 ;\n"
                                    "  wire net_child_1 ;\n"
                                    "ENT_CHILD_TWO gate_0_ent_two (\n"
                                    "  .\\I_c2 (I_c1 ),\n"
                                    "  .\\O_c2 (net_child_0 )\n"
                                    " ) ;\n"
                                    "ENT_CHILD_TWO gate_1_ent_two (\n"
                                    "  .\\I_c2 (net_child_0 ),\n"
                                    "  .\\O_c2 (net_child_1 )\n"
                                    " ) ;\n"
                                    "INV gate_child_one (\n"
                                    "  .\\I (net_child_1 ),\n"
                                    "  .\\O (O_c1 )\n"
                                    " ) ;\n"
                                    "endmodule\n"
                                    "\n"
                                    "module ENT_TOP (\n"
                                    "  net_global_in,\n"
                                    "  net_global_out\n"
                                    " ) ;\n"
                                    "  input net_global_in ;\n"
                                    "  output net_global_out ;\n"
                                    "  wire net_0 ;\n"
                                    "  wire net_1 ;\n"
                                    "ENT_CHILD_ONE #(\n"
                                    "  .child_one_mod_key(1234)\n"
                                    ") child_one_mod (\n"
                                    "  .\\I_c1 (net_global_in ),\n"
                                    "  .\\O_c1 (net_0 )\n"
                                    " ) ;\n"
                                    "ENT_CHILD_TWO child_two_mod (\n"
                                    "  .\\I_c2 (net_0 ),\n"
                                    "  .\\O_c2 (net_1 )\n"
                                    " ) ;\n"
                                    "INV gate_top (\n"
                                    "  .\\I (net_1 ),\n"
                                    "  .\\O (net_global_out )\n"
                                    " ) ;\n"
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
    TEST_END
}

/**
 * Testing the port assigment of the signals '0' and '1' (by 'b0 and 'b1)
 *
 * Functions: parse
 */
TEST_F(hdl_parser_verilog_test, check_zero_and_one_nets)
{
    TEST_START
        {
            // Port map gets multiple nets
            std::stringstream input;
            input << "module top (\n"
                     "  global_out_0,\n"
                     "  global_out_1 \n"
                     " ) ;\n"
                     "  output global_out_0 ;\n"
                     "  output global_out_1 ;\n"
                     "INV gate_0 (\n"
                     "  .\\I ('b0 ),\n"
                     "  .\\O (global_out_0)"
                     " ) ;\n"
                     "INV gate_1 (\n"
                     "  .\\I ('b1 ),\n"
                     "  .\\O (global_out_1)"
                     " ) ;\n"
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
            /* FIXME
            EXPECT_TRUE(net_gnd->get_src_by_type("GND").get_gate()->is_global_gnd_gate());
            EXPECT_TRUE(net_vcc->get_src_by_type("VCC").get_gate()->is_global_vcc_gate());
             */
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
        {
            // The passed gate library name is unknown
            NO_COUT_TEST_BLOCK;
            std::stringstream input;
            input << "module top (\n"
                     "  global_in,\n"
                     "  global_out\n"
                     " ) ;\n"
                     "  input global_in ;\n"
                     "  output global_out ;\n"
                     "INV gate_0 (\n"
                     "  .\\I (global_in ),\n"
                     "  .\\O (global_out )\n"
                     " ) ;\n"
                     "endmodule";
            hdl_parser_verilog verilog_parser(input);
            std::shared_ptr<netlist> nl = verilog_parser.parse("unknown_gate_lib");
            EXPECT_EQ(nl, nullptr);
        }
        {
            // The module has no identifier
            NO_COUT_TEST_BLOCK;
            std::stringstream input;
            input << "module (\n"
                     "  global_in,\n"
                     "  global_out\n"
                     " ) ;\n"
                     "  input global_in ;\n"
                     "  output global_out ;\n"
                     "INV gate_0 (\n"
                     "  .\\I (global_in ),\n"
                     "  .\\O (global_out )\n"
                     " ) ;\n"
                     "endmodule";
            hdl_parser_verilog verilog_parser(input);
            std::shared_ptr<netlist> nl = verilog_parser.parse(g_lib_name);
            EXPECT_EQ(nl, nullptr);
        }
        {
            // The input does not contain any module (is empty in this case)
            NO_COUT_TEST_BLOCK;
            std::stringstream input;
            input << "";
            hdl_parser_verilog verilog_parser(input);
            std::shared_ptr<netlist> nl = verilog_parser.parse(g_lib_name);
            EXPECT_EQ(nl, nullptr);
        }
        /*{
            // one side of the direct assignment is empty
            NO_COUT_TEST_BLOCK;
            std::stringstream input;
            input << "module top (\n"
                     "  global_in,\n"
                     "  global_out\n"
                     " ) ;\n"
                     "  input global_in ;\n"
                     "  output global_out ;\n"
                     "  wire signal_0 ;\n"
                     "  assign signal_0 = ;\n"
                     "endmodule";
            hdl_parser_verilog verilog_parser(input);
            std::shared_ptr<netlist> nl = verilog_parser.parse(g_lib_name);
            EXPECT_EQ(nl, nullptr);
        }*/
        {
            // The two sides of assignment do not have the same width
            NO_COUT_TEST_BLOCK;
            std::stringstream input;
            input << "module top (\n"
                     "  global_in,\n"
                     "  global_out\n"
                     " ) ;\n"
                     "  input global_in ;\n"
                     "  output global_out ;\n"
                     "  wire [0:4] signal_vec_0 ;\n"
                     "  wire [0:4] signal_vec_1 ;\n"
                     "  assign signal_vec_0[0:3] = signal_vec_0[0:4];\n"
                     "INV gate_0 (\n"
                     "  .\\I (global_in ),\n"
                     "  .\\O (global_out )\n"
                     " ) ;\n"
                     "endmodule";
            hdl_parser_verilog verilog_parser(input);
            std::shared_ptr<netlist> nl = verilog_parser.parse(g_lib_name);
            EXPECT_EQ(nl, nullptr);
        }
        /*{
            // Having a cyclic module hierarchy (NOTE: Infinite Loop)
            NO_COUT_TEST_BLOCK;
            std::stringstream input;
            input << "module ENT_0 (\n"
                     "  IE0,\n"
                     "  OE0 \n"
                     " ) ;\n"
                     "  input IE0 ;\n"
                     "  output OE0 ;\n"
                     "ENT_1 gate_0 (\n"
                     "  .\\IE1 (IE0 ),\n"
                     "  .\\OE1 (OE0 )\n"
                     " ) ;\n"
                     "endmodule\n"
                     "module ENT_1 (\n"
                     "  IE1,\n"
                     "  OE1 \n"
                     " ) ;\n"
                     "  input IE1 ;\n"
                     "  output OE1 ;\n"
                     "ENT_0 gate_0 (\n"
                     "  .\\IE0 (IE1 ),\n"
                     "  .\\OE0 (OE1 )\n"
                     " ) ;\n"
                     "endmodule";
            hdl_parser_verilog verilog_parser(input);
            std::shared_ptr<netlist> nl = verilog_parser.parse(g_lib_name);
            EXPECT_EQ(nl, nullptr);
        }*/
        {
            // Having a cyclic assignment hierarchy
            NO_COUT_TEST_BLOCK;
            std::stringstream input;
            input << "module top (\n"
                     "  global_in,\n"
                     "  global_out \n"
                     " ) ;\n"
                     "  input global_in ;\n"
                     "  output global_out ;\n"
                     "  wire net_0;\n"
                     "  wire net_1;\n"
                     "  assign net_0 = net_1;\n"
                     "  assign net_1 = net_0;\n"
                     "INV gate_0 (\n"
                     "  .\\I (global_in ),\n"
                     "  .\\O (net_0 )\n"
                     " ) ;\n"
                     "INV gate_1 (\n"
                     "  .\\I (net_1 ),\n"
                     "  .\\O (global_out )\n"
                     " ) ;\n"
                     "endmodule";
            hdl_parser_verilog verilog_parser(input);
            std::shared_ptr<netlist> nl = verilog_parser.parse(g_lib_name);
            EXPECT_EQ(nl, nullptr);
        }
        /*{
            // Port map gets multiple nets
            NO_COUT_TEST_BLOCK;
            std::stringstream input;
            input << "module top (\n"
                     "  global_in,\n"
                     "  global_out \n"
                     " ) ;\n"
                     "  input global_in ;\n"
                     "  output global_out ;\n"
                     "  wire net_0;\n"
                     "INV gate_0 (\n"
                     "  .\\I ({global_in, net_0} )\n"
                     " ) ;\n"
                     "INV gate_1 (\n"
                     "  .\\I (net_0 ),\n"
                     "  .\\O (global_out )\n"
                     " ) ;\n"
                     "endmodule";
            hdl_parser_verilog verilog_parser(input);
            std::shared_ptr<netlist> nl = verilog_parser.parse(g_lib_name);
            EXPECT_EQ(nl, nullptr);
        }*/
        {
            // Port name is unknown
            NO_COUT_TEST_BLOCK;
            std::stringstream input;
            input << "module top (\n"
                     "  global_in,\n"
                     "  global_out \n"
                     " ) ;\n"
                     "  input global_in ;\n"
                     "  output global_out ;\n"
                     "  wire net_0;\n"
                     "INV gate_0 (\n"
                     "  .\\UNKNOWN_PORT_NAME (net_0)\n"
                     " ) ;\n"
                     "INV gate_1 (\n"
                     "  .\\I (net_0 ),\n"
                     "  .\\O (global_out )\n"
                     " ) ;\n"
                     "endmodule";
            hdl_parser_verilog verilog_parser(input);
            std::shared_ptr<netlist> nl = verilog_parser.parse(g_lib_name);
            EXPECT_EQ(nl, nullptr);
        }
        {
            // Store an unknown data type
            NO_COUT_TEST_BLOCK;
            std::stringstream input("module top (\n"
                                    "  global_in,\n"
                                    "  global_out\n"
                                    " ) ;\n"
                                    "  input global_in ;\n"
                                    "  output global_out ;\n"
                                    "INV #(\n"
                                    ".key_unknown(#Unkn0wn!)) \n"
                                    "gate_0 (\n"
                                    "  .\\I (global_in ),\n"
                                    "  .\\O (global_out )\n"
                                    " ) ;\n"
                                    "endmodule");
            hdl_parser_verilog verilog_parser(input);
            std::shared_ptr<netlist> nl = verilog_parser.parse(g_lib_name);
            EXPECT_EQ(nl, nullptr);
        }
        {
            // Assign one net to another, but booth are connected to different source gates
            NO_COUT_TEST_BLOCK;
            std::stringstream input("module top (\n"
                                    "  global_in,\n"
                                    "  global_out\n"
                                    " ) ;\n"
                                    "  input global_in ;\n"
                                    "  output global_out ;\n"
                                    "  wire net_0 ;\n"
                                    "  wire net_1 ;\n"
                                    "  assign net_0 = net_1 ;\n"
                                    "INV gate_0 (\n"
                                    "  .\\I (global_in ),\n"
                                    "  .\\O (net_0 )\n"
                                    " ) ;\n"
                                    "INV gate_1 (\n"
                                    "  .\\I (global_in ),\n"
                                    "  .\\O (net_1 )\n"
                                    " ) ;\n"
                                    "AND2 gate_1 (\n"
                                    "  .\\I0 (net_0 ),\n"
                                    "  .\\I1 (net_1 ),\n"
                                    "  .\\O (global_out )\n"
                                    " ) ;\n"
                                    "endmodule");
            hdl_parser_verilog verilog_parser(input);
            std::shared_ptr<netlist> nl = verilog_parser.parse(g_lib_name);

            EXPECT_EQ(nl, nullptr);
        }
        {
            // MAYBE SOME MORE TESTS HERE LATER...
        }
    TEST_END
}
