#include "test_def.h"
#include "gtest/gtest.h"
#include <core/interface_cli.h>
#include <core/log.h>
#include <core/plugin_manager.h>
#include <iostream>
#include "netlist_test_utils.h"

// These tests are currently disables, since there are unfixed bugs within the testcases. Will be fixed soon...
//#ifdef DONT_BUILD

using std::cout;
using std::endl;

// Counts the calls of the test_callback hook
static int callback_hooks_called;
// The last parameters, passed to the test_callback hook
static std::tuple<bool, std::string, std::string> callback_hook_params;

class plugin_manager_test : public ::testing::Test
{
protected:
    const std::string reference_library_name = "libtest_plugin";
    const std::string reference_name = "test_plugin";
    // List of (cli option flag, cli option description) pairs
    const std::vector<std::pair<std::string, std::string>> reference_lib_options = { std::make_pair("--option_one", "option_one_description"),
                                                                                    std::make_pair("--option_two", "option_two_description")};

    virtual void SetUp()
    {
        NO_COUT_BLOCK;
        callback_hooks_called = 0;
        plugin_manager::unload_all_plugins();
    }

    virtual void TearDown()
    {
    }


    // Searches in all plugin_directorys (give by core_utils::get_plugin_directories()) to find a plugin named plugin_name.
    // But: The function only looks for the first existence, but not if the plugin file can be loaded
    hal::path get_plugin_path(std::string plugin_name)
    {
        static std::map<std::string, hal::path> plugin_name_to_found_path;
        // If the function was already called, the previous result is stored (to speed it up a bit)
        if (plugin_name_to_found_path.find(plugin_name) != plugin_name_to_found_path.end()){
            if(fs::exists(plugin_name_to_found_path[plugin_name].string()))
            {
                return plugin_name_to_found_path[plugin_name];
            }
        }
        // Search in all plugin directories for the plugin
        for (auto directory : core_utils::get_plugin_directories())
        {
            if (fs::exists((directory / (plugin_name + ".so")).string()))
            {
                plugin_name_to_found_path[plugin_name] = (directory / (plugin_name + ".so"));
                return plugin_name_to_found_path[plugin_name];
            }
        }
        std::cout << "\n";
        log_error("tests","No pluguin \"'{}'\" found in the plugin build directories!", plugin_name);
        return hal::path();
    }

    // Loads the test_plugin. Requires the correctness of pluign_manager::load and plugin_manager::get_plugin_names
    bool load_reference_plugin()
    {
        std::set<std::string> loaded_plugins = plugin_manager::get_plugin_names();
        if(loaded_plugins.find(reference_library_name) != loaded_plugins.end()){
            // If the plugin is already loaded, return true
            return true;
        }

        hal::path plugin_path = get_plugin_path(reference_library_name);
        if (plugin_path.empty()){
            return false;
        }

        bool load_suc = plugin_manager::load(reference_library_name ,plugin_path);
        if (!load_suc){
            cout << "A plugin file ("<< plugin_path.string() <<") is found, but can't be loaded. Is the plugin build corrupted? ";
        }
        return load_suc;
    }



