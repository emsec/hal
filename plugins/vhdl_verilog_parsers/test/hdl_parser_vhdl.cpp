#include "vhdl_verilog_parsers/hdl_parser_vhdl.h"

#include "netlist_test_utils.h"

#include <experimental/filesystem>

namespace hal {

    class HDLParserVHDLTest : public ::testing::Test {
    protected:
        GateLibrary* m_gl;

        virtual void SetUp() {
            NO_COUT_BLOCK;
            test_utils::init_log_channels();
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
     *                  '--=                |
     *                       AND2 (1) =-----'
     *                     =              net_1
     */
    /**
     * Testing the correct usage of the vhdl parser by parse a small vhdl-format string, which describes the netlist
     * shown above.
     *
     * Functions: parse
     */
    TEST_F(HDLParserVHDLTest, check_main_example) {
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
                                    "  );\n"
                                    "end TEST_Comp;\n"
                                    "\n"
                                    "architecture STRUCTURE of TEST_Comp is\n"
                                    "  signal net_0 : STD_LOGIC;\n"
                                    "  signal net_1 : STD_LOGIC;\n"
                                    "begin\n"
                                    "  gate_0 : gate_1_to_1\n"
                                    "    port map (\n"
                                    "      I => net_global_in,\n"
                                    "      O => net_0\n"
                                    "    );\n"
                                    "  gate_1 : gate_2_to_1\n"
                                    "    port map (\n"
                                    "      I0 => net_global_in,\n"
                                    "      I1 => net_global_in,\n"
                                    "      O => net_1\n"
                                    "    );\n"
                                    "  gate_2 : gate_3_to_1\n"
                                    "    port map (\n"
                                    "      I0 => net_0,\n"
                                    "      I1 => net_1,\n"
                                    "      O => net_global_out\n"
                                    "    );\n"
                                    "end STRUCTURE;\n"
                                    "");
            test_def::capture_stdout();
            HDLParserVHDL vhdl_parser;
            std::unique_ptr<Netlist> nl = vhdl_parser.parse_and_instantiate(input, m_gl);
            if (nl == nullptr) {
                std::cout << test_def::get_captured_stdout();
            } else {
                test_def::get_captured_stdout();
            }

            ASSERT_NE(nl, nullptr);

            // Check if the device name is parsed correctly
            EXPECT_EQ(nl->get_design_name(), "TEST_Comp");

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

        TEST_END
    }

