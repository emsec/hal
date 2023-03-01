// MIT License
//
// Copyright (c) 2019 Ruhr University Bochum, Chair for Embedded Security. All Rights reserved.
// Copyright (c) 2019 Marc Fyrbiak, Sebastian Wallat, Max Hoffmann ("ORIGINAL AUTHORS"). All rights reserved.
// Copyright (c) 2021 Max Planck Institute for Security and Privacy. All Rights reserved.
// Copyright (c) 2021 Jörn Langheinrich, Julian Speith, Nils Albartus, René Walendy, Simon Klix ("ORIGINAL AUTHORS"). All Rights reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once

#include "hal_core/defines.h"
#include "hal_core/utilities/log.h"

#include <set>
#include <string>
#include <vector>

namespace hal
{
    class AbstractExtensionInterface;

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
     * @ingroup plugins
     */
    class CORE_API BasePluginInterface
    {
    protected:
        std::vector<AbstractExtensionInterface*> m_extensions;

    public:
        BasePluginInterface() {;}
        virtual ~BasePluginInterface();

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
         * Get short description about functionality
         *
         * @return description should not have more than 40 characters
         */
        virtual std::string get_description() const {return std::string(); };

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
         * Get GUI/CLI/FAC extension functionality if implemented by derived class
         * @return pointer to instance implementing extensions
         */
        virtual std::vector<AbstractExtensionInterface*> get_extensions() const;

        /**
         * Get first extension of given type T.
         * @return pointer to first extension of type T or nullptr if no such extension exists.
         */
        template<typename T> T* get_first_extension() const {
            for (AbstractExtensionInterface* aeif : get_extensions())
            {
                T* retval = dynamic_cast<T*>(aeif);
                if (retval) return retval;
            }
            return nullptr;
        }

        /**
         * Delete extension instance and free memory
         * @param aeif The extenstion instance to be deleted. Nothing will be done if not found in list.
         */
        void delete_extension(AbstractExtensionInterface* aeif);
    };

    using instantiate_plugin_function = std::unique_ptr<BasePluginInterface> (*)();
}    // namespace hal
