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

#include "gui/gui_def.h"

#include <QColor>
#include <QStyle>
#include <QGraphicsItem>

namespace hal
{
    class GraphicsItem : public QGraphicsItem
    {
    public:
        static void load_settings();
        static void set_lod(const qreal lod);

        GraphicsItem(const hal::item_type type, const u32 id);

        hal::item_type item_type() const;
        u32 id() const;
        void setHightlight(bool hl);

    protected:
        static qreal s_lod;
        static QColor s_selectionColor;
        static QColor s_highlightColor;

        QColor penColor(QStyle::State state) const;

        hal::item_type m_item_type; // USE QT ITEM TYPE SYSTEM INSTEAD ???
        bool m_highlight;  // paint with hightlight color
        u32 m_id;
        QColor m_color;
    };
}
