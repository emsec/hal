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
#include <QList>

namespace hal
{
    class SelectionDetailsIconProvider : public QObject
    {
        Q_OBJECT

        SelectionDetailsIconProvider(QObject* parent = nullptr);
        static SelectionDetailsIconProvider* inst;
    public:
        enum IconCategory { ModuleIcon = -1, GateIcon = -2, NetIcon = -3, ViewDir = -4, ViewCtx = -5};
        enum IconSize { NoIcon, SmallIcon, BigIcon };
        Q_ENUM(IconSize)

    private Q_SLOTS:
        void loadIcons(int istyle);
        void handleModuleColorChanged(u32 id);
    private:
        QHash<IconCategory,const QIcon*> mDefaultIcons;
        QHash<int,const QIcon*> mGateIcons;
        QHash<u32,const QIcon*> mModuleIcons;
        static bool sSettingsInitialized;
        static bool initSettings();
    public:
        /**
         * Returns an icon for the given styled netlist element.
         * If the element is a module, a module icon is returned, 
         * colored after the modules color.
         * If the item is a gate, the gate icon corresponding to 
         * the gates type is returned.
         * 
         * @param catg - The type of the netlist element, i.e. Module or Gate.
         * @param itemId - The id of the netlist element.
         * 
         * @return The icon for the specified netlist element.
         */
        const QIcon *getIcon(IconCategory catg, u32 itemId);

        /**
         * Since module icons get cached the appropriate icon
         * needs to be removed when the module gets removed
         *
         * @param moduleId - The id of the module removed
         */
        void handleModuleRemoved(u32 moduleId);

        /**
         * Get the singleton instance of the SettingsManager.
         *
         * @return The singleton instance.
         */
        static SelectionDetailsIconProvider* instance();

        /**
         * The current icon setting. Changed by user in the settings.
         */
        static SettingsItemDropdown* sIconSizeSetting;
    };
}
