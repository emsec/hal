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

#include <QObject>
#include "hal_core/defines.h"
#include <string>

namespace hal
{
    /** @ingroup gui
     *  @brief Forwards plugin related events from the core to the gui.
     */
    class PluginRelay : public QObject
    {
        Q_OBJECT

    public:
        /**
         * The constructor. Connects to the core's plugin manager.
         *
         * @param parent - The relay's parent.
         */
        explicit PluginRelay(QObject* parent = nullptr);

        /**
         * The destructor. Removes the relay's connection from the core.
         */
        ~PluginRelay();

        /**
         * Callback function that is connected to the core. Emits the appropriate signals.
         *
         * @param is_load - True if the plugin is loaded, False if unloaded.
         * @param plugin_name - The plugin's name.
         * @param plugin_path - The plugin's path.
         */
        void pluginManagerCallback(bool is_load, const std::string& plugin_name, const std::string& plugin_path);

    Q_SIGNALS:
        /**
         * Q_SIGNAL that is emitted when a plugin is loaded.
         *
         * @param name - The plugin's name.
         * @param path - The plugin's path.
         */
        void pluginLoaded(const QString& name, const QString& path);

        /**
         * Q_SIGNAL that is emitted when a plugin is unloaded.
         *
         * @param name - The plugin's name.
         * @param path - The plugin's path.
         */
        void pluginUnloaded(const QString& name, const QString& path);

    private:
        u64 mCallbackId;
    };
}
