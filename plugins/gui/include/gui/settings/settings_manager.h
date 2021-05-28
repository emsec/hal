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
#include <QList>
#include <QSettings>
#include "gui/settings/settings_items/settings_item.h"
#include <QPoint>
#include <QSize>

namespace hal
{
    /**
     * @ingroup settings
     * @brief Manages and persists the settings on the lowest level.
     *
     * This class is the connection between the actual settings file that is
     * saved on the disk and the logical SettingsItem%s.There exist two different
     * settings files, one for the default values and on for the actual user
     * specified values. This class is implemented as a singleton pattern.
     */
    class SettingsManager : public QObject
    {
        Q_OBJECT

    public:
        /**
         * Get the singleton instance of the SettingsManager.
         *
         * @return The singleton instance.
         */
        static SettingsManager* instance();
        
        /**
         * Appends the given item to its list and either sets the value from
         * the either the defaultFile or UserSettingsFile if it exits in one of them.
         *
         * @param item - The item to register.
         */
        void registerSetting(SettingsItem* item);
        QList<SettingsItem*> mSettingsList;

        /**
         * Writes all settings to the user settings file.
         */
        void persistUserSettings();

        /**
         * Get the MainWindow's position that is saved in the user settings file.
         *
         * @return The position.
         */
        QPoint mainWindowPosition() const;

        /**
         * Get the MainWindow's size that is saved in the user settings file.
         * @return
         */
        QSize mainWindowSize() const;

        /**
         * Writes the MainWindow's position and size to the user settings file.
         *
         * @param pos - The MainWindow's position.
         * @param size - The MainWindow's size.
         */
        void mainWindowSaveGeometry(const QPoint& pos, const QSize& size);

        /**
         * Get the value in the user settings file for a given key.
         *
         * @param tag - The key.
         * @return The value encapsulated in a QVariant.
         */
        QVariant settingsValue(const QString& tag) const;

        /**
         * Get the value in the default settings file for a given key.
         *
         * @param tag - The key.
         * @return The value encapsulated in a QVariant.
         */
        QVariant defaultValue(const QString& tag) const;

    public Q_SLOTS:
        /**
         * Removes the item -if possible- from the manager's settings list.
         *
         * @param obj - The item to be removed.
         */
        void handleItemDestroyed(QObject* obj);

    private:
        SettingsManager(QObject *parent = nullptr);
        static SettingsManager* inst;

        QSettings* mDefaultSettingsFile;
        QSettings* mUserSettingsFile;
    };
}
