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
#include "hal_core/plugin_system/gui_extension_interface.h"
#include "hal_core/plugin_system/plugin_interface_base.h"

#define SECRET_PASSWORD "test12345"

/**
 * @brief Header file for the cross probing plugin. This plugin is used by to control some HAL GUI features from external tool
 * 
 */

namespace hal
{
    extern Netlist* gNetlist;

    class GuiExtensionCrossProbing;
    class CrossProbingServer;

    class PLUGIN_API CrossProbingPlugin : public BasePluginInterface
    {
        GuiExtensionCrossProbing* m_gui_extension;
        CrossProbingServer* m_server;

    public:
        std::string get_name() const override;
        std::string get_version() const override;
        void initialize() override;

        CrossProbingPlugin();

        void on_load() override;
        void on_unload() override;

        std::set<std::string> get_dependencies() const override;

        void open_popup();
    };

    class GuiExtensionCrossProbing : public GuiExtensionInterface
    {
    public:
        CrossProbingPlugin* m_parent;

        GuiExtensionCrossProbing();

        std::vector<ContextMenuContribution> get_context_contribution(const Netlist* nl, const std::vector<u32>& mods, const std::vector<u32>& gates, const std::vector<u32>& nets) override;

        std::vector<PluginParameter> get_parameter() const override;

        void set_parameter(const std::vector<PluginParameter>& params) override;

        /**
         * Register function to indicate work progress when busy
         * @param pif Progress Indicator Function to register
         */
        static std::function<void(int, const std::string&)> s_progress_indicator_function;
        virtual void register_progress_indicator(std::function<void(int, const std::string&)> pif) override;
    };
}    // namespace hal
