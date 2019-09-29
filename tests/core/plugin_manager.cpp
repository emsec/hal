#include "test_def.h"
#include "gtest/gtest.h"
#include <core/interface_cli.h>
#include <core/log.h>
#include <core/plugin_manager.h>
#include <iostream>

/*
 * This test isn't finished
 */

using std::cout;
using std::endl;

class plugin_manager_test : public ::testing::Test
{
protected:
    const std::string reference_library_name = "libgraph_algorithm";
    const std::string reference_name = "graph_algorithm";

    virtual void SetUp()
    {
    }

    virtual void TearDown()
    {
    }

    bool is_reference_plugin_loaded()
    {
        auto names = plugin_manager::get_plugin_names();
        return (names.find(reference_library_name) != names.end());
    }

    // Loads the test_plugin
    bool load_reference_plugin()
    {
        testing::internal::CaptureStdout();
        for (auto directory : core_utils::get_plugin_directories())
        {
            bool suc = plugin_manager::load(reference_library_name, directory / (reference_library_name + ".so"));
            if (suc)
            {
                testing::internal::GetCapturedStdout();
                return true;
            }
        }
        // If no test plugin is found print the captured output
        std::cout << testing::internal::GetCapturedStdout();
        return false;
    }
};

/**
 * Remove me later
 *
 * Functions: <functions>
 */

TEST_F(plugin_manager_test, demonstrate_error_1)
{
    TEST_START

    // Load all plugins without directory hints
    plugin_manager::load_all_plugins();
    //plugin_manager::unload_all_plugins();
    //plugin_manager::load_all_plugins();

    program_options cli_plugin_opts = plugin_manager::get_cli_plugin_options();

    std::cout << "Loaded: " << std::endl;
    for ( auto pn : plugin_manager::get_plugin_names()) {
        std::cout << pn << std::endl;
    }

    std::cout << "\n===========\nDirectories: " << std::endl;
    for( auto dir : core_utils::get_plugin_directories() )
        std::cout << dir << std::endl;

    TEST_END
}

/**
 * Testing things
 *
 * Functions: <functions>
 */

TEST_F(plugin_manager_test, demonstrate_error_2)
{
    TEST_START
    bool suc_1 = load_reference_plugin();
    plugin_manager::unload(reference_library_name);
    //bool suc_2 = load_reference_plugin();
    EXPECT_TRUE(suc_1);
    //EXPECT_TRUE(suc_2);
    TEST_END
}

/**
 * Try to load all plugins, get their names and unload them all. After try to
 * load them successively by calling the load function successivley with all
 * plugin names load_all_plugins found.
 *
 * Functions: load_all_plugins, get_plugin_names, unload_all_plugins
 *
 */

TEST_F(plugin_manager_test, check_load_all) {
    TEST_START
        std::set<std::string> plugin_names;
        {
            // Load all plugins without directory hints
            plugin_manager::load_all_plugins();
            //plugin_manager::load_all_plugins();
            plugin_names = plugin_manager::get_plugin_names();
            int plugin_amount = plugin_names.size();
            if (plugin_amount == 0) {
                std::cout << "Warning: No builded plugins are found! Tests might not find issues...";
            }

            // Unload all plugins
            plugin_manager::unload_all_plugins();
            EXPECT_EQ(plugin_manager::get_plugin_names().size(), 0);
        }
        {
            // Load all plugins without directory hints by passing them the plugin folders given
            // by core_utils::get_plugin_directories
            auto dirs = core_utils::get_plugin_directories();
            plugin_manager::load_all_plugins(dirs);

            int plugin_amount = plugin_manager::get_plugin_names().size();
            if (plugin_amount == 0) {
                std::cout << "Warning: No builded plugins are found! Tests might not find issues...";
            }
            // Unload all plugins
            plugin_manager::unload_all_plugins();
            EXPECT_EQ(plugin_manager::get_plugin_names().size(), 0);
        }
        {

            // Try all to load the plugins from all possible plugin directories
            for (auto& directory : core_utils::get_plugin_directories()){
                for (auto& p_name : plugin_names){
                    plugin_manager::load(p_name, directory);
                }
            }

        }

    TEST_END
}

/**
 * Try to load and unload all plugins with and without directory hints.
 *
 * Functions: load_all_plugins, get_plugin_names, unload_all_plugins
 *
 */
TEST_F(plugin_manager_test, check_load_unload_all)
{
    TEST_START
    NO_COUT_TEST_BLOCK;
    // Load all plugins without directory hints
    plugin_manager::load_all_plugins();
    if (plugin_manager::get_plugin_names().empty())
    {
        std::cout << "Warning: No builded plugins are found! Tests might not find issues..." << std::endl;
    }
    plugin_manager::unload_all_plugins();
    EXPECT_TRUE(plugin_manager::get_plugin_names().empty());

    // Load all plugins with directory hints
    plugin_manager::load_all_plugins(core_utils::get_plugin_directories()); // ISSUE: Fails with SIGSEGV
    plugin_manager::unload_all_plugins();

    EXPECT_TRUE(plugin_manager::get_plugin_names().empty());

    TEST_END
}

