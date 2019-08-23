#include "netlist/gate.h"
#include "netlist/gate_library/gate_library_manager.h"
#include "netlist/netlist.h"
#include "netlist/netlist_factory.h"
#include "netlist/persistent/netlist_serializer.h"
#include "test_def.h"
#include "gtest/gtest.h"
#include <core/log.h>
#include <hdl_parser/hdl_parser_verilog.h>
#include <iostream>
#include <sstream>
#include <boost/filesystem.hpp>

class hdl_parser_verilog_test : public ::testing::Test
{
protected:
    const std::string g_lib_name = "EXAMPLE_GATE_LIBRARY";
    const std::string temp_lib_name = "TEMP_GATE_LIBRARY";
    // Minimum id for netlists, gates, nets and modules
    //const u32 INVALID_GATE_ID = 0;
    //const u32 INVALID_NET_ID = 0;
    //const u32 INVALID_MODULE_ID = 0;
    //const u32 MIN_MODULE_ID = 2;
    const u32 MIN_GATE_ID = 1;
    const u32 MIN_NET_ID = 1;
    //const u32 MIN_NETLIST_ID = 1;
    //const u32 TOP_MODULE_ID = 1;
    // Path used, to create a custom gate library (used to test certain behaviour of input and output vectors)
    hal::path temp_lib_path;

    virtual void SetUp()
    {
        NO_COUT_BLOCK;
        temp_lib_path = core_utils::get_gate_library_directories()[0] / "temp_lib.json";
        gate_library_manager::load_all();
    }

    virtual void TearDown()
    {
        boost::filesystem::remove(temp_lib_path);
    }

    // Creates an endpoint from a gate and a pin_type
    endpoint get_endpoint(std::shared_ptr<gate> g, std::string pin_type)
    {
        endpoint ep;
        ep.gate     = g;
        ep.pin_type = pin_type;
        return ep;
    }

    // NOTE: temporary
    std::shared_ptr<netlist> create_example_parse_netlist(int id = -1)
    {
        //NO_COUT_BLOCK;
        std::shared_ptr<gate_library> gl = gate_library_manager::get_gate_library(g_lib_name);
        std::shared_ptr<netlist> nl      = std::make_shared<netlist>(gl);
        if (id >= 0)
        {
            nl->set_id(id);
        }

        // Create the gates
        std::shared_ptr<gate> gate_0 = nl->create_gate(MIN_GATE_ID+0, "AND2", "gate_0");
        std::shared_ptr<gate> gate_1 = nl->create_gate(MIN_GATE_ID+1, "GND", "gate_1");
        std::shared_ptr<gate> gate_2 = nl->create_gate(MIN_GATE_ID+2, "VCC", "gate_2");
        std::shared_ptr<gate> gate_3 = nl->create_gate(MIN_GATE_ID+3, "INV", "gate_3");
        std::shared_ptr<gate> gate_4 = nl->create_gate(MIN_GATE_ID+4, "INV", "gate_4");
        std::shared_ptr<gate> gate_5 = nl->create_gate(MIN_GATE_ID+5, "AND2", "gate_5");
        //std::shared_ptr<gate> gate_6 = nl->create_gate(MIN_GATE_ID+6, "BUF", "gate_6");
        std::shared_ptr<gate> gate_6 = nl->create_gate(MIN_GATE_ID+6, "OR2", "gate_6");
        std::shared_ptr<gate> gate_7 = nl->create_gate(MIN_GATE_ID+7, "OR2", "gate_7");

        // Add the nets (net_x_y1_y2... := net between the gate with id x and the gates y1,y2,...)
        std::shared_ptr<net> net_1_3 = nl->create_net(MIN_NET_ID+13, "0_net");
        net_1_3->set_src(gate_1, "O");
        net_1_3->add_dst(gate_3, "I");

        std::shared_ptr<net> net_3_0 = nl->create_net(MIN_NET_ID+30, "net_3_0");
        net_3_0->set_src(gate_3, "O");
        net_3_0->add_dst(gate_0, "I0");

        std::shared_ptr<net> net_2_0 = nl->create_net(MIN_NET_ID+20, "1_net");
        net_2_0->set_src(gate_2, "O");
        net_2_0->add_dst(gate_0, "I1");

        std::shared_ptr<net> net_0_4_5 = nl->create_net(MIN_NET_ID+045, "net_0_4_5");
        net_0_4_5->set_src(gate_0, "O");
        net_0_4_5->add_dst(gate_4, "I");
        net_0_4_5->add_dst(gate_5, "I0");

        std::shared_ptr<net> net_6_7 = nl->create_net(MIN_NET_ID+67, "net_6_7");
        net_6_7->set_src(gate_6, "O");
        net_6_7->add_dst(gate_7, "I0");

        std::shared_ptr<net> net_4_out = nl->create_net(MIN_NET_ID+400, "net_4_out");
        net_4_out->set_src(gate_4, "O");

        std::shared_ptr<net> net_5_out = nl->create_net(MIN_NET_ID+500, "net_5_out");
        net_5_out->set_src(gate_5, "O");

        std::shared_ptr<net> net_7_out = nl->create_net(MIN_NET_ID+700, "net_7_out");
        net_7_out->set_src(gate_7, "O");

        return nl;
    }

