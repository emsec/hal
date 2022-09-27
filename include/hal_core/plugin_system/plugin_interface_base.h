//  MIT License
//
//  Copyright (c) 2019 Ruhr University Bochum, Chair for Embedded Security. All Rights reserved.
//  Copyright (c) 2021 Max Planck Institute for Security and Privacy. All Rights reserved.
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
#include "hal_core/utilities/log.h"
#include "hal_core/plugin_system/plugin_parameter.h"
#include "hal_core/netlist/netlist.h"

#include <set>
#include <string>

namespace hal
{
/**
 * Automatically declares a factory function when including this header.
 * Has to be implemented in the plugin.
 */
#ifdef COMPILER_CLANG
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wreturn-type-c-linkage"
#endif
    class BasePluginInterface;
    extern "C" PLUGIN_API std::unique_ptr<BasePluginInterface> create_plugin_instance();
#ifdef COMPILER_CLANG
#pragma clang diagnostic pop
#endif

    /**
     * Enum for all possible plugin types
     * 
     * @ingroup plugins
     */
    enum class CORE_API PluginInterfaceType
    {
        base,
        cli,
        interactive_ui,
        gui
    };

    /**
     * @ingroup plugins
     */
    class CORE_API BasePluginInterface
    {
    public:
        BasePluginInterface()          = default;
        virtual ~BasePluginInterface() = default;

        /**
         * Plugins utilize two phase construction.
         * Always populate all members etc in the initialize function which is automatically called by
         * plugin_manager::get_plugin_instance()
         */
        virtual void initialize();

        /**
         * Get the name of the plugin.
         *
         * @returns The name of the plugin.
         */
        virtual std::string get_name() const = 0;

        /**
         * Get the version of the plugin.
         *
         * @returns The version of the plugin.
         */
        virtual std::string get_version() const = 0;

        /**
         * Check whether the plugin has a specific type.
         *
         * @param[in] t - the type to check
         * @returns True, if the type is supported.
         */
        bool has_type(PluginInterfaceType t) const;

        /**
         * Shorthand for fast text logging.
         *
         * @param[in] args - The format string, followed by values.
         */
        template<typename... Args>
        inline void log(const Args&... args) const
        {
            log_info(get_name(), args...);
        }

        /**
         * Get all plugin dependencies of this plugin.
         *
         * @return A set of plugins that this plugin depends on.
         */
        virtual std::set<std::string> get_dependencies() const;

        /**
         * This function is automatically executed when the factory is loaded by the plugin manager
         */
        virtual void on_load();

        /**
         * This function is automatically executed when the factory is unloaded by the plugin manager
         */
        virtual void on_unload();

        /**
         * Initializes the logging channel(s) of a plugin. <br>
         * If not overwritten, a logging channel equal to the plugin name is created.
         */
        virtual void initialize_logging();

        /**
         * Get list of configurable parameter
         * @return List of configurable parameter
         */
        virtual std::vector<PluginParameter> get_parameter() const;

        /**
         * Set values for configurable parameter
         * @param[in] nl The current netlist
         * @param[in] params List of configurable parameter with values
         */
        virtual void set_parameter(Netlist* nl, const std::vector<PluginParameter>& params);

        /**
         * Register function to indicate work progress when busy
         * @param pif Progress Indicator Function to register
         */
        virtual void register_progress_indicator(std::function<void(int,const std::string&)> pif);
    };

    using instantiate_plugin_function = std::unique_ptr<BasePluginInterface> (*)();
}    // namespace hal
