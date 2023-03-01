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

#include "hal_core/plugin_system/plugin_interface_ui.h"
#include "hal_core/plugin_system/cli_extension_interface.h"

namespace hal
{
    class CliExtensionPythonShell : public CliExtensionInterface
    {
    public:
        CliExtensionPythonShell() = default;
        ~CliExtensionPythonShell() = default;

        /**
         * Returns command line interface options
         *
         * @returns The program options description.
         */
        ProgramOptions get_cli_options() const override;

        virtual bool handle_cli_call(Netlist*, ProgramArguments&) override {return false; }
    };

    class PluginPythonShell : virtual public UIPluginInterface
    {
    public:
        PluginPythonShell() { m_extensions.push_back(new CliExtensionPythonShell); }

        /*
         *      interface implementations
         */

        /**
         * Get the name of the plugin.
         *
         * @returns Plugin name.
         */
        std::string get_name() const override;

        /**
         * Get the version of the plugin.
         *
         * @returns Plugin version.
         */
        std::string get_version() const override;

        /**
         * Excutes the plugin with given command line parameters.
         *
         * @param[in] args - The command line parameters.
         * @returns True on success.
         */
        bool exec(ProgramArguments& args) override;

        /**
         * Nothing to do for GUI layout locker
         * @param enable[in] unused
         */
        void set_layout_locker(bool) override {;}
    };
}    // namespace hal
