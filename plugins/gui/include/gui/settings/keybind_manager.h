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
    class KeybindManager : public QObject
    {
        Q_OBJECT

    public:
        explicit KeybindManager(QObject* parent = nullptr);
        void bind(Action* action, const QString& key);
        void bind(QShortcut* shortcut, const QString& key);
        void release(Action* action);
        void release(QShortcut* shortcut);
        QShortcut* makeShortcut(QWidget* parent, const QString& key);

    private Q_SLOTS:
        void handleGlobalSettingChanged(void* sender, const QString& key, const QVariant& value);

    private:
        QHash<QString, Action*> mBindingsActions;
        QSet<Action*> mBoundActions;

        QHash<QString, QShortcut*> mBindingsShortcuts;
        QSet<QShortcut*> mBoundShortcuts;
    };

    template<typename T>
    void deleteAllValues(QHash<QString, T> map, T value)
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
