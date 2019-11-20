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

#include "def.h"

#include "core/callback_hook.h"
#include "core/interface_base.h"
#include "core/library_loader.h"
#include "core/program_options.h"

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <tuple>

/**
 * Plugin manager to load and unload plugins
 *
 * @ingroup core
 */
namespace plugin_manager
{
    /**
     * Add existing program options.<br>
     * Required to detect whether a plugin reuses existing flags.
     *
     * @param[in] existing_options - The existing options.
     */
    void add_existing_options_description(const program_options& existing_options);

    /**
     * Get the names of all loaded plugins.
     *
     * @returns The set of plugin names.
     */
    std::set<std::string> get_plugin_names();

    /**
     * Get a mapping of flags pointing to their corresponding plugin.
     *
     * @returns A map from flag to plugin name.
     */
    std::map<std::string, std::string> get_flag_to_plugin_mapping();

    /**
     * Returns command line interface options for all plugins
     *
     * @returns The program options
     */
    program_options get_cli_plugin_options();

    /**
     * Load all plugins in the specified diretories.<br>
     * If \p directory_names is empty, the default directories will be searched.
     *
     * @param[in] directory_names - A vector of directory paths.
     * @returns True on success.
     */
    bool load_all_plugins(const std::vector<hal::path>& directory_names = {});

    /**
     * Load a single plugin by specifying a name and the file path,.
     *
     * @param[in] plugin_name - The desired name, unique in the framework.
     * @param[in] file_name - The path to the plugin file.
     * @returns True on success.
     */
    bool load(const std::string& plugin_name, const hal::path& file_name);

    /**
     * Releases all plugins and associated resources.
     *
     * @returns True on success.
     */
    bool unload_all_plugins();

    /**
     * Releases all associated resources related to one plugin.
     *
     * @param[in] plugin_name - The plugin to unload.
     * @returns True on success.
     */
    bool unload(const std::string& plugin_name);

    /**
     * Gets the basic interface for a plugin specified by name.
     * By default calls the initialize() function of the plugin.
     *
     * @param[in] plugin_name - The plugin name.
     * @param[in] initialize - If false, the plugin's initialize function is not called.
     * @returns A plugin instance.
     */
    std::shared_ptr<i_base> get_plugin_instance(const std::string& plugin_name, bool initialize = true);

    /**
     * Gets a specific interface for a plugin specified by name.
     * By default calls the initialize() function of the plugin.
     *
     * @param[in] plugin_name - The plugin name.
     * @param[in] initialize - If false, the plugin's initialize function is not called.
     * @returns A plugin instance.
     */
    template<typename T>
    std::shared_ptr<T> get_plugin_instance(const std::string& plugin_name, bool initialize = true)
    {
        return std::dynamic_pointer_cast<T>(get_plugin_instance(plugin_name, initialize));
    }

    /**
     * Add a callback to notify the GUI about loaded or unloaded plugins.
     *
     * @param[in] callback - The callback function. Parameters are:
     * * bool:True = load, false = unload.
     * * std::string - The plugin name.
     * * std::string - The plugin path.
     * @returns The id of the registered callback.
    */
    u64 add_model_changed_callback(std::function<void(bool, std::string const&, std::string const&)> callback);

    /**
     * Remove a registered callback.
     *
     * @param[in] id - The id of the registered callback.
    */
    void remove_model_changed_callback(u64 id);
}    // namespace plugin_manager
