#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/gate_library/gate_library_manager.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/netlist/netlist_factory.h"
#include "hal_core/plugin_system/plugin_manager.h"
#include "hal_core/utilities/log.h"
#include "hal_core/utilities/utils.h"
#include "netlist_test_utils.h"

#include "gtest/gtest.h"
#include <fstream>
#include <iostream>

namespace hal
{
    class GateLibraryManagerTest : public ::testing::Test
    {
    protected:
        // The path, where the library is temporary stored
        std::filesystem::path m_test_lib_path;
        std::string m_lib_file_name;
        std::string m_test_lib_name;

        virtual void SetUp()
        {
            NO_COUT_BLOCK;
            test_utils::init_log_channels();
            plugin_manager::load_all_plugins();
            m_lib_file_name = "test_lib";
            m_test_lib_name = "TEST_GATE_LIBRARY_FOR_GATE_LIBRARY_MANAGER_TESTS";
            m_test_lib_path = (utils::get_gate_library_directories()[0]) / (m_lib_file_name + ".lib");
        }

        virtual void TearDown()
        {
            std::filesystem::remove(m_test_lib_path);
            plugin_manager::unload_all_plugins();
        }

        /**
         * Creates a minimal custom Gate library used for testing the Gate library manager
         */
        void create_test_lib()
        {
            std::ofstream test_lib(m_test_lib_path.string());
            test_lib << "/* This file only exists for testing purposes and should be already destroyed*/\n"
                        "library ("
                     << m_test_lib_name
                     << ") {\n"
                        "    define(cell);\n"
                        "    cell(GATE_A) {\n"
                        "        pin(I) {\n"
                        "            direction: input;\n"
                        "        }\n"
                        "        pin(O) {\n"
                        "            direction: output;\n"
                        "            function: \"I\";\n"
                        "        }\n"
                        "    }\n"
                        "    cell(GATE_B) {\n"
                        "        pin(I) {\n"
                        "            direction: input;\n"
                        "        }\n"
                        "        pin(O) {\n"
                        "            direction: output;\n"
                        "            function: \"!I\";\n"
                        "        }\n"
                        "    }\n"
                        "    cell(GND) {\n"
                        "        pin(O) {\n"
                        "            direction: output;\n"
                        "            function: \"0\";\n"
                        "        }\n"
                        "    }\n"
                        "    cell(VCC) {\n"
                        "        pin(O) {\n"
                        "            direction: output;\n"
                        "            function: \"1\";\n"
                        "        }\n"
                        "    }\n"
                        "}";

            test_lib.close();
        }
    };

    /**
     * Testing the access on a single Gate library via the get_gate_library function.
     *
     * Functions: get_gate_library, get_gate_libraries
     */
    TEST_F(GateLibraryManagerTest, check_get_gate_library)
    {
        TEST_START
        NO_COUT_TEST_BLOCK;
        create_test_lib();
        // Load the Gate library twice by its filename
        GateLibrary* test_lib_0 = gate_library_manager::get_gate_library(m_test_lib_path);
        GateLibrary* test_lib_1 = gate_library_manager::get_gate_library(m_test_lib_path);
        EXPECT_NE(test_lib_0, nullptr);
        EXPECT_NE(test_lib_1, nullptr);

        // Check that the test library can be found in the get_gate_libraries vector
        bool found_test_lib = false;
        for (GateLibrary* gl : gate_library_manager::get_gate_libraries())
        {
            if (gl->get_name() == m_test_lib_name)
            {
                found_test_lib = true;
                break;
            }
        }
        EXPECT_TRUE(found_test_lib);
        TEST_END
    }

    /**
     * Testing the load_all function that loads all gate_libraries found in the directories got by
     * utils::get_gate_library_directories().
     *
     * Functions: get_gate_library, get_gate_libraries
     */
    TEST_F(GateLibraryManagerTest, check_load_all)
    {
        TEST_START
        // Check that load_all also loads the test Gate library
        NO_COUT_TEST_BLOCK;
        create_test_lib();
        gate_library_manager::load_all();

        // Check that the test library can be found in the get_gate_libraries vector
        bool found_test_lib = false;
        for (GateLibrary* gl : gate_library_manager::get_gate_libraries())
        {
            if (gl->get_name() == m_test_lib_name)
            {
                found_test_lib = true;
                break;
            }
        }
        EXPECT_TRUE(found_test_lib);
        TEST_END
    }

    /**
     * Testing that a GND/VCC Gate type is added to the Gate library, if the file does not contain any.
     *
     * Functions: get_gate_library, get_gate_libraries
     */
    TEST_F(GateLibraryManagerTest, check_prepare_library)
    {
        TEST_START
        {
            // Parse a file that does not contain a GND or VCC Gate type (constant 0 / constant 1)
            NO_COUT_TEST_BLOCK;
            std::ofstream test_lib(m_test_lib_path.string());
            test_lib << "/* This file only exists for testing purposes and should be already destroyed*/\n"
                        "library (check_prepare_library_1) {\n"
                        "    define(cell);\n"
                        "}";

            test_lib.close();
            GateLibrary* empty_lib = gate_library_manager::get_gate_library(m_test_lib_path);
            ASSERT_NE(empty_lib, nullptr);
            auto g_types = empty_lib->get_gate_types();
            // Check the creation of a gnd Gate type
            ASSERT_TRUE(g_types.find("GND") != g_types.end());
            auto gnd_bf = g_types.at("GND")->get_boolean_functions();
            ASSERT_TRUE(gnd_bf.find("O") != gnd_bf.end());
            EXPECT_TRUE(gnd_bf.at("O").is_constant_zero());
            // Check the creation of a vcc Gate type
            ASSERT_TRUE(g_types.find("VCC") != g_types.end());
            auto vcc_bf = g_types.at("VCC")->get_boolean_functions();
            ASSERT_TRUE(vcc_bf.find("O") != vcc_bf.end());
            EXPECT_TRUE(vcc_bf.at("O").is_constant_one());
        }
        std::filesystem::remove(m_test_lib_path);
        TEST_END
    }

    /**
    * Testing the handling of various invalid inputs.
    *
    * Functions: get_gate_library, get_gate_libraries
    */
    TEST_F(GateLibraryManagerTest, check_invalid)
    {
        TEST_START
        {
            // The file path does not exist
            NO_COUT_TEST_BLOCK;
            GateLibrary* test_lib = gate_library_manager::get_gate_library("/non/existing/path.lib");
            EXPECT_EQ(test_lib, nullptr);
        }
        TEST_END
    }
}    //namespace hal
