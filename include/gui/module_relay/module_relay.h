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

#include "def.h"

#include "netlist/event_system/module_event_handler.h"

#include "module_model/module_item.h"
#include "module_model/module_model.h"

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

        struct module_pin
        {
            std::string name;
            u32 net;
        };

        // PROBABLY OBSOLETE, INTEGRATE INTO ITEMS
        struct module_extension
        {
            module_extension(const u32 module_id) : id(module_id) {}
            ~module_extension();

            u32 id;
            QSet<module_pin> input_pins; // UNSURE
            QSet<module_pin> output_pins; // UNSURE
            bool isolated = true; // UNSURE
            QColor color = QColor(255, 255, 255);
            bool compressed = false;
            bool hidden = false;
            QVector<u32> stray_nets; // UNSURE
        };

    public:
        explicit ModuleRelay(QObject* parent = nullptr);

        void add_selection_to_module(const u32 id);

        void set_module_color(const u32 id, const QColor& color);
        void set_module_compressed(const u32 id, const bool compressed);
        void set_module_hidden(const u32 id, const bool hidden);

    public Q_SLOTS:
        void handle_module_event(module_event_handler::event ev, std::shared_ptr<Module> object, u32 associated_data);

    private:
        QMap<u32, ModuleItem*> m_ModuleItems;
        ModuleModel* m_model;
    };
}
