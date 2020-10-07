#include "hal_core/plugin_system/plugin_manager.h"

#include "hal_core/plugin_system/plugin_interface_cli.h"
#include "hal_core/utilities/log.h"
#include "netlist_test_utils.h"
#include "test_def.h"

#include "gtest/gtest.h"
#include <iostream>

namespace hal
{
    // Counts the calls of the test_callback hook
    static int g_callback_hooks_called;
    // The last parameters, passed to the test_callback hook
    static std::tuple<bool, std::string, std::string> g_callback_hook_params;

    class PluginManagerTest : public ::testing::Test
    {
    protected:
        const std::string m_reference_library_name = "libtest_plugin";
        const std::string m_reference_name         = "test_plugin";
        // List of (cli option flag, cli option description) pairs
        const std::vector<std::pair<std::string, std::string>> m_reference_lib_options = {std::make_pair("--option_one", "option_one_description"),
                                                                                          std::make_pair("--option_two", "option_two_description")};

        virtual void SetUp()
        {
            NO_COUT_BLOCK;
            test_utils::init_log_channels();
            g_callback_hooks_called = 0;
            plugin_manager::unload_all_plugins();
        }

        virtual void TearDown()
        {
            NO_COUT(plugin_manager::unload_all_plugins());
        }

        // Searches in all plugin_directorys (give by utils::get_plugin_directories()) to find a plugin named plugin_name.
        // But: The function only looks for the first existence, but not if the plugin file can be loaded
        std::filesystem::path get_plugin_path(std::string plugin_name)
        {
            static std::map<std::string, std::filesystem::path> plugin_name_to_found_path;
            // If the function was already called, the previous result is stored (to speed it up a bit)
            if (plugin_name_to_found_path.find(plugin_name) != plugin_name_to_found_path.end())
            {
                if (std::filesystem::exists(plugin_name_to_found_path[plugin_name].string()))
                {
                    return plugin_name_to_found_path[plugin_name];
                }
            }
            // Search in all plugin directories for the plugin
            for (auto directory : utils::get_plugin_directories())
            {
                if (std::filesystem::exists((directory / (plugin_name + ".so")).string()))
                {
                    plugin_name_to_found_path[plugin_name] = (directory / (plugin_name + ".so"));
                    return plugin_name_to_found_path[plugin_name];
                }
            }
            std::cout << "\n";
            log_error("tests", "No pluguin \"'{}'\" found in the plugin build directories!", plugin_name);
            return std::filesystem::path();
        }

        // Loads the test_plugin. Requires the correctness of pluign_manager::load and plugin_manager::get_plugin_names
        bool load_reference_plugin()
        {
            std::set<std::string> loaded_plugins = plugin_manager::get_plugin_names();
            if (loaded_plugins.find(m_reference_library_name) != loaded_plugins.end())
            {
                // If the plugin is already loaded, return true
                return true;
            }

            std::filesystem::path plugin_path = get_plugin_path(m_reference_library_name);
            if (plugin_path.empty())
            {
                return false;
            }

            bool load_suc = plugin_manager::load(m_reference_library_name, plugin_path);
            if (!load_suc)
            {
                std::cout << "A plugin file (" << plugin_path.string() << ") is found, but can't be loaded. Is the plugin build corrupted? ";
            }
            return load_suc;
        }

