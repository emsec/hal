//  MIT License
//
//  Copyright (c) 2019 Ruhr-University Bochum, Germany, Chair for Embedded Security. All Rights reserved.
//  Copyright (c) 2019 Marc Fyrbiak, Sebastian Wallat, Max Hoffmann ("ORIGINAL AUTHORS"). All rights reserved.
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
#include <QSettings>

namespace hal
{
    /**
     * Global settings management and file-backed storage.<br>
     * Default settings can be shipped in the application-wide config directory
     * (@see core_utils::get_config_directory) via guidefaults.ini.
     * Per-user settings will be stored in the user's config directory
     * (@see core_utils::get_user_config_directory) and will override any
     * default settings.
     * 
     * Settings are identified by a key string "group/item" and contain any
     * QVariant.
     */
    class SettingsManager : public QObject
    {
        Q_OBJECT

    public:
        /**
         * Constructs a new settings manager.
         * 
         * @param[in] parent - The Qt parent object.
         */
        explicit SettingsManager(QObject* parent = nullptr);
        ~SettingsManager();

        /**
         * Get the value of the specified setting.<br>
         * 
         * If no user specified value exists in guisettings.ini, then the
         * value from guidefaults.ini is used. If no value exists there
         * either, QVariant() is returned.
         * 
         * @param[in] key - The setting's key.
         * @returns The setting's value or QVariant() if nonexistent.
         */
        QVariant get(const QString& key);

        /**
         * Get the value of the specified setting.<br>
         * 
         * If no user specified value exists in guisettings.ini, then
         * defaultVal is returned without checking guidefaults.ini.
         * 
         * @param[in] key - The setting's key.
         * @returns The setting's value or defaultValue if nonexistent.
         */
        QVariant get(const QString& key, const QVariant& defaultVal);

        /**
         * Get the package maintainer's default of the specified setting.<br>
         * 
         * If no maintainer specified value exists in guidefaults.ini,
         * QVariant() is returned.
         * 
         * @param[in] key - The setting's key.
         * @returns The setting's default value or QVariant() if nonexistent.
         */
        QVariant get_default(const QString& key);

        /**
         * Clear the user specified value of the specified setting if
         * one exists, reverting to the default value from guidefaults.ini.
         * This will remove the key from guisettings.ini.
         * 
         * A setting changed signal will fire from the settings relay.
         * 
         * @param[in] key - The setting's key.
         * @returns The default value of the setting.
         */
        QVariant reset(const QString& key);

        /**
         * Set the setting specified by the key to the given value.<br>
         * 
         * If the supplied value matches the default value from
         * guidefaults.ini, then the key is removed from guisettings.ini.<br>
         * 
         * A setting changed signal will fire from the settings relay.
         * 
         * @param[in] key - The setting's key.
         * @param[in] value - The setting's new value.
         */
        void update(const QString& key, const QVariant& value);

        /**
         * Force the user specified settings to synchronize.<br>
         * 
         * External changes to the settings file will NOT fire
         * setting changed signals from the settings relay.
         * 
         */
        void sync();

    Q_SIGNALS:
        /**
         * Signals the change of a global setting.
         * 
         * @param[in] sender - The signal's sender.
         * @param[in] key - The setting's key.
         * @param[in] val - The setting's new value.
         */
        void setting_changed(void* sender, const QString& key, const QVariant& val);

    private:
        QSettings* m_settings;
        QSettings* m_defaults;
    };
}
