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

#include "hal_core/defines.h"

#include "hal_core/netlist/event_system/module_event_handler.h"

#include "gui/module_model/module_item.h"
#include "gui/module_model/module_model.h"

#include <QColor>
#include <QMap>
#include <QObject>
#include <QSet>
#include <QVector>

namespace hal
{
    class ModuleRelay : public QObject
    {
        Q_OBJECT

        struct ModulePin
        {
            std::string mName;
            u32 mNet;
        };

    public:
        explicit ModuleRelay(QObject* parent = nullptr);

        void addSelectionToModule(const u32 id);

        void setModuleColor(const u32 id, const QColor& color);
        void setModuleCompressed(const u32 id, const bool mCompressed);
        void setModuleHidden(const u32 id, const bool hidden);

    public Q_SLOTS:
        void handleModuleEvent(module_event_handler::event ev, Module* object, u32 associated_data);

    private:
        QMap<u32, ModuleItem*> mModuleItems;
        ModuleModel* mModel;
    };
}