    // Create and load temporarily a custom gate library, which contains gates with input and output vectors up to dimension 3 (this is not the gate_lib of the vhdl_parser test)
    void create_temp_gate_lib()
    {
        NO_COUT_BLOCK;
        std::ofstream test_lib(temp_lib_path.string());
        test_lib << "{\n"
                    "    \"library\": {\n"
                    "        \"library_name\": \"TEMP_GATE_LIBRARY\",\n"
                    "        \"elements\": {\n"
                    "            \"GATE0\" : [[\"I\"], [], [\"O\"]],\n"
                    "            \"GATE1\" : [[\"I(0)\",\"I(1)\",\"I(2)\",\"I(3)\"], [], [\"O\"]],\n"
                    "            \"GATE2\" : [[\"I\"], [], [\"O(0)\",\"O(1)\",\"O(2)\",\"O(3)\"]],\n"
                    "            \"GATE3\" : [[\"I(0)\",\"I(1)\",\"I(2)\",\"I(3)\"], [], [\"O(0)\",\"O(1)\",\"O(2)\",\"O(3)\"]],\n"
                    //"            \"GATE4\" : [[\"I(0, 0)\",\"I(0, 1)\",\"I(1, 0)\",\"I(1, 1)\"], [], [\"O(0, 0)\",\"O(0, 1)\",\"O(1, 0)\",\"O(1, 1)\"]],\n"
                    //"            \"GATE5\" : [[\"I(0, 0, 0)\",\"I(0, 0, 1)\",\"I(0, 1, 0)\",\"I(0, 1, 1)\",\"I(1, 0, 0)\",\"I(1, 0, 1)\",\"I(1, 1, 0)\",\"I(1, 1, 1)\"], [], [\"O(0, 0, 0)\",\"O(0, 0, 1)\",\"O(0, 1, 0)\",\"O(0, 1, 1)\",\"O(1, 0, 0)\",\"O(1, 0, 1)\",\"O(1, 1, 0)\",\"O(1, 1, 1)\"]],\n"
                    "\n"
                    "            \"GND\" : [[], [], [\"O\"]],\n"
                    "            \"VCC\" : [[], [], [\"O\"]]\n"
                    "        },\n"
                    "        \"vhdl_includes\": [],\n"
                    "        \"global_gnd_nodes\": [\"GND\"],\n"
                    "        \"global_vcc_nodes\": [\"VCC\"]\n"
                    "    }\n"
                    "}";
        test_lib.close();

        gate_library_manager::load_all();
    }

    // Checks if two vectors have the same content regardless of their order
    template<typename T>
    bool vectors_have_same_content(std::vector<T> vec_1, std::vector<T> vec_2)
    {
        if (vec_1.size() != vec_2.size())
        {
            return false;
        }

        // Each element of vec_1 must be found in vec_2
        while (vec_1.size() > 0)
        {
            auto it_1       = vec_1.begin();
            bool found_elem = false;
            for (auto it_2 = vec_2.begin(); it_2 != vec_2.end(); it_2++)
            {
                if (*it_1 == *it_2)
                {
                    found_elem = true;
                    vec_2.erase(it_2);
                    break;
                }
            }
            if (!found_elem)
            {
                return false;
            }
            vec_1.erase(it_1);
        }

        return true;
    }

};


/**
 * Used by clueless testers to understand the verilog syntax used by the writer (will be removed later)
 */
 /*
#include "hdl_writer/hdl_writer_verilog.h"
TEST_F(hdl_parser_verilog_test, check_temporary)
{
    TEST_START
        {
            // Write and parse the example netlist (with some additions) and compare the result with the original netlist
            std::shared_ptr<netlist> nl = create_example_parse_netlist(0);


            // Mark the global gates as such
            nl->get_gate_by_id(MIN_GATE_ID+1)->set_data("generic","my_key","string","test_test");
            nl->get_gate_by_id(MIN_GATE_ID+1)->set_data("generic","my_second_key","string","test_2");

            nl->mark_global_gnd_gate(nl->get_gate_by_id(MIN_GATE_ID+1));
            nl->mark_global_vcc_gate(nl->get_gate_by_id(MIN_GATE_ID+2));

            // Mark global output nets
            nl->mark_global_output_net(nl->get_net_by_id(MIN_NET_ID+400));
            nl->mark_global_input_net(nl->get_net_by_id(MIN_NET_ID+500));
            nl->mark_global_inout_net(nl->get_net_by_id(MIN_NET_ID+700));

            nl->set_device_name("test_device_name");


            // Write and parse the netlist now
            test_def::capture_stdout();
            std::stringstream parser_input;
            hdl_writer_verilog verilog_writer(parser_input);


            // Writes the netlist in the sstream
            bool writer_suc = verilog_writer.write(nl);

            if(!writer_suc){
                std::cout << test_def::get_captured_stdout() << std::endl;
            }
            ASSERT_TRUE(writer_suc);

            hdl_parser_verilog verilog_parser(parser_input);
            // Parse the .v file
            std::shared_ptr<netlist> parsed_nl = verilog_parser.parse(g_lib_name);

            if(parsed_nl == nullptr){
                std::cout << test_def::get_captured_stdout() << std::endl;
            }
            //ASSERT_NE(parsed_nl, nullptr);
            test_def::get_captured_stdout();

            std::cout << "\n---OUTPUT---\n" << parser_input.str() << "\n---OUTPUT---\n";
        }
    TEST_END
}*/