        static void test_callback(bool loaded, std::string plugin_name, std::string file_name)
        {
            g_callback_hooks_called = 1;
            g_callback_hook_params  = std::make_tuple(loaded, plugin_name, file_name);
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
    TEST_F(PluginManagerTest, check_load_all)
    {
        TEST_START
        std::set<std::string> plugin_names;
        {
            // Load all plugins without directory hints
            plugin_manager::load_all_plugins();
            plugin_names = plugin_manager::get_plugin_names();

            int plugin_amount = plugin_names.size();
            if (plugin_amount == 0)
            {
                std::cout << "Warning: No builded plugins are found! Tests might not find issues...";
            }

            // Unload all plugins
            NO_COUT(plugin_manager::unload_all_plugins());
            EXPECT_EQ(plugin_manager::get_plugin_names().size(), 0);
        }
        {
            // Load all plugins without directory hints by passing them the plugin folders given
            // by utils::get_plugin_directories
            auto dirs = utils::get_plugin_directories();
            plugin_manager::load_all_plugins(dirs);

            int plugin_amount = plugin_manager::get_plugin_names().size();
            if (plugin_amount == 0)
            {
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
    TEST_F(PluginManagerTest, check_load_unload_all)
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
        plugin_manager::load_all_plugins(utils::get_plugin_directories());
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
    TEST_F(PluginManagerTest, check_load)
    {
        TEST_START
        std::filesystem::path test_plugin_path = get_plugin_path(m_reference_library_name);
        if (test_plugin_path == std::filesystem::path(""))
        {
            std::cout << "Can't find the reference plugin (" << m_reference_library_name << "). Test might not find issues..." << std::endl;
        }
        {
            // Load a plugin at a non-existing directory
            NO_COUT_TEST_BLOCK;
            plugin_manager::unload_all_plugins();
            bool suc = plugin_manager::load(m_reference_library_name, std::filesystem::path("/this/dir/does/not/exist"));
            EXPECT_FALSE(suc);
        }
        {
            // Try to load an non-existing plugin at an existing directory
            NO_COUT_TEST_BLOCK;
            plugin_manager::unload_all_plugins();
            bool suc = plugin_manager::load(m_reference_library_name, test_plugin_path / "non_existing_plugin.so");
            EXPECT_FALSE(suc);
        }
        {
            // Try to load an existing plugin but pass an empty name
            NO_COUT_TEST_BLOCK;
            plugin_manager::unload_all_plugins();
            bool suc = plugin_manager::load("", test_plugin_path / (m_reference_library_name + ".so"));
            EXPECT_FALSE(suc);
        }
        {
            // Path is empty string
            NO_COUT_TEST_BLOCK;
            plugin_manager::unload_all_plugins();
            bool suc = plugin_manager::load(m_reference_library_name, std::filesystem::path(""));
            EXPECT_FALSE(suc);
        }
        {
            // Load an already loaded plugin (should return true)
            NO_COUT_TEST_BLOCK;
            plugin_manager::unload_all_plugins();
            bool suc_first = plugin_manager::load(m_reference_library_name, test_plugin_path);
            bool suc       = plugin_manager::load(m_reference_library_name, test_plugin_path);
            EXPECT_TRUE(suc);
            EXPECT_TRUE(suc_first);
        }
        NO_COUT(plugin_manager::unload_all_plugins());
        TEST_END
    }


    /**
     * Testing the unload function with the test_plugin as well as with invalid
     * values
     *
     * Functions: load, unload
     *
     */
    TEST_F(PluginManagerTest, check_unload)
    {
        TEST_START
        std::filesystem::path test_plugin_path = get_plugin_path(m_reference_library_name);
        if (test_plugin_path == std::filesystem::path(""))
        {
            std::cout << "Can't find libtest_plugin. Test might not find issues..." << std::endl;
        }
        {
            // Unload the libtest_plugin loaded in the previous step
            NO_COUT_TEST_BLOCK;
            EXPECT_TRUE(plugin_manager::unload(m_reference_library_name));
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
     * Testing the get_cli_plugin_flags function
     * Functions: get_cli_plugin_flags
     *
     */
    TEST_F(PluginManagerTest, check_cli_options)
    {
        TEST_START

        // ########################
        // POSITIVE TESTS
        // ########################

        // Load the test_plugin
        if (load_reference_plugin())
        {
            auto cli_opts = plugin_manager::get_cli_plugin_flags();
            for (auto ref_flag_and_desc : m_reference_lib_options)
            {
                EXPECT_NE(cli_opts.find(ref_flag_and_desc.first), cli_opts.end());
                if (cli_opts.find(ref_flag_and_desc.first) != cli_opts.end())
                {
                    EXPECT_EQ(cli_opts[ref_flag_and_desc.first], m_reference_library_name);
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
    TEST_F(PluginManagerTest, check_cli_plugin_options)
    {
        TEST_START

        // ########################
        // POSITIVE TESTS
        // ########################

        // Load the test_plugin
        if (load_reference_plugin())
        {
            // Get the options and find the options of the test plugin
            ProgramOptions cli_plugin_opts = plugin_manager::get_cli_plugin_options();

            for (auto ref_flag_and_desc : m_reference_lib_options)
            {
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
    TEST_F(PluginManagerTest, check_get_plugin_instance)
    {
        TEST_START
        // ########################
        // POSITIVE TESTS
        // ########################

        // Load the test_plugin
        if (load_reference_plugin())
        {
            auto test_plugin_instance = plugin_manager::get_plugin_instance<CLIPluginInterface>(m_reference_library_name);
            ASSERT_NE(test_plugin_instance, nullptr);
            EXPECT_EQ(test_plugin_instance->get_name(), m_reference_name);
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
            auto plugin_instance = plugin_manager::get_plugin_instance<CLIPluginInterface>("unknown_plugin");
            EXPECT_EQ(plugin_instance, nullptr);
        }
        {
            // Passing an empty string
            NO_COUT_TEST_BLOCK;
            auto plugin_instance = plugin_manager::get_plugin_instance<CLIPluginInterface>("");
            EXPECT_EQ(plugin_instance, nullptr);
        }

        TEST_END
    }

    /**
     * Testing the usage of callback hooks to track the loading and unloading of plugins
     *
     * Functions: add_model_changed_callback, remove_model_changed_callback
     */
    TEST_F(PluginManagerTest, check_callback_hooks)
    {
        TEST_START
        // Add a callback hook
        plugin_manager::unload_all_plugins();    // Just to assure no plugin is loaded
        g_callback_hooks_called = 0;
        u64 callback_id         = plugin_manager::add_model_changed_callback(test_callback);
        EXPECT_EQ(g_callback_hooks_called, 0);
        if (load_reference_plugin())    // <- should call the callback hook once
        {
            EXPECT_EQ(g_callback_hooks_called, 1);
            EXPECT_EQ(g_callback_hook_params, std::make_tuple(true, m_reference_library_name, get_plugin_path(m_reference_library_name).string()));

            // Reset the counter
            g_callback_hooks_called = 0;

            // Unload all plugin (only one is loaded). The hook should be called once.
            plugin_manager::unload_all_plugins();

            EXPECT_EQ(g_callback_hooks_called, 1);
            EXPECT_EQ(g_callback_hook_params, std::make_tuple(false, m_reference_library_name, get_plugin_path(m_reference_library_name).string()));

            // Reset the counter
            g_callback_hooks_called = 0;

            // Unregister the hook
            plugin_manager::remove_model_changed_callback(callback_id);

            load_reference_plugin();
            plugin_manager::unload_all_plugins();

            EXPECT_EQ(g_callback_hooks_called, 0);
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
    TEST_F(PluginManagerTest, check_existing_options_description)
    {
        TEST_START
        // Try the load plugin, but add an existing option (add_existing_options_description) that overlaps
        // with the plugins options
        std::filesystem::path test_plugin_path = get_plugin_path(m_reference_library_name);
        if (test_plugin_path == std::filesystem::path(""))
        {
            std::cout << "Can't find the reference plugin (" << m_reference_library_name << "). Test might not find issues..." << std::endl;
        }
        if (!m_reference_lib_options.empty())
        {
            NO_COUT_TEST_BLOCK;
            plugin_manager::unload_all_plugins();
            ProgramOptions overlapping_opt;
            overlapping_opt.add(m_reference_lib_options[0].first, m_reference_lib_options[0].second);
            plugin_manager::add_existing_options_description(overlapping_opt);
            EXPECT_FALSE(plugin_manager::load(m_reference_library_name, test_plugin_path));
        }
        else
        {
            std::cout << "Warning: Please add options to the reference plugin and put them in reference_lib_options..." << std::endl;
        }
        TEST_END
    }

}    //namespace hal
