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
 * @file plugin_netlist_preprocessing.h 
 * @brief This file contains all functions related to the HAL plugin API.
 */

#pragma once

#include "hal_core/plugin_system/gui_extension_interface.h"
#include "hal_core/plugin_system/plugin_interface_base.h"

namespace hal
{
    class Netlist;

    /**
     * @class NetlistPreprocessingPlugin
     * @brief Plugin interface for netlist preprocessing.
     *
     * This class provides an interface to integrate the netlist preprocessing as a plugin within the HAL framework.
     */
    class PLUGIN_API NetlistPreprocessingPlugin : public BasePluginInterface
    {
    public:
        /**
         * @brief Constructor for `NetlistPreprocessingPlugin` that registers the GUI extension.
         */
        NetlistPreprocessingPlugin();

        /** 
         * @brief Default destructor for `NetlistPreprocessingPlugin`.
         */
        ~NetlistPreprocessingPlugin() = default;

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

    /**
     * @class GuiExtensionNetlistPreprocessing
     * @brief GUI extension interface for the netlist preprocessing plugin.
     *
     * Contributes the most commonly used preprocessing steps to the context menus of the GUI, so that they can be
     * applied to the current selection or to the entire netlist without writing a script.
     */
    class PLUGIN_API GuiExtensionNetlistPreprocessing : public GuiExtensionInterface
    {
    public:
        /**
         * @brief Default constructor for `GuiExtensionNetlistPreprocessing`.
         */
        GuiExtensionNetlistPreprocessing() : GuiExtensionInterface("Netlist Preprocessing")
        {
        }

        /**
         * @brief Get the context menu entries contributed for the given selection.
         *
         * If modules or gates are selected, only the entries operating on that selection are contributed. The entries
         * operating on the entire netlist are contributed when nothing is selected.
         *
         * @param[in] nl - The netlist that is currently open.
         * @param[in] mods - The IDs of the currently selected modules.
         * @param[in] gats - The IDs of the currently selected gates.
         * @param[in] nets - The IDs of the currently selected nets.
         * @returns The contributed context menu entries.
         */
        std::vector<ContextMenuContribution> get_context_contribution(const Netlist* nl, const std::vector<u32>& mods, const std::vector<u32>& gats, const std::vector<u32>& nets) override;

        /**
         * @brief Execute the context menu entry identified by the given tag.
         *
         * @param[in] tag - The tag of the entry to execute.
         * @param[in] nl - The netlist that is currently open.
         * @param[in] mods - The IDs of the currently selected modules.
         * @param[in] gats - The IDs of the currently selected gates.
         * @param[in] nets - The IDs of the currently selected nets.
         */
        void execute_function(std::string tag, Netlist* nl, const std::vector<u32>& mods, const std::vector<u32>& gats, const std::vector<u32>& nets) override;
    };

}    // namespace hal