/*                                    net_0
 *                  .--= INV (0) =----.
 *  global_in       |                   '-=                     global_out
 *      ------------|                   .-= AND3 (2) = ----------
 *                  |                   | =
 *                  '--=                |
 *                       AND2 (1) =---'
 *                     =              net_1
 *                                                              global_inout
 *      -----------------------------------------------------------
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
        /*{ // NOTE: inout nets can't be handled
            std::stringstream input("module  (\n"
                                    "  global_in,\n"
                                    "  global_out \n"
                                    //"  global_inout\n"
                                    " ) ;\n"
                                    "  input global_in ;\n"
                                    "  output global_out ;\n"
                                    "  wire global_inout ;\n"
                                    "  wire net_0 ;\n"
                                    "  wire net_1 ;\n"
                                    "INV gate_0 (\n"
                                    "  .\\I (global_in ),\n"
                                    "  .\\O (net_0 )\n"
                                    " ) ;\n"
                                    "AND2 gate_1 (\n"
                                    "  .\\I0 (global_in ),\n"
                                    "  .\\O (net_1 )\n"
                                    " ) ;\n"
                                    "AND3 gate_2 (\n"
                                    "  .\\I0 (net_0 ),\n"
                                    "  .\\I1 (net_1 ),\n"
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


            // Check if the gates are parsed correctly
            ASSERT_EQ(nl->get_gates("INV").size(), 1);
            std::shared_ptr<gate> gate_0 = *(nl->get_gates("INV").begin());
            ASSERT_EQ(nl->get_gates("AND2").size(), 1);
            std::shared_ptr<gate> gate_1 = *(nl->get_gates("AND2").begin());
            ASSERT_EQ(nl->get_gates("AND3").size(), 1);
            std::shared_ptr<gate> gate_2 = *(nl->get_gates("AND3").begin());

            ASSERT_NE(gate_0, nullptr);
            EXPECT_EQ(gate_0->get_name(), "gate_0");

            ASSERT_NE(gate_1, nullptr);
            EXPECT_EQ(gate_1->get_name(), "gate_1");

            ASSERT_NE(gate_2, nullptr);
            EXPECT_EQ(gate_2->get_name(), "gate_2");

            // Check if the nets are parsed correctly
            ASSERT_FALSE(nl->get_nets("net_0").empty());
            std::shared_ptr<net> net_0            = *(nl->get_nets("net_0").begin());
            ASSERT_FALSE(nl->get_nets("net_1").empty());
            std::shared_ptr<net> net_1            = *(nl->get_nets("net_1").begin());
            ASSERT_FALSE(nl->get_nets("global_in").empty());
            std::shared_ptr<net> net_global_in    = *(nl->get_nets("global_in").begin());
            ASSERT_FALSE(nl->get_nets("global_out").empty());
            std::shared_ptr<net> net_global_out   = *(nl->get_nets("global_out").begin());
            ASSERT_FALSE(nl->get_nets("global_inout").empty());
            std::shared_ptr<net> net_global_inout = *(nl->get_nets("global_inout").begin());

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

            ASSERT_NE(net_global_inout, nullptr);
            EXPECT_EQ(net_global_inout->get_name(), "global_inout");
            EXPECT_EQ(net_global_inout->get_src(), get_endpoint(nullptr, ""));
            EXPECT_TRUE(net_global_inout->get_dsts().empty());
            //EXPECT_TRUE(nl->is_global_inout_net(net_global_inout));

            EXPECT_EQ(nl->get_global_input_nets().size(), 1);
            EXPECT_EQ(nl->get_global_output_nets().size(), 1);
            //EXPECT_EQ(nl->get_global_inout_nets().size(), 1);
        }*/
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
            // Use the one-line-comment ('//')
            std::stringstream input("module  ( \n"
                                    " ) ;\n"
                                    "  wire net_0 ;\n"
                                    "  //wire comment_net ;\n"
                                    "  wire net_1;\n"
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

            EXPECT_TRUE(nl->get_nets("comment_net").empty());
            EXPECT_EQ(nl->get_nets().size(), 2);
        }
        {
            // Use a multi-line comment ('/ *', '* /') for a complete single line
            std::stringstream input("module  ( \n"
                                    " ) ;\n"
                                    "  wire net_0 ;\n"
                                    "/*  wire comment_net;*/\n"
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

            EXPECT_TRUE(nl->get_nets("comment_net").empty());
            EXPECT_EQ(nl->get_nets().size(), 1);
        }
        {
            // Use a multi-line comment ('/ *', '* /') inside one single line
            std::stringstream input("module  ( \n"
                                    " ) ;\n"
                                    "  wire net_0 ;\n"
                                    "  wire /*comment_net*/ net_1;\n"
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

            EXPECT_TRUE(nl->get_nets("comment_net").empty());
            EXPECT_EQ(nl->get_nets().size(), 2);
        }
//        {
//            // Use the multi-line-comment over multiple lines ('/ *' and '* /')
//            std::stringstream input("module  ( \n"
//                                    " ) ;\n"
//                                    "  wire net_0 ;\n"
//                                    "/*  wire comment_net_0 ;\n"
//                                    "  wire comment_net_1 ;\n"
//                                    "  wire comment_net_2 ;*/\n"
//                                    "  wire net_1;\n"
//                                    "endmodule");
//            test_def::capture_stdout();
//            hdl_parser_verilog verilog_parser(input);
//            std::shared_ptr<netlist> nl = verilog_parser.parse(g_lib_name);
//            if (nl == nullptr)
//            {
//                std::cout << test_def::get_captured_stdout();
//            }
//            else
//            {
//                test_def::get_captured_stdout();
//            }
//
//            ASSERT_NE(nl, nullptr);
//
//            EXPECT_TRUE(nl->get_nets("comment_net_0").empty());
//            EXPECT_TRUE(nl->get_nets("comment_net_1").empty());
//            EXPECT_TRUE(nl->get_nets("comment_net_2").empty());
//            EXPECT_EQ(nl->get_nets().size(), 2);
//        }
        {
            // Use a multi-line attributes ('(*', '*)') for a complete single line
            std::stringstream input("module  ( \n"
                                    " ) ;\n"
                                    "  wire net_0 ;\n"
                                    "(*  wire comment_net;*)\n"
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

            EXPECT_TRUE(nl->get_nets("comment_net").empty());
            EXPECT_EQ(nl->get_nets().size(), 1);
        }
        {
            // Use an attribute ('(*', '*)') inside one single line
            std::stringstream input("module  ( \n"
                                    " ) ;\n"
                                    "  wire net_0 ;\n"
                                    "  wire (*comment_net*) net_1;\n"
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

            EXPECT_TRUE(nl->get_nets("comment_net").empty());
            EXPECT_EQ(nl->get_nets().size(), 2);
        }
        /*{ ISSUE: fails (comments_net_1 is created)
            // Use an attribute over multiple lines ('(*' and '*)')
            std::stringstream input("module  ( \n"
                                    " ) ;\n"
                                    "  wire net_0 ;\n"
                                    "(*  wire comment_net_0 ;\n"
                                    "  wire comment_net_1 ;\n"
                                    "  wire comment_net_2 ;*)\n"
                                    "  wire net_1;\n"
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

            EXPECT_TRUE(nl->get_nets("comment_net_0").empty());
            EXPECT_TRUE(nl->get_nets("comment_net_1").empty());
            EXPECT_TRUE(nl->get_nets("comment_net_2").empty());
            EXPECT_EQ(nl->get_nets().size(), 2);
        }*/

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
        /*{ // ISSUE: value has ')' at the end
            // Store an instance of all possible data types in one gate
            std::stringstream input("module  (\n"
                                    "  global_in,\n"
                                    "  global_out\n"
                                    " ) ;\n"
                                    "  input global_in ;\n"
                                    "  output global_out ;\n"
                                    "INV #(\n"
                                    //".key_integer(1234),\n"
                                    //".key_floating_point(1.234),\n"
                                    ".key_string(\"test_string\"),\n"
                                    ".key_bit_vector_hex('habc),\n"
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

            ASSERT_EQ(nl->get_gates("INV","gate_0").size(), 1);
            std::shared_ptr<gate> gate_0 = *nl->get_gates("INV","gate_0").begin();

            EXPECT_EQ(gate_0->get_data_by_key("generic","key_integer"), std::make_tuple("integer", "1234"));
            // NOTE: check the other data types
        }*/
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
            std::stringstream input("module  (\n"
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
            ASSERT_EQ(nl->get_nets("net_vec[1]").size(), 1);
            ASSERT_EQ(nl->get_nets("net_vec[2]").size(), 1);
            ASSERT_EQ(nl->get_nets("net_vec[3]").size(), 1);

            std::shared_ptr<net> net_vec_1 = *nl->get_nets("net_vec[1]").begin();
            std::shared_ptr<net> net_vec_2 = *nl->get_nets("net_vec[2]").begin();
            std::shared_ptr<net> net_vec_3 = *nl->get_nets("net_vec[3]").begin();

            // Check if all gates are created
            for (std::string g_name : std::set<std::string>({"gate_0", "gate_1", "gate_2", "gate_3"})){
                ASSERT_EQ(nl->get_gates(DONT_CARE, g_name).size(), 1);
            }

            EXPECT_EQ(net_vec_1->get_src().get_gate(), *nl->get_gates(DONT_CARE, "gate_1").begin());
            ASSERT_EQ(net_vec_1->get_dsts().size(), 1);
            EXPECT_EQ((*net_vec_1->get_dsts().begin()).get_gate(), *nl->get_gates(DONT_CARE, "gate_0").begin());
        }
        {
            // Declare multiple wire vectors in one line
            std::stringstream input("module  (\n"
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
            ASSERT_EQ(nl->get_nets("net_vec_0[0]").size(), 1);
            ASSERT_EQ(nl->get_nets("net_vec_0[1]").size(), 1);
            ASSERT_EQ(nl->get_nets("net_vec_0[0]").size(), 1);
            ASSERT_EQ(nl->get_nets("net_vec_0[1]").size(), 1);
        }
    TEST_END
}


/**
 * Testing the correct handling of the 'assign' keyword. The assign-keyword is only used, to assign one net to another one. Therefore only statements like
 * "assing signal_1 = signal_0" are valid, which would be realized like:
 *
 *
 * ... -<signal_0>--+------=| gate_0 |= ...
 *                  |    '-- ...
 *                  |
 *                  '------=| buffer_gate |=-----<signal_1>---- ...
 *
 *  Note: assignments like "assign net_0 = net_1 ^ net_2" are not (yet?) supported
 *
 * Functions: parse
 */
TEST_F(hdl_parser_verilog_test, check_assign)
{
    TEST_START
        {
            // Use the assign statement. The netlist should be created like this:
            /*
             *
             *     ---<global_in>---=| gate_0 |=--<net_0>---+---=| gate_1 |=---<global_out_0>---
             *                                              |
             *                                              '---=| buffer |=---<net_1>---=| gate_2 |=---<global_out_1>
             */
            std::stringstream input("module  (\n"
                                    "  global_in,\n"
                                    "  global_out\n"
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

            ASSERT_EQ(nl->get_gates(DONT_CARE,"gate_0").size(), 1);
            ASSERT_EQ(nl->get_gates(DONT_CARE,"gate_1").size(), 1);
            ASSERT_EQ(nl->get_gates(DONT_CARE,"gate_2").size(), 1);
            std::shared_ptr<gate> gate_0 = *nl->get_gates(DONT_CARE,"gate_0").begin();
            std::shared_ptr<gate> gate_1 = *nl->get_gates(DONT_CARE,"gate_1").begin();
            std::shared_ptr<gate> gate_2 = *nl->get_gates(DONT_CARE,"gate_2").begin();

            ASSERT_EQ(nl->get_gates(DONT_CARE, "net_1_buffer").size(), 1); // Name creation: "<net_name>_buffer"
            std::shared_ptr<gate> buffer_gate = *nl->get_gates(DONT_CARE, "net_1_buffer").begin();

            ASSERT_EQ(nl->get_nets("net_0").size(), 1);
            ASSERT_EQ(nl->get_nets("net_1").size(), 1);
            std::shared_ptr<net> net_0 = *nl->get_nets("net_0").begin();
            std::shared_ptr<net> net_1 = *nl->get_nets("net_1").begin();

            EXPECT_EQ(net_0->get_src().get_gate(), gate_0);
            ASSERT_EQ(net_0->get_num_of_dsts(), 2);
            ASSERT_EQ(net_0->get_dsts("INV").size(), 1);
            EXPECT_EQ((*net_0->get_dsts("INV").begin()).get_gate(), gate_1);
            ASSERT_EQ(net_0->get_dsts(buffer_gate->get_type()).size(), 1);
            EXPECT_EQ((*net_0->get_dsts(buffer_gate->get_type()).begin()).get_gate(), buffer_gate);

            ASSERT_EQ(net_1->get_num_of_dsts(), 1);
            EXPECT_EQ(net_1->get_src().get_gate(), buffer_gate);
            ASSERT_EQ(net_1->get_num_of_dsts(), 1);
            EXPECT_EQ((*net_1->get_dsts().begin()).get_gate(), gate_2);

        }
    TEST_END
}

/**
 * Testing various types of net assignments (not the 'assign'-statement, but i.e. '.\ Pin_0 (net_0)'. That includes the usage
 * of number literals like  4'hA , the usage of pin vectors (i.e. we use I to represent I(0),...,I(3)) as well as using multiple
 * nets in curly brackets '{net_0, net_1, ...}'
 *
 * Testing the assingnments of multiple '0'- and '1'-nets to pins (we use I to represent I(0),...,I(3)) using number literals
 * For example: .\I (4'hA)
 *
 * Functions: parse
 */
TEST_F(hdl_parser_verilog_test, check_number_literal)
{
    TEST_START
        create_temp_gate_lib();
        for (std::string len : std::vector<std::string>{"4",""})
        {
            {
                // Using of numeric_literals like 4'hA (as hexadecimal value) to assign multiple input pins at once to global input nets

                // Build a the parsed string
                std::stringstream instances;
                std::stringstream module_block;
                std::stringstream net_block;
                int ctr = 0;
                for (std::string num_literal : std::vector<std::string>{"0", "1", "2", "3", "4", "5", "6", "7", "8",
                                                                        "9", "A", "B", "C", "D", "E", "F"}) {
                    // For every possible number literal (with length 4 bits), create a gate and a global output net
                    instances << "GATE1 gate_" << ctr << " (\n  .\\I ( "<< len <<"'h" << num_literal << "),\n  .\\O (global_out_"
                              << ctr << ")\n ) ;\n";

                    module_block << ",\n  global_out_" << ctr;

                    net_block << "  output global_out_" << ctr << " ;\n";
                    ctr++;
                }
                std::stringstream input;
                input << "module  (\n"
                      << "  global_in "
                      << module_block.str()
                      << " ) ;\n"
                      << "  input global_in ;\n"
                      << net_block.str()
                      << instances.str()
                      << "endmodule";


                test_def::capture_stdout();
                hdl_parser_verilog verilog_parser(input);
                std::shared_ptr<netlist> nl = verilog_parser.parse(temp_lib_name);
                if (nl == nullptr) {
                    std::cout << test_def::get_captured_stdout();
                } else {
                    test_def::get_captured_stdout();
                }

                ASSERT_NE(nl, nullptr);
                // Verify that every gate is connected correctly
                for (int i = 0; i < 16; i++) {

                    ASSERT_EQ(nl->get_gates("GATE1", "gate_" + std::to_string(i)).size(), 1);
                    std::shared_ptr<gate> gate_i = *nl->get_gates("GATE1", "gate_" + std::to_string(i)).begin();
                    for (int idx = 0; idx < 4; idx++) {
                        // Calculate if the pin should be connected to a gnd (0) or vcc (1) gate
                        std::string bit = ((i >> (3 - idx)) & 1) ? "1" : "0";
                        std::string pin = "I(" + std::to_string(idx) + ")";
                        ASSERT_NE(gate_i->get_fan_in_net(pin), nullptr);
                        EXPECT_EQ(gate_i->get_fan_in_net(pin)->get_name(), "1'b" + bit);
                        ASSERT_NE(gate_i->get_fan_in_net(pin)->get_src().get_gate(), nullptr);
                        // Check if the connected gate is a global gnd/vcc gate
                        if (bit == "0")
                            EXPECT_TRUE(nl->get_gate_library()->get_global_gnd_gate_types()->find(
                                    gate_i->get_fan_in_net(pin)->get_src().get_gate()->get_type()) !=
                                        nl->get_gate_library()->get_global_gnd_gate_types()->end());
                        else
                            EXPECT_TRUE(nl->get_gate_library()->get_global_vcc_gate_types()->find(
                                    gate_i->get_fan_in_net(pin)->get_src().get_gate()->get_type()) !=
                                        nl->get_gate_library()->get_global_gnd_gate_types()->end());


                    }
                }

            }
            {
                // Using of numeric_literals like 4'hA (as hexadecimal value) to assign multiple input pins at once to global input nets

                // Build a the parsed string
                std::stringstream instances;
                std::stringstream module_block;
                std::stringstream net_block;
                int ctr = 0;
                for (std::string num_literal : std::vector<std::string>{"0", "1", "2", "3", "4", "5", "6", "7", "8",
                                                                        "9", "A", "B", "C", "D", "E", "F"}) {
                    // For every possible number literal (with length 4 bits), create a gate and a global output net
                    instances << "GATE1 gate_" << ctr << " (\n  .\\I ( "<< len <<"'h" << num_literal << "),\n  .\\O (global_out_"
                              << ctr << ")\n ) ;\n";

                    module_block << ",\n  global_out_" << ctr;

                    net_block << "  output global_out_" << ctr << " ;\n";
                    ctr++;
                }
                std::stringstream input;
                input << "module  (\n"
                      << "  global_in "
                      << module_block.str()
                      << " ) ;\n"
                      << "  input global_in ;\n"
                      << net_block.str()
                      << instances.str()
                      << "endmodule";


                test_def::capture_stdout();
                hdl_parser_verilog verilog_parser(input);
                std::shared_ptr<netlist> nl = verilog_parser.parse(temp_lib_name);
                if (nl == nullptr) {
                    std::cout << test_def::get_captured_stdout();
                } else {
                    test_def::get_captured_stdout();
                }

                ASSERT_NE(nl, nullptr);
                // Verify that every gate is connected correctly
                for (int i = 0; i < 16; i++) {

                    ASSERT_EQ(nl->get_gates("GATE1", "gate_" + std::to_string(i)).size(), 1);
                    std::shared_ptr<gate> gate_i = *nl->get_gates("GATE1", "gate_" + std::to_string(i)).begin();
                    for (int idx = 0; idx < 4; idx++) {
                        // Calculate if the pin should be connected to a gnd (0) or vcc (1) gate
                        std::string bit = ((i >> (3 - idx)) & 1) ? "1" : "0";
                        std::string pin = "I(" + std::to_string(idx) + ")";
                        ASSERT_NE(gate_i->get_fan_in_net(pin), nullptr);
                        EXPECT_EQ(gate_i->get_fan_in_net(pin)->get_name(), "1'b" + bit);
                        ASSERT_NE(gate_i->get_fan_in_net(pin)->get_src().get_gate(), nullptr);
                        // Check if the connected gate is a global gnd/vcc gate
                        if (bit == "0")
                            EXPECT_TRUE(nl->get_gate_library()->get_global_gnd_gate_types()->find(
                                    gate_i->get_fan_in_net(pin)->get_src().get_gate()->get_type()) !=
                                        nl->get_gate_library()->get_global_gnd_gate_types()->end());
                        else
                            EXPECT_TRUE(nl->get_gate_library()->get_global_vcc_gate_types()->find(
                                    gate_i->get_fan_in_net(pin)->get_src().get_gate()->get_type()) !=
                                        nl->get_gate_library()->get_global_gnd_gate_types()->end());


                    }
                }

            }
            {
                // Using of numeric_literals like 4'hA (binary value) to assign multiple input pins at once to global input nets (with explicite length )

                // Build a the parsed string
                std::stringstream instances;
                std::stringstream module_block;
                std::stringstream net_block;
                int ctr = 0;
                for (std::string num_literal : std::vector<std::string>{"0000", "0001", "0010", "0011", "0100", "0101",
                                                                        "0110", "0111", "1000", "1001", "1010", "1011",
                                                                        "1100", "1101", "1110", "1111"}) {
                    // For every possible number literal (with length 4 bits), create a gate and a global output net
                    instances << "GATE1 gate_" << ctr << " (\n  .\\I ( "<< len <<"'b" << num_literal << "),\n  .\\O (global_out_"
                              << ctr << ")\n ) ;\n";

                    module_block << ",\n  global_out_" << ctr;

                    net_block << "  output global_out_" << ctr << " ;\n";
                    ctr++;
                }
                std::stringstream input;
                input << "module  (\n"
                      << "  global_in "
                      << module_block.str()
                      << " ) ;\n"
                      << "  input global_in ;\n"
                      << net_block.str()
                      << instances.str()
                      << "endmodule";


                test_def::capture_stdout();
                hdl_parser_verilog verilog_parser(input);
                std::shared_ptr<netlist> nl = verilog_parser.parse(temp_lib_name);
                if (nl == nullptr) {
                    std::cout << test_def::get_captured_stdout();
                } else {
                    test_def::get_captured_stdout();
                }

                ASSERT_NE(nl, nullptr);
                // Verify that every gate is connected correctly
                for (int i = 0; i < 16; i++) {

                    ASSERT_EQ(nl->get_gates("GATE1", "gate_" + std::to_string(i)).size(), 1);
                    std::shared_ptr<gate> gate_i = *nl->get_gates("GATE1", "gate_" + std::to_string(i)).begin();
                    for (int idx = 0; idx < 4; idx++) {
                        // Calculate if the pin should be connected to a gnd (0) or vcc (1) gate
                        std::string bit = ((i >> (3 - idx)) & 1) ? "1" : "0";
                        std::string pin = "I(" + std::to_string(idx) + ")";
                        ASSERT_NE(gate_i->get_fan_in_net(pin), nullptr);
                        EXPECT_EQ(gate_i->get_fan_in_net(pin)->get_name(), "1'b" + bit);
                        ASSERT_NE(gate_i->get_fan_in_net(pin)->get_src().get_gate(), nullptr);
                        // Check if the connected gate is a global gnd/vcc gate
                        if (bit == "0")
                            EXPECT_TRUE(nl->get_gate_library()->get_global_gnd_gate_types()->find(
                                    gate_i->get_fan_in_net(pin)->get_src().get_gate()->get_type()) !=
                                        nl->get_gate_library()->get_global_gnd_gate_types()->end());
                        else
                            EXPECT_TRUE(nl->get_gate_library()->get_global_vcc_gate_types()->find(
                                    gate_i->get_fan_in_net(pin)->get_src().get_gate()->get_type()) !=
                                        nl->get_gate_library()->get_global_gnd_gate_types()->end());


                    }
                }

            }
            if (len == "4")
            {
                // Using of numeric_literals like 4'hA (as decimal value) to assign multiple input pins at once to global input nets

                // Build a the parsed string
                std::stringstream instances;
                std::stringstream module_block;
                std::stringstream net_block;
                int ctr = 0;
                for (std::string num_literal : std::vector<std::string>{"0", "1", "2", "3", "4", "5", "6", "7", "8",
                                                                        "9", "10", "11", "12", "13", "14", "15"}) {
                    // For every possible number literal (with length 4 bits), create a gate and a global output net
                    instances << "GATE1 gate_" << ctr << " (\n  .\\I ( "<< len <<"'d" << num_literal << "),\n  .\\O (global_out_"
                              << ctr << ")\n ) ;\n";

                    module_block << ",\n  global_out_" << ctr;

                    net_block << "  output global_out_" << ctr << " ;\n";
                    ctr++;
                }
                std::stringstream input;
                input << "module  (\n"
                      << "  global_in "
                      << module_block.str()
                      << " ) ;\n"
                      << "  input global_in ;\n"
                      << net_block.str()
                      << instances.str()
                      << "endmodule";


                test_def::capture_stdout();
                hdl_parser_verilog verilog_parser(input);
                std::shared_ptr<netlist> nl = verilog_parser.parse(temp_lib_name);
                if (nl == nullptr) {
                    std::cout << test_def::get_captured_stdout();
                } else {
                    test_def::get_captured_stdout();
                }

                ASSERT_NE(nl, nullptr);
                // Verify that every gate is connected correctly
                for (int i = 0; i < 16; i++) {

                    ASSERT_EQ(nl->get_gates("GATE1", "gate_" + std::to_string(i)).size(), 1);
                    std::shared_ptr<gate> gate_i = *nl->get_gates("GATE1", "gate_" + std::to_string(i)).begin();
                    for (int idx = 0; idx < 4; idx++) {
                        // Calculate if the pin should be connected to a gnd (0) or vcc (1) gate
                        std::string bit = ((i >> (3 - idx)) & 1) ? "1" : "0";
                        std::string pin = "I(" + std::to_string(idx) + ")";
                        ASSERT_NE(gate_i->get_fan_in_net(pin), nullptr);
                        EXPECT_EQ(gate_i->get_fan_in_net(pin)->get_name(), "1'b" + bit);
                        ASSERT_NE(gate_i->get_fan_in_net(pin)->get_src().get_gate(), nullptr);
                        // Check if the connected gate is a global gnd/vcc gate
                        if (bit == "0")
                            EXPECT_TRUE(nl->get_gate_library()->get_global_gnd_gate_types()->find(
                                    gate_i->get_fan_in_net(pin)->get_src().get_gate()->get_type()) !=
                                        nl->get_gate_library()->get_global_gnd_gate_types()->end());
                        else
                            EXPECT_TRUE(nl->get_gate_library()->get_global_vcc_gate_types()->find(
                                    gate_i->get_fan_in_net(pin)->get_src().get_gate()->get_type()) !=
                                        nl->get_gate_library()->get_global_gnd_gate_types()->end());


                    }
                }

            }
            else
            {
                // Using of numeric_literals like 4'hA (as decimal value) to assign multiple input pins at once to global input nets

                // Build a the parsed string
                std::stringstream instances;
                std::stringstream module_block;
                std::stringstream net_block;
                int ctr = 8;
                for (std::string num_literal : std::vector<std::string>{"8", "9", "10", "11", "12", "13", "14", "15"}) {
                    // For every possible number literal (with length 4 bits), create a gate and a global output net
                    instances << "GATE1 gate_" << ctr << " (\n  .\\I ( "<< len <<"'d" << num_literal << "),\n  .\\O (global_out_"
                              << ctr << ")\n ) ;\n";

                    module_block << ",\n  global_out_" << ctr;

                    net_block << "  output global_out_" << ctr << " ;\n";
                    ctr++;
                }
                std::stringstream input;
                input << "module  (\n"
                      << "  global_in "
                      << module_block.str()
                      << " ) ;\n"
                      << "  input global_in ;\n"
                      << net_block.str()
                      << instances.str()
                      << "endmodule";


                test_def::capture_stdout();
                hdl_parser_verilog verilog_parser(input);
                std::shared_ptr<netlist> nl = verilog_parser.parse(temp_lib_name);
                if (nl == nullptr) {
                    std::cout << test_def::get_captured_stdout();
                } else {
                    test_def::get_captured_stdout();
                }

                ASSERT_NE(nl, nullptr);
                // Verify that every gate is connected correctly
                for (int i = 8; i < 16; i++) {

                    ASSERT_EQ(nl->get_gates("GATE1", "gate_" + std::to_string(i)).size(), 1);
                    std::shared_ptr<gate> gate_i = *nl->get_gates("GATE1", "gate_" + std::to_string(i)).begin();
                    for (int idx = 0; idx < 4; idx++) {
                        // Calculate if the pin should be connected to a gnd (0) or vcc (1) gate
                        std::string bit = ((i >> (3 - idx)) & 1) ? "1" : "0";
                        std::string pin = "I(" + std::to_string(idx) + ")";
                        ASSERT_NE(gate_i->get_fan_in_net(pin), nullptr);
                        EXPECT_EQ(gate_i->get_fan_in_net(pin)->get_name(), "1'b" + bit);
                        ASSERT_NE(gate_i->get_fan_in_net(pin)->get_src().get_gate(), nullptr);
                        // Check if the connected gate is a global gnd/vcc gate
                        if (bit == "0")
                            EXPECT_TRUE(nl->get_gate_library()->get_global_gnd_gate_types()->find(
                                    gate_i->get_fan_in_net(pin)->get_src().get_gate()->get_type()) !=
                                        nl->get_gate_library()->get_global_gnd_gate_types()->end());
                        else
                            EXPECT_TRUE(nl->get_gate_library()->get_global_vcc_gate_types()->find(
                                    gate_i->get_fan_in_net(pin)->get_src().get_gate()->get_type()) !=
                                        nl->get_gate_library()->get_global_gnd_gate_types()->end());


                    }
                }

            }
            {
                // testing the usage of high impedance (i.e. "4'hx" or "1'bz")

                // Build a the parsed string
                std::stringstream instances;
                std::stringstream module_block;
                std::stringstream net_block;
                int ctr = 0;
                std::vector<std::string> test_vector = {"4'hx", "4'hz", "4'bxzxz"};
                std::vector<std::vector<std::string>> result_vector = {
                        std::vector<std::string>{"1'bx", "1'bx", "1'bx", "1'bx"},
                        std::vector<std::string>{"1'bz", "1'bz", "1'bz", "1'bz"},
                        std::vector<std::string>{"1'bx", "1'bz", "1'bx", "1'bz"}};
                for (std::string num_literal : test_vector) {
                    // For every possible number literal (with length 4 bits), create a gate and a global output net
                    instances << "GATE1 gate_" << ctr << " (\n  .\\I ( " << num_literal << " ),\n  .\\O (global_out_" << ctr
                              << ")\n ) ;\n";
                    module_block << ",\n  global_out_" << ctr;
                    net_block << "  output global_out_" << ctr << " ;\n";
                    ctr++;
                }
                std::stringstream input;
                input << "module  (\n"
                      << "  global_in "
                      << module_block.str()
                      << " ) ;\n"
                      << "  input global_in ;\n"
                      << net_block.str()
                      << instances.str()
                      << "endmodule";


                test_def::capture_stdout();
                hdl_parser_verilog verilog_parser(input);
                std::shared_ptr<netlist> nl = verilog_parser.parse(temp_lib_name);
                if (nl == nullptr) {
                    std::cout << test_def::get_captured_stdout();
                } else {
                    test_def::get_captured_stdout();
                }

                ASSERT_NE(nl, nullptr);
                // Verify that every gate is connected correctly
                int i = 0;
                for (auto &res : result_vector) {

                    ASSERT_EQ(nl->get_gates("GATE1", "gate_" + std::to_string(i)).size(), 1);
                    std::shared_ptr<gate> gate_i = *nl->get_gates("GATE1", "gate_" + std::to_string(i)).begin();
                    for (int idx = 0; idx < 4; idx++) {
                        // Calculate if the pin should be connected to a gnd (0) or vcc (1) gate
                        //std::string bit = ((i >> (3-idx)) & 1) ? "1" : "0";
                        std::string pin = "I(" + std::to_string(idx) + ")";
                        ASSERT_NE(gate_i->get_fan_in_net(pin), nullptr);
                        EXPECT_EQ(gate_i->get_fan_in_net(pin)->get_name(), res[idx]);

                    }
                    i++;
                }

            }
        }
        // NOTE: Other types are in progress
    TEST_END
}

/**
 * Testing the assignments of multiple nets to multiple pins using curley brackets ('{net_0,...,net_4}')
 * or wire vectors (like:  'wire [1:4] net_vector' ... '.\I (net_vec )' or '.\I (net_vec[1:4])')
 *
 * Functions: parse
 */
TEST_F(hdl_parser_verilog_test, check_vector_assignment)
{
    TEST_START
        create_temp_gate_lib();
        {
            // Assing a vector of nets to a vector (via '{' and '}') of input pins
            std::stringstream input;
            input << "module  (\n"
                     "  global_in,\n"
                     "  global_out\n"
                     " ) ;\n"
                     "  input global_in ;\n"
                     "  output global_out ;\n"
                     "  wire net_0;\n"
                     "  wire net_1;\n"
                     "  wire net_2;\n"
                     "  wire net_3;\n"
                     "GATE1 gate_0 (\n"
                     "  .\\I ({net_0, net_1, net_2, net_3}),\n"
                     "  .\\O (global_out )\n"
                     " ) ;\n"
                     "endmodule";
            hdl_parser_verilog verilog_parser(input);
            std::shared_ptr<netlist> nl = verilog_parser.parse(temp_lib_name);
            ASSERT_NE(nl, nullptr);

            ASSERT_EQ(nl->get_gates(DONT_CARE, "gate_0").size(), 1);
            std::shared_ptr<gate> gate_0 = *nl->get_gates(DONT_CARE, "gate_0").begin();

            ASSERT_EQ(nl->get_nets("net_0").size(), 1);
            ASSERT_EQ(nl->get_nets("net_1").size(), 1);
            ASSERT_EQ(nl->get_nets("net_2").size(), 1);
            ASSERT_EQ(nl->get_nets("net_3").size(), 1);
            std::shared_ptr<net> net_0 = *nl->get_nets("net_0").begin();
            std::shared_ptr<net> net_1 = *nl->get_nets("net_1").begin();
            std::shared_ptr<net> net_2 = *nl->get_nets("net_2").begin();
            std::shared_ptr<net> net_3 = *nl->get_nets("net_3").begin();

            EXPECT_EQ(gate_0->get_fan_in_net("I(0)"), net_0);
            EXPECT_EQ(gate_0->get_fan_in_net("I(1)"), net_1);
            EXPECT_EQ(gate_0->get_fan_in_net("I(2)"), net_2);
            EXPECT_EQ(gate_0->get_fan_in_net("I(3)"), net_3);

        }
        {
            // Assing a vector of nets to a vector (i.e. net_vec[1:4]) of input pins (ISSUE: warning for bounds like [1:4]  (instead of [4:1]) is misleading ?)
            std::stringstream input;
            input << "module  (\n"
                     "  global_in,\n"
                     "  global_out\n"
                     " ) ;\n"
                     "  input global_in ;\n"
                     "  output global_out ;\n"
                     "  wire [1:4] net_vec ;\n"
                     "GATE1 gate_0 (\n"
                     "  .\\I (net_vec[1:4]),\n"
                     "  .\\O (global_out )\n"
                     " ) ;\n"
                     "endmodule";
            hdl_parser_verilog verilog_parser(input);
            std::shared_ptr<netlist> nl = verilog_parser.parse(temp_lib_name);
            ASSERT_NE(nl, nullptr);

            ASSERT_EQ(nl->get_gates(DONT_CARE, "gate_0").size(), 1);
            std::shared_ptr<gate> gate_0 = *nl->get_gates(DONT_CARE, "gate_0").begin();

            ASSERT_EQ(nl->get_nets("net_vec[1]").size(), 1);
            ASSERT_EQ(nl->get_nets("net_vec[2]").size(), 1);
            ASSERT_EQ(nl->get_nets("net_vec[3]").size(), 1);
            ASSERT_EQ(nl->get_nets("net_vec[4]").size(), 1);
            std::shared_ptr<net> net_vec_1 = *nl->get_nets("net_vec[1]").begin();
            std::shared_ptr<net> net_vec_2 = *nl->get_nets("net_vec[2]").begin();
            std::shared_ptr<net> net_vec_3 = *nl->get_nets("net_vec[3]").begin();
            std::shared_ptr<net> net_vec_4 = *nl->get_nets("net_vec[4]").begin();

            EXPECT_EQ(gate_0->get_fan_in_net("I(0)"), net_vec_1);
            EXPECT_EQ(gate_0->get_fan_in_net("I(1)"), net_vec_2);
            EXPECT_EQ(gate_0->get_fan_in_net("I(2)"), net_vec_3);
            EXPECT_EQ(gate_0->get_fan_in_net("I(3)"), net_vec_4);

        }
        {
            // Assing a vector of nets to a vector (i.e. net_vec[1:4]) of input pins (ISSUE: reverse order?)
            std::stringstream input;
            input << "module  (\n"
                     "  global_in,\n"
                     "  global_out\n"
                     " ) ;\n"
                     "  input global_in ;\n"
                     "  output global_out ;\n"
                     "  wire [4:1] net_vec ;\n"
                     "GATE1 gate_0 (\n"
                     "  .\\I (net_vec),\n"
                     "  .\\O (global_out )\n"
                     " ) ;\n"
                     "endmodule";
            hdl_parser_verilog verilog_parser(input);
            std::shared_ptr<netlist> nl = verilog_parser.parse(temp_lib_name);
            ASSERT_NE(nl, nullptr);

           ASSERT_EQ(nl->get_gates(DONT_CARE, "gate_0").size(), 1);
            std::shared_ptr<gate> gate_0 = *nl->get_gates(DONT_CARE, "gate_0").begin();

            ASSERT_EQ(nl->get_nets("net_vec[1]").size(), 1);
            ASSERT_EQ(nl->get_nets("net_vec[2]").size(), 1);
            ASSERT_EQ(nl->get_nets("net_vec[3]").size(), 1);
            ASSERT_EQ(nl->get_nets("net_vec[4]").size(), 1);
            std::shared_ptr<net> net_vec_1 = *nl->get_nets("net_vec[1]").begin();
            std::shared_ptr<net> net_vec_2 = *nl->get_nets("net_vec[2]").begin();
            std::shared_ptr<net> net_vec_3 = *nl->get_nets("net_vec[3]").begin();
            std::shared_ptr<net> net_vec_4 = *nl->get_nets("net_vec[4]").begin();

            /* Why not this order?
            EXPECT_EQ(gate_0->get_fan_in_net("I(0)"), net_vec_1);
            EXPECT_EQ(gate_0->get_fan_in_net("I(1)"), net_vec_2);
            EXPECT_EQ(gate_0->get_fan_in_net("I(2)"), net_vec_3);
            EXPECT_EQ(gate_0->get_fan_in_net("I(3)"), net_vec_4);
             */
            /* This will pass
            EXPECT_EQ(gate_0->get_fan_in_net("I(0)"), net_vec_4);
            EXPECT_EQ(gate_0->get_fan_in_net("I(1)"), net_vec_3);
            EXPECT_EQ(gate_0->get_fan_in_net("I(2)"), net_vec_2);
            EXPECT_EQ(gate_0->get_fan_in_net("I(3)"), net_vec_1);*/

        }

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
            std::stringstream input("module  (\n"
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
            ASSERT_EQ(nl->get_gates("VCC","gate_0").size(), 1);
            std::shared_ptr<gate> gate_0 = *nl->get_gates("VCC","gate_0").begin();
            EXPECT_FALSE(gate_0->is_global_gnd_gate());
            EXPECT_TRUE(gate_0->is_global_vcc_gate());
        }
        {
            // Testing the usage of a global GND gate (gate type 'GND' is global GND gate in our test gate library)
            std::stringstream input("module  (\n"
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
            ASSERT_EQ(nl->get_gates("GND","gate_0").size(), 1);
            std::shared_ptr<gate> gate_0 = *nl->get_gates("GND","gate_0").begin();
            EXPECT_TRUE(gate_0->is_global_gnd_gate());
            EXPECT_FALSE(gate_0->is_global_vcc_gate());
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
            input << "module  (\n"
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
            std::shared_ptr<netlist> nl = verilog_parser.parse(temp_lib_name);
            EXPECT_EQ(nl, nullptr);
        }
        {
            // MAYBE SOME MORE TESTS HERE LATER...
        }
    TEST_END
}