    static void test_callback(bool loaded, std::string plugin_name, std::string file_name)
    {
        callback_hooks_called = 1;
        callback_hook_params = std::make_tuple(loaded, plugin_name, file_name);
    }
};

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
            plugin_names = plugin_manager::get_plugin_names();

            int plugin_amount = plugin_names.size();
            if (plugin_amount == 0) {
                std::cout << "Warning: No builded plugins are found! Tests might not find issues...";
            }

            // Unload all plugins
            NO_COUT(plugin_manager::unload_all_plugins());
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
            NO_COUT(plugin_manager::unload_all_plugins());
            EXPECT_EQ(plugin_manager::get_plugin_names().size(), 0);
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
    NO_COUT(plugin_manager::unload_all_plugins());
    EXPECT_TRUE(plugin_manager::get_plugin_names().empty());

    // Load all plugins with directory hints
    plugin_manager::load_all_plugins(core_utils::get_plugin_directories());
    NO_COUT(plugin_manager::unload_all_plugins());

    EXPECT_TRUE(plugin_manager::get_plugin_names().empty());

    TEST_END
}

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
        hal::path test_plugin_path = get_plugin_path(reference_library_name);
        if (test_plugin_path == hal::path(""))
        {
            std::cout << "Can't find the reference plugin ("<< reference_library_name <<"). Test might not find issues..." << std::endl;
        }
        {
            // Load a plugin at a non-existing directory
            NO_COUT_TEST_BLOCK;
            plugin_manager::unload_all_plugins();
            bool suc = plugin_manager::load(reference_library_name, hal::path("/this/dir/does/not/exist"));
            EXPECT_FALSE(suc);
        }
        {
            // Try to load an non-existing plugin at an existing directory
            NO_COUT_TEST_BLOCK;
            plugin_manager::unload_all_plugins();
            bool suc = plugin_manager::load(reference_library_name, test_plugin_path / "non_existing_plugin.so");
            EXPECT_FALSE(suc);
        }
        {
            // Try to load an existing plugin but pass an empty name
            NO_COUT_TEST_BLOCK;
            plugin_manager::unload_all_plugins();
            bool suc = plugin_manager::load("", test_plugin_path / (reference_library_name + ".so"));
            EXPECT_FALSE(suc);
        }
        {
            // Path is empty string
            NO_COUT_TEST_BLOCK;
            plugin_manager::unload_all_plugins();
            bool suc = plugin_manager::load(reference_library_name, hal::path(""));
            EXPECT_FALSE(suc);
        }
        /*{ // ISSUE: Fails
            // Load an already loaded plugin (should return true)
            NO_COUT_TEST_BLOCK;
            plugin_manager::unload_all_plugins();
            bool suc_first = plugin_manager::load(reference_library_name, test_plugin_path);
            bool suc       = plugin_manager::load(reference_library_name, test_plugin_path);
            EXPECT_TRUE(suc);
            EXPECT_TRUE(suc_first);
        }*/
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
    hal::path test_plugin_path = get_plugin_path(reference_library_name);
    if (test_plugin_path == hal::path(""))
    {
        std::cout << "Can't find libtest_plugin. Test might not find issues..." << std::endl;
    }
    {
        // Unload the libtest_plugin loaded in the previous step
        NO_COUT_TEST_BLOCK;
        EXPECT_TRUE(plugin_manager::unload(reference_library_name));
    }
    {
        // Unload an unknown plugin
        NO_COUT_TEST_BLOCK;
        bool suc = plugin_manager::unload("non_existing_plugin");
        EXPECT_TRUE(suc);
    }
    {
        // Passing unload an empty string
        NO_COUT_TEST_BLOCK;
        bool suc = plugin_manager::unload("");
        EXPECT_TRUE(suc);
    }

    TEST_END
}

/**
 * Testing the get_flag_to_plugin_mapping function
 * Functions: get_flag_to_plugin_mapping
 *
 */
TEST_F(plugin_manager_test, check_cli_options)
{
    TEST_START

    // ########################
    // POSITIVE TESTS
    // ########################

    // Load the test_plugin
    if (load_reference_plugin())
    {
        std::map<std::string, std::string> cli_opts = plugin_manager::get_flag_to_plugin_mapping();
        for (auto ref_flag_and_desc : reference_lib_options){
            EXPECT_NE(cli_opts.find(ref_flag_and_desc.first), cli_opts.end());
            if (cli_opts.find(ref_flag_and_desc.first) != cli_opts.end())
            {
                EXPECT_EQ(cli_opts[ref_flag_and_desc.first], reference_library_name);
            }
        }
    }
    else
    {
        std::cout << "Can't load libtest_plugin. Some tests are skipped..." << std::endl;
    }

    NO_COUT(plugin_manager::unload_all_plugins());

    TEST_END
}

/**
 * Testing the get_cli_plugin_options function
 *
 * Functions: get_cli_plugin_options
 */
TEST_F(plugin_manager_test, check_cli_plugin_options)
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

        for (auto ref_flag_and_desc : reference_lib_options){
            EXPECT_TRUE(cli_plugin_opts.is_registered(ref_flag_and_desc.first));
        }
        NO_COUT_TEST_BLOCK;
        plugin_manager::unload_all_plugins();
    }
    else
    {
        std::cout << "Warning: Can't load libtest_plugin. Some tests are skipped..." << std::endl;
    }

    TEST_END
}

/**
 * Testing the get_plugin_instance function
 *
 * Functions: get_plugin_instance
 */
