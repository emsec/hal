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

#include "content_widget/content_widget.h"
#include "def.h"
#include "selection_details_widget/gate_details_widget.h"
#include "selection_details_widget/net_details_widget.h"
#include "selection_details_widget/module_details_widget.h"

class QTableWidget;
class QStackedWidget;

namespace hal
{
    class searchbar;

    class selection_details_widget : public ContentWidget
    {
        Q_OBJECT

    public:
        selection_details_widget(QWidget* parent = 0);
        void clear();

    public Q_SLOTS:
        void handle_selection_update(void* sender);
        QList<QShortcut*> create_shortcuts() Q_DECL_OVERRIDE;

    private:
        QStackedWidget* m_stacked_widget;

        QWidget* m_empty_widget;
        gate_details_widget* m_gate_details;
        net_details_widget* m_net_details;
        module_details_widget* m_module_details;
        QLabel* m_item_deleted_label;

        searchbar* m_searchbar;

        void toggle_searchbar();
    };
}
