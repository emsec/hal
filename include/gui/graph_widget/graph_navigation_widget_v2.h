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

#include "netlist/net.h"

#include "gui/gui_def.h"

#include <QTreeWidget>

namespace hal
{
    class GraphGraphicsView;

    class GraphNavigationWidgetV2 : public QTreeWidget
    {
        Q_OBJECT

    public:
        explicit GraphNavigationWidgetV2(QWidget *parent = nullptr);

        void setup(bool direction);
        void setup(hal::node origin, Net* via_net, bool direction);

    Q_SIGNALS:
        void navigation_requested(const hal::node origin, const u32 via_net, const QSet<u32>& to_gates, const QSet<u32>& to_modules);
        void close_requested();
        void reset_focus();

    private Q_SLOTS:
        void handle_selection_changed();

    protected:
        void keyPressEvent(QKeyEvent* event) override;

    private:
        void fill_table(bool direction);
        void resize_to_fit();
        int sum_row_heights(const QTreeWidgetItem *itm, bool top = true);
        void handle_item_double_clicked(QTreeWidgetItem* item);
        void commit_selection();

        hal::node m_origin;
        Net* m_via_net;
        QSet<QTreeWidgetItem*> m_previous_selection;

    };
}