TEST_F(plugin_manager_test, check_get_plugin_instance)
{
    TEST_START
    // ########################
    // POSITIVE TESTS
    // ########################

    // Load the test_plugin
    if (load_reference_plugin())
    {
        std::shared_ptr<i_cli> test_plugin_instance = plugin_manager::get_plugin_instance<i_cli>(reference_library_name);
        ASSERT_NE(test_plugin_instance, nullptr);
        EXPECT_EQ(test_plugin_instance->get_name(), reference_name);
        NO_COUT_TEST_BLOCK;
        test_plugin_instance = nullptr;
        plugin_manager::unload_all_plugins();
    }
    else
    {
        std::cout << "Can't load libtest_plugin. Some tests are skipped..." << std::endl;
    }

    // ########################
    // NEGATIVE TESTS
    // ########################
    {
        // Passing an unknown plugin name
        NO_COUT_TEST_BLOCK;
        std::shared_ptr<i_cli> plugin_instance = plugin_manager::get_plugin_instance<i_cli>("unknown_plugin");
        EXPECT_EQ(plugin_instance, nullptr);
    }
    {
        // Passing an empty string
        NO_COUT_TEST_BLOCK;
        std::shared_ptr<i_cli> plugin_instance = plugin_manager::get_plugin_instance<i_cli>("");
        EXPECT_EQ(plugin_instance, nullptr);
    }

    TEST_END
}

/**
 * Testing the usage of callback hooks to track the loading and unloading of plugins
 *
 * Functions: add_model_changed_callback, remove_model_changed_callback
 */
TEST_F(plugin_manager_test, check_callback_hooks)
{
    TEST_START
        // Add a callback hook
        plugin_manager::unload_all_plugins(); // Just to assure no plugin is loaded
        callback_hooks_called = 0;
        u64 callback_id = plugin_manager::add_model_changed_callback(test_callback);
        EXPECT_EQ(callback_hooks_called, 0);
        if (load_reference_plugin()) // <- should call the callback hook once
        {
            EXPECT_EQ(callback_hooks_called, 1);
            EXPECT_EQ(callback_hook_params, std::make_tuple(true, reference_library_name, get_plugin_path(reference_library_name).string()));

            // Reset the counter
            callback_hooks_called = 0;

            // Unload all plugin (only one is loaded). The hook should be called once.
            plugin_manager::unload_all_plugins();

            EXPECT_EQ(callback_hooks_called, 1);
            EXPECT_EQ(callback_hook_params, std::make_tuple(false, reference_library_name, get_plugin_path(reference_library_name).string()));

            // Reset the counter
            callback_hooks_called = 0;


            // Unregister the hook
            plugin_manager::remove_model_changed_callback(callback_id);

            load_reference_plugin();
            plugin_manager::unload_all_plugins();

            EXPECT_EQ(callback_hooks_called, 0);

        }
        else
        {
            std::cout << "Can't load libtest_plugin. Some tests are skipped..." << std::endl;
        }
        {
            // The function is a nullptr
            // NO_COUT_TEST_BLOCK;
            callback_id = plugin_manager::add_model_changed_callback(nullptr);
            EXPECT_EQ(callback_id, 0);
        }

        NO_COUT(plugin_manager::unload_all_plugins());
    TEST_END
}

/**
 * Testing the adding of existing option flags that should be reserved, so plugins must not use them.
 *
 * IMPORTANT: Since the manipulation the test fixture does can't be reset, it must be positioned BELOW all others!
 *
 * Functions: add_existing_options_description, load
 */
TEST_F(plugin_manager_test, check_existing_options_description)
{
    TEST_START
        // Try the load plugin, but add an existing option (add_existing_options_description) that overlaps
        // with the plugins options
        hal::path test_plugin_path = get_plugin_path(reference_library_name);
        if (test_plugin_path == hal::path(""))
        {
            std::cout << "Can't find the reference plugin ("<< reference_library_name <<"). Test might not find issues..." << std::endl;
        }
        if (!reference_lib_options.empty()){
            NO_COUT_TEST_BLOCK;
            plugin_manager::unload_all_plugins();
            program_options overlapping_opt;
            overlapping_opt.add(reference_lib_options[0].first, reference_lib_options[0].second);
            plugin_manager::add_existing_options_description(overlapping_opt);
            EXPECT_FALSE(plugin_manager::load(reference_library_name, test_plugin_path));
        }
        else {
            cout << "Warning: Please add options to the reference plugin and put them in reference_lib_options..." << endl;
        }
    TEST_END
}

// #endif // DONT_BUILD
