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

#include "hal_core/defines.h"
#include "hal_core/netlist/gate_library/enums/gate_type_property.h"
#include "gui/settings/settings_items/settings_item_dropdown.h"
#include <QIcon>
#include <QHash>
#include <QObject>

namespace hal
{
    class SelectionDetailsIconProvider : public QObject
    {
        Q_OBJECT

        SelectionDetailsIconProvider(QObject* parent = nullptr);
        static SelectionDetailsIconProvider* inst;
    public:
        enum IconCategory { ModuleIcon = -1, GateIcon = -2, NetIcon = -3};
        enum IconSize { NoIcon, SmallIcon, BigIcon };
        Q_ENUM(IconSize)

    private Q_SLOTS:
        void loadIcons(int istyle);
    private:
        QHash<IconCategory,const QIcon*> mDefaultIcons;
        QHash<int,const QIcon*> mGateIcons;
        QHash<void*,const QIcon*> mModuleIcons;
        static bool sSettingsInitialized;
        static bool initSettings();
    public:
        const QIcon *getIcon(IconCategory catg, u32 itemId);
        static SelectionDetailsIconProvider* instance();
        static SettingsItemDropdown* sIconSizeSetting;
    };
}

