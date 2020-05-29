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

    virtual void SetUp()
    {
        //NO_COUT_BLOCK;
        //gate_library_manager::load_all();
    }

    virtual void TearDown()
    {
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

/*
 * TODO: Needs fixes
 */

// Empty
TEST_F(netlist_factory_test, check_empty)
{
    EXPECT_TRUE(true);
}

#ifdef DONT_BUILD
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
                // Create a netlist of an existing gate library
                //::testing::internal::CaptureStdout();
                std::shared_ptr<netlist> nl = netlist_factory::create_netlist("EXAMPLE_GATE_LIBRARY");
                //std::shared_ptr<netlist> nl_2 = netlist_factory::create_netlist("example_library");
                //::testing::internal::GetCapturedStdout();
                EXPECT_EQ(nl->get_gate_library()->get_name(), g_lib_name);
                //EXPECT_EQ(nl_2->get_gate_library()->get_name(), g_lib_name);
            }
        }
        {
            // Try to create a netlist by passing a non-existing gate library name
            NO_COUT_TEST_BLOCK;
            std::shared_ptr<netlist> nl = netlist_factory::create_netlist("non_existing_g_lib");
            EXPECT_EQ(nl, nullptr);
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
        {
            // Create a netlist by a temporary created vhdl file
            std::string tmp_hdl_file_path = core_utils::get_binary_directory().string() + "/tmp.vdl";
            std::ofstream hdl_file(tmp_hdl_file_path);
            std::stringstream input;

            hdl_file << "-- Device\t: device_name\n"
                        "entity TEST_Comp is\n"
                        "  port (\n"
                        "    net_global_in : in STD_LOGIC := 'X';\n"
                        "    net_global_out : out STD_LOGIC := 'X';\n"
                        "  );\n"
                        "end TEST_Comp;\n"
                        "architecture STRUCTURE of TEST_Comp is\n"
                        "begin\n"
                        "  gate_0 : INV\n"
                        "    port map (\n"
                        "      I => net_global_in,\n"
                        "      O => net_global_out\n"
                        "    );\n"
                        "end STRUCTURE;";
            hdl_file.close();
            //test_def::capture_stdout();
            std::shared_ptr<netlist> nl = netlist_factory::load_netlist(hal::path(tmp_hdl_file_path), "vhdl", g_lib_name);
            //test_def::get_captured_stdout();

            ASSERT_NE(nl, nullptr);
            EXPECT_EQ(nl->get_gate_library()->get_name(), test_utils::g_lib_name);

            fs::remove(tmp_hdl_file_path);
        }
        {
            // Try to create a netlist by a non-accessible (non-existing) file
            NO_COUT_TEST_BLOCK;
            std::shared_ptr<netlist> nl = netlist_factory::load_netlist(hal::path("/this/file/does/not/exist"), "vhdl", g_lib_name);

            EXPECT_EQ(nl, nullptr);
        }
        {
            // Try to create a netlist by passing a non existing gate library
            NO_COUT_TEST_BLOCK;
            std::string tmp_hdl_file_path = core_utils::get_binary_directory().string() + "/tmp.vdl";
            std::ofstream hdl_file(tmp_hdl_file_path);
            hdl_file << "This file does not contain a valid vdl format";

            hdl_file.close();

            std::shared_ptr<netlist> nl = netlist_factory::load_netlist(hal::path(tmp_hdl_file_path), "vhdl", "non_existing_g_lib");

            EXPECT_EQ(nl, nullptr);

            fs::remove(tmp_hdl_file_path);
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
        if (gate_library_exists(g_lib_name))
        {
            {
                // Create a netlist by a temporary created hal file
                std::string tmp_hal_file_path = core_utils::get_binary_directory().string() + "/tmp.hal";

                test_def::capture_stdout();
                std::shared_ptr<netlist> empty_nl = netlist_factory::create_netlist(g_lib_name);    //empty netlist
                netlist_serializer::serialize_to_file(empty_nl, hal::path(tmp_hal_file_path));
                std::shared_ptr<netlist> nl = netlist_factory::load_netlist(hal::path(tmp_hal_file_path));
                test_def::get_captured_stdout();

                EXPECT_NE(nl, nullptr);

               fs::remove(tmp_hal_file_path);
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
                std::string tmp_hal_file_path = core_utils::get_binary_directory().string() + "/tmp.hal";
                std::ofstream hal_file(tmp_hal_file_path);
                // the .hal file is invalid because of the non existing gate library
                hal_file << "{\n"
                            "    \"gate_library\": \"non_existing_g_lib\",\n"
                            "    \"id\": 0,\n"
                            "    \"input_file\": \"\",\n"
                            "    \"design_name\": \"\",\n"
                            "    \"device_name\": \"\",\n"
                            "    \"gates\": [],\n"
                            "    \"nets\": [],\n"
                            "    \"modules\": []\n"
                            "}";
                hal_file.close();

                std::shared_ptr<netlist> nl = netlist_factory::load_netlist(hal::path(tmp_hal_file_path));

                EXPECT_EQ(nl, nullptr);

                fs::remove(tmp_hal_file_path);
            }
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
        if (gate_library_exists(g_lib_name))
        {
            {
                // Create a netlist by passing a .hal file-path via program arguments
                std::string tmp_hal_file_path = core_utils::get_binary_directory().string() + "/tmp.hal";
                test_def::capture_stdout();
                std::shared_ptr<netlist> empty_nl = netlist_factory::create_netlist(g_lib_name);    //empty netlist
                netlist_serializer::serialize_to_file(empty_nl, hal::path(tmp_hal_file_path));      // create the .hal file

                program_arguments p_args;
                p_args.set_option("--input-file", std::vector<std::string>({tmp_hal_file_path}));

                std::shared_ptr<netlist> nl = netlist_factory::load_netlist(p_args);
                test_def::get_captured_stdout();

                EXPECT_NE(nl, nullptr);

                fs::remove(tmp_hal_file_path);
            }
            {
                // Create a netlist by passing a .hal file-path via program arguments. Set volatile-mode
                std::string tmp_hdl_file_path = core_utils::get_binary_directory().string() + "/tmp.vhdl";
                std::ofstream hdl_file(tmp_hdl_file_path);    // create a temporary hdl file
                hdl_file << "-- Device\t: device_name\n"
                            "entity TEST_Comp is\n"
                            "  port (\n"
                            "    net_global_in : in STD_LOGIC := 'X';\n"
                            "    net_global_out : out STD_LOGIC := 'X';\n"
                            "  );\n"
                            "end TEST_Comp;\n"
                            "architecture STRUCTURE of TEST_Comp is\n"
                            "begin\n"
                            "  gate_0 : INV\n"
                            "    port map (\n"
                            "      I => net_global_in,\n"
                            "      O => net_global_out\n"
                            "    );\n"
                            "end STRUCTURE;";

                hdl_file.close();

                program_arguments p_args;
                p_args.set_option("--input-file", std::vector<std::string>({tmp_hdl_file_path}));
                p_args.set_option("--language", std::vector<std::string>({"vhdl"}));
                p_args.set_option("--gate-library", std::vector<std::string>({g_lib_name}));
                p_args.set_option("--volatile-mode", std::vector<std::string>({}));

                NO_COUT_TEST_BLOCK;
                std::shared_ptr<netlist> nl = netlist_factory::load_netlist(p_args);

                EXPECT_NE(nl, nullptr);

               fs::remove(tmp_hdl_file_path);
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
                // Create a netlist but of an invalid .hal file
                NO_COUT_TEST_BLOCK;
                std::string tmp_hdl_file_path = core_utils::get_binary_directory().string() + "/tmp.vdl";
                std::ofstream hdl_file(tmp_hdl_file_path);
                hdl_file << "This file does not contain a valid vdl format";

                hdl_file.close();

                program_arguments p_args;
                p_args.set_option("--input-file", std::vector<std::string>({tmp_hdl_file_path}));
                p_args.set_option("--language", std::vector<std::string>({"vhdl"}));
                p_args.set_option("--gate-library", std::vector<std::string>({g_lib_name}));
                std::shared_ptr<netlist> nl = netlist_factory::load_netlist(p_args);

                EXPECT_EQ(nl, nullptr);

                fs::remove(tmp_hdl_file_path);
            }
        }

    TEST_END
}

#endif // DONT_BUILD

