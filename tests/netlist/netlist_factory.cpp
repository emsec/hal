#include "netlist/netlist_factory.h"
#include "core/program_arguments.h"
#include "core/utils.h"
#include "netlist/gate_library/gate_library_manager.h"
#include "netlist/netlist.h"
#include "netlist/persistent/netlist_serializer.h"
#include "netlist_test_utils.h"
#include "gtest/gtest.h"
#include <experimental/filesystem>
#include <core/log.h>
#include <fstream>
#include <iostream>

/*
 * In this file aren't the tests for the various parsers, however it uses the vhdl parser as an example. If there occure
 * any errors, it can be an issue of the vhdl parser as well...
 */

using namespace test_utils;

class netlist_factory_test : public ::testing::Test
{
protected:
    hal::path m_g_lib_path;
    const std::string m_min_gl_content =     "library (MIN_TEST_GATE_LIBRARY) {\n"
                                             "    define(cell);\n"
                                             "    cell(gate_1_to_1) {\n"
                                             "        pin(I) { direction: input; }\n"
                                             "        pin(O) { direction: output; } }\n"
                                             "    cell(gnd) {\n"
                                             "        pin(O) { direction: output; function: \"0\"; } }\n"
                                             "    cell(vcc) {\n"
                                             "        pin(O) { direction: output; function: \"1\"; } }\n"
                                             "}";

    virtual void SetUp()
    {
        test_utils::init_log_channels();
        create_sandbox_directory();
        m_g_lib_path = create_sandbox_file("min_test_gate_lib.lib", m_min_gl_content);
    }

    virtual void TearDown()
    {
        remove_sandbox_directory();
    }

    /**
     * Check if a gate_library with name lib_name can be loaded. Print an error message
     * if print_error is true and the library can't be loaded
     */
    bool gate_library_exists(std::string lib_name, bool print_error = true)
    {
        std::shared_ptr<gate_library> gLib;
        {
            NO_COUT_BLOCK;
            gLib = gate_library_manager::get_gate_library(g_lib_name);
        }
        if (gLib == nullptr)
        {
            if (print_error)
            {
                std::cout << "Warning: Gate library " << lib_name << " can't be loaded. "
                          << "Some tests are skipped!" << std::endl;
            }
            return false;
        }
        else
        {
            return true;
        }
    }
};

/**
 * Testing the creation of an empty netlist by passing a library name.
 *
 * Functions: create_netlist(gate_library_name)
 */
TEST_F(netlist_factory_test, check_create_netlist_by_lib_name)
{
    TEST_START
        if (gate_library_exists(g_lib_name))
        {
            {
                std::shared_ptr<netlist> nl = netlist_factory::create_netlist(get_testing_gate_library());
                EXPECT_EQ(nl->get_gate_library()->get_name(), get_testing_gate_library()->get_name());
            }
        }
        // NEGATIVE
        {
            // Try to create a netlist by passing a nullptr
            NO_COUT_TEST_BLOCK;
            std::shared_ptr<netlist> nl = netlist_factory::create_netlist(nullptr);
            // ISSUE: should be nullptr
            //EXPECT_EQ(nl, nullptr);
            // ISSUE: if nl != nullptr, the following expression leads to a segfault:
            //nl->create_gate( 815, get_testing_gate_library()->get_gate_types().begin()->second, "dont_crush");
        }
    TEST_END
}

/**
 * Testing the creation of an empty netlist by passing an hdl file path
 *
 * Functions: load_netlist(hdl_file, ...)
 */
