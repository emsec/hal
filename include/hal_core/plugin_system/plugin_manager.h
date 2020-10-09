//  MIT License
//
//  Copyright (c) 2019 Ruhr-University Bochum, Germany, Chair for Embedded Security. All Rights reserved.
//  Copyright (c) 2019 Marc Fyrbiak, Sebastian Wallat, Max Hoffmann ("ORIGINAL AUTHORS"). All rights reserved.
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in all
//  copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//  SOFTWARE.

#pragma once

#include "hal_core/defines.h"
#include "hal_core/plugin_system/plugin_interface_base.h"
#include "hal_core/utilities/callback_hook.h"
#include "hal_core/utilities/program_options.h"

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <tuple>

namespace hal
{
    /**
     * The plugin manager takes care of loading and unloading plugins at runtime.
     *
     * @ingroup plugins
     */
    namespace plugin_manager
    {
        /**
         * TODO Python binding.
         * 
         * Register existing program options to avoid reuse by plugins.
         *
         * @param[in] existing_options - The program options.
         */
        void add_existing_options_description(const ProgramOptions& existing_options);

        /**
         * Get the names of all loaded plugins.
         *
         * @returns The set of plugin names.
         */
        std::set<std::string> get_plugin_names();

        /**
         * TODO Python binding.
         * 
         * Get a mapping of flags pointing to their corresponding CLI plugin.
         *
         * @returns A map from flag to plugin name.
         */
        std::unordered_map<std::string, std::string> get_cli_plugin_flags();

        /**
         * TODO Python binding.
         * 
         * Get a mapping of flags pointing to their corresponding UI plugin.
         *
         * @returns A map from flag to plugin name.
         */
        std::unordered_map<std::string, std::string> get_ui_plugin_flags();

        /**
         * TODO Python binding.
         * 
         * Get command line interface options for all plugins.
         *
         * @returns The program options.
         */
        ProgramOptions get_cli_plugin_options();

        /**
         * Load all plugins in the specified diretories.<br>
         * If \p directory_names is empty, the default directories will be searched.
         *
         * @param[in] directory_names - A vector of directory paths.
         * @returns True on success, false otherwise.
         */
        bool load_all_plugins(const std::vector<std::filesystem::path>& directory_names = {});

        /**
         * Load a single plugin by specifying its name and file path.
         *
         * @param[in] plugin_name - The desired name that is unique in the framework.
         * @param[in] file_path - The path to the plugin file.
         * @returns True on success, false otherwise.
         */
        bool load(const std::string& plugin_name, const std::filesystem::path& file_path);

        /**
         * Releases all plugins and their associated resources.
         *
         * @returns True on success, false otherwise.
         */
        bool unload_all_plugins();

        /**
         * Releases a single plugin and its associated ressources.
         *
         * @param[in] plugin_name - The name of the plugin to unload.
         * @returns True on success, false otherwise.
         */
        bool unload(const std::string& plugin_name);

        /**
         * Gets the basic interface for a plugin specified by name.
         * By default calls the initialize() function of the plugin.
         *
         * @param[in] plugin_name - The name of the plugin.
         * @param[in] initialize - If false, the plugin's initialize function is not called.
         * @returns The basic plugin interface.
         */
        BasePluginInterface* get_plugin_instance(const std::string& plugin_name, bool initialize = true);

        /**
         * TODO Python bindings for different types and extend by initialize flag.
         * 
         * Gets a specific interface for a plugin specified by name.
         * By default calls the initialize() function of the plugin.
         *
         * @param[in] plugin_name - The name of the plugin.
         * @param[in] initialize - If false, the plugin's initialize function is not called.
         * @returns The specific plugin interface.
         */
        template<typename T>
        T* get_plugin_instance(const std::string& plugin_name, bool initialize = true)
        {
            return dynamic_cast<T*>(get_plugin_instance(plugin_name, initialize));
        }

        /**
         * TODO Python binding.
         * 
         * Add a callback to notify the GUI about loaded or unloaded plugins.
         *
         * @param[in] callback - The callback function. Parameters are:
         * * bool - True = load, false = unload.
         * * std::string - The plugin name.
         * * std::string - The plugin path.
         * @returns The id of the registered callback.
         */
        u64 add_model_changed_callback(std::function<void(bool, std::string const&, std::string const&)> callback);

        /**
         * TODO Python binding.
         * 
         * Remove a registered callback.
         *
         * @param[in] id - The id of the registered callback.
         */
        void remove_model_changed_callback(u64 id);
    }    // namespace plugin_manager
}    // namespace hal
