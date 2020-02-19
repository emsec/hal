#include "netlist_test_utils.h"
#include "netlist/gate.h"
#include "netlist/gate_library/gate_library_manager.h"
#include "netlist/netlist.h"
#include "netlist/netlist_factory.h"
#include "netlist/persistent/netlist_serializer.h"
#include "gtest/gtest.h"
#include "netlist/hdl_parser/hdl_parser_vhdl.h"
#include <iostream>
#include <sstream>
#include <experimental/filesystem>


using namespace test_utils;

class hdl_parser_vhdl_test : public ::testing::Test
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
 *                  .--= INV (0) =----.
 *  global_in       |                   '-=                     global_out
 *      ------------|                   .-= AND3 (2) = ----------
 *                  |                   | =
 *                  '--=                |
 *                       AND2 (1) =---'
 *                     =              vec_net(2)
 *                                                              global_inout
 *      -----------------------------------------------------------
 *
 */

/**
 * Testing the correct usage of the vhdl parser by parse a small vhdl-format string, which describes the netlist
 * shown above.
 *
 * Functions: parse
 */
TEST_F(hdl_parser_vhdl_test, check_main_example)
{
    TEST_START
        std::stringstream input("-- Device\t: device_name\n"
                                "library IEEE;\n"
                                "use IEEE.STD_LOGIC_1164.ALL;\n"
                                "library SIMPRIM;\n"
                                "use SIMPRIM.VCOMPONENTS.ALL;\n"
                                "use SIMPRIM.VPKG.ALL;\n"
                                "\n"
                                "entity TEST_Comp is\n"
                                "  port (\n"
                                "    net_global_in : in STD_LOGIC := 'X';\n"
                                "    net_global_out : out STD_LOGIC := 'X';\n"
                                //"    net_global_input : in STD_LOGIC := 'X';\n"
                                "  );\n"
                                "end AES_Comp;\n"
                                "\n"
                                "architecture STRUCTURE of TEST_Comp is\n"
                                "  signal net_0 : STD_LOGIC;\n"
                                "  signal vec_net : STD_LOGIC_VECTOR ( 2 downto 0 );\n"
                                "  attribute PTYPE: IGNORE_ME;\n"
                                "begin\n"
                                "  gate_0 : INV\n"
                                "    port map (\n"
                                "      I => net_global_in,\n"
                                "      O => net_0\n"
                                "    );\n"
                                "  gate_1 : AND2\n"
                                "    port map (\n"
                                "      I0 => net_global_in,\n"
                                "      I1 => net_global_in,\n"
                                "      O => vec_net(2)\n"
                                "    );\n"
                                "  gate_2 : AND3\n"
                                "    port map (\n"
                                "      I0 => net_0,\n"
                                "      I1 => vec_net(2),\n"
                                "      O => net_global_out\n"
                                "    );\n"
                                "end STRUCTURE;\n"
                                "");
        test_def::capture_stdout();
        hdl_parser_vhdl vhdl_parser(input);
        std::shared_ptr<netlist> nl = vhdl_parser.parse(g_lib_name);
        if (nl == nullptr)
        {
            std::cout << test_def::get_captured_stdout();
        }
        else
        {
            test_def::get_captured_stdout();
        }

        ASSERT_NE(nl, nullptr);

        // Check if the device name is parsed correctly
        EXPECT_EQ(nl->get_design_name(), "TEST_Comp");

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
        std::shared_ptr<net> vec_net_2        = *(nl->get_nets(net_name_filter("vec_net(2)")).begin());
        std::shared_ptr<net> net_global_in    = *(nl->get_nets(net_name_filter("net_global_in")).begin());
        std::shared_ptr<net> net_global_out   = *(nl->get_nets(net_name_filter("net_global_out")).begin());

        ASSERT_NE(net_0, nullptr);
        EXPECT_EQ(net_0->get_name(), "net_0");
        EXPECT_EQ(net_0->get_src(), endpoint(gate_0, "O", false));
        std::vector<endpoint> exp_net_0_dsts = {endpoint(gate_2, "I0", true)};
        EXPECT_TRUE(vectors_have_same_content(net_0->get_dsts(), std::vector<endpoint>({endpoint(gate_2, "I0", true)})));

        ASSERT_NE(vec_net_2, nullptr);
        EXPECT_EQ(vec_net_2->get_name(), "vec_net(2)");
        EXPECT_EQ(vec_net_2->get_src(), endpoint(gate_1, "O", false));
        EXPECT_TRUE(vectors_have_same_content(vec_net_2->get_dsts(), std::vector<endpoint>({endpoint(gate_2, "I1", true)})));

        ASSERT_NE(net_global_in, nullptr);
        EXPECT_EQ(net_global_in->get_name(), "net_global_in");
        EXPECT_EQ(net_global_in->get_src(), endpoint(nullptr, "", false));
        EXPECT_TRUE(vectors_have_same_content(net_global_in->get_dsts(), std::vector<endpoint>({endpoint(gate_0, "I", true), endpoint(gate_1, "I0", true), endpoint(gate_1, "I1", true)})));
        EXPECT_TRUE(nl->is_global_input_net(net_global_in));

        ASSERT_NE(net_global_out, nullptr);
        EXPECT_EQ(net_global_out->get_name(), "net_global_out");
        EXPECT_EQ(net_global_out->get_src(), endpoint(gate_2, "O", false));
        EXPECT_TRUE(net_global_out->get_dsts().empty());
        EXPECT_TRUE(nl->is_global_output_net(net_global_out));

        EXPECT_EQ(nl->get_global_input_nets().size(), 1);
        EXPECT_EQ(nl->get_global_output_nets().size(), 1);

    TEST_END
}

/**
 * Testing the correct storage of data, passed by the "generic map" keyword
 *
 * Functions: parse
 */