TEST_F(netlist_factory_test, check_load_netlist_by_hdl_file)
{
    TEST_START
        // Create a netlist by a temporary created vhdl file
        hal::path tmp_hdl_file_path = create_sandbox_file("nl_factory_test_file.vhdl",
                                                          "-- Device\t: device_name\n"
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
        {
            std::shared_ptr<netlist> nl = netlist_factory::load_netlist(tmp_hdl_file_path, "vhdl", m_g_lib_path);

            ASSERT_NE(nl, nullptr);
            EXPECT_EQ(nl->get_gate_library()->get_name(), "MIN_TEST_GATE_LIBRARY");

        }
        {
            // Try to create a netlist by a non-accessible (non-existing) file
            NO_COUT_TEST_BLOCK;
            std::shared_ptr<netlist> nl = netlist_factory::load_netlist(hal::path("/this/file/does/not/exist"), "vhdl", m_g_lib_path);

            EXPECT_EQ(nl, nullptr);
        }
        {
            // Try to create a netlist by passing a path that does not lead to a gate library
            NO_COUT_TEST_BLOCK;

            std::shared_ptr<netlist> nl = netlist_factory::load_netlist(tmp_hdl_file_path, "vhdl", hal::path("/this/file/does/not/exist"));

            EXPECT_EQ(nl, nullptr);

        }

    TEST_END
}

/**
 * Testing the creation of an empty netlist by passing a hal file. The hal-file is
 * created by the netlist serializer
 *
 * Functions: load_netlist(hdl_file, ...)
 */
TEST_F(netlist_factory_test, check_load_netlist_by_hal_file)
{
    TEST_START
        {
            // Create a netlist by using a temporary created hal file
            hal::path tmp_hal_file_path = create_sandbox_path("test_hal_file.hal");

            std::shared_ptr<netlist> empty_nl = netlist_factory::create_netlist(gate_library_manager::get_gate_library(m_g_lib_path));    //empty netlist
            netlist_serializer::serialize_to_file(empty_nl, tmp_hal_file_path);
            std::shared_ptr<netlist> nl = netlist_factory::load_netlist(tmp_hal_file_path);

            EXPECT_NE(nl, nullptr);

        }
        {
            // Pass an invalid file path
            NO_COUT_TEST_BLOCK;
            std::shared_ptr<netlist> nl = netlist_factory::load_netlist(hal::path("/this/file/does/not/exists.hal"));

            EXPECT_EQ(nl, nullptr);
        }
        {
            // The hal file can't be parsed (invalid format)
            NO_COUT_TEST_BLOCK;
            hal::path tmp_hal_file_path = create_sandbox_file("invalid_hal_test_file.hal",
                                                                "{\n"
                                                                "    \"gate_library\": \"non_existing_g_lib\",\n"
                                                                "    \"id\": 0,\n"
                                                                "    \"input_file\": \"\",\n"
                                                                "    \"design_name\": \"\",\n"
                                                                    "    \"device_name\": \"\",\n"
                                                                    "    \"gates\": [],\n"
                                                                    "    \"nets\": [],\n"
                                                                    "    \"modules\": []\n"
                                                                    "}");

            std::shared_ptr<netlist> nl = netlist_factory::load_netlist(tmp_hal_file_path);

            EXPECT_EQ(nl, nullptr);
        }
    TEST_END

}

/**
 * Testing the creation of an empty netlist by passing a hal or hdl file via
 * program arguments.
 *
 * Functions: create_netlist(hdl_file, ...)
 */
TEST_F(netlist_factory_test, check_create_netlist_by_program_args)
{
    TEST_START
        {
            // Create a netlist by passing a .hal file-path via program arguments
            hal::path tmp_hal_file_path = create_sandbox_path("test_hal_file.hal");

            std::shared_ptr<netlist> empty_nl = netlist_factory::create_netlist(gate_library_manager::get_gate_library(m_g_lib_path));    //empty netlist
            netlist_serializer::serialize_to_file(empty_nl, tmp_hal_file_path);      // create the .hal file

            program_arguments p_args;
            p_args.set_option("--input-file", std::vector<std::string>({tmp_hal_file_path}));

            std::shared_ptr<netlist> nl = netlist_factory::load_netlist(p_args);


            EXPECT_NE(nl, nullptr);

        }
        {
            // Create a netlist by passing a .vhdl file-path via program arguments.
            hal::path tmp_hdl_file_path = create_sandbox_file("tmp.vhdl",
                                                              "-- Device\t: device_name\n"
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


            program_arguments p_args;
            p_args.set_option("--input-file", std::vector<std::string>({tmp_hdl_file_path}));
            p_args.set_option("--language", std::vector<std::string>({"vhdl"}));
            p_args.set_option("--gate-library", std::vector<std::string>({m_g_lib_path}));

            // NO_COUT_TEST_BLOCK;
            std::shared_ptr<netlist> nl = netlist_factory::load_netlist(p_args);

            EXPECT_NE(nl, nullptr);

        }
        {
            // Create a netlist but leaving out the input path
            NO_COUT_TEST_BLOCK;
            program_arguments p_args;
            std::shared_ptr<netlist> nl = netlist_factory::load_netlist(p_args);
            EXPECT_EQ(nl, nullptr);
        }
        {
            // Create a netlist but with an invalid (non-existing) input file path
            NO_COUT_TEST_BLOCK;
            program_arguments p_args;
            p_args.set_option("--input-file", std::vector<std::string>({"/this/file/does/not/exist"}));
            std::shared_ptr<netlist> nl = netlist_factory::load_netlist(p_args);
            EXPECT_EQ(nl, nullptr);
        }
        {
            // Create a netlist but of an invalid .vhdl file
            NO_COUT_TEST_BLOCK;

            hal::path tmp_hdl_file_path = create_sandbox_file("tmp_2.vhdl",
                                                              "This file does not contain a valid vdl format...");

            program_arguments p_args;
            p_args.set_option("--input-file", std::vector<std::string>({tmp_hdl_file_path}));
            p_args.set_option("--language", std::vector<std::string>({"vhdl"}));
            p_args.set_option("--gate-library", std::vector<std::string>({g_lib_name}));
            std::shared_ptr<netlist> nl = netlist_factory::load_netlist(p_args);

            EXPECT_EQ(nl, nullptr);
        }
    TEST_END
}
