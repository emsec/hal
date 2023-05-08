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

    class plugin_dataflow;

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
        dataflow::Configuration m_config;
        std::string m_output_path;
        bool m_write_dot;
        bool m_write_txt;
        bool m_create_modules;
        bool m_button_clicked;

    public:
        GuiExtensionDataflow() : m_output_path("/tmp"), m_write_dot(false), m_write_txt(false), m_create_modules(false), m_button_clicked(false)
        {
        }

        /**
         * Get list of configurable parameter
         *
         * @returns  list of parameter
         */
        std::vector<PluginParameter> get_parameter() const override;

        /**
         * Set configurable parameter to values
         * @param params The parameter with values
         */
        void set_parameter(const std::vector<PluginParameter>& params) override;

        void execute_function(std::string tag, Netlist* nl, const std::vector<u32>& mods, const std::vector<u32>& gats, const std::vector<u32>& nets) override;

        /**
         * Register function to indicate work progress when busy
         * @param pif Progress Indicator Function to register
         */
        virtual void register_progress_indicator(std::function<void(int, const std::string&)> pif) override;

        static std::function<void(int, const std::string&)> s_progress_indicator_function;
    };

    class PLUGIN_API plugin_dataflow : public BasePluginInterface
    {
    public:
        /*
         *      interface implementations
         */

        plugin_dataflow();
        ~plugin_dataflow() = default;

        /**
         * Get the name of the plugin.
         *
         * @returns The name of the plugin.
         */
        std::string get_name() const override;

        /**
         * Get short description for plugin.
         *
         * @return The short description.
         */
        std::string get_description() const override;

        /**
         * Get the version of the plugin.
         *
         * @returns The version of the plugin.
         */
        std::string get_version() const override;

        /**
         * \deprecated
         */
        [[deprecated("Will be removed in a future version, use dataflow::analyze instead.")]] std::vector<std::vector<Gate*>> execute(Netlist* nl,
                                                                                                                                      std::string out_path,
                                                                                                                                      const std::vector<u32> sizes,
                                                                                                                                      bool draw_graph,
                                                                                                                                      bool create_modules                        = false,
                                                                                                                                      bool register_stage_identification         = false,
                                                                                                                                      std::vector<std::vector<u32>> known_groups = {},
                                                                                                                                      u32 min_group_size                         = 8);
    };
}    // namespace hal