TEST_F(hdl_parser_vhdl_test, check_generic_map){
    TEST_START
        {
            // A boolean value is passed
            std::stringstream input("-- Device\t: device_name\n"
                                    "entity TEST_Comp is\n"
                                    "  port (\n"
                                    "    net_global_input : in STD_LOGIC := 'X';\n"
                                    "  );\n"
                                    "end TEST_Comp;\n"
                                    "architecture STRUCTURE of TEST_Comp is\n"
                                    "begin\n"
                                    "  gate_0 : INV\n"
                                    "    generic map(\n"
                                    "      key_bool => true\n"
                                    "    )\n"
                                    "    port map (\n"
                                    "      I => net_global_input\n"
                                    "    );\n"
                                    "end STRUCTURE;");
            test_def::capture_stdout();
            hdl_parser_vhdl vhdl_parser(input);
            std::shared_ptr<netlist> nl = vhdl_parser.parse(g_lib_name);
            if (nl == nullptr)
            {
                std::cout << test_def::get_captured_stdout();
            }
            else
            {
                test_def::get_captured_stdout();
            }

            ASSERT_NE(nl, nullptr);
            ASSERT_NE(nl->get_gates(gate_type_filter("INV")).size(), 0);
            std::shared_ptr<gate> g = *nl->get_gates(gate_type_filter("INV")).begin();
            EXPECT_EQ(g->get_data_by_key("generic", "key_bool"), std::make_tuple("boolean", "true"));
        }
        {
            // An integer is passed
            std::stringstream input("-- Device\t: device_name\n"
                                    "entity TEST_Comp is\n"
                                    "  port (\n"
                                    "    net_global_input : in STD_LOGIC := 'X';\n"
                                    "  );\n"
                                    "end TEST_Comp;\n"
                                    "architecture STRUCTURE of TEST_Comp is\n"
                                    "begin\n"
                                    "  gate_0 : INV\n"
                                    "    generic map(\n"
                                    "      key_integer => 123\n"
                                    "    )\n"
                                    "    port map (\n"
                                    "      I => net_global_input\n"
                                    "    );\n"
                                    "end STRUCTURE;");
            test_def::capture_stdout();
            hdl_parser_vhdl vhdl_parser(input);
            std::shared_ptr<netlist> nl = vhdl_parser.parse(g_lib_name);
            if (nl == nullptr)
            {
                std::cout << test_def::get_captured_stdout();
            }
            else
            {
                test_def::get_captured_stdout();
            }

            ASSERT_NE(nl, nullptr);
            ASSERT_NE(nl->get_gates(gate_type_filter("INV")).size(), 0);
            std::shared_ptr<gate> g = *nl->get_gates(gate_type_filter("INV")).begin();
            EXPECT_EQ(g->get_data_by_key("generic", "key_integer"), std::make_tuple("integer", "123"));
        }
        {
            // A floating point number is passed
            std::stringstream input("-- Device\t: device_name\n"
                                    "entity TEST_Comp is\n"
                                    "  port (\n"
                                    "    net_global_input : in STD_LOGIC := 'X';\n"
                                    "  );\n"
                                    "end TEST_Comp;\n"
                                    "architecture STRUCTURE of TEST_Comp is\n"
                                    "begin\n"
                                    "  gate_0 : INV\n"
                                    "    generic map(\n"
                                    "      key_floating_point => 1.23\n"
                                    "    )\n"
                                    "    port map (\n"
                                    "      I => net_global_input\n"
                                    "    );\n"
                                    "end STRUCTURE;");
            test_def::capture_stdout();
            hdl_parser_vhdl vhdl_parser(input);
            std::shared_ptr<netlist> nl = vhdl_parser.parse(g_lib_name);
            if (nl == nullptr)
            {
                std::cout << test_def::get_captured_stdout();
            }
            else
            {
                test_def::get_captured_stdout();
            }

            ASSERT_NE(nl, nullptr);
            ASSERT_NE(nl->get_gates(gate_type_filter("INV")).size(), 0);
            std::shared_ptr<gate> g = *nl->get_gates(gate_type_filter("INV")).begin();
            EXPECT_EQ(g->get_data_by_key("generic", "key_floating_point"), std::make_tuple("floating_point", "1.23"));
        }
        {
            // A time is passed
            std::stringstream input("-- Device\t: device_name\n"
                                    "entity TEST_Comp is\n"
                                    "  port (\n"
                                    "    net_global_input : in STD_LOGIC := 'X';\n"
                                    "  );\n"
                                    "end TEST_Comp;\n"
                                    "architecture STRUCTURE of TEST_Comp is\n"
                                    "begin\n"
                                    "  gate_0 : INV\n"
                                    "    generic map(\n"
                                    "      key_time => 1.23sec\n"
                                    "    )\n"
                                    "    port map (\n"
                                    "      I => net_global_input\n"
                                    "    );\n"
                                    "end STRUCTURE;");
            test_def::capture_stdout();
            hdl_parser_vhdl vhdl_parser(input);
            std::shared_ptr<netlist> nl = vhdl_parser.parse(g_lib_name);
            if (nl == nullptr)
            {
                std::cout << test_def::get_captured_stdout();
            }
            else
            {
                test_def::get_captured_stdout();
            }

            ASSERT_NE(nl, nullptr);
            ASSERT_NE(nl->get_gates(gate_type_filter("INV")).size(), 0);
            std::shared_ptr<gate> g = *nl->get_gates(gate_type_filter("INV")).begin();
            EXPECT_EQ(g->get_data_by_key("generic", "key_time"), std::make_tuple("time", "1.23sec"));
        }
        {
            // A bit-vector is passed via ' '
            std::stringstream input("-- Device\t: device_name\n"
                                    "entity TEST_Comp is\n"
                                    "  port (\n"
                                    "    net_global_input : in STD_LOGIC := 'X';\n"
                                    "  );\n"
                                    "end TEST_Comp;\n"
                                    "architecture STRUCTURE of TEST_Comp is\n"
                                    "begin\n"
                                    "  gate_0 : INV\n"
                                    "    generic map(\n"
                                    "      key_bit_vector => B\"0000_1111_0000\"\n"
                                    "    )\n"
                                    "    port map (\n"
                                    "      I => net_global_input\n"
                                    "    );\n"
                                    "end STRUCTURE;");
            test_def::capture_stdout();
            hdl_parser_vhdl vhdl_parser(input);
            std::shared_ptr<netlist> nl = vhdl_parser.parse(g_lib_name);
            if (nl == nullptr)
            {
                std::cout << test_def::get_captured_stdout();
            }
            else
            {
                test_def::get_captured_stdout();
            }

            ASSERT_NE(nl, nullptr);
            ASSERT_NE(nl->get_gates(gate_type_filter("INV")).size(), 0);
            std::shared_ptr<gate> g = *nl->get_gates(gate_type_filter("INV")).begin();
            EXPECT_EQ(g->get_data_by_key("generic", "key_bit_vector"), std::make_tuple("bit_vector", "f0"));
        }
        {
            // A bit-vector is passed via X" " and B" " and O" "
            std::stringstream input("-- Device\t: device_name\n"
                                    "entity TEST_Comp is\n"
                                    "  port (\n"
                                    "    net_global_input : in STD_LOGIC := 'X';\n"
                                    "  );\n"
                                    "end TEST_Comp;\n"
                                    "architecture STRUCTURE of TEST_Comp is\n"
                                    "begin\n"
                                    "  gate_0 : INV\n"
                                    "    generic map(\n"
                                    "      key_bit_vector_0 => X\"abcdef\",\n"
                                    "      key_bit_vector_1 => B\"101010111100110111101111\",\n" // <- binary: 'abcdef' in hex
                                    "      key_bit_vector_2 => O\"52746757\",\n" // <- octal: 'abcdef' in hex
                                    "      key_bit_vector_3 => D\"11259375\"\n" // <- decimal: 'abcdef' in hex
                                    "    )\n"
                                    "    port map (\n"
                                    "      I => net_global_input\n"
                                    "    );\n"
                                    "end STRUCTURE;");
            test_def::capture_stdout();
            hdl_parser_vhdl vhdl_parser(input);
            std::shared_ptr<netlist> nl = vhdl_parser.parse(g_lib_name);
            if (nl == nullptr)
            {
                std::cout << test_def::get_captured_stdout();
            }
            else
            {
                test_def::get_captured_stdout();
            }

            ASSERT_NE(nl, nullptr);
            ASSERT_NE(nl->get_gates(gate_type_filter("INV")).size(), 0);
            std::shared_ptr<gate> g = *nl->get_gates(gate_type_filter("INV")).begin();
            EXPECT_EQ(g->get_data_by_key("generic", "key_bit_vector_0"), std::make_tuple("bit_vector", "abcdef"));
            EXPECT_EQ(g->get_data_by_key("generic", "key_bit_vector_1"), std::make_tuple("bit_vector", "abcdef"));
            EXPECT_EQ(g->get_data_by_key("generic", "key_bit_vector_2"), std::make_tuple("bit_vector", "abcdef"));
            EXPECT_EQ(g->get_data_by_key("generic", "key_bit_vector_3"), std::make_tuple("bit_vector", "abcdef"));
        }
        {
            // A string is passed
            std::stringstream input("-- Device\t: device_name\n"
                                    "entity TEST_Comp is\n"
                                    "  port (\n"
                                    "    net_global_input : in STD_LOGIC := 'X';\n"
                                    "  );\n"
                                    "end TEST_Comp;\n"
                                    "architecture STRUCTURE of TEST_Comp is\n"
                                    "begin\n"
                                    "  gate_0 : INV\n"
                                    "    generic map(\n"
                                    "      key_string => \"one_two_three\"\n"
                                    "    )\n"
                                    "    port map (\n"
                                    "      I => net_global_input\n"
                                    "    );\n"
                                    "end STRUCTURE;");
            test_def::capture_stdout();
            hdl_parser_vhdl vhdl_parser(input);
            std::shared_ptr<netlist> nl = vhdl_parser.parse(g_lib_name);
            if (nl == nullptr)
            {
                std::cout << test_def::get_captured_stdout();
            }
            else
            {
                test_def::get_captured_stdout();
            }

            ASSERT_NE(nl, nullptr);
            ASSERT_NE(nl->get_gates(gate_type_filter("INV")).size(), 0);
            std::shared_ptr<gate> g = *nl->get_gates(gate_type_filter("INV")).begin();
            EXPECT_EQ(g->get_data_by_key("generic", "key_string"), std::make_tuple("string", "one_two_three"));
        }
        {
            // A bit-value is passed
            std::stringstream input("-- Device\t: device_name\n"
                                    "entity TEST_Comp is\n"
                                    "  port (\n"
                                    "    net_global_input : in STD_LOGIC := 'X';\n"
                                    "  );\n"
                                    "end TEST_Comp;\n"
                                    "architecture STRUCTURE of TEST_Comp is\n"
                                    "begin\n"
                                    "  gate_0 : INV\n"
                                    "    generic map(\n"
                                    "      key_bit_value => \'11001010\'\n"
                                    "    )\n"
                                    "    port map (\n"
                                    "      I => net_global_input\n"
                                    "    );\n"
                                    "end STRUCTURE;");
            test_def::capture_stdout();
            hdl_parser_vhdl vhdl_parser(input);
            std::shared_ptr<netlist> nl = vhdl_parser.parse(g_lib_name);
            if (nl == nullptr)
            {
                std::cout << test_def::get_captured_stdout();
            }
            else
            {
                test_def::get_captured_stdout();
            }

            ASSERT_NE(nl, nullptr);
            ASSERT_NE(nl->get_gates(gate_type_filter("INV")).size(), 0);
            std::shared_ptr<gate> g = *nl->get_gates(gate_type_filter("INV")).begin();
            EXPECT_EQ(g->get_data_by_key("generic", "key_bit_value"), std::make_tuple("bit_value", "11001010"));
        }


    TEST_END
}

/**
 * Testing the implicit addition of global gnd and vcc gates by using '0' and '1'
 *
 * Functions: parse
 */