/**
 * Try to load all plugins successively
 *
 * Functions: load_all_plugins, get_plugin_names, unload_all_plugins
 *
 */
/*TEST_F(plugin_manager_test, check_load) {
    TEST_START
        for (const auto& directory : directories)
        {
            if (!hal::fs::exists(directory))
                continue;
            u32 num_of_loaded_plugins = 0;
            for (const auto& file : core_utils::directory_range(directory))
            {
                if (!plugin_manager_helper::has_valid_file_extension(file) || hal::fs::is_directory(file))
                    continue;

                auto plugin_name = file.path().stem().string();
                if (this->load(plugin_name, file.path()))
                    num_of_loaded_plugins++;
            }
            log_info("core", "loaded {} plugins from '{}'", num_of_loaded_plugins, directory.string());
        }
        return true;
    TEST_END
}*/

/**
 * Testing the load function with the test_plugin as well as with invalid
 * values
 *
 * Functions: load, unload
 *
 */

TEST_F(plugin_manager_test, check_load)
{
    TEST_START
    // Figure out the path of the reference plugin by trying to load it in the possible
    // plugin directories
    hal::path test_plugin_path;

    testing::internal::CaptureStdout();
    for (auto directory : core_utils::get_plugin_directories())
    {
        bool suc = plugin_manager::load(reference_library_name, directory / (reference_library_name + ".so"));
        if (suc)
        {
            test_plugin_path = directory;
            break;
        }
    }
    testing::internal::GetCapturedStdout();
    if (test_plugin_path == hal::path(""))
    {
        std::cout << "Can't find the reference plugin ("<< reference_library_name <<"). Test might not find issues..." << std::endl;
    }
    // Unload the libtest_plugin loaded in the previous step
    EXPECT_TRUE(plugin_manager::unload(reference_library_name));

    {
        // Load a plugin at a non-existing directory
        testing::internal::CaptureStdout();
        bool suc = plugin_manager::load(reference_library_name, hal::path("/this/dir/does/not/exist"));
        testing::internal::GetCapturedStdout();
        EXPECT_FALSE(suc);
    }
    {
        // Try to load an non-existing plugin at an existing directory
        testing::internal::CaptureStdout();
        bool suc = plugin_manager::load("plugin_name", test_plugin_path / "non_existing_plugin.so");
        testing::internal::GetCapturedStdout();
        EXPECT_FALSE(suc);
    }
    {
        // Try to load an existing plugin but pass an empty name
        testing::internal::CaptureStdout();
        bool suc = plugin_manager::load("", test_plugin_path / (reference_library_name + ".so"));
        testing::internal::GetCapturedStdout();
        EXPECT_FALSE(suc);
    }
    {
        // Path is empty string
        testing::internal::CaptureStdout();
        bool suc = plugin_manager::load("plugin_name", hal::path(""));
        testing::internal::GetCapturedStdout();
        EXPECT_FALSE(suc);
    }
    {
        // Load an already loaded plugin (should return true)
        //testing::internal::CaptureStdout();
        bool suc_first = plugin_manager::load(reference_library_name, (reference_library_name + ".so"));
        bool suc       = plugin_manager::load(reference_library_name, (reference_library_name + ".so"));
        //testing::internal::GetCapturedStdout();
        EXPECT_TRUE(suc);
        EXPECT_TRUE(suc_first);
    }

    TEST_END
}

/**
 * Testing the unload function with the test_plugin as well as with invalid
 * values
 *
 * Functions: load, unload
 *
 */

TEST_F(plugin_manager_test, check_unload)
{
    TEST_START
    // Figure out the path of test_plugin by try to loading it in the possible
    // plugin directories
    hal::path test_plugin_path;

    testing::internal::CaptureStdout();
    for (auto directory : core_utils::get_plugin_directories())
    {
        bool suc = plugin_manager::load(reference_library_name, (reference_library_name + ".so"));
        if (suc)
        {
            test_plugin_path = directory;
            break;
        }
    }
    testing::internal::GetCapturedStdout();
    if (test_plugin_path == hal::path(""))
    {
        std::cout << "Can't find libtest_plugin. Test might not find issues..." << std::endl;
    }
    // Unload the libtest_plugin loaded in the previous step
    EXPECT_TRUE(plugin_manager::unload(reference_library_name));

    {
        // Unload an unknown plugin
        testing::internal::CaptureStdout();
        bool suc = plugin_manager::unload("non_existing_plugin");
        testing::internal::GetCapturedStdout();
        EXPECT_TRUE(suc);
    }
    {
        // Passing unload an empty string
        testing::internal::CaptureStdout();
        bool suc = plugin_manager::unload("");
        testing::internal::GetCapturedStdout();
        EXPECT_TRUE(suc);
    }

    // NOTE: do some positive tests, if unload is fixed

    TEST_END
}

