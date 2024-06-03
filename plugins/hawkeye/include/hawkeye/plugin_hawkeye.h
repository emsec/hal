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

/**
 * @file plugin_hawkeye.h 
 * @brief This file contains all functions related to the HAL plugin API.
 */

#pragma once

#include "hal_core/plugin_system/plugin_interface_base.h"

namespace hal
{
    /**
     * @class HawkeyePlugin
     * @brief Plugin interface for HAWKEYE.
     * 
     * This class provides an interface to integrate the HAWKEYE tool as a plugin within the HAL framework.
     */
    class PLUGIN_API HawkeyePlugin : public BasePluginInterface
    {
    public:
        /** 
         * @brief Default constructor for `HawkeyePlugin`.
         */
        HawkeyePlugin() = default;

        /** 
         * @brief Default destructor for `HawkeyePlugin`.
         */
        ~HawkeyePlugin() = default;

        /**
         * @brief Get the name of the plugin.
         *
         * @returns The name of the plugin.
         */
        std::string get_name() const override;

        /**
         * @brief Get the version of the plugin.
         *
         * @returns The version of the plugin.
         */
        std::string get_version() const override;

        /**
         * @brief Get a short description of the plugin.
         *
         * @returns The short description of the plugin.
         */
        std::string get_description() const override;

        /**
         * @brief Get the plugin dependencies.
         * 
         * @returns A set of plugin names that this plugin depends on.
         */
        std::set<std::string> get_dependencies() const override;
    };
}    // namespace hal
