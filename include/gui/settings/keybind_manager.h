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

#include "gui/action/action.h"

#include <QAction>
#include <QHash>
#include <QObject>
#include <QSet>
#include <QShortcut>

namespace hal
{
    /**
     * Helper to create keybinds from global settings and update them
     * when that setting's value changes.
     */
    class KeybindManager : public QObject
    {
        Q_OBJECT

    public:
        /**
         * Constructs a new keybind manager.
         * 
         * @param[in] parent - The Qt parent object.
         */
        explicit KeybindManager(QObject* parent = nullptr);

        /**
         * Associate an Action with a global setting and keep it updated.
         * 
         * @param[in] action - The Action object.
         * @param[in] key - The setting's key.
         */
        void bind(Action* action, const QString& key);

        /**
         * Associate a QShortcut with a global setting and keep it updated.
         * 
         * @param[in] shortcut - The QShortcut object.
         * @param[in] key - The setting's key.
         */
        void bind(QShortcut* shortcut, const QString& key);

        /**
         * Stop keeping track of an Action.
         * 
         * @param[in] action - The Action object.
         */
        void release(Action* action);

        /**
         * Stop keeping track of a QShortcut.
         * 
         * @param[in] shortcut - The QShortcut object.
         */
        void release(QShortcut* shortcut);

        /**
         * Convenience method to create a QShortcut and keep track of it immediately,
         * as if bind was called on it.
         * 
         * @param[in] parent - The QShortcut's Qt parent object.
         * @param[in] key - The key of the setting to associate.
         */
        QShortcut* make_shortcut(QWidget* parent, const QString& key);

    private Q_SLOTS:
        void handle_global_setting_changed(void* sender, const QString& key, const QVariant& value);

    private:
        QHash<QString, Action*> m_bindings_actions;
        QSet<Action*> m_bound_actions;

        QHash<QString, QShortcut*> m_bindings_shortcuts;
        QSet<QShortcut*> m_bound_shortcuts;
    };

    template<typename T>
    void delete_all_values(QHash<QString, T> map, T value)
    {
        for (auto it = map.begin(); it != map.end();)
        {
            if (it.value() == value)
            {
                it = map.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }
}