TEST_F(hdl_parser_vhdl_test, check_global_gates_implicit){
    TEST_START
        {
            // Add a global_gnd implicitly by using '0'
            std::stringstream input("-- Device\t: device_name\n"
                                    "entity TEST_Comp is\n"
                                    "  port (\n"
                                    "    net_global_input : in STD_LOGIC := 'X';\n"
                                    "  );\n"
                                    "end TEST_Comp;\n"
                                    "architecture STRUCTURE of TEST_Comp is\n"
                                    "begin\n"
                                    "  gate_0 : INV\n"
                                    "    port map (\n"
                                    "      I => '0'\n"
                                    "    );\n"
                                    "end STRUCTURE;");
            test_def::capture_stdout();
            hdl_parser_vhdl vhdl_parser(input);
            std::shared_ptr<netlist> nl = vhdl_parser.parse(g_lib_name);
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

            ASSERT_NE(nl->get_gates(gate_type_filter("INV")).size(), 0);
            std::shared_ptr<gate> g = *nl->get_gates(gate_type_filter("INV")).begin();

            ASSERT_NE(g->get_predecessors().size(), 0);
            endpoint pred = *g->get_predecessors().begin();
            EXPECT_EQ(pred.get_gate(), global_gnd);
        }
        {
            // Add a global_vcc implicitly by using '1'
            std::stringstream input("-- Device\t: device_name\n"
                                    "entity TEST_Comp is\n"
                                    "  port (\n"
                                    "    net_global_input : in STD_LOGIC := 'X';\n"
                                    "  );\n"
                                    "end TEST_Comp;\n"
                                    "architecture STRUCTURE of TEST_Comp is\n"
                                    "begin\n"
                                    "  gate_0 : INV\n"
                                    "    port map (\n"
                                    "      I => '1'\n"
                                    "    );\n"
                                    "end STRUCTURE;");
            test_def::capture_stdout();
            hdl_parser_vhdl vhdl_parser(input);
            std::shared_ptr<netlist> nl = vhdl_parser.parse(g_lib_name);
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

            ASSERT_NE(nl->get_gates(gate_type_filter("INV")).size(), 0);
            std::shared_ptr<gate> g = *nl->get_gates(gate_type_filter("INV")).begin();

            ASSERT_NE(g->get_predecessors().size(), 0);
            endpoint pred = *g->get_predecessors().begin();
            EXPECT_EQ(pred.get_gate(), global_vcc);
        }
    TEST_END
}

/**
 * Testing the explicit addition of a global gnd and vcc gates
 *
 * Functions: parse
 */
