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
#include <QList>
#include <QObject>
#include <QEvent>
#include <QApplication>
#include <QTextStream>

Q_DECLARE_METATYPE(u32)

namespace hal
{
    class LayoutLocker;

    class CliExtensionsGui : public CliExtensionInterface
    {
    public:
        CliExtensionsGui() = default;
        ~CliExtensionsGui() = default;

        /**
         * Returns command line interface options
         *
         * @returns The program options description.
         */
        ProgramOptions get_cli_options() const override;

        virtual bool handle_cli_call(Netlist*, ProgramArguments&) override {return false; }
    };

    class DebugApplication : public QApplication
    {
        QFile mFile;
        QTextStream* mOut;
    public:
        DebugApplication(int& argc, char** argv);
        virtual ~DebugApplication() { if (mOut) delete mOut; };
        virtual bool notify(QObject *receiver, QEvent *e) override;
    };

    /**
     * @ingroup gui
     * PluginGui is a huge plugin that provides a graphical user interface to load and work with netlists. If compiled
     * it can be started using the '-g'/'--gui' flag.
     */
    class PluginGui : public UIPluginInterface
    {
        QList<LayoutLocker*> mLayoutLockerList;

    public:
        PluginGui() { m_extensions.push_back(new CliExtensionsGui); }

        /**
         * Returns the plugin name: 'hal_gui'
         *
         * @returns the plugin name
         */
        std::string get_name() const override;

        /**
         * Returns the current version of the plugin.
         *
         * @returns the current version number of the plugin
         */
        std::string get_version() const override;

        /**
         * Short description of plugin.
         *
         * @returns a short description of plugin
         */
        std::string get_description() const override;

        /**
         * Adds all gui related channels to the LogManager.
         */
        void initialize_logging() override;

        /**
         * Executes the gui plugin.
         *
         * @param args - The given program arguments.
         * @return True on success, False otherwise.
         */
        bool exec(ProgramArguments& args) override;

        /**
         * Call to block layouter.
         *
         * Can be enabled multiple times, but each enable must be match by disable to remove the lock.
         * @param[in] enable Enable lock on true, disable on false
         */
        void set_layout_locker(bool enable) override;
    };
}    // namespace hal
