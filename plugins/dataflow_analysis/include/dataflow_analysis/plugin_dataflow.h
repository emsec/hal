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

#include "dataflow_analysis/api/dataflow.h"
#include "hal_core/plugin_system/cli_extension_interface.h"
#include "hal_core/plugin_system/gui_extension_interface.h"
#include "hal_core/plugin_system/plugin_interface_base.h"

#include <vector>

namespace hal
{
    /* forward declaration */
    class Netlist;
    class Gate;

    class DataflowPlugin;

    class CliExtensionDataflow : public CliExtensionInterface
    {
    public:
        CliExtensionDataflow()
        {
        }
        virtual ProgramOptions get_cli_options() const override;
        virtual bool handle_cli_call(Netlist* netlist, ProgramArguments& args) override;
    };

    class GuiExtensionDataflow : public GuiExtensionInterface
    {
        std::string m_output_path = "/tmp";
        bool m_write_dot          = false;
        bool m_write_txt          = false;
        bool m_create_modules     = false;
        bool m_button_clicked     = false;

        std::vector<u32> m_expected_sizes = {};
        u32 m_min_group_size              = 8;
        bool m_enable_stages              = false;

    public:
        /**
         * @brief Default constructor for `GuiExtensionDataflow`.
         */
        GuiExtensionDataflow() = default;

        /**
         * @brief Get a vector of configurable parameters.
         *
         * @returns The vector of parameters.
         */
        std::vector<PluginParameter> get_parameter() const override;

        /**
         * @brief Set values for a vector of configurable parameters.
         * 
         * @param[in] params - The parameters including their values.
         */
        void set_parameter(const std::vector<PluginParameter>& params) override;

        /**
         * @brief Execute the plugin on the given netlist. 
         * 
         * All parameters but the netlist are ignored.
         * 
         * @param[in] tag - A tag (ignored).
         * @param[in] nl - The netlist to operate on.
         * @param[in] mods - Module IDs (ignored).
         * @param[in] gats - Gate IDs (ignored).
         * @param[in] nets - Net IDs (ignored).
         */
        void execute_function(std::string tag, Netlist* nl, const std::vector<u32>& mods, const std::vector<u32>& gats, const std::vector<u32>& nets) override;

        /**
         * Register function to indicate work progress when busy
         * @param pif Progress Indicator Function to register
         */
        virtual void register_progress_indicator(std::function<void(int, const std::string&)> pif) override;

        static std::function<void(int, const std::string&)> s_progress_indicator_function;
    };

    /**
     * @class DataflowPlugin
     * @brief Plugin interface for the dataflow analysis plugin (DANA).
     * 
     * This class provides an interface to integrate the DANA tool as a plugin within the HAL framework.
     */
    class PLUGIN_API DataflowPlugin : public BasePluginInterface
    {
    public:
        /**
         * @brief Constructor for `DataflowPlugin`.
         */
        DataflowPlugin();

        /** 
         * @brief Default destructor for `DataflowPlugin`.
         */
        ~DataflowPlugin() = default;

        /**
         * @brief Get the name of the plugin.
         *
         * @returns The name of the plugin.
         */
        std::string get_name() const override;

        /**
         * @brief Get a short description of the plugin.
         *
         * @return The short description of the plugin.
         */
        std::string get_description() const override;

        /**
         * @brief Get the version of the plugin.
         *
         * @returns The version of the plugin.
         */
        std::string get_version() const override;
    };
}    // namespace hal