TEST_F(hdl_parser_vhdl_test, check_global_gates_explicit){
    TEST_START
        {
            // Add a global_gnd explicitly
            std::stringstream input("-- Device\t: device_name\n"
                                    "entity TEST_Comp is\n"
                                    "  port (\n"
                                    "    net_global_input : in STD_LOGIC := 'X';\n"
                                    "  );\n"
                                    "end TEST_Comp;\n"
                                    "architecture STRUCTURE of TEST_Comp is\n"
                                    "begin\n"
                                    "  g_gnd_gate : GND\n"
                                    "    port map (\n"
                                    "      O => net_global_input\n"
                                    "    );\n"
                                    "end STRUCTURE;");
            test_def::capture_stdout();
            hdl_parser_vhdl vhdl_parser(input);
            std::shared_ptr<netlist> nl = vhdl_parser.parse(g_lib_name);
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
            // Add a global_vcc explicitly
            std::stringstream input("-- Device\t: device_name\n"
                                    "entity TEST_Comp is\n"
                                    "  port (\n"
                                    "    net_global_input : in STD_LOGIC := 'X';\n"
                                    "  );\n"
                                    "end TEST_Comp;\n"
                                    "architecture STRUCTURE of TEST_Comp is\n"
                                    "begin\n"
                                    "  g_vcc_gate : VCC\n"
                                    "    port map (\n"
                                    "      O => net_global_input\n"
                                    "    );\n"
                                    "end STRUCTURE;");
            test_def::capture_stdout();
            hdl_parser_vhdl vhdl_parser(input);
            std::shared_ptr<netlist> nl = vhdl_parser.parse(g_lib_name);
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
 * Testing the usage and the correct handling of library prefixes
 *
 * Functions: parse
 */
TEST_F(hdl_parser_vhdl_test, check_lib_prefix)
{
    TEST_START
        // The prefix of the library SIMPRIM.VCOMPONENTS should be removed from the INV gate type
        std::stringstream input("-- Device\t: device_name\n"
                                "library SIMPRIM;\n"
                                "use SIMPRIM.VCOMPONENTS.ALL;\n"
                                "entity TEST_Comp is\n"
                                "  port (\n"
                                "    net_global_input : in STD_LOGIC := 'X';\n"
                                "  );\n"
                                "end TEST_Comp;\n"
                                "architecture STRUCTURE of TEST_Comp is\n"
                                "begin\n"
                                "  gate_0 : SIMPRIM.VCOMPONENTS.INV\n"
                                "    port map (\n"
                                "      I => net_global_input\n"
                                "    );\n"
                                "end STRUCTURE;");
        test_def::capture_stdout();
        hdl_parser_vhdl vhdl_parser(input);
        std::shared_ptr<netlist> nl = vhdl_parser.parse(g_lib_name);
        if (nl == nullptr)
        {
            std::cout << test_def::get_captured_stdout();
        }
        else
        {
            test_def::get_captured_stdout();
        }

        ASSERT_NE(nl, nullptr);

        EXPECT_EQ(nl->get_gates(gate_type_filter("INV")).size(), 1);
    TEST_END
}

/**
 * Testing the handling of logic-vectors in dimension 1-3
 *
 * Functions: parse
 */
TEST_F(hdl_parser_vhdl_test, check_logic_vectors)
{
    TEST_START
        create_temp_gate_lib();
        {
            // Use two logic vectors with dimension 1. One uses the 'downto' the other the 'to' statement
            /*
             *                           l_vec_1              l_vec_2
             *                        =-----------=        =-----------=
             *                        =-----------=        =-----------=
             *  global_in ---= gate_0 =-----------= gate_1 =-----------= gate_2 =--- global_out
             *                        =-----------=        =-----------=
             *
             */
            std::stringstream input("-- Device\t: device_name\n"
                                    "entity TEST_Comp is\n"
                                    "  port (\n"
                                    "    net_global_in : in STD_LOGIC := 'X';\n"
                                    "    net_global_out : out STD_LOGIC := 'X';\n"
                                    "  );\n"
                                    "end TEST_Comp;\n"
                                    "architecture STRUCTURE of TEST_Comp is\n"
                                    "  signal l_vec_1 : STD_LOGIC_VECTOR ( 3 downto 0 );\n"
                                    "  signal l_vec_2 : STD_LOGIC_VECTOR ( 0 to 3 );\n"
                                    "begin\n"
                                    "  gate_0 : GATE_1^0_IN_4^1_OUT\n"
                                    "    port map (\n"
                                    "      I => net_global_in,\n"
                                    "      O(0) => l_vec_1(0),\n"
                                    "      O(1) => l_vec_1(1),\n"
                                    "      O(2) => l_vec_1(2),\n"
                                    "      O(3) => l_vec_1(3)\n"
                                    "    );\n"
                                    "  gate_1 : GATE_4^1_IN_4^1_OUT\n"
                                    "    port map (\n"
                                    "      I(0) => l_vec_1(0),\n"
                                    "      I(1) => l_vec_1(1),\n"
                                    "      I(2) => l_vec_1(2),\n"
                                    "      I(3) => l_vec_1(3),\n"
                                    "      O(0) => l_vec_2(0),\n"
                                    "      O(1) => l_vec_2(1),\n"
                                    "      O(2) => l_vec_2(2),\n"
                                    "      O(3) => l_vec_2(3)\n"
                                    "    );\n"
                                    "  gate_2 : GATE_4^1_IN_1^0_OUT\n"
                                    "    port map (\n"
                                    "      I(0) => l_vec_2(0),\n"
                                    "      I(1) => l_vec_2(1),\n"
                                    "      I(2) => l_vec_2(2),\n"
                                    "      I(3) => l_vec_2(3),\n"
                                    "      O => net_global_out\n"
                                    "    );\n"
                                    "end STRUCTURE;");
            //std::cout << "\n=====\n" << input.str() << "\n=====\n";
            test_def::capture_stdout();
            hdl_parser_vhdl vhdl_parser(input);
            std::shared_ptr<netlist> nl = vhdl_parser.parse(temp_lib_name);
            if (nl == nullptr)
            {
                std::cout << test_def::get_captured_stdout();
            }
            else
            {
                test_def::get_captured_stdout();
            }

            ASSERT_NE(nl, nullptr);
            EXPECT_EQ(nl->get_nets().size(), 10); // net_global_in + net_global_out + 4 nets in l_vec_1 + 4 nets in l_vec_2
            for(auto net_name : std::set<std::string>({"l_vec_1(0)","l_vec_1(1)","l_vec_1(2)","l_vec_1(3)","l_vec_2(0)","l_vec_2(1)","l_vec_2(2)","l_vec_2(3)"})){
                EXPECT_FALSE(nl->get_nets(net_name_filter(net_name)).empty());
            }
        }
        {
            // Use a logic vector of dimension two ISSUE: Very strict regarding the usage of Spaces (i.e: l_vec(0,2) isn't allowed, while l_vec(0, 2) is...)
            std::stringstream input("-- Device\t: device_name\n"
                                    "entity TEST_Comp is\n"
                                    "  port (\n"
                                    "    net_global_in : in STD_LOGIC := 'X';\n"
                                    "    net_global_out : out STD_LOGIC := 'X';\n"
                                    "  );\n"
                                    "end TEST_Comp;\n"
                                    "architecture STRUCTURE of TEST_Comp is\n"
                                    "  signal l_vec : STD_LOGIC_VECTOR2 ( 0 to 1, 2 to 3 );\n"
                                    "begin\n"
                                    "  gate_0 : GATE_1^0_IN_4^1_OUT\n"
                                    "    port map (\n"
                                    "      I => net_global_in,\n"
                                    "      O(0) => l_vec(0, 2),\n"
                                    "      O(1) => l_vec(0, 3),\n"
                                    "      O(2) => l_vec(1, 2),\n"
                                    "      O(3) => l_vec(1, 3)\n"
                                    "    );\n"
                                    "  gate_1 : GATE_4^1_IN_1^0_OUT\n"
                                    "    port map (\n"
                                    "      I(0) => l_vec(0, 2),\n"
                                    "      I(1) => l_vec(0, 3),\n"
                                    "      I(2) => l_vec(1, 2),\n"
                                    "      I(3) => l_vec(1, 3),\n"
                                    "      O => net_global_out\n"
                                    "    );\n"
                                    "end STRUCTURE;");
            test_def::capture_stdout();
            hdl_parser_vhdl vhdl_parser(input);
            std::shared_ptr<netlist> nl = vhdl_parser.parse(temp_lib_name);
            if (nl == nullptr)
            {
                std::cout << test_def::get_captured_stdout();
            }
            else
            {
                test_def::get_captured_stdout();
            }

            ASSERT_NE(nl, nullptr);
            EXPECT_EQ(nl->get_nets().size(), 6); // net_global_in + global_out + 4 nets in l_vec
            for(auto net_name : std::set<std::string>({"l_vec(0,2)","l_vec(0,3)","l_vec(1,2)","l_vec(1,3)"})){
                EXPECT_FALSE(nl->get_nets(net_name_filter(net_name)).empty());
            }


        }
        {
            // Use a logic vector of dimension three
            std::stringstream input("-- Device\t: device_name\n"
                                    "entity TEST_Comp is\n"
                                    "  port (\n"
                                    "    net_global_in : in STD_LOGIC := 'X';\n"
                                    "    net_global_out : out STD_LOGIC := 'X';\n"
                                    "  );\n"
                                    "end TEST_Comp;\n"
                                    "architecture STRUCTURE of TEST_Comp is\n"
                                    "  signal l_vec : STD_LOGIC_VECTOR3 ( 0 to 1, 1 downto 0, 0 to 1 );\n"
                                    "begin\n"
                                    "  gate_0 : GATE_1^0_IN_2^3_OUT\n"
                                    "    port map (\n"
                                    "      I => net_global_in,\n"
                                    "      O(0, 0, 0) => l_vec(0, 0, 0),\n"
                                    "      O(0, 0, 1) => l_vec(0, 0, 1),\n"
                                    "      O(0, 1, 0) => l_vec(0, 1, 0),\n"
                                    "      O(0, 1, 1) => l_vec(0, 1, 1),\n"
                                    "      O(1, 0, 0) => l_vec(1, 0, 0),\n"
                                    "      O(1, 0, 1) => l_vec(1, 0, 1),\n"
                                    "      O(1, 1, 0) => l_vec(1, 1, 0),\n"
                                    "      O(1, 1, 1) => l_vec(1, 1, 1)\n"
                                    "    );\n"
                                    "  gate_1 : GATE_2^3_IN_1^0_OUT\n"
                                    "    port map (\n"
                                    "      I(0, 0, 0) => l_vec(0, 0, 0),\n"
                                    "      I(0, 0, 1) => l_vec(0, 0, 1),\n"
                                    "      I(0, 1, 0) => l_vec(0, 1, 0),\n"
                                    "      I(0, 1, 1) => l_vec(0, 1, 1),\n"
                                    "      I(1, 0, 0) => l_vec(1, 0, 0),\n"
                                    "      I(1, 0, 1) => l_vec(1, 0, 1),\n"
                                    "      I(1, 1, 0) => l_vec(1, 1, 0),\n"
                                    "      I(1, 1, 1) => l_vec(1, 1, 1),\n"
                                    "      O => net_global_out\n"
                                    "    );\n"
                                    "end STRUCTURE;");
            test_def::capture_stdout();
            hdl_parser_vhdl vhdl_parser(input);
            std::shared_ptr<netlist> nl = vhdl_parser.parse(temp_lib_name);
            if (nl == nullptr)
            {
                std::cout << test_def::get_captured_stdout();
            }
            else
            {
                test_def::get_captured_stdout();
            }
            ASSERT_NE(nl, nullptr);
            EXPECT_EQ(nl->get_nets().size(), 10); // net_global_in + net_global_out + 8 nets in l_vec
            for(auto net_name : std::set<std::string>({"l_vec(0,0,0)","l_vec(0,0,1)","l_vec(0,1,0)","l_vec(0,1,1)",
                                                       "l_vec(1,0,0)","l_vec(1,0,1)","l_vec(1,1,0)","l_vec(1,1,1)"})){
                EXPECT_FALSE(nl->get_nets(net_name_filter(net_name)).empty());
            }

        }
        // NEGATIVE
        {
            // The amount of bounds does not match with the vector dimension
            NO_COUT_TEST_BLOCK;
            std::stringstream input("-- Device\t: device_name\n"
                                    "entity TEST_Comp is\n"
                                    "  port (\n"
                                    "    net_global_input : in STD_LOGIC := 'X';\n"
                                    "  );\n"
                                    "end TEST_Comp;\n"
                                    "architecture STRUCTURE of TEST_Comp is\n"
                                    "  signal l_vec : STD_LOGIC_VECTOR3 ( 0 to 1, 0 to 1);\n" // <- two bounds, but dimension 3
                                    "begin\n"
                                    "  gate_0 : INV\n"
                                    "    port map (\n"
                                    "      O => net_global_input\n"
                                    "    );\n"
                                    "end STRUCTURE;");
            hdl_parser_vhdl vhdl_parser(input);
            std::shared_ptr<netlist> nl = vhdl_parser.parse(g_lib_name);
            if(nl != nullptr){
                EXPECT_EQ(nl->get_nets().size(), 1);
            }
        }
        remove_temp_gate_lib();
    TEST_END
}

/**
 * Testing the different ways of port assignments
 *
 * Functions: parse
 */
TEST_F(hdl_parser_vhdl_test, check_port_assignment) {
    TEST_START
        // We need to create another gate library with multiple output ports
        create_temp_gate_lib();
        {
            // Connect a single set with a single output pin
            std::stringstream input("-- Device\t: device_name\n"
                                    "entity TEST_Comp is\n"
                                    "  port (\n"
                                    "  );\n"
                                    "end TEST_Comp;\n"
                                    "architecture STRUCTURE of TEST_Comp is\n"
                                    "  signal net_0 : STD_LOGIC;\n"
                                    "  signal net_1 : STD_LOGIC;\n"
                                    "begin\n"
                                    "  gate_0 : GATE_4^1_IN_4^1_OUT\n"
                                    "    port map (\n"
                                    "      O(0) => net_0,\n"
                                    "      O(1) => net_1\n"
                                    "    );\n"
                                    "end STRUCTURE;");
            test_def::capture_stdout();
            hdl_parser_vhdl vhdl_parser(input);
            std::shared_ptr<netlist> nl = vhdl_parser.parse(temp_lib_name);
            if (nl == nullptr)
            {
                std::cout << test_def::get_captured_stdout();
            }
            else
            {
                test_def::get_captured_stdout();
            }

            ASSERT_NE(nl, nullptr);
            ASSERT_FALSE(nl->get_gates(gate_filter("GATE_4^1_IN_4^1_OUT", "gate_0")).empty());
            std::shared_ptr<gate> gate_0 = *(nl->get_gates(gate_filter("GATE_4^1_IN_4^1_OUT" ,"gate_0")).begin());

            std::shared_ptr<net> net_0 = gate_0->get_fan_out_net("O(0)");
            ASSERT_NE(net_0, nullptr);
            EXPECT_EQ(net_0->get_name(), "net_0");

            std::shared_ptr<net> net_1 = gate_0->get_fan_out_net("O(1)");
            ASSERT_NE(net_1, nullptr);
            EXPECT_EQ(net_1->get_name(), "net_1");
        }
        /*{
            // Connect two dimensional ports
            std::stringstream input("-- Device\t: device_name\n"
                                    "entity TEST_Comp is\n"
                                    "  port (\n"
                                    "    net_0 : in STD_LOGIC := 'X';\n"
                                    "  );\n"
                                    "end TEST_Comp;\n"
                                    "architecture STRUCTURE of TEST_Comp is\n"
                                    "  signal net_0 : STD_LOGIC;\n"
                                    "begin\n"
                                    "  gate_0 : GATE_2^2_IN_2^2_OUT\n"
                                    "    port map (\n"
                                    "      I(0,1) => net_0\n" // <- connect I(0,1) to net_0
                                    "    );\n"
                                    "end STRUCTURE;");
            //test_def::capture_stdout();
            hdl_parser_vhdl vhdl_parser(input);
            std::shared_ptr<netlist> nl = vhdl_parser.parse(temp_lib_name);
            if (nl == nullptr) {
                //std::cout << test_def::get_captured_stdout();
            } else {
                //test_def::get_captured_stdout();
            }

            ASSERT_NE(nl, nullptr);
            ASSERT_FALSE(nl->get_gates(gate_filter("GATE_2^2_IN_2^2_OUT", "gate_0")).empty());
            std::shared_ptr<gate> gate_0 = *(nl->get_gates(gate_filter("GATE_2^2_IN_2^2_OUT", "gate_0")).begin());

            std::shared_ptr<net> net_i_0 = gate_0->get_fan_in_net("I(0,1)");
            ASSERT_NE(net_i_0, nullptr);
            EXPECT_EQ(net_i_0->get_name(), "net_0");
        }*/
        /*{
            // Connect a vector of output pins with global input nets by using a binary string (B"10101010")
            std::stringstream input("-- Device\t: device_name\n"
                                    "entity TEST_Comp is\n"
                                    "  port (\n"
                                    "  );\n"
                                    "end TEST_Comp;\n"
                                    "architecture STRUCTURE of TEST_Comp is\n"
                                    "begin\n"
                                    "  gate_0 : GATE_4^1_IN_4^1_OUT\n"
                                    "    port map (\n"
                                    "      I(1 to 3) => B\"101\"\n"
                                    "    );\n"
                                    "end STRUCTURE;");
            test_def::capture_stdout();
            hdl_parser_vhdl vhdl_parser(input);
            std::shared_ptr<netlist> nl = vhdl_parser.parse(temp_lib_name);
            if (nl == nullptr)
            {
                std::cout << test_def::get_captured_stdout();
            }
            else
            {
                test_def::get_captured_stdout();
            }

            ASSERT_NE(nl, nullptr);
            ASSERT_FALSE(nl->get_gates(gate_filter("GATE_4^1_IN_4^1_OUT", "gate_0")).empty());
            std::shared_ptr<gate> gate_0 = *(nl->get_gates(gate_filter("GATE_4^1_IN_4^1_OUT" ,"gate_0")).begin());

            EXPECT_EQ(gate_0->get_fan_in_nets().size(), 2);

            std::shared_ptr<net> net_0 = gate_0->get_fan_in_net("I(1)");
            ASSERT_NE(net_0, nullptr);
            EXPECT_EQ(net_0->get_name(), "'1'");

            std::shared_ptr<net> net_1 = gate_0->get_fan_in_net("I(2)");
            ASSERT_NE(net_1, nullptr);
            EXPECT_EQ(net_1->get_name(), "'0'");

            std::shared_ptr<net> net_2 = gate_0->get_fan_in_net("I(3)");
            ASSERT_NE(net_2, nullptr);
            EXPECT_EQ(net_2->get_name(), "'1'");
        }*/
        /*{
            // Connect a vector of output pins with a vector of nets (O(1)=>l_vec(0), O(2)=>l_vec(1), O(3)=>l_vec(2))
            std::stringstream input("-- Device\t: device_name\n"
                                    "entity TEST_Comp is\n"
                                    "  port (\n"
                                    "  );\n"
                                    "end TEST_Comp;\n"
                                    "architecture STRUCTURE of TEST_Comp is\n"
                                    "  signal l_vec : STD_LOGIC_VECTOR ( 0 to 3 );\n"
                                    "begin\n"
                                    "  gate_0 : GATE_4^1_IN_4^1_OUT\n"
                                    "    port map (\n"
                                    "      O(1 to 3) => l_vec(0 to 2)\n"
                                    "    );\n"
                                    "end STRUCTURE;");
            test_def::capture_stdout();
            hdl_parser_vhdl vhdl_parser(input);
            std::shared_ptr<netlist> nl = vhdl_parser.parse(temp_lib_name);
            if (nl == nullptr)
            {
                std::cout << test_def::get_captured_stdout();
            }
            else
            {
                test_def::get_captured_stdout();
            }

            ASSERT_NE(nl, nullptr);
            ASSERT_FALSE(nl->get_gates(gate_filter("GATE_4^1_IN_4^1_OUT", "gate_0")).empty());
            std::shared_ptr<gate> gate_0 = *(nl->get_gates(gate_filter("GATE_4^1_IN_4^1_OUT" ,"gate_0")).begin());

            EXPECT_EQ(gate_0->get_fan_out_nets().size(), 3);

            std::shared_ptr<net> net_0 = gate_0->get_fan_out_net("O(1)");
            ASSERT_NE(net_0, nullptr);
            EXPECT_EQ(net_0->get_name(), "l_vec(0)");

            std::shared_ptr<net> net_1 = gate_0->get_fan_out_net("O(2)");
            ASSERT_NE(net_1, nullptr);
            EXPECT_EQ(net_1->get_name(), "l_vec(1)");

            std::shared_ptr<net> net_2 = gate_0->get_fan_out_net("O(3)");
            ASSERT_NE(net_2, nullptr);
            EXPECT_EQ(net_2->get_name(), "l_vec(2)");
        }*/
        {
            // Connect a vector of output pins with a vector of nets, but using downto statements
            std::stringstream input("-- Device\t: device_name\n"
                                    "entity TEST_Comp is\n"
                                    "  port (\n"
                                    "  );\n"
                                    "end TEST_Comp;\n"
                                    "architecture STRUCTURE of TEST_Comp is\n"
                                    "  signal l_vec : STD_LOGIC_VECTOR ( 0 to 3 );\n"
                                    "begin\n"
                                    "  gate_0 : GATE_4^1_IN_4^1_OUT\n"
                                    "    port map (\n"
                                    "      O(3 downto 2) => l_vec(2 downto 1)\n"
                                    "    );\n"
                                    "end STRUCTURE;");
            test_def::capture_stdout();
            hdl_parser_vhdl vhdl_parser(input);
            std::shared_ptr<netlist> nl = vhdl_parser.parse(temp_lib_name);
            if (nl == nullptr)
            {
                std::cout << test_def::get_captured_stdout();
            }
            else
            {
                test_def::get_captured_stdout();
            }

            ASSERT_NE(nl, nullptr);
            ASSERT_FALSE(nl->get_gates(gate_filter("GATE_4^1_IN_4^1_OUT", "gate_0")).empty());
            std::shared_ptr<gate> gate_0 = *(nl->get_gates(gate_filter("GATE_4^1_IN_4^1_OUT" ,"gate_0")).begin());

            EXPECT_EQ(gate_0->get_fan_out_nets().size(), 2);

            std::shared_ptr<net> net_1 = gate_0->get_fan_out_net("O(2)");
            ASSERT_NE(net_1, nullptr);
            EXPECT_EQ(net_1->get_name(), "l_vec(1)");

            std::shared_ptr<net> net_2 = gate_0->get_fan_out_net("O(3)");
            ASSERT_NE(net_2, nullptr);
            EXPECT_EQ(net_2->get_name(), "l_vec(2)");
        }
        /*{
            // Connect a vector of output pins with a vector of nets, but using a mix of to and downto statements
            std::stringstream input("-- Device\t: device_name\n"
                                    "entity TEST_Comp is\n"
                                    "  port (\n"
                                    "  );\n"
                                    "end TEST_Comp;\n"
                                    "architecture STRUCTURE of TEST_Comp is\n"
                                    "  signal l_vec : STD_LOGIC_VECTOR ( 0 to 3 );\n"
                                    "begin\n"
                                    "  gate_0 : GATE_4^1_IN_4^1_OUT\n"
                                    "    port map (\n"
                                    "      O(2 to 3) => l_vec(2 downto 1)\n"
                                    "    );\n"
                                    "end STRUCTURE;");
            test_def::capture_stdout();
            hdl_parser_vhdl vhdl_parser(input);
            std::shared_ptr<netlist> nl = vhdl_parser.parse(temp_lib_name);
            if (nl == nullptr)
            {
                std::cout << test_def::get_captured_stdout();
            }
            else
            {
                test_def::get_captured_stdout();
            }

            ASSERT_NE(nl, nullptr);
            ASSERT_FALSE(nl->get_gates(gate_filter("GATE_4^1_IN_4^1_OUT", "gate_0")).empty());
            std::shared_ptr<gate> gate_0 = *(nl->get_gates(gate_filter("GATE_4^1_IN_4^1_OUT" ,"gate_0")).begin());

            EXPECT_EQ(gate_0->get_fan_out_nets().size(), 2);

            std::shared_ptr<net> net_1 = gate_0->get_fan_out_net("O(2)");
            ASSERT_NE(net_1, nullptr);
            EXPECT_EQ(net_1->get_name(), "l_vec(2)");

            std::shared_ptr<net> net_2 = gate_0->get_fan_out_net("O(3)");
            ASSERT_NE(net_2, nullptr);
            EXPECT_EQ(net_2->get_name(), "l_vec(1)");
        }*/
        // NEGATIVE
        {
            // The range of the vectors does not match in size
            std::stringstream input("-- Device\t: device_name\n"
                                    "entity TEST_Comp is\n"
                                    "  port (\n"
                                    "  );\n"
                                    "end TEST_Comp;\n"
                                    "architecture STRUCTURE of TEST_Comp is\n"
                                    "  signal l_vec : STD_LOGIC_VECTOR ( 0 to 3 );\n"
                                    "begin\n"
                                    "  gate_0 : GATE_4^1_IN_4^1_OUT\n"
                                    "    port map (\n"
                                    "      O(0 to 2) => l_vec(0 to 3)\n"
                                    "    );\n"
                                    "end STRUCTURE;");
            test_def::capture_stdout();
            hdl_parser_vhdl vhdl_parser(input);
            std::shared_ptr<netlist> nl = vhdl_parser.parse(temp_lib_name);
            if (nl == nullptr)
            {
                std::cout << test_def::get_captured_stdout();
            }
            else
            {
                test_def::get_captured_stdout();
            }

            ASSERT_NE(nl, nullptr);
            ASSERT_FALSE(nl->get_gates(gate_filter("GATE_4^1_IN_4^1_OUT", "gate_0")).empty());
            std::shared_ptr<gate> gate_0 = *(nl->get_gates(gate_filter("GATE_4^1_IN_4^1_OUT" ,"gate_0")).begin());

            EXPECT_EQ(gate_0->get_fan_out_nets().size(), 0);
        }
        {
            // The right side does no match any vector format
            std::stringstream input("-- Device\t: device_name\n"
                                    "entity TEST_Comp is\n"
                                    "  port (\n"
                                    "  );\n"
                                    "end TEST_Comp;\n"
                                    "architecture STRUCTURE of TEST_Comp is\n"
                                    "begin\n"
                                    "  gate_0 : GATE_4^1_IN_4^1_OUT\n"
                                    "    port map (\n"
                                    "      I(1 to 3) => Unkn0wn_Format\n" // <- unknown format
                                    "    );\n"
                                    "end STRUCTURE;");
            test_def::capture_stdout();
            hdl_parser_vhdl vhdl_parser(input);
            std::shared_ptr<netlist> nl = vhdl_parser.parse(temp_lib_name);
            if (nl == nullptr)
            {
                std::cout << test_def::get_captured_stdout();
            }
            else
            {
                test_def::get_captured_stdout();
            }

            ASSERT_NE(nl, nullptr);
            ASSERT_FALSE(nl->get_gates(gate_filter("GATE_4^1_IN_4^1_OUT", "gate_0")).empty());
            std::shared_ptr<gate> gate_0 = *(nl->get_gates(gate_filter("GATE_4^1_IN_4^1_OUT" ,"gate_0")).begin());

            EXPECT_EQ(gate_0->get_fan_in_nets().size(), 0);

        }
        remove_temp_gate_lib();
    TEST_END
}

/**
 * Testing the usage of components, which should define new gate types with custom input/output/inout pins.
 * (currently unsupported...)
 *
 * Functions: parse
 */
TEST_F(hdl_parser_vhdl_test, check_component)
{
    TEST_START
        /*{
            // Add a component in the architecture header and instantiate a gate of it
            std::stringstream input("-- Device\t: device_name\n"
                                    "entity TEST_Comp is\n"
                                    "  port (\n"
                                    "    net_global_in : in STD_LOGIC := 'X';\n"
                                    "    net_global_out : out STD_LOGIC := 'X';\n"
                                    "  );\n"
                                    "end TEST_Comp;\n"
                                    "architecture STRUCTURE of TEST_Comp is\n"
                                    "  component COMP_GATE\n"
                                    "    port (\n"
                                    "      COMP_IN : in STD_LOGIC;\n"
                                    "      COMP_OUT : out STD_LOGIC;\n"
                                    "      COMP_INOUT : in STD_LOGIC;\n"
                                    "    );\n"
                                    "  end component ;\n"
                                    "begin\n"
                                    "  gate_0 : COMP_GATE\n"
                                    "    port map (\n"
                                    "      COMP_IN => net_global_in,\n"
                                    "      COMP_OUT => net_global_out\n"
                                    "    );\n"
                                    "end STRUCTURE;");
            test_def::capture_stdout();
            hdl_parser_vhdl vhdl_parser(input);
            std::shared_ptr<netlist> nl = vhdl_parser.parse(g_lib_name);
            if (nl == nullptr) {
                std::cout << test_def::get_captured_stdout();
            } else {
                test_def::get_captured_stdout();
            }
            ASSERT_NE(nl, nullptr);
            ASSERT_EQ(nl->get_gates(gate_type_filter("COMP_GATE")).size(), 1);
            std::shared_ptr<gate> comp_gate = *nl->get_gates(gate_type_filter("COMP_GATE")).begin();
            EXPECT_NE(comp_gate->get_fan_in_net("COMP_IN"), nullptr);
            EXPECT_NE(comp_gate->get_fan_out_net("COMP_OUT"), nullptr);
            // EXPECT_EQ(comp_gate->get_inout_pin_types(), std::vector<std::string>({"COMP_INOUT"})); FIXME
        }*/
    TEST_END
}

/**
 * Testing the usage of user-defined attributes within the architecture and the entity header.
 *
 * Functions: parse
 */
TEST_F(hdl_parser_vhdl_test, check_attributes)
{
    TEST_START
        {
            // Add a custom attribute for a gate
            std::stringstream input("-- Device\t: device_name\n"
                                    "entity TEST_Comp is\n"
                                    "  port (\n"
                                    "    net_global_in : in STD_LOGIC := 'X';\n"
                                    "    net_global_out : out STD_LOGIC := 'X';\n"
                                    "  );\n"
                                    "end TEST_Comp;\n"
                                    "architecture STRUCTURE of TEST_Comp is\n"
                                    "  attribute attri_name : attri_type;\n"
                                    "  attribute attri_name of gate_0 : item_class is attri_value;\n"
                                    "begin\n"
                                    "  gate_0 : INV\n"
                                    "    port map (\n"
                                    "      I => net_global_in,\n"
                                    "      O => net_global_out\n"
                                    "    );\n"
                                    "end STRUCTURE;");
            hdl_parser_vhdl vhdl_parser(input);
            std::shared_ptr<netlist> nl = vhdl_parser.parse(g_lib_name);

            ASSERT_NE(nl, nullptr);
            ASSERT_EQ(nl->get_gates(gate_type_filter("INV")).size(), 1);
            std::shared_ptr<gate> attri_gate = *nl->get_gates(gate_type_filter("INV")).begin();
            EXPECT_EQ(attri_gate->get_data_by_key("vhdl_attribute", "attri_name"), std::make_tuple("attri_type", "attri_value"));
        }
        {
            // Add a custom attribute for a net
            std::stringstream input("-- Device\t: device_name\n"
                                    "entity TEST_Comp is\n"
                                    "  port (\n"
                                    "    net_global_in : in STD_LOGIC := 'X';\n"
                                    "    net_global_out : out STD_LOGIC := 'X';\n"
                                    "  );\n"
                                    "end TEST_Comp;\n"
                                    "architecture STRUCTURE of TEST_Comp is\n"
                                    "  signal net_0 : STD_LOGIC;\n"
                                    "  attribute attri_name : attri_type;\n"
                                    "  attribute attri_name of net_0 : signal is \"attri_value\";\n"
                                    "begin\n"
                                    "  gate_0 : INV\n"
                                    "    port map (\n"
                                    "      I => net_global_in,\n"
                                    "      O => net_0\n"
                                    "    );\n"
                                    "  gate_1 : INV\n"
                                    "    port map (\n"
                                    "      I => net_0,\n"
                                    "      O => net_global_out\n"
                                    "    );\n"
                                    "end STRUCTURE;");
            hdl_parser_vhdl vhdl_parser(input);
            std::shared_ptr<netlist> nl = vhdl_parser.parse(g_lib_name);

            ASSERT_NE(nl, nullptr);
            ASSERT_EQ(nl->get_nets(net_name_filter("net_0")).size(), 1);
            std::shared_ptr<net> attri_net = *nl->get_nets(net_name_filter("net_0")).begin();
            EXPECT_NE(attri_net, nullptr);
            EXPECT_EQ(attri_net->get_data_by_key("vhdl_attribute", "attri_name"), std::make_tuple("attri_type", "attri_value"));
        }
    TEST_END
}

/**
 * Testing direct assignment (E.g. something like "net_0 <= net_1", so that both net_0 and net_1 can be used for the same net)
 * NOTE: Is that true?
 *
 * Functions: parse
 */
TEST_F(hdl_parser_vhdl_test, check_direct_assignment)
{
    TEST_START
        {
            // Assign net_1 to net_0
            std::stringstream input("-- Device\t: device_name\n"
                                    "entity TEST_Comp is\n"
                                    "  port (\n"
                                    "    net_global_in : in STD_LOGIC := 'X';\n"
                                    "    net_global_out : out STD_LOGIC := 'X';\n"
                                    "  );\n"
                                    "end TEST_Comp;\n"
                                    "architecture STRUCTURE of TEST_Comp is\n"
                                    "  signal net_0 : STD_LOGIC;\n"
                                    "  signal net_1 : STD_LOGIC;\n"
                                    "begin\n"
                                    "  net_0 <= net_1;\n"
                                    "  gate_0 : INV\n"
                                    "    port map (\n"
                                    "      I => net_global_in,\n"
                                    "      O => net_1\n"
                                    "    );\n"
                                    "  gate_1 : INV\n"
                                    "    port map (\n"
                                    "      I => net_0,\n"
                                    "      O => net_global_out\n"
                                    "    );\n"
                                    "end STRUCTURE;");
            hdl_parser_vhdl vhdl_parser(input);
            std::shared_ptr<netlist> nl = vhdl_parser.parse(g_lib_name);

            ASSERT_NE(nl, nullptr);
            ASSERT_EQ(nl->get_nets(net_name_filter("net_1")).size(), 1);
            std::shared_ptr<net> dir_ass_net = *nl->get_nets(net_name_filter("net_1")).begin();

            ASSERT_EQ(nl->get_gates(gate_filter("INV","gate_0")).size(), 1);
            ASSERT_EQ(nl->get_gates(gate_filter("INV","gate_1")).size(), 1);

            std::shared_ptr<gate> g_0 = *nl->get_gates(gate_filter("INV","gate_0")).begin();
            std::shared_ptr<gate> g_1 = *nl->get_gates(gate_filter("INV","gate_1")).begin();

            EXPECT_EQ(g_0->get_fan_out_net("O"), dir_ass_net);
            EXPECT_EQ(g_1->get_fan_in_net("I"), dir_ass_net);


        }
        {
            // Build up a master-slave hierarchy as follows:
            /*                                  .--- net_slave_1
             *   net_master <--- net_slave_0 <--+
             *                                  '--- net_slave_2
             */
            // Testing the correct creation of the master net by considering the inheritance of the attributes and connections
            // of its slaves
            std::stringstream input("-- Device\t: device_name\n"
                                    "entity TEST_Comp is\n"
                                    "  port (\n"
                                    "    net_global_in : in STD_LOGIC := 'X';\n"
                                    "    net_global_out : out STD_LOGIC := 'X';\n"
                                    "  );\n"
                                    "end TEST_Comp;\n"
                                    "architecture STRUCTURE of TEST_Comp is\n"
                                    "  signal net_slave_0 : STD_LOGIC;\n"
                                    "  signal net_slave_1 : STD_LOGIC;\n"
                                    "  signal net_slave_2 : STD_LOGIC;\n"
                                    "  signal net_master : STD_LOGIC;\n"
                                    "  attribute slave_0_attr : string;\n"
                                    "  attribute slave_0_attr of net_slave_0 : signal is \"slave_0_attr\";\n"
                                    "  attribute slave_1_attr : string;\n"
                                    "  attribute slave_1_attr of net_slave_1 : signal is \"slave_1_attr\";\n"
                                    "  attribute slave_2_attr : string;\n"
                                    "  attribute slave_2_attr of net_slave_2 : signal is \"slave_2_attr\";\n"
                                    "  attribute master_attr : string;\n"
                                    "  attribute master_attr of net_master : signal is \"master_attr\";\n"
                                    "begin\n"
                                    "  net_slave_1 <= net_slave_0; \n"
                                    "  net_slave_0 <= net_master;\n"
                                    "  net_slave_2 <= net_slave_0;\n"
                                    "  gate_0 : INV\n"
                                    "    port map (\n"
                                    "      I => net_global_in,\n"
                                    "      O => net_slave_0\n"
                                    "    );\n"
                                    "  gate_1 : AND3\n"
                                    "    port map (\n"
                                    "      I0 => net_master,\n"
                                    "      I1 => net_slave_1,\n"
                                    "      I2 => net_slave_2,\n"
                                    "      O => net_global_out\n"
                                    "    );\n"
                                    "end STRUCTURE;");
            hdl_parser_vhdl vhdl_parser(input);
            std::shared_ptr<netlist> nl = vhdl_parser.parse(g_lib_name);

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

            // Check the attributes
            EXPECT_EQ(net_master->get_data_by_key("vhdl_attribute", "master_attr"), std::make_tuple("string", "master_attr"));
            EXPECT_EQ(net_master->get_data_by_key("vhdl_attribute", "slave_0_attr"), std::make_tuple("string", "slave_0_attr"));
            EXPECT_EQ(net_master->get_data_by_key("vhdl_attribute", "slave_1_attr"), std::make_tuple("string", "slave_1_attr"));
            EXPECT_EQ(net_master->get_data_by_key("vhdl_attribute", "slave_2_attr"), std::make_tuple("string", "slave_2_attr"));




        }
    TEST_END
}

/**
 * Testing the usage of additional entities. Entities that are used by the main entity (the last one) are recursively
 * split in gates that are part of the gate library, while the original entity hierarchy is represented by the module-
 * hierarchy of the netlist. Therefore, there can be multiple gates with the same name, so names that occur twice or
 * more will be given a unique suffix.
 *
 * Functions: parse
 */
TEST_F(hdl_parser_vhdl_test, check_multiple_entities)
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
            std::stringstream input("-- Device\t: device_name\n"
                                    "entity ENT_CHILD is\n"
                                    "  attribute child_attri : string;\n"
                                    "  attribute child_attri of ENT_CHILD : item_class is \"child_attribute\";\n"
                                    "  port (\n"
                                    "    child_in : in STD_LOGIC := 'X';\n"
                                    "    child_out : out STD_LOGIC := 'X';\n"
                                    "  );\n"
                                    "end ENT_CHILD;\n"
                                    "architecture STRUCTURE_CHILD of ENT_CHILD is\n"
                                    "  signal net_0_child : STD_LOGIC;\n"
                                    "  attribute child_net_attri : string;\n"
                                    "  attribute child_net_attri of child_in : signal is \"child_net_attribute\";\n"
                                    "begin\n"
                                    "  gate_0_child : INV\n"
                                    "    port map (\n"
                                    "      I => child_in,\n"
                                    "      O => net_0_child\n"
                                    "    );\n"
                                    "  gate_1_child : INV\n"
                                    "    port map (\n"
                                    "      I => net_0_child,\n"
                                    "      O => child_out\n"
                                    "    );\n"
                                    "end STRUCTURE_CHILD;\n"
                                    "\n"
                                    "entity ENT_TOP is\n"
                                    "  port (\n"
                                    "    net_global_in : in STD_LOGIC := 'X';\n"
                                    "    net_global_out : out STD_LOGIC := 'X';\n"
                                    "  );\n"
                                    "end ENT_TOP;\n"
                                    "architecture STRUCTURE of ENT_TOP is\n"
                                    "  signal net_0 : STD_LOGIC;\n"
                                    "  signal net_1 : STD_LOGIC;\n"
                                    "\n"
                                    "begin\n"
                                    "  gate_0 : INV\n"
                                    "    port map (\n"
                                    "      I => net_global_in,\n"
                                    "      O => net_0\n"
                                    "    );\n"
                                    "  child_mod : ENT_CHILD\n"
                                    "    port map (\n"
                                    "      child_in => net_0,\n"
                                    "      child_out => net_1\n"
                                    "    );\n"
                                    "  gate_1 : INV\n"
                                    "    port map (\n"
                                    "      I => net_1,\n"
                                    "      O => net_global_out\n"
                                    "    );\n"
                                    "end ENT_TOP;");
            hdl_parser_vhdl vhdl_parser(input);
            std::shared_ptr<netlist> nl = vhdl_parser.parse(g_lib_name);

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
            // Check that the attributes of the child entities port are inherit correctly to the connecting net
            EXPECT_EQ(net_0->get_data_by_key("vhdl_attribute", "child_net_attri"), std::make_tuple("string","child_net_attribute"));

            // Test that the modules are created and assigned correctly
            std::shared_ptr<module> top_mod = nl->get_top_module();
            ASSERT_EQ(top_mod->get_submodules().size(), 1);
            std::shared_ptr<module> child_mod = *top_mod->get_submodules().begin();
            EXPECT_EQ(child_mod->get_name(), "ENT_CHILD");
            EXPECT_EQ(top_mod->get_gates(), std::set<std::shared_ptr<gate>>({gate_0, gate_1}));
            EXPECT_EQ(child_mod->get_gates(), std::set<std::shared_ptr<gate>>({gate_0_child, gate_1_child}));
            EXPECT_EQ(child_mod->get_data_by_key("vhdl_attribute", "child_attri"), std::make_tuple("string","child_attribute"));
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

            std::stringstream input("-- Device\t: device_name\n"
                                    "entity ENT_CHILD_TWO is\n"
                                    "  port (\n"
                                    "    I_c2 : in STD_LOGIC := 'X';\n"
                                    "    O_c2 : out STD_LOGIC := 'X';\n"
                                    "  );\n"
                                    "end ENT_CHILD_TWO;\n"
                                    "architecture STRUCTURE_CHILD_TWO of ENT_CHILD_TWO is\n"
                                    "begin\n"
                                    "  gate_child_two : INV\n"
                                    "    port map (\n"
                                    "      I => I_c2,\n"
                                    "      O => O_c2\n"
                                    "    );\n"
                                    "end STRUCTURE_CHILD_TWO;\n"
                                    "\n"
                                    "entity ENT_CHILD_ONE is\n"
                                    "  port (\n"
                                    "    I_c1 : in STD_LOGIC := 'X';\n"
                                    "    O_c1 : out STD_LOGIC := 'X';\n"
                                    "  );\n"
                                    "end ENT_ONE;\n"
                                    "architecture STRUCTURE_CHILD_ONE of ENT_CHILD_ONE is\n"
                                    "  signal net_child_0 : STD_LOGIC;\n"
                                    "  signal net_child_1 : STD_LOGIC;\n"
                                    "begin\n"
                                    "  gate_0_ent_two : ENT_CHILD_TWO\n"
                                    "    port map (\n"
                                    "      I_c2 => I_c1,\n"
                                    "      O_c2 => net_child_0\n"
                                    "    );\n"
                                    "  gate_1_ent_two : ENT_CHILD_TWO\n"
                                    "    port map (\n"
                                    "      I_c2 => net_child_0,\n"
                                    "      O_c2 => net_child_1\n"
                                    "    );\n"
                                    "  gate_child_one : INV\n"
                                    "    port map (\n"
                                    "      I => net_child_1,\n"
                                    "      O => O_c1\n"
                                    "    );\n"
                                    "end STRUCTURE_CHILD_ONE;\n"
                                    "\n"
                                    "entity ENT_TOP is\n"
                                    "  port (\n"
                                    "    net_global_in : in STD_LOGIC := 'X';\n"
                                    "    net_global_out : out STD_LOGIC := 'X';\n"
                                    "  );\n"
                                    "end ENT_TOP;\n"
                                    "architecture STRUCTURE of ENT_TOP is\n"
                                    "  signal net_0 : STD_LOGIC;\n"
                                    "  signal net_1 : STD_LOGIC;\n"
                                    "begin\n"
                                    "  child_one_mod : ENT_CHILD_ONE\n"
                                    "    port map (\n"
                                    "      I_c1 => net_global_in,\n"
                                    "      O_c1 => net_0\n"
                                    "    );\n"
                                    "  child_two_mod : ENT_CHILD_TWO\n"
                                    "    port map (\n"
                                    "      I_c2 => net_0,\n"
                                    "      O_c2 => net_1\n"
                                    "    );\n"
                                    "  gate_top : INV\n"
                                    "    port map (\n"
                                    "      I => net_1,\n"
                                    "      O => net_global_out\n"
                                    "    );\n"
                                    "end ENT_TOP;");
            hdl_parser_vhdl vhdl_parser(input);
            std::shared_ptr<netlist> nl = vhdl_parser.parse(g_lib_name);

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
            std::string gate_suffix = "_module_inst"; // NOTE: Why module_inst ?

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
        }
        {
            // Use the 'entity'-keyword in the context of a gate type (should be ignored)
            std::stringstream input("-- Device\t: device_name\n"
                                    "entity TEST_Comp is\n"
                                    "  port (\n"
                                    "    net_global_input : in STD_LOGIC := 'X';\n"
                                    "  );\n"
                                    "end TEST_Comp;\n"
                                    "architecture STRUCTURE of TEST_Comp is\n"
                                    "begin\n"
                                    "  gate_0 : entity INV\n" // <- usage of the 'entity' keyword
                                    "    port map (\n"
                                    "      O => net_global_input\n"
                                    "    );\n"
                                    "end STRUCTURE;");
            test_def::capture_stdout();
            hdl_parser_vhdl vhdl_parser(input);
            std::shared_ptr<netlist> nl = vhdl_parser.parse(g_lib_name);
            if (nl == nullptr)
            {
                std::cout << test_def::get_captured_stdout();
            }
            else
            {
                test_def::get_captured_stdout();
            }

            ASSERT_NE(nl, nullptr);
            ASSERT_FALSE(nl->get_gates(gate_filter("INV", "gate_0")).empty());

        }
    TEST_END
}