/**
 * Testing the get_flag_to_plugin_mapping function
 * Functions: get_flag_to_plugin_mapping
 *
 */
/*TEST_F(plugin_manager_test, check_cli_options)
{
    TEST_START

    // ########################
    // POSITIVE TESTS
    // ########################

    // Load the test_plugin
    if (load_reference_plugin())
    {
        std::map<std::string, std::string> cli_opts = plugin_manager::get_flag_to_plugin_mapping();
        EXPECT_NE(cli_opts.find("--"+reference_name), cli_opts.end());
        if (cli_opts.find("--"+reference_name) != cli_opts.end())
        {
            EXPECT_EQ(cli_opts["--"+reference_name], reference_library_name);
        }
    }
    else
    {
        std::cout << "Can't load libtest_plugin. Some tests are skipped..." << std::endl;
    }

    plugin_manager::unload(reference_library_name);

    TEST_END
}*/

/**
 * Testing the get_cli_plugin_options function
 * Functions: get_cli_plugin_options
 *
 */
/*TEST_F(plugin_manager_test, check_cli_plugin_options)
{
    TEST_START

    // ########################
    // POSITIVE TESTS
    // ########################

    // Load the test_plugin
    if (load_reference_plugin())
    {
        // Get the options and find the options of the test plugin
        program_options cli_plugin_opts = plugin_manager::get_cli_plugin_options();

        EXPECT_TRUE(cli_plugin_opts.is_registered("--test_plugin"));
        std::vector<std::tuple<std::set<std::string>, std::string>> opts = cli_plugin_opts.get_options();
        EXPECT_EQ(std::get<1>(opts[0]), "test_plugin_description");
    }
    else
    {
        std::cout << "Warning: Can't load libtest_plugin. Some tests are skipped..." << std::endl;
    }

    plugin_manager::unload("libtest_plugin");

    TEST_END
}*/

/**
 * Testing the get_plugin_factory function
 *
 * Functions: get_plugin_factory
 */
/*TEST_F(plugin_manager_test, check_get_plugin_factory)
{
    TEST_START
    // ########################
    // POSITIVE TESTS
    // ########################

    // Load the test_plugin
    if (load_reference_plugin())
    {
        i_factory* test_p_factory = plugin_manager::get_plugin_factory("libtest_plugin");
        EXPECT_NE(test_p_factory, nullptr);
    }
    else
    {
        std::cout << "Can't load libtest_plugin. Some tests are skipped..." << std::endl;
    }

    plugin_manager::unload("libtest_plugin");

    // ########################
    // NEGATIVE TESTS
    // ########################

    {
        // Passing an unknown plugin name
        testing::internal::CaptureStdout();
        i_factory* factory = plugin_manager::get_plugin_factory("unknown_plugin");
        testing::internal::GetCapturedStdout();
        EXPECT_EQ(factory, nullptr);
    }
    {
        // Passing an empty string
        testing::internal::CaptureStdout();
        i_factory* factory = plugin_manager::get_plugin_factory("");
        testing::internal::GetCapturedStdout();
        EXPECT_EQ(factory, nullptr);
    }

    TEST_END
}*/

/**
 * Testing the get_plugin_instance function
 *
 * Functions: get_plugin_instance
 */
/*TEST_F(plugin_manager_test, check_get_plugin_instance)
{
    TEST_START
    // ########################
    // POSITIVE TESTS
    // ########################

    // Load the test_plugin
    if (load_reference_plugin())
    {
        std::shared_ptr<i_cli> test_plugin_instance = plugin_manager::get_plugin_instance<i_cli>("libtest_plugin");
        EXPECT_EQ(test_plugin_instance->get_name(), "test_plugin");
    }
    else
    {
        std::cout << "Can't load libtest_plugin. Some tests are skipped..." << std::endl;
    }

    plugin_manager::unload("libtest_plugin");

    // ########################
    // NEGATIVE TESTS
    // ########################

    {
        // Passing an unknown plugin name
        testing::internal::CaptureStdout();
        std::shared_ptr<i_cli> plugin_instance = plugin_manager::get_plugin_instance<i_cli>("unknown_plugin");
        testing::internal::GetCapturedStdout();
        EXPECT_EQ(plugin_instance, nullptr);
    }
    {
        // Passing an empty string
        testing::internal::CaptureStdout();
        std::shared_ptr<i_cli> plugin_instance = plugin_manager::get_plugin_instance<i_cli>("");
        testing::internal::GetCapturedStdout();
        EXPECT_EQ(plugin_instance, nullptr);
    }

    TEST_END
}*/