    /**
     * The same test, as the main example, but use white spaces of different types (' ','\n','\t') in various locations (or remove some unnecessary ones)
     *
     * Functions: parse
     */
    TEST_F(HDLParserVHDLTest, check_whitespace_chaos_) {

        TEST_START
            {
                std::stringstream input("-- Device\t: device_name\n"
                                        "library IEEE;use IEEE.STD_LOGIC_1164.ALL;library SIMPRIM;\n"
                                        "use SIMPRIM.VCOMPONENTS.ALL;use SIMPRIM.VPKG.ALL;\n"
                                        "entity TEST_Comp is port(net_global_in:in STD_LOGIC := 'X';net_global_out\n"
                                        "\n"
                                        " : out STD_LOGIC := 'X';);end TEST_Comp;architecture \n"
                                        " STRUCTURE of TEST_Comp is signal net_0 : STD_LOGIC;\n"
                                        "signal net_1 : STD_LOGIC;\n"
                                        "begin gate_0 : gate_1_to_1 port map(\n"
                                        "  I => net_global_in,\n"
                                        "  \n"
                                        "  \t O => net_0\n"
                                        "    );\n"
                                        "  gate_1 : gate_2_to_1\n"
                                        "    port map ( I0 \n\t"
                                        "      => net_global_in\n"
                                        "      \t,\n"
                                        "      I1 => net_global_in,O => net_1);\n"
                                        "gate_2:gate_3_to_1 port map(I0 => net_0,I1 => net_1,O => net_global_out);end STRUCTURE;");
                test_def::capture_stdout();
                HDLParserVHDL vhdl_parser;
                std::unique_ptr<Netlist> nl = vhdl_parser.parse_and_instantiate(input, m_gl);
                if (nl == nullptr) {
                    std::cout << test_def::get_captured_stdout();
                } else {
                    test_def::get_captured_stdout();
                }

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
     * Testing the correct storage of data, passed by the "generic map" keyword
     *
     * Functions: parse
     */
    TEST_F(HDLParserVHDLTest, check_generic_map) {

        TEST_START
            {
                // Store an instance of all possible data types in one Gate + some special cases
                std::stringstream input("-- Device\t: device_name\n"
                                        "entity TEST_Comp is "
                                        "  port ( "
                                        "    net_global_input : in STD_LOGIC := 'X'; "
                                        "  ); "
                                        "end TEST_Comp; "
                                        "architecture STRUCTURE of TEST_Comp is "
                                        "begin"
                                        "  gate_0 : gate_1_to_1"
                                        "    generic map("
                                        "      key_integer => 1234,"
                                        "      key_floating_point => 1.234,"
                                        "      key_string => \"test_string\","
                                        "      key_bit_vector_hex => X\"abc\","    // <- all values are 0xABC
                                        "      key_bit_vector_dec => D\"2748\","
                                        "      key_bit_vector_oct => O\"5274\","
                                        "      key_bit_vector_bin => B\"1010_1011_1100\","
                                        // special characters in '"'
                                        "      key_negative_comma_string => \"test,1,2,3\","
                                        "      key_negative_float_string => \"1.234\","
                                        // -- VHDL specific Data Types:
                                        "      key_boolean => true,"
                                        "      key_time => 1.234sec,"
                                        "      key_bit_value => '1'"

                                        "    )"
                                        "    port map ( "
                                        "      I => net_global_input "
                                        "    ); "
                                        "end STRUCTURE;");
                test_def::capture_stdout();
                HDLParserVHDL vhdl_parser;
                std::unique_ptr<Netlist> nl = vhdl_parser.parse_and_instantiate(input, m_gl);
                if (nl == nullptr) {
                    std::cout << test_def::get_captured_stdout();
                } else {
                    test_def::get_captured_stdout();
                }

                ASSERT_NE(nl, nullptr);

                ASSERT_EQ(nl->get_gates(test_utils::gate_filter("gate_1_to_1", "gate_0")).size(), 1);
                Gate* gate_0 = *nl->get_gates(test_utils::gate_filter("gate_1_to_1", "gate_0")).begin();

                // Integers are stored in their hex representation
                EXPECT_EQ(gate_0->get_data("generic", "key_integer"), std::make_tuple("integer", "1234"));
                EXPECT_EQ(gate_0->get_data("generic", "key_floating_point"),
                          std::make_tuple("floating_point", "1.234"));
                EXPECT_EQ(gate_0->get_data("generic", "key_string"), std::make_tuple("string", "test_string"));
                EXPECT_EQ(gate_0->get_data("generic", "key_bit_vector_hex"),
                          std::make_tuple("bit_vector", "abc"));
                EXPECT_EQ(gate_0->get_data("generic", "key_bit_vector_dec"),
                          std::make_tuple("bit_vector", "abc"));
                EXPECT_EQ(gate_0->get_data("generic", "key_bit_vector_oct"),
                          std::make_tuple("bit_vector", "abc"));
                EXPECT_EQ(gate_0->get_data("generic", "key_bit_vector_bin"),
                          std::make_tuple("bit_vector", "abc"));
                // Special Characters
                EXPECT_EQ(gate_0->get_data("generic", "key_negative_comma_string"),
                          std::make_tuple("string", "test,1,2,3"));
                EXPECT_EQ(gate_0->get_data("generic", "key_negative_float_string"),
                          std::make_tuple("string", "1.234"));
                // -- VHDL specific Data Types:
                EXPECT_EQ(gate_0->get_data("generic", "key_boolean"), std::make_tuple("boolean", "true"));
                EXPECT_EQ(gate_0->get_data("generic", "key_time"), std::make_tuple("time", "1.234sec"));
                EXPECT_EQ(gate_0->get_data("generic", "key_bit_value"), std::make_tuple("bit_value", "1"));
            }
        TEST_END
    }

    /**
     * Testing the handling of Net-vectors in dimension 1-3
     *
     * Functions: parse
     */
    TEST_F(HDLParserVHDLTest, check_net_vectors) {
        TEST_START
            {
                // Use two logic vectors with dimension 1. One uses the 'downto' the other the 'to' statement
                /*
                 *                           n_vec_1              n_vec_2
                 *                        =-----------=        =-----------=
                 *                        =-----------=        =-----------=
                 *  global_in ---= gate_0 =-----------= gate_1 =-----------= gate_2 =--- global_out
                 *                        =-----------=        =-----------=
                 *
                 */
                std::stringstream input("-- Device\t: device_name \n"
                                        "entity TEST_Comp is "
                                        "  port ( "
                                        "    net_global_in : in STD_LOGIC := 'X'; "
                                        "    net_global_out : out STD_LOGIC := 'X'; "
                                        "  ); "
                                        "end TEST_Comp; "
                                        "architecture STRUCTURE of TEST_Comp is "
                                        "  signal n_vec_1 : STD_LOGIC_VECTOR ( 3 downto 0 ); "
                                        "  signal n_vec_2 : STD_LOGIC_VECTOR ( 0 to 3 ); "
                                        "begin "
                                        "  gate_0 : gate_1_to_4 "
                                        "    port map ( "
                                        "      I => net_global_in, "
                                        "      O0 => n_vec_1(0), "
                                        "      O1 => n_vec_1(1), "
                                        "      O2 => n_vec_1(2), "
                                        "      O3 => n_vec_1(3) "
                                        "    ); "
                                        "  gate_1 : gate_4_to_4 "
                                        "    port map ( "
                                        "      I0 => n_vec_1(0), "
                                        "      I1 => n_vec_1(1), "
                                        "      I2 => n_vec_1(2), "
                                        "      I3 => n_vec_1(3), "
                                        "      O0 => n_vec_2(0), "
                                        "      O1 => n_vec_2(1), "
                                        "      O2 => n_vec_2(2), "
                                        "      O3 => n_vec_2(3) "
                                        "    ); "
                                        "  gate_2 : gate_4_to_1 "
                                        "    port map ( "
                                        "      I0 => n_vec_2(0), "
                                        "      I1 => n_vec_2(1), "
                                        "      I2 => n_vec_2(2), "
                                        "      I3 => n_vec_2(3), "
                                        "      O => net_global_out "
                                        "    ); "
                                        "end STRUCTURE;");
                test_def::capture_stdout();
                HDLParserVHDL vhdl_parser;
                std::unique_ptr<Netlist> nl = vhdl_parser.parse_and_instantiate(input, m_gl);
                if (nl == nullptr) {
                    std::cout << test_def::get_captured_stdout();
                } else {
                    test_def::get_captured_stdout();
                }

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
                    Net*
                        n_vec_1_i = *nl->get_nets(test_utils::net_name_filter("n_vec_1(" + i_str + ")")).begin();
                    Net*
                        n_vec_2_i = *nl->get_nets(test_utils::net_name_filter("n_vec_2(" + i_str + ")")).begin();
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
                std::stringstream input("-- Device\t: device_name\n"
                                        "entity TEST_Comp is "
                                        "  port ( "
                                        "    net_global_in : in STD_LOGIC := 'X'; "
                                        "    net_global_out : out STD_LOGIC := 'X'; "
                                        "  ); "
                                        "end TEST_Comp; "
                                        "architecture STRUCTURE of TEST_Comp is "
                                        "  signal n_vec : STD_LOGIC_VECTOR2 ( 0 to 1, 2 to 3 ); "
                                        "begin "
                                        "  gate_0 : gate_1_to_4 "
                                        "    port map ( "
                                        "      I => net_global_in, "
                                        "      O0 => n_vec(0,2), "
                                        "      O1 => n_vec(0,3), "
                                        "      O2 => n_vec(1, 2), "
                                        "      O3 => n_vec(1, 3) "
                                        "    ); "
                                        "  gate_1 : gate_4_to_1 "
                                        "    port map ( "
                                        "      I0 => n_vec(0, 2), "
                                        "      I1 => n_vec(0, 3), "
                                        "      I2 => n_vec(1, 2), "
                                        "      I3 => n_vec(1, 3), "
                                        "      O => net_global_out "
                                        "    ); "
                                        "end STRUCTURE;");
                test_def::capture_stdout();
                HDLParserVHDL vhdl_parser;
                std::unique_ptr<Netlist> nl = vhdl_parser.parse_and_instantiate(input, m_gl);
                if (nl == nullptr) {
                    std::cout << test_def::get_captured_stdout();
                } else {
                    test_def::get_captured_stdout();
                }

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
                std::stringstream input("-- Device\t: device_name\n"
                                        "entity TEST_Comp is "
                                        "  port ( "
                                        "    net_global_in : in STD_LOGIC := 'X'; "
                                        "    net_global_out : out STD_LOGIC := 'X'; "
                                        "  ); "
                                        "end TEST_Comp; "
                                        "architecture STRUCTURE of TEST_Comp is "
                                        "  signal n_vec : STD_LOGIC_VECTOR3 ( 0 to 1, 1 downto 0, 0 to 1 ); "
                                        "begin "
                                        "  gate_0 : gate_1_to_8 "
                                        "    port map ( "
                                        "      I => net_global_in, "
                                        "      O0 => n_vec(0, 0, 0), "
                                        "      O1 => n_vec(0, 0, 1), "
                                        "      O2 => n_vec(0, 1, 0), "
                                        "      O3 => n_vec(0, 1, 1), "
                                        "      O4 => n_vec(1, 0, 0), "
                                        "      O5 => n_vec(1, 0, 1), "
                                        "      O6 => n_vec(1, 1, 0), "
                                        "      O7 => n_vec(1, 1, 1) "
                                        "    ); "
                                        "  gate_1 : gate_8_to_1 "
                                        "    port map ( "
                                        "      I0 => n_vec(0, 0, 0), "
                                        "      I1 => n_vec(0, 0, 1), "
                                        "      I2 => n_vec(0, 1, 0), "
                                        "      I3 => n_vec(0, 1, 1), "
                                        "      I4 => n_vec(1, 0, 0), "
                                        "      I5 => n_vec(1, 0, 1), "
                                        "      I6 => n_vec(1, 1, 0), "
                                        "      I7 => n_vec(1, 1, 1), "
                                        "      O => net_global_out "
                                        "    ); "
                                        "end STRUCTURE;");
                test_def::capture_stdout();
                HDLParserVHDL vhdl_parser;
                std::unique_ptr<Netlist> nl = vhdl_parser.parse_and_instantiate(input, m_gl);
                if (nl == nullptr) {
                    std::cout << test_def::get_captured_stdout();
                } else {
                    test_def::get_captured_stdout();
                }
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
     * Testing the addition of a gnd and vcc gates
     *
     * Functions: parse
     */
    TEST_F(HDLParserVHDLTest, check_gnd_vcc_gates) {

        TEST_START
            {
                // Add a global_gnd
                std::stringstream input("-- Device\t: device_name\n"
                                        "entity TEST_Comp is "
                                        "  port (\n"
                                        "    net_global_output : out STD_LOGIC := 'X'; "
                                        "  ); "
                                        "end TEST_Comp; "
                                        "architecture STRUCTURE of TEST_Comp is "
                                        "begin "
                                        "  g_gnd_gate : gnd "
                                        "    port map ( "
                                        "      O => net_global_output "
                                        "    ); "
                                        "end STRUCTURE;");
                test_def::capture_stdout();
                HDLParserVHDL vhdl_parser;
                std::unique_ptr<Netlist> nl = vhdl_parser.parse_and_instantiate(input, m_gl);
                if (nl == nullptr) {
                    std::cout << test_def::get_captured_stdout();
                } else {
                    test_def::get_captured_stdout();
                }

                ASSERT_NE(nl, nullptr);

                ASSERT_NE(nl->get_gnd_gates().size(), 0);
                Gate* global_gnd = *nl->get_gnd_gates().begin();
                EXPECT_EQ(global_gnd->get_name(), "g_gnd_gate");
            }
            {
                // Add a global_vcc
                std::stringstream input("-- Device\t: device_name\n"
                                        "entity TEST_Comp is "
                                        "  port ( "
                                        "    net_global_output : out STD_LOGIC := 'X'; "
                                        "  ); "
                                        "end TEST_Comp; "
                                        "architecture STRUCTURE of TEST_Comp is "
                                        "begin "
                                        "  g_vcc_gate : vcc "
                                        "    port map ( "
                                        "      O => net_global_output "
                                        "    ); "
                                        "end STRUCTURE;");
                test_def::capture_stdout();
                HDLParserVHDL vhdl_parser;
                std::unique_ptr<Netlist> nl = vhdl_parser.parse_and_instantiate(input, m_gl);
                if (nl == nullptr) {
                    std::cout << test_def::get_captured_stdout();
                } else {
                    test_def::get_captured_stdout();
                }

                ASSERT_NE(nl, nullptr);

                ASSERT_NE(nl->get_vcc_gates().size(), 0);
                Gate* global_vcc = *nl->get_vcc_gates().begin();
                EXPECT_EQ(global_vcc->get_name(), "g_vcc_gate");
            }
        TEST_END
    }

    /**
     * Testing the port assigment of the nets '0' and '1', that are connected to gnd/vcc gates.
     *
     * Functions: parse
     */
    TEST_F(HDLParserVHDLTest, check_zero_and_one_nets) {

        TEST_START
            {
                // Use zero and one as inputs (via '0' and '1')
                std::stringstream input("-- Device\t: device_name\n"
                                        "entity TEST_Comp is "
                                        "  port ( "
                                        "    net_global_out_0 : out STD_LOGIC := 'X'; "
                                        "    net_global_out_1 : out STD_LOGIC := 'X'; "
                                        "  ); "
                                        "end TEST_Comp; "
                                        "architecture STRUCTURE of TEST_Comp is "
                                        "begin "
                                        "  gate_0 : gate_1_to_1 "
                                        "    port map ( "
                                        "      I => '0' "
                                        "      O => net_global_out_0 "
                                        "    ); "
                                        "  gate_1 : gate_1_to_1 "
                                        "    port map ( "
                                        "      I => '1' "
                                        "      0 => net_global_out_1 "
                                        "    ); "
                                        "end STRUCTURE;");
                test_def::capture_stdout();
                HDLParserVHDL vhdl_parser;
                std::unique_ptr<Netlist> nl = vhdl_parser.parse_and_instantiate(input, m_gl);
                if (nl == nullptr) {
                    std::cout << test_def::get_captured_stdout();
                } else {
                    test_def::get_captured_stdout();
                }

                ASSERT_NE(nl, nullptr);
                ASSERT_EQ(nl->get_gates(test_utils::gate_filter("gate_1_to_1", "gate_0")).size(), 1);
                ASSERT_EQ(nl->get_gates(test_utils::gate_filter("gate_1_to_1", "gate_1")).size(), 1);
                Gate* gate_0 = *nl->get_gates(test_utils::gate_filter("gate_1_to_1", "gate_0")).begin();
                Gate* gate_1 = *nl->get_gates(test_utils::gate_filter("gate_1_to_1", "gate_1")).begin();

                // Test that the nets '0' and '1' are created and connected
                Net* net_gnd = gate_0->get_fan_in_net("I");
                Net* net_vcc = gate_1->get_fan_in_net("I");
                ASSERT_NE(net_gnd, nullptr);
                ASSERT_NE(net_vcc, nullptr);
                EXPECT_EQ(net_gnd->get_name(), "\'0\'");
                EXPECT_EQ(net_vcc->get_name(), "\'1\'");

                // Test that the nets '0' and '1' are connected to a created global gnd/vcc Gate
                ASSERT_EQ(net_gnd->get_sources().size(), 1);
                ASSERT_NE(net_gnd->get_sources()[0]->get_gate(), nullptr);
                ASSERT_EQ(net_vcc->get_sources().size(), 1);
                ASSERT_NE(net_vcc->get_sources()[0]->get_gate(), nullptr);
                EXPECT_TRUE(net_gnd->get_sources()[0]->get_gate()->is_gnd_gate());
                EXPECT_TRUE(net_vcc->get_sources()[0]->get_gate()->is_vcc_gate());
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
    TEST_F(HDLParserVHDLTest, check_multiple_entities) {

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
                                        "entity ENT_CHILD is "
                                        "  attribute child_attri : string; "
                                        "  attribute child_attri of ENT_CHILD : entity is \"child_attribute\"; "
                                        "  port ( "
                                        "    child_in : in STD_LOGIC := 'X'; "
                                        "    child_out : out STD_LOGIC := 'X'; "
                                        "  ); "
                                        "end ENT_CHILD; "
                                        "architecture STRUCTURE_CHILD of ENT_CHILD is "
                                        "  signal net_0_child : STD_LOGIC; "
                                        "  attribute child_net_attri : string; "
                                        "  attribute child_net_attri of child_in : signal is \"child_net_attribute\"; "
                                        "begin "
                                        "  gate_0_child : gate_1_to_1 "
                                        "    port map ( "
                                        "      I => child_in, "
                                        "      O => net_0_child "
                                        "    ); "
                                        "  gate_1_child : gate_1_to_1 "
                                        "    port map ( "
                                        "      I => net_0_child, "
                                        "      O => child_out "
                                        "    ); "
                                        "end STRUCTURE_CHILD; "
                                        ""
                                        "entity ENT_TOP is "
                                        "  port ( "
                                        "    net_global_in : in STD_LOGIC := 'X'; "
                                        "    net_global_out : out STD_LOGIC := 'X'; "
                                        "  ); "
                                        "end ENT_TOP; "
                                        "architecture STRUCTURE of ENT_TOP is "
                                        "  signal net_0 : STD_LOGIC; "
                                        "  signal net_1 : STD_LOGIC; "
                                        " "
                                        "begin "
                                        "  gate_0 : gate_1_to_1 "
                                        "    port map ( "
                                        "      I => net_global_in, "
                                        "      O => net_0 "
                                        "    ); "
                                        "  child_mod : ENT_CHILD "
                                        "    port map ( "
                                        "      child_in => net_0, "
                                        "      child_out => net_1 "
                                        "    ); "
                                        "  gate_1 : gate_1_to_1 "
                                        "    port map ( "
                                        "      I => net_1, "
                                        "      O => net_global_out "
                                        "    ); "
                                        "end ENT_TOP;");
                test_def::capture_stdout();
                HDLParserVHDL vhdl_parser;
                std::unique_ptr<Netlist> nl = vhdl_parser.parse_and_instantiate(input, m_gl);
                if (nl == nullptr) {
                    std::cout << test_def::get_captured_stdout();
                } else {
                    test_def::get_captured_stdout();
                }

                // Test that all gates are created
                ASSERT_NE(nl, nullptr);
                ASSERT_EQ(nl->get_gates(test_utils::gate_filter("gate_1_to_1", "gate_0")).size(), 1);
                ASSERT_EQ(nl->get_gates(test_utils::gate_filter("gate_1_to_1", "gate_1")).size(), 1);
                ASSERT_EQ(nl->get_gates(test_utils::gate_filter("gate_1_to_1", "gate_0_child")).size(), 1);
                ASSERT_EQ(nl->get_gates(test_utils::gate_filter("gate_1_to_1", "gate_1_child")).size(), 1);
                Gate* gate_0 = *nl->get_gates(test_utils::gate_filter("gate_1_to_1", "gate_0")).begin();
                Gate* gate_1 = *nl->get_gates(test_utils::gate_filter("gate_1_to_1", "gate_1")).begin();
                Gate*
                    gate_0_child = *nl->get_gates(test_utils::gate_filter("gate_1_to_1", "gate_0_child")).begin();
                Gate*
                    gate_1_child = *nl->get_gates(test_utils::gate_filter("gate_1_to_1", "gate_1_child")).begin();

                // Test that all nets are created
                ASSERT_EQ(nl->get_nets(test_utils::net_name_filter("net_0")).size(), 1);
                ASSERT_EQ(nl->get_nets(test_utils::net_name_filter("net_1")).size(), 1);
                ASSERT_EQ(nl->get_nets(test_utils::net_name_filter("net_global_in")).size(), 1);
                ASSERT_EQ(nl->get_nets(test_utils::net_name_filter("net_global_out")).size(), 1);
                ASSERT_EQ(nl->get_nets(test_utils::net_name_filter("net_0_child")).size(), 1);
                Net* net_0 = *nl->get_nets(test_utils::net_name_filter("net_0")).begin();
                Net* net_1 = *nl->get_nets(test_utils::net_name_filter("net_1")).begin();
                Net*
                    net_global_in = *nl->get_nets(test_utils::net_name_filter("net_global_in")).begin();
                Net*
                    net_global_out = *nl->get_nets(test_utils::net_name_filter("net_global_out")).begin();
                Net* net_0_child = *nl->get_nets(test_utils::net_name_filter("net_0_child")).begin();

                // Test that all nets are connected correctly
                EXPECT_EQ(gate_0->get_fan_in_net("I"), net_global_in);
                EXPECT_EQ(gate_0->get_fan_out_net("O"), net_0);
                EXPECT_EQ(gate_1->get_fan_in_net("I"), net_1);
                EXPECT_EQ(gate_1->get_fan_out_net("O"), net_global_out);
                EXPECT_EQ(gate_0_child->get_fan_in_net("I"), net_0);
                EXPECT_EQ(gate_0_child->get_fan_out_net("O"), net_0_child);
                EXPECT_EQ(gate_1_child->get_fan_in_net("I"), net_0_child);
                EXPECT_EQ(gate_1_child->get_fan_out_net("O"), net_1);
                // Check that the attributes of the child entities port are inherit correctly to the connecting Net
                EXPECT_EQ(net_0->get_data("attribute", "child_net_attri"),
                          std::make_tuple("string", "child_net_attribute"));

                // Test that the modules are created and assigned correctly
                Module* top_mod = nl->get_top_module();
                ASSERT_EQ(top_mod->get_submodules().size(), 1);
                Module* child_mod = *top_mod->get_submodules().begin();
                EXPECT_EQ(child_mod->get_name(), "child_mod");
                EXPECT_EQ(top_mod->get_gates(), std::vector<Gate*>({gate_0, gate_1}));
                EXPECT_EQ(child_mod->get_gates(), std::vector<Gate*>({gate_0_child, gate_1_child}));
                EXPECT_EQ(child_mod->get_data("attribute", "child_attri"),
                          std::make_tuple("string", "child_attribute"));
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

                std::stringstream input("-- Device\t: device_name \n"
                                        "entity ENT_CHILD_TWO is "
                                        "  port ( "
                                        "    I_c2 : in STD_LOGIC := 'X'; "
                                        "    O_c2 : out STD_LOGIC := 'X'; "
                                        "  ); "
                                        "end ENT_CHILD_TWO; "
                                        "architecture STRUCTURE_CHILD_TWO of ENT_CHILD_TWO is "
                                        "begin "
                                        "  gate_child_two : gate_1_to_1 "
                                        "    port map ( "
                                        "      I => I_c2, "
                                        "      O => O_c2 "
                                        "    ); "
                                        "end STRUCTURE_CHILD_TWO; "
                                        "  "
                                        "entity ENT_CHILD_ONE is "
                                        "  port ( "
                                        "    I_c1 : in STD_LOGIC := 'X'; "
                                        "    O_c1 : out STD_LOGIC := 'X'; "
                                        "  ); "
                                        "end ENT_ONE; "
                                        "architecture STRUCTURE_CHILD_ONE of ENT_CHILD_ONE is "
                                        "  signal net_child_0 : STD_LOGIC; "
                                        "  signal net_child_1 : STD_LOGIC; "
                                        "begin "
                                        "  gate_0_ent_two : ENT_CHILD_TWO "
                                        "    port map ( "
                                        "      I_c2 => I_c1, "
                                        "      O_c2 => net_child_0 "
                                        "    ); "
                                        "  gate_1_ent_two : ENT_CHILD_TWO "
                                        "    port map ( "
                                        "      I_c2 => net_child_0, "
                                        "      O_c2 => net_child_1 "
                                        "    ); "
                                        "  gate_child_one : gate_1_to_1 "
                                        "    port map ( "
                                        "      I => net_child_1, "
                                        "      O => O_c1 "
                                        "    ); "
                                        "end STRUCTURE_CHILD_ONE; "
                                        "  "
                                        "entity ENT_TOP is "
                                        "  port ( "
                                        "    net_global_in : in STD_LOGIC := 'X'; "
                                        "    net_global_out : out STD_LOGIC := 'X'; "
                                        "  ); "
                                        "end ENT_TOP; "
                                        "architecture STRUCTURE of ENT_TOP is "
                                        "  signal net_0 : STD_LOGIC; "
                                        "  signal net_1 : STD_LOGIC; "
                                        "begin "
                                        "  child_one_mod : ENT_CHILD_ONE "
                                        "    port map ( "
                                        "      I_c1 => net_global_in, "
                                        "      O_c1 => net_0 "
                                        "    ); "
                                        "  child_two_mod : ENT_CHILD_TWO "
                                        "    port map ( "
                                        "      I_c2 => net_0, "
                                        "      O_c2 => net_1 "
                                        "    ); "
                                        "  gate_top : gate_1_to_1 "
                                        "    port map ( "
                                        "      I => net_1, "
                                        "      O => net_global_out "
                                        "    ); "
                                        "end ENT_TOP;");
                test_def::capture_stdout();
                HDLParserVHDL vhdl_parser;
                std::unique_ptr<Netlist> nl = vhdl_parser.parse_and_instantiate(input, m_gl);
                if (nl == nullptr) {
                    std::cout << test_def::get_captured_stdout();
                } else {
                    test_def::get_captured_stdout();
                }

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
                std::stringstream input("-- Device\t: device_name\n"
                                        "entity ENT_MODULE is "
                                        "  port ( "
                                        "    mod_in : in STD_LOGIC := 'X'; "
                                        "    mod_out : out STD_LOGIC := 'X'; "
                                        "  ); "
                                        "end ENT_MODULE; "
                                        "architecture STRUCTURE_MODULE of ENT_MODULE is "
                                        "  signal mod_inner : STD_LOGIC; "
                                        "begin "
                                        "  mod_inner <= mod_out; "
                                        "  gate_a : gate_1_to_1 "
                                        "    port map ( "
                                        "      I => mod_in, "
                                        "      O => mod_inner "
                                        "    ); "
                                        "  gate_b : gate_1_to_1 "
                                        "    port map ( "
                                        "      I => mod_inner "
                                        "    ); "
                                        "end STRUCTURE_MODULE;\n"
                                        "---------------------------------------\n"
                                        "entity ENT_TOP is "
                                        "  port ( "
                                        "    net_global_in : in STD_LOGIC := 'X'; "
                                        "    net_global_out : out STD_LOGIC := 'X'; "
                                        "  ); "
                                        "end ENT_TOP; "
                                        "architecture STRUCTURE_TOP of ENT_TOP is "
                                        "  signal net_0 : STD_LOGIC; "
                                        "begin "
                                        "  mod : ENT_MODULE "
                                        "    port map ( "
                                        "      mod_in => net_global_in, "
                                        "      mod_out => net_0 "
                                        "    ); "
                                        "  gate_top : gate_1_to_1 "
                                        "    port map ( "
                                        "      I => net_0, "
                                        "      O => net_global_out "
                                        "    ); "
                                        "end STRUCTURE_TOP;");
                test_def::capture_stdout();
                HDLParserVHDL vhdl_parser;
                std::unique_ptr<Netlist> nl = vhdl_parser.parse_and_instantiate(input, m_gl);
                if (nl == nullptr) {
                    std::cout << test_def::get_captured_stdout();
                } else {
                    test_def::get_captured_stdout();
                }

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
                std::stringstream input("-- Device\t: device_name\n"
                                        "entity MODULE_A is\n"
                                        "  port (\n"
                                        "    I_A : in STD_LOGIC := 'X';\n"
                                        "    O_A : out STD_LOGIC := 'X';\n"
                                        "  );\n"
                                        "end MODULE_A;\n"
                                        "architecture STRUCT_MODULE_A of MODULE_A is\n"
                                        "  signal shared_net_name : STD_LOGIC;\n"
                                        "  signal net_a : STD_LOGIC;\n"
                                        "begin\n"
                                        "  shared_gate_name : gate_1_to_2\n"
                                        "    port map (\n"
                                        "      I => I_A,\n"
                                        "      O0 => shared_net_name,\n"
                                        "      O1 => net_a\n"
                                        "    );\n"
                                        "  gate_a : gate_2_to_1\n"
                                        "    port map (\n"
                                        "      I0 => shared_net_name,\n"
                                        "      I1 => net_a,\n"
                                        "      O => O_A\n"
                                        "    );\n"
                                        "end STRUCT_MODULE_A;\n"
                                        "-----\n"
                                        "entity MODULE_B is\n"
                                        "  port (\n"
                                        "    I_B : in STD_LOGIC := 'X';\n"
                                        "    O_B : out STD_LOGIC := 'X';\n"
                                        "  );\n"
                                        "end MODULE_B;\n"
                                        "architecture STRUCT_MODULE_B of MODULE_B is\n"
                                        "  signal shared_net_name : STD_LOGIC;\n"
                                        "  signal net_b : STD_LOGIC;\n"
                                        "begin\n"
                                        "  shared_gate_name : gate_1_to_2\n"
                                        "    port map (\n"
                                        "      I => I_B,\n"
                                        "      O0 => shared_net_name,\n"
                                        "      O1 => net_b\n"
                                        "    );\n"
                                        "  gate_b : gate_2_to_1\n"
                                        "    port map (\n"
                                        "      I0 => shared_net_name,\n"
                                        "      I1 => net_b,\n"
                                        "      O => O_B\n"
                                        "    );\n"
                                        "end STRUCT_MODULE_B;\n"
                                        "-----\n"
                                        "entity ENT_TOP is\n"
                                        "  port (\n"
                                        "    net_global_in : in STD_LOGIC := 'X';\n"
                                        "    net_global_out : out STD_LOGIC := 'X';\n"
                                        "  );\n"
                                        "end ENT_TOP;\n"
                                        "architecture STRUCTURE_TOP of ENT_TOP is\n"
                                        "  signal net_0 : STD_LOGIC;\n"
                                        "  signal net_1 : STD_LOGIC;\n"
                                        "  signal net_2 : STD_LOGIC;\n"
                                        "begin\n"
                                        "  mod_b_0 : MODULE_B\n"
                                        "    port map (\n"
                                        "      I_B => net_global_in,\n"
                                        "      O_B => net_0\n"
                                        "    );\n"
                                        "  mod_a_0 : MODULE_A\n"
                                        "    port map (\n"
                                        "      I_A => net_0,\n"
                                        "      O_A => net_1\n"
                                        "    );\n"
                                        "  mod_b_1 : MODULE_B\n"
                                        "    port map (\n"
                                        "      I_B => net_1,\n"
                                        "      O_B => net_2\n"
                                        "    );\n"
                                        "  gate_top : gate_1_to_1\n"
                                        "    port map (\n"
                                        "      I => net_2,\n"
                                        "      O => net_global_out\n"
                                        "    );\n"
                                        "end STRUCTURE_TOP;");
                HDLParserVHDL vhdl_parser;
                std::unique_ptr<Netlist> nl = vhdl_parser.parse_and_instantiate(input, m_gl);

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
     * NOTE: Are multi dim assignments supported?
     * Testing the correct handling of direct assignment (e.g. 'net_slave = net_master;'), where two
     * (signal-)identifiers address the same Net. The Net (signal) at the left side of the expression is mapped to the Net
     * at the right side, so only the Net with the right identifier will be created.
     *
     * However, logic expressions like 'A <= B*C' are NOT supported.
     *
     * Functions: parse
     */
    TEST_F(HDLParserVHDLTest, check_direct_assignment) {
        TEST_START
            {
                // Build up a master-slave hierarchy as follows:
                /*                                  .--- net_slave_1 (is global input)
                 *   net_master <--- net_slave_0 <--+
                 *                                  '--- net_slave_2
                 */
                // Testing the correct creation of the master Net by considering the inheritance of the attributes and connections
                // of its slaves (vhdl specific)
                std::stringstream input("-- Device\t: device_name\n"
                                        "entity TEST_Comp is "
                                        "  port ( "
                                        "    net_global_in : in STD_LOGIC := 'X'; "
                                        "    net_global_out : out STD_LOGIC := 'X'; "
                                        "    net_slave_1: in STD_LOGIC := 'X'; "
                                        "  ); "
                                        "end TEST_Comp; "
                                        "architecture STRUCTURE of TEST_Comp is "
                                        "  signal net_slave_0 : STD_LOGIC; "
                                        "  signal net_slave_2 : STD_LOGIC; "
                                        "  signal net_master : STD_LOGIC; "
                                        "  attribute slave_0_attr : string; "    // <- signal attributes are vhdl specific
                                        "  attribute slave_0_attr of net_slave_0 : signal is \"slave_0_attr\"; "
                                        "  attribute slave_1_attr : string; "
                                        "  attribute slave_1_attr of net_slave_1 : signal is \"slave_1_attr\"; "
                                        "  attribute slave_2_attr : string; "
                                        "  attribute slave_2_attr of net_slave_2 : signal is \"slave_2_attr\"; "
                                        "  attribute master_attr : string; "
                                        "  attribute master_attr of net_master : signal is \"master_attr\"; "
                                        "begin "
                                        "  net_slave_1 <= net_slave_0;  "
                                        "  net_slave_0 <= net_master; "
                                        "  net_slave_2 <= net_slave_0; "
                                        "  gate_0 : gate_1_to_1 "
                                        "    port map ( "
                                        "      I => net_global_in, "
                                        "      O => net_slave_0 "
                                        "    ); "
                                        "  gate_1 : gate_3_to_1 "
                                        "    port map ( "
                                        "      I0 => net_master, "
                                        "      I1 => net_slave_1, "
                                        "      I2 => net_slave_2, "
                                        "      O => net_global_out "
                                        "    ); "
                                        "end STRUCTURE;");
                test_def::capture_stdout();
                HDLParserVHDL vhdl_parser;
                std::unique_ptr<Netlist> nl = vhdl_parser.parse_and_instantiate(input, m_gl);
                if (nl == nullptr) {
                    std::cout << test_def::get_captured_stdout();
                } else {
                    test_def::get_captured_stdout();
                }

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

                // VHDL specific: Check the Net attribute propagation
                EXPECT_EQ(net_master->get_data("attribute", "master_attr"),
                          std::make_tuple("string", "master_attr"));
                EXPECT_EQ(net_master->get_data("attribute", "slave_0_attr"),
                          std::make_tuple("string", "slave_0_attr"));
                EXPECT_EQ(net_master->get_data("attribute", "slave_1_attr"),
                          std::make_tuple("string", "slave_1_attr"));
                EXPECT_EQ(net_master->get_data("attribute", "slave_2_attr"),
                          std::make_tuple("string", "slave_2_attr"));
            }
            {
                //Testing the assignment of logic vectors
                std::stringstream input("-- Device\t: device_name\n"
                                        "entity TEST_Comp is "
                                        "  port ( "
                                        "    net_global_in : in STD_LOGIC := 'X'; "
                                        "    net_global_out : out STD_LOGIC := 'X'; "
                                        "    net_slave_1: out STD_LOGIC := 'X'; "
                                        "  ); "
                                        "end TEST_Comp; "
                                        "architecture STRUCTURE of TEST_Comp is "
                                        "  signal net_slave : STD_LOGIC_VECTOR ( 0 to 3 ); "
                                        "  signal net_master : STD_LOGIC_VECTOR ( 0 to 3 ); "
                                        "begin "
                                        "  net_slave <= net_master; "
                                        "  gate_0 : gate_1_to_1 "
                                        "    port map ( "
                                        "      I => net_global_in, "
                                        "      O => net_slave(0) "
                                        "    ); "
                                        "  gate_1 : gate_3_to_1 "
                                        "    port map ( "
                                        "      I0 => net_master(0), "
                                        "      I1 => net_slave(1), "
                                        "      I2 => net_slave(2), "
                                        "      O => net_global_out "
                                        "    ); "
                                        "end STRUCTURE;");
                test_def::capture_stdout();
                HDLParserVHDL vhdl_parser;
                std::unique_ptr<Netlist> nl = vhdl_parser.parse_and_instantiate(input, m_gl);
                if (nl == nullptr) {
                    std::cout << test_def::get_captured_stdout();
                } else {
                    test_def::get_captured_stdout();
                }

                ASSERT_NE(nl, nullptr);

                ASSERT_EQ(nl->get_gates(test_utils::gate_name_filter("gate_0")).size(), 1);
                ASSERT_EQ(nl->get_gates(test_utils::gate_name_filter("gate_1")).size(), 1);
                Gate* gate_0 = *nl->get_gates(test_utils::gate_name_filter("gate_0")).begin();
                Gate* gate_1 = *nl->get_gates(test_utils::gate_name_filter("gate_1")).begin();

                ASSERT_NE(gate_0->get_fan_out_net("O"), nullptr);
                ASSERT_NE(gate_1->get_fan_in_net("I0"), nullptr);
                ASSERT_NE(gate_1->get_fan_in_net("I1"), nullptr);
                ASSERT_NE(gate_1->get_fan_in_net("I2"), nullptr);

                EXPECT_EQ(gate_0->get_fan_out_net("O")->get_name(), "net_master(0)");
                EXPECT_EQ(gate_1->get_fan_in_net("I0")->get_name(), "net_master(0)");
                EXPECT_EQ(gate_1->get_fan_in_net("I1")->get_name(), "net_master(1)");
                EXPECT_EQ(gate_1->get_fan_in_net("I2")->get_name(), "net_master(2)");

            }
        TEST_END
    }

    /**
     * Testing the port assignment of multiple pins and nets using pin groups and signal vectors
     *
     * Functions: parse
     */
    TEST_F(HDLParserVHDLTest, check_pin_group_port_assignment) {

        TEST_START
            {
                // Connect an entire output pin group with global input nets by using a binary string (B"0101")
                std::stringstream input("-- Device\t: device_name\n"
                                        "entity TEST_Comp is\n"
                                        "  port (\n"
                                        "  );\n"
                                        "end TEST_Comp;\n"
                                        "architecture STRUCTURE of TEST_Comp is\n"
                                        "begin\n"
                                        "  gate_0 : pin_group_gate_4_to_4\n"
                                        "    port map (\n"
                                        "      I => B\"0101\"\n"
                                        "    );\n"
                                        "end STRUCTURE;");
                test_def::capture_stdout();
                HDLParserVHDL vhdl_parser;
                std::unique_ptr<Netlist> nl = vhdl_parser.parse_and_instantiate(input, m_gl);
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
                EXPECT_EQ(net_0->get_name(), "'0'");

                Net* net_1 = gate_0->get_fan_in_net("I(1)");
                ASSERT_NE(net_1, nullptr);
                EXPECT_EQ(net_1->get_name(), "'1'");

                Net* net_2 = gate_0->get_fan_in_net("I(2)");
                ASSERT_NE(net_2, nullptr);
                EXPECT_EQ(net_2->get_name(), "'0'");

                Net* net_3 = gate_0->get_fan_in_net("I(3)");
                ASSERT_NE(net_3, nullptr);
                EXPECT_EQ(net_3->get_name(), "'1'");
            }
            {
                // Connect a vector of output pins with a vector of nets (O(0) with l_vec(0),...,O(4) with l_vec(4))
                std::stringstream input("-- Device\t: device_name\n"
                                        "entity TEST_Comp is\n"
                                        "  port (\n"
                                        "  );\n"
                                        "end TEST_Comp;\n"
                                        "architecture STRUCTURE of TEST_Comp is\n"
                                        "  signal l_vec : STD_LOGIC_VECTOR ( 0 to 3 );\n"
                                        "begin\n"
                                        "  gate_0 : pin_group_gate_4_to_4\n"
                                        "    port map (\n"
                                        "      O => l_vec\n"
                                        "    );\n"
                                        "end STRUCTURE;");
                test_def::capture_stdout();
                HDLParserVHDL vhdl_parser;
                std::unique_ptr<Netlist> nl = vhdl_parser.parse_and_instantiate(input, m_gl);
                if (nl == nullptr) {
                    std::cout << test_def::get_captured_stdout();
                } else {
                    test_def::get_captured_stdout();
                }

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
            // ---- Vhdl specific: Pin ranges  ----
            {
                // Connect a vector of output pins with a vector of nets by using the to and the downto statment
                std::stringstream input("-- Device\t: device_name\n"
                                        "entity TEST_Comp is\n"
                                        "  port (\n"
                                        "  );\n"
                                        "end TEST_Comp;\n"
                                        "architecture STRUCTURE of TEST_Comp is\n"
                                        "  signal l_vec : STD_LOGIC_VECTOR ( 0 to 3 );\n"
                                        "begin\n"
                                        "  gate_0 : pin_group_gate_4_to_4\n"
                                        "    port map (\n"
                                        "      O(3 downto 2) => l_vec(1 to 2)\n"
                                        "    );\n"
                                        "end STRUCTURE;");
                test_def::capture_stdout();
                HDLParserVHDL vhdl_parser;
                std::unique_ptr<Netlist> nl = vhdl_parser.parse_and_instantiate(input, m_gl);
                if (nl == nullptr) {
                    std::cout << test_def::get_captured_stdout();
                } else {
                    test_def::get_captured_stdout();
                }

                ASSERT_NE(nl, nullptr);
                ASSERT_FALSE(nl->get_gates(test_utils::gate_filter("pin_group_gate_4_to_4", "gate_0")).empty());
                Gate* gate_0 = *(nl->get_gates(test_utils::gate_filter("pin_group_gate_4_to_4", "gate_0")).begin());

                EXPECT_EQ(gate_0->get_fan_out_nets().size(), 2);

                Net* net_1 = gate_0->get_fan_out_net("O(3)");
                ASSERT_NE(net_1, nullptr);
                EXPECT_EQ(net_1->get_name(), "l_vec(1)");

                Net* net_2 = gate_0->get_fan_out_net("O(2)");
                ASSERT_NE(net_2, nullptr);
                EXPECT_EQ(net_2->get_name(), "l_vec(2)");
            }
            {
                // Connect a vector of output pins with a vector of nets by using the to and the downto statment (flipped around)
                std::stringstream input("-- Device\t: device_name\n"
                                        "entity TEST_Comp is\n"
                                        "  port (\n"
                                        "  );\n"
                                        "end TEST_Comp;\n"
                                        "architecture STRUCTURE of TEST_Comp is\n"
                                        "  signal l_vec : STD_LOGIC_VECTOR ( 0 to 3 );\n"
                                        "begin\n"
                                        "  gate_0 : pin_group_gate_4_to_4\n"
                                        "    port map (\n"
                                        "      O(2 to 3) => l_vec(2 downto 1)\n"
                                        "    );\n"
                                        "end STRUCTURE;");
                test_def::capture_stdout();
                HDLParserVHDL vhdl_parser;
                std::unique_ptr<Netlist> nl = vhdl_parser.parse_and_instantiate(input, m_gl);
                if (nl == nullptr) {
                    std::cout << test_def::get_captured_stdout();
                } else {
                    test_def::get_captured_stdout();
                }

                ASSERT_NE(nl, nullptr);
                ASSERT_FALSE(nl->get_gates(test_utils::gate_filter("pin_group_gate_4_to_4", "gate_0")).empty());
                Gate* gate_0 = *(nl->get_gates(test_utils::gate_filter("pin_group_gate_4_to_4", "gate_0")).begin());

                EXPECT_EQ(gate_0->get_fan_out_nets().size(), 2);

                Net* net_1 = gate_0->get_fan_out_net("O(2)");
                ASSERT_NE(net_1, nullptr);
                EXPECT_EQ(net_1->get_name(), "l_vec(2)");

                Net* net_2 = gate_0->get_fan_out_net("O(3)");
                ASSERT_NE(net_2, nullptr);
                EXPECT_EQ(net_2->get_name(), "l_vec(1)");
            }
        TEST_END
    }

    /*#########################################################################
       VHDL Specific Tests (Tests that can not be directly applied to Verilog)
      #########################################################################*/

    /**
     * Testing the correct detection of single line comments (initiated with '--') within the vhdl file.
     *
     * Functions: parse
     */
    TEST_F(HDLParserVHDLTest, check_comments) {

        TEST_START
            {
                // Store an instance of all possible data types in one Gate
                std::stringstream input("-- Device\t: device_name\n"
                                        "entity TEST_Comp is "
                                        "  port ( "
                                        "    net_global_input : in STD_LOGIC := 'X'; "
                                        "  ); "
                                        "end TEST_Comp; "
                                        "architecture STRUCTURE of TEST_Comp is "
                                        "begin"
                                        "  test_gate : gate_1_to_1"
                                        "    generic map(\n"
                                        "      no_comment_0 => 123, -- comment_0 => 123, \t --comment_1 => 123\n"
                                        "      no_comment_1 => 123,\n"
                                        "      -- comment_2 => 123,\n"
                                        "      no_comment_2 => 123\n"
                                        "    )\n"
                                        "    port map ( "
                                        "      I => net_global_input "
                                        "    ); "
                                        "end STRUCTURE;");
                test_def::capture_stdout();
                HDLParserVHDL vhdl_parser;
                std::unique_ptr<Netlist> nl = vhdl_parser.parse_and_instantiate(input, m_gl);
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
                for (std::string key : std::set<std::string>({"no_comment_0", "no_comment_1", "no_comment_2"})) {
                    EXPECT_NE(test_gate->get_data("generic", key), std::make_tuple("", ""));
                    if (test_gate->get_data("generic", key) == std::make_tuple("", "")) {
                        std::cout << "comment test failed for: " << key << std::endl;
                    }
                }

                // Test that the comments are not interpreted (all comment_n generics shouldn't be created)
                for (std::string key : std::set<std::string>({"comment_0", "comment_1", "comment_2"})) {
                    EXPECT_EQ(test_gate->get_data("generic", key), std::make_tuple("", ""));
                    if (test_gate->get_data("generic", key) != std::make_tuple("", "")) {
                        std::cout << "comment failed for: " << key << std::endl;
                    }
                }
            }
        TEST_END
    }

    /**
     * Testing the usage of user-defined attributes within the architecture and the entity header.
     *
     * Functions: parse
     */
    TEST_F(HDLParserVHDLTest, check_attributes) {

        TEST_START
            {
                // Add a custom attribute for a Gate
                std::stringstream input("-- Device\t: device_name\n"
                                        "entity TEST_Comp is\n"
                                        "  port (\n"
                                        "    net_global_in : in STD_LOGIC := 'X';\n"
                                        "    net_global_out : out STD_LOGIC := 'X';\n"
                                        "  );\n"
                                        "end TEST_Comp;\n"
                                        "architecture STRUCTURE of TEST_Comp is\n"
                                        "  attribute attri_name : attri_type;\n"
                                        "  attribute attri_name of gate_0 : label is attri_value;\n"
                                        "begin\n"
                                        "  gate_0 : gate_1_to_1\n"
                                        "    port map (\n"
                                        "      I => net_global_in,\n"
                                        "      O => net_global_out\n"
                                        "    );\n"
                                        "end STRUCTURE;");
                HDLParserVHDL vhdl_parser;
                std::unique_ptr<Netlist> nl = vhdl_parser.parse_and_instantiate(input, m_gl);

                ASSERT_NE(nl, nullptr);
                ASSERT_EQ(nl->get_gates(test_utils::gate_type_filter("gate_1_to_1")).size(), 1);
                Gate* attri_gate = *nl->get_gates(test_utils::gate_type_filter("gate_1_to_1")).begin();
                EXPECT_EQ(attri_gate->get_data("attribute", "attri_name"),
                          std::make_tuple("attri_type", "attri_value"));
            }
            {
                // Add a custom attribute for a Net
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
                                        "  gate_0 : gate_1_to_1\n"
                                        "    port map (\n"
                                        "      I => net_global_in,\n"
                                        "      O => net_0\n"
                                        "    );\n"
                                        "  gate_1 : gate_1_to_1\n"
                                        "    port map (\n"
                                        "      I => net_0,\n"
                                        "      O => net_global_out\n"
                                        "    );\n"
                                        "end STRUCTURE;");
                HDLParserVHDL vhdl_parser;
                std::unique_ptr<Netlist> nl = vhdl_parser.parse_and_instantiate(input, m_gl);

                ASSERT_NE(nl, nullptr);
                ASSERT_EQ(nl->get_nets(test_utils::net_name_filter("net_0")).size(), 1);
                Net* attri_net = *nl->get_nets(test_utils::net_name_filter("net_0")).begin();
                EXPECT_NE(attri_net, nullptr);
                EXPECT_EQ(attri_net->get_data("attribute", "attri_name"),
                          std::make_tuple("attri_type", "attri_value"));
            }
            {
                // Use atrribute strings with special characters (',','.')
                std::stringstream input("-- Device\t: device_name\n"
                                        "entity TEST_Comp is\n"
                                        "  port (\n"
                                        "    net_global_in : in STD_LOGIC := 'X';\n"
                                        "    net_global_out : out STD_LOGIC := 'X';\n"
                                        "  );\n"
                                        "end TEST_Comp;\n"
                                        "architecture STRUCTURE of TEST_Comp is\n"
                                        "  signal net_0 : STD_LOGIC;\n"
                                        "  attribute attri_comma_string : attri_type_0;\n"
                                        "  attribute attri_comma_string of net_0 : signal is \"test, 1, 2, 3\";\n"
                                        "  attribute attri_float_string : attri_type_1;\n"
                                        "  attribute attri_float_string of gate_0 : label is \"1.234\";\n"
                                        "begin\n"
                                        "  gate_0 : gate_1_to_1\n"
                                        "    port map (\n"
                                        "      I => net_global_in,\n"
                                        "      O => net_0\n"
                                        "    );\n"
                                        "  gate_1 : gate_1_to_1\n"
                                        "    port map (\n"
                                        "      I => net_0,\n"
                                        "      O => net_global_out\n"
                                        "    );\n"
                                        "end STRUCTURE;");
                HDLParserVHDL vhdl_parser;
                std::unique_ptr<Netlist> nl = vhdl_parser.parse_and_instantiate(input, m_gl);

                ASSERT_NE(nl, nullptr);
                ASSERT_EQ(nl->get_nets(test_utils::net_name_filter("net_0")).size(), 1);
                Net* attri_net = *nl->get_nets(test_utils::net_name_filter("net_0")).begin();
                EXPECT_NE(attri_net, nullptr);

                ASSERT_EQ(nl->get_gates(test_utils::gate_name_filter("gate_0")).size(), 1);
                Gate* attri_gate = *nl->get_gates(test_utils::gate_name_filter("gate_0")).begin();

                // Check the attributes
                EXPECT_EQ(attri_net->get_data("attribute", "attri_comma_string"),
                          std::make_tuple("attri_type_0", "test, 1, 2, 3"));
                EXPECT_EQ(attri_gate->get_data("attribute", "attri_float_string"),
                          std::make_tuple("attri_type_1", "1.234"));
            }
        TEST_END
    }

    /**
     * Testing the usage and the correct handling of library prefixes
     *
     * Functions: parse
     */
    TEST_F(HDLParserVHDLTest, check_lib_prefix) {
        TEST_START
            // The prefix of the library SIMPRIM.VCOMPONENTS should be removed from the INV Gate type
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
                                    "  gate_0 : SIMPRIM.VCOMPONENTS.gate_1_to_1\n"
                                    "    port map (\n"
                                    "      I => net_global_input\n"
                                    "    );\n"
                                    "end STRUCTURE;");
            test_def::capture_stdout();
            HDLParserVHDL vhdl_parser;
            std::unique_ptr<Netlist> nl = vhdl_parser.parse_and_instantiate(input, m_gl);
            if (nl == nullptr) {
                std::cout << test_def::get_captured_stdout();
            } else {
                test_def::get_captured_stdout();
            }

            ASSERT_NE(nl, nullptr);

            EXPECT_EQ(nl->get_gates(test_utils::gate_type_filter("gate_1_to_1")).size(), 1);
        TEST_END
    }

    /** NOTE: Currently Unsupported...
     * Testing the usage of components, which should define new Gate types with custom input/output/inout pins.
     * (currently unsupported...)
     *
     * Functions: parse
     */
    TEST_F(HDLParserVHDLTest, check_component) {

        TEST_START
            /*{ // NOTE: Currently Unsupported
                // Add a component in the architecture header and instantiate a Gate of it
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
                HDLParserVHDL vhdl_parser;
                std::unique_ptr<Netlist> nl = vhdl_parser.parse_and_instantiate(g_lib_name);
                if (nl == nullptr) {
                    std::cout << test_def::get_captured_stdout();
                } else {
                    test_def::get_captured_stdout();
                }
                ASSERT_NE(nl, nullptr);
                ASSERT_EQ(nl->get_gates(gate_type_filter("COMP_GATE")).size(), 1);
                Gate* comp_gate = *nl->get_gates(gate_type_filter("COMP_GATE")).begin();
                EXPECT_NE(comp_gate->get_fan_in_net("COMP_IN"), nullptr);
                EXPECT_NE(comp_gate->get_fan_out_net("COMP_OUT"), nullptr);
                // EXPECT_EQ(comp_gate->get_inout_pin_types(), std::vector<std::string>({"COMP_INOUT"})); FIXME
            }*/

        TEST_END
    }

    /**
     * Testing the correct handling of invalid input
     *
     * Functions: parse
     */
    TEST_F(HDLParserVHDLTest, check_invalid_input) {
        TEST_START
            // ------ Tests that are present in booth parsers ------
            {
                // Try to connect to a pin, that does not exist
                NO_COUT_TEST_BLOCK;
                std::stringstream input("-- Device\t: device_name\n"
                                        "entity TEST_Comp is\n"
                                        "  port (\n"
                                        "    global_in: in STD_LOGIC := 'X';\n"
                                        "  );\n"
                                        "end TEST_Comp;\n"
                                        "architecture STRUCTURE of TEST_Comp is\n"
                                        "begin\n"
                                        "  gate_0 : gate_1_to_1\n"
                                        "    port map (\n"
                                        "      NON_EXISTING_PIN => global_in\n"
                                        "    );\n"
                                        "end STRUCTURE;");
                HDLParserVHDL vhdl_parser;
                std::unique_ptr<Netlist> nl = vhdl_parser.parse_and_instantiate(input, m_gl);
                ASSERT_EQ(nl, nullptr);
            }
            {
                // Use an unknown Gate type (not in Gate library)
                NO_COUT_TEST_BLOCK;
                std::stringstream input("-- Device\t: device_name\n"
                                        "entity TEST_Comp is\n"
                                        "  port (\n"
                                        "    net_global_in : in STD_LOGIC := 'X';\n"
                                        "    net_global_out : out STD_LOGIC := 'X';\n"
                                        "  );\n"
                                        "end TEST_Comp;\n"
                                        "architecture STRUCTURE of TEST_Comp is\n"
                                        "begin\n"
                                        "  gate_0 : UNKNOWN_GATE_TYPE\n"
                                        "    port map (\n"
                                        "      I => net_global_in,\n"
                                        "      O => net_global_out\n"
                                        "    );\n"
                                        "end STRUCTURE;");
                HDLParserVHDL vhdl_parser;
                std::unique_ptr<Netlist> nl = vhdl_parser.parse_and_instantiate(input, m_gl);

                EXPECT_EQ(nl, nullptr);
            }
            {
                // The input does not contain any entity (is empty)
                NO_COUT_TEST_BLOCK;
                std::stringstream input("");
                HDLParserVHDL vhdl_parser;
                std::unique_ptr<Netlist> nl = vhdl_parser.parse_and_instantiate(input, m_gl);

                EXPECT_EQ(nl, nullptr);
            }
            {
                // Create a non-used entity (should not create any problems...)
                NO_COUT_TEST_BLOCK;
                std::stringstream input("-- Device\t: device_name\n"
                                        "entity IGNORE_ME is\n"
                                        "  port (\n"
                                        "    min : in STD_LOGIC := 'X';\n"
                                        "    mout : out STD_LOGIC := 'X';\n"
                                        "  );\n"
                                        "end TEST_Comp;\n"
                                        "architecture STRUCTURE of IGNORE_ME is\n"
                                        "begin\n"
                                        "  gate_0 : gate_1_to_1\n"
                                        "    port map (\n"
                                        "      I => min,\n"
                                        "      O => mout\n"
                                        "    );\n"
                                        "end STRUCTURE;\n"
                                        ""
                                        "entity TEST_Comp is\n"
                                        "  port (\n"
                                        "    net_global_in : in STD_LOGIC := 'X';\n"
                                        "    net_global_out : out STD_LOGIC := 'X';\n"
                                        "  );\n"
                                        "end TEST_Comp;\n"
                                        "architecture STRUCTURE of TEST_Comp is\n"
                                        "begin\n"
                                        "  gate_0 : gate_1_to_1\n"
                                        "    port map (\n"
                                        "      I => net_global_in,\n"
                                        "      O => net_global_out\n"
                                        "    );\n"
                                        "end STRUCTURE;");
                HDLParserVHDL vhdl_parser;
                std::unique_ptr<Netlist> nl = vhdl_parser.parse_and_instantiate(input, m_gl);

                EXPECT_NE(nl, nullptr);
            }
            {
                // Create a cyclic master-slave Net hierarchy
                NO_COUT_TEST_BLOCK;
                std::stringstream input("-- Device\t: device_name\n"
                                        "entity TEST_Comp is "
                                        "  port ( "
                                        "    net_global_in : in STD_LOGIC := 'X'; "
                                        "    net_global_out : out STD_LOGIC := 'X'; "
                                        "  ); "
                                        "end TEST_Comp; "
                                        "architecture STRUCTURE of TEST_Comp is "
                                        "  signal net_0 : STD_LOGIC; "
                                        "  signal net_1 : STD_LOGIC; "
                                        "begin "
                                        "  net_0 <= net_1;  "
                                        "  net_1 <= net_0; "
                                        "  gate_0 : gate_1_to_1 "
                                        "    port map ( "
                                        "      I => net_global_in, "
                                        "      O => net_0 "
                                        "    ); "
                                        "  gate_1 : gate_1_to_1 "
                                        "    port map ( "
                                        "      I => net_1, "
                                        "      O => net_global_out "
                                        "    ); "
                                        "end STRUCTURE;");
                HDLParserVHDL vhdl_parser;
                std::unique_ptr<Netlist> nl = vhdl_parser.parse_and_instantiate(input, m_gl);

                EXPECT_EQ(nl, nullptr);
            }
            if(test_utils::known_issue_tests_active())
            {
                    // Use non-numeric ranges (invalid) (ISSUE: stoi failure l.827)
                    std::stringstream input("-- Device\t: device_name\n"
                                            "entity TEST_Comp is\n"
                                            "  port (\n"
                                            "  );\n"
                                            "end TEST_Comp;\n"
                                            "architecture STRUCTURE of TEST_Comp is\n"
                                            "  signal l_vec : STD_LOGIC_VECTOR ( 4 downto 0 );\n"
                                            "begin\n"
                                            "  gate_0 : pin_group_gate_4_to_4\n"
                                            "    port map (\n"
                                            "      O(p downto q) => l_vec(p downto q)\n" // <- fails booth independently (l.827)
                                            "    );\n"
                                            "end STRUCTURE;");
                    HDLParserVHDL vhdl_parser;
                    std::unique_ptr<Netlist> nl = vhdl_parser.parse_and_instantiate(input, m_gl);
                    EXPECT_EQ(nl, nullptr);
            }
            // ------ VHDL specific tests ------
            {
                // The entity contains unknown direction keywords (neither 'in', 'out' nor 'inout')
                NO_COUT_TEST_BLOCK;
                std::stringstream input("-- Device\t: device_name\n"
                                        "entity TEST_Comp is\n"
                                        "  port (\n"
                                        "    net_global_input : invalid_direction STD_LOGIC := 'X';\n"    // <- invalid direction
                                        "  );\n"
                                        "end TEST_Comp;\n"
                                        "architecture STRUCTURE of TEST_Comp is\n"
                                        "begin\n"
                                        "  gate_0 : gate_1_to_1\n"
                                        "    port map (\n"
                                        "      O => net_global_input\n"
                                        "    );\n"
                                        "end STRUCTURE;");
                HDLParserVHDL vhdl_parser;
                std::unique_ptr<Netlist> nl = vhdl_parser.parse_and_instantiate(input, m_gl);

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
                                        "unknown_keyword some_signal : STD_LOGIC;"    // <- invalid keyword
                                        "begin\n"
                                        "  gate_0 : gate_1_to_1\n"
                                        "    port map (\n"
                                        "      O => net_global_input\n"
                                        "    );\n"
                                        "end STRUCTURE;");
                HDLParserVHDL vhdl_parser;
                std::unique_ptr<Netlist> nl = vhdl_parser.parse_and_instantiate(input, m_gl);

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
                                        "  gate_0 : gate_1_to_1\n"
                                        "    generic map(\n"
                                        "      key_invalid_type => Inv4lid_type\n"    // <- The format of 'Inv4lid_type' matches with no data_type
                                        "    )\n"
                                        "    port map (\n"
                                        "      I => net_global_input\n"
                                        "    );\n"
                                        "end STRUCTURE;");
                HDLParserVHDL vhdl_parser;
                std::unique_ptr<Netlist> nl = vhdl_parser.parse_and_instantiate(input, m_gl);
                EXPECT_EQ(nl, nullptr);
            }
            {
                // Leave the 'port map' block empty (Gate is not connected)
                NO_COUT_TEST_BLOCK;
                std::stringstream input("-- Device\t: device_name\n"
                                        "entity TEST_Comp is\n"
                                        "  port (\n"
                                        "    net_global_input : in STD_LOGIC := 'X';\n"
                                        "  );\n"
                                        "end TEST_Comp;\n"
                                        "architecture STRUCTURE of TEST_Comp is\n"
                                        "begin\n"
                                        "  gate_0 : gate_1_to_1\n"
                                        "    port map (\n"
                                        "    );\n"
                                        "end STRUCTURE;");
                HDLParserVHDL vhdl_parser;
                std::unique_ptr<Netlist> nl = vhdl_parser.parse_and_instantiate(input, m_gl);
                ASSERT_NE(nl, nullptr);
                ASSERT_FALSE(nl->get_gates(test_utils::gate_filter("gate_1_to_1", "gate_0")).empty());
                Gate* gate_0 = *nl->get_gates(test_utils::gate_filter("gate_1_to_1", "gate_0")).begin();
                EXPECT_TRUE(gate_0->get_fan_out_nets().empty());
                EXPECT_TRUE(gate_0->get_fan_in_nets().empty());
            }
            {
                // Use an undeclared attribute (an attribute of type 'unknown' should be created)
                NO_COUT_TEST_BLOCK;
                std::stringstream input("-- Device\t: device_name\n"
                                        "entity TEST_Comp is\n"
                                        "  port (\n"
                                        "    net_global_in : in STD_LOGIC := 'X';\n"
                                        "    net_global_out : out STD_LOGIC := 'X';\n"
                                        "  );\n"
                                        "end TEST_Comp;\n"
                                        "architecture STRUCTURE of TEST_Comp is\n"
                                        "  attribute attri_name of gate_0 : label is attri_value;\n"
                                        "begin\n"
                                        "  gate_0 : gate_1_to_1\n"
                                        "    port map (\n"
                                        "      I => net_global_in,\n"
                                        "      O => net_global_out\n"
                                        "    );\n"
                                        "end STRUCTURE;");
                HDLParserVHDL vhdl_parser;
                std::unique_ptr<Netlist> nl = vhdl_parser.parse_and_instantiate(input, m_gl);

                EXPECT_NE(nl, nullptr);
                ASSERT_EQ(nl->get_gates(test_utils::gate_type_filter("gate_1_to_1")).size(), 1);
                Gate* attri_gate = *nl->get_gates(test_utils::gate_type_filter("gate_1_to_1")).begin();
                EXPECT_EQ(attri_gate->get_data("attribute", "attri_name"),
                          std::make_tuple("unknown", "attri_value"));
            }
            {
                // Use the 'attribute'-keyword in an unexpected way
                NO_COUT_TEST_BLOCK;
                std::stringstream input("-- Device\t: device_name\n"
                                        "entity TEST_Comp is\n"
                                        "  port (\n"
                                        "    net_global_in : in STD_LOGIC := 'X';\n"
                                        "    net_global_out : out STD_LOGIC := 'X';\n"
                                        "  );\n"
                                        "end TEST_Comp;\n"
                                        "architecture STRUCTURE of TEST_Comp is\n"
                                        "  attribute WAMBO;\n"    // <- attributes do not work like this
                                        "begin\n"
                                        "  gate_0 : gate_1_to_1\n"
                                        "    port map (\n"
                                        "      I => net_global_in,\n"
                                        "      O => net_global_out\n"
                                        "    );\n"
                                        "end STRUCTURE;");
                HDLParserVHDL vhdl_parser;
                std::unique_ptr<Netlist> nl = vhdl_parser.parse_and_instantiate(input, m_gl);

                EXPECT_EQ(nl, nullptr);
            }
            {
                // The amount of bounds does not match with the vector dimension (vhdl specific)
                NO_COUT_TEST_BLOCK;
                std::stringstream input("-- Device\t: device_name\n"
                                        "entity TEST_Comp is\n"
                                        "  port (\n"
                                        "    net_global_input : in STD_LOGIC := 'X';\n"
                                        "  );\n"
                                        "end TEST_Comp;\n"
                                        "architecture STRUCTURE of TEST_Comp is\n"
                                        "  signal n_vec : STD_LOGIC_VECTOR3 ( 0 to 1, 0 to 1);\n"    // <- two bounds, but dimension 3
                                        "begin\n"
                                        "  gate_0 : gate_1_to_1\n"
                                        "    port map (\n"
                                        "      O => net_global_input\n"
                                        "    );\n"
                                        "end STRUCTURE;");
                HDLParserVHDL vhdl_parser;
                std::unique_ptr<Netlist> nl = vhdl_parser.parse_and_instantiate(input, m_gl);
                if (nl != nullptr) {
                    EXPECT_EQ(nl->get_nets().size(), 1);
                }
            }
            {
                // The ranges of the pin vectors do not match in size
                std::stringstream input("-- Device\t: device_name\n"
                                        "entity TEST_Comp is\n"
                                        "  port (\n"
                                        "  );\n"
                                        "end TEST_Comp;\n"
                                        "architecture STRUCTURE of TEST_Comp is\n"
                                        "  signal l_vec : STD_LOGIC_VECTOR ( 0 to 3 );\n"
                                        "begin\n"
                                        "  gate_0 : pin_group_gate_4_to_4\n"
                                        "    port map (\n"
                                        "      O(0 to 2) => l_vec(0 to 3)\n"
                                        "    );\n"
                                        "end STRUCTURE;");
                test_def::capture_stdout();
                HDLParserVHDL vhdl_parser;
                std::unique_ptr<Netlist> nl = vhdl_parser.parse_and_instantiate(input, m_gl);
                if (nl == nullptr)
                {
                    std::cout << test_def::get_captured_stdout();
                }
                else
                {
                    test_def::get_captured_stdout();
                }

                ASSERT_NE(nl, nullptr);
                ASSERT_FALSE(nl->get_gates(test_utils::gate_filter("pin_group_gate_4_to_4", "gate_0")).empty());
                Gate* gate_0 = *(nl->get_gates(test_utils::gate_filter("pin_group_gate_4_to_4", "gate_0")).begin());
                EXPECT_EQ(gate_0->get_fan_out_nets().size(), 3);
                EXPECT_EQ(gate_0->get_fan_out_endpoint("O(0)")->get_net()->get_name(), "l_vec(0)");
                EXPECT_EQ(gate_0->get_fan_out_endpoint("O(1)")->get_net()->get_name(), "l_vec(1)");
                EXPECT_EQ(gate_0->get_fan_out_endpoint("O(2)")->get_net()->get_name(), "l_vec(2)");
            }
            {
                // The right side of a pin assignment does no match any vector format
                NO_COUT_TEST_BLOCK;
                std::stringstream input("-- Device\t: device_name\n"
                                        "entity TEST_Comp is\n"
                                        "  port (\n"
                                        "  );\n"
                                        "end TEST_Comp;\n"
                                        "architecture STRUCTURE of TEST_Comp is\n"
                                        "begin\n"
                                        "  gate_0 : pin_group_gate_4_to_4\n"
                                        "    port map (\n"
                                        "      I(1 to 3) => Unkn0wn_Format\n"    // <- unknown format
                                        "    );\n"
                                        "end STRUCTURE;");
                HDLParserVHDL vhdl_parser;
                std::unique_ptr<Netlist> nl = vhdl_parser.parse_and_instantiate(input, m_gl);

                ASSERT_EQ(nl, nullptr);
            }
        TEST_END
    }
} //namespace hal