/**
 * Testing the correct handling of invalid input
 *
 * Functions: parse
 */
TEST_F(hdl_parser_vhdl_test, check_invalid_input)
{
    TEST_START
        {
            // The entity contains unknown direction keywords (neither 'in', 'out' nor 'inout')
            NO_COUT_TEST_BLOCK;
            std::stringstream input("-- Device\t: device_name\n"
                                    "entity TEST_Comp is\n"
                                    "  port (\n"
                                    "    net_global_input : invalid_direction STD_LOGIC := 'X';\n" // <- invalid direction
                                    "  );\n"
                                    "end TEST_Comp;\n"
                                    "architecture STRUCTURE of TEST_Comp is\n"
                                    "begin\n"
                                    "  gate_0 : INV\n"
                                    "    port map (\n"
                                    "      O => net_global_input\n"
                                    "    );\n"
                                    "end STRUCTURE;");
            hdl_parser_vhdl vhdl_parser(input);
            std::shared_ptr<netlist> nl = vhdl_parser.parse(g_lib_name);

            EXPECT_EQ(nl, nullptr);
        }
        {
            // The architecture contains an invalid keyword (neither 'signal' nor 'attribute')
            NO_COUT_TEST_BLOCK;
            std::stringstream input("-- Device\t: device_name\n"
                                    "entity TEST_Comp is\n"
                                    "  port (\n"
                                    "    net_global_input : in STD_LOGIC := 'X';\n"
                                    "  );\n"
                                    "end TEST_Comp;\n"
                                    "architecture STRUCTURE of TEST_Comp is\n"
                                    "unknown_keyword some_signal : STD_LOGIC;" // <- invalid keyword
                                    "begin\n"
                                    "  gate_0 : INV\n"
                                    "    port map (\n"
                                    "      O => net_global_input\n"
                                    "    );\n"
                                    "end STRUCTURE;");
            hdl_parser_vhdl vhdl_parser(input);
            std::shared_ptr<netlist> nl = vhdl_parser.parse(g_lib_name);

            EXPECT_EQ(nl, nullptr);
        }
        {
            // The passed gate_library does not exist
            NO_COUT_TEST_BLOCK;
            std::stringstream input("-- Device\t: device_name\n"
                                    "entity TEST_Comp is\n"
                                    "  port (\n"
                                    "    net_global_input : in STD_LOGIC := 'X';\n"
                                    "  );\n"
                                    "end TEST_Comp;\n"
                                    "architecture STRUCTURE of TEST_Comp is\n"
                                    "begin\n"
                                    "  gate_0 : INV\n"
                                    "    port map (\n"
                                    "      O => net_global_input\n"
                                    "    );\n"
                                    "end STRUCTURE;");                      // <- no 'end STRUCTURE;'
            hdl_parser_vhdl vhdl_parser(input);
            std::shared_ptr<netlist> nl = vhdl_parser.parse("inv4lid_gate_library");

            EXPECT_EQ(nl, nullptr);
        }
        {
            // Testing incorrect data_types in the "generic map" block
            NO_COUT_TEST_BLOCK;
            std::stringstream input("-- Device\t: device_name\n"
                                    "entity TEST_Comp is\n"
                                    "  port (\n"
                                    "    net_global_input : in STD_LOGIC := 'X';\n"
                                    "  );\n"
                                    "end TEST_Comp;\n"
                                    "architecture STRUCTURE of TEST_Comp is\n"
                                    "begin\n"
                                    "  gate_0 : INV\n"
                                    "    generic map(\n"
                                    "      key_invalid_type => Inv4lid_type\n"        // <- The format of 'Inv4lid_type' matches with no data_type
                                    "    )\n"
                                    "    port map (\n"
                                    "      I => net_global_input\n"
                                    "    );\n"
                                    "end STRUCTURE;");
            hdl_parser_vhdl vhdl_parser(input);
            std::shared_ptr<netlist> nl = vhdl_parser.parse(g_lib_name);
            EXPECT_EQ(nl, nullptr);
        }
        /*{ // NOTE: Fails without error message (should work...)
            // Leave the 'port map' block empty (gate is not connected)
            NO_COUT_TEST_BLOCK;
            std::stringstream input("-- Device\t: device_name\n"
                                    "entity TEST_Comp is\n"
                                    "  port (\n"
                                    "    net_global_input : in STD_LOGIC := 'X';\n"
                                    "  );\n"
                                    "end TEST_Comp;\n"
                                    "architecture STRUCTURE of TEST_Comp is\n"
                                    "begin\n"
                                    "  gate_0 : INV\n"
                                    "    port map (\n"
                                    "    );\n"
                                    "end STRUCTURE;");
            hdl_parser_vhdl vhdl_parser(input);
            std::shared_ptr<netlist> nl = vhdl_parser.parse(g_lib_name);
            ASSERT_NE(nl, nullptr);
            ASSERT_FALSE(nl->get_gates(gate_filter("INV", "gate_0")).empty());
            std::shared_ptr<gate> gate_0 = *nl->get_gates(gate_filter("INV", "gate_0")).begin();
            EXPECT_TRUE(gate_0->get_fan_out_nets().empty());
            EXPECT_TRUE(gate_0->get_fan_in_nets().empty());
        }*/
        { //NOTE: Fails because of mistype issue line 108: (inout_pin_type.end() <-> input_pin_type.end())
            // Try to connect to a pin, which does not exist
            NO_COUT_TEST_BLOCK;
            std::stringstream input("-- Device\t: device_name\n"
                                    "entity TEST_Comp is\n"
                                    "  port (\n"
                                    "    net_global_input : in STD_LOGIC := 'X';\n"
                                    "  );\n"
                                    "end TEST_Comp;\n"
                                    "architecture STRUCTURE of TEST_Comp is\n"
                                    "begin\n"
                                    "  gate_0 : INV\n"
                                    "    port map (\n"
                                    "      NOT_EXISTING_PIN => net_global_input\n"
                                    "    );\n"
                                    "end STRUCTURE;");
            hdl_parser_vhdl vhdl_parser(input);
            std::shared_ptr<netlist> nl = vhdl_parser.parse(g_lib_name);
            ASSERT_EQ(nl, nullptr);
        }
    TEST_END
}
