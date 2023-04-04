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

#include <vector>
#include <mutex>
#include "hal_core/defines.h"
#include "hal_core/plugin_system/plugin_parameter.h"

namespace hal {
    class GuiExtensionPythonBase
    {
        std::string m_tagname;
        std::string m_label;

        std::vector<u32> m_mods_selected;
        std::vector<u32> m_gats_selected;
        std::vector<u32> m_nets_selected;

        std::string m_function_call;
    public:
        GuiExtensionPythonBase(const std::string& tag, const std::string& lab);
        ~GuiExtensionPythonBase();

        std::string get_tagname() const { return m_tagname; }

        std::string get_label() const { return m_label; }

        std::vector<PluginParameter> get_parameter() const;

        virtual void add_module_context(const std::string tagname, const std::string label);

        virtual void add_gate_context(const std::string tagname, const std::string label);

        virtual void add_net_context(const std::string tagname, const std::string label);

        virtual void add_main_menu(const std::vector<PluginParameter>& params);

        virtual void set_selection(const std::vector<u32>& mods, const std::vector<u32>& gats, const std::vector<u32>& nets);

        virtual void set_function_call(const std::string& fc);

        std::string get_function_call() const;

        std::vector<u32> get_selected_modules() const;

        std::vector<u32> get_selected_gates() const;

        std::vector<u32> get_selected_nets() const;

        void clear();
    };
}
