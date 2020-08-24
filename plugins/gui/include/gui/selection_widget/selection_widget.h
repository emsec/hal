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

#include "gui/content_widget/content_widget.h"
#include "hal_core/defines.h"
#include "gui/searchbar/searchbar.h"
#include "gui/selection_widget/models/gate_table_model.h"
#include "gui/selection_widget/models/gate_table_proxy_model.h"
#include "gui/selection_widget/models/net_table_model.h"
#include "gui/selection_widget/models/net_table_proxy_model.h"

#include <QTableView>

namespace hal
{
    class selection_widget : public ContentWidget
    {
        Q_OBJECT

    public:
        selection_widget(QWidget* parent = nullptr);

    public Q_SLOTS:
        void handle_selection_update(void* sender);

    private:
        Searchbar mSearchbar;

        QTableView* m_gate_table_view;
        QTableView* m_net_table_view;

        gate_table_model* m_gate_table_model;
        net_table_model* m_net_table_model;

        gate_table_proxy_model* m_gate_table_proxy_model;
        net_table_proxy_model* m_net_table_proxy_model;

        void toggle_searchbar();
        QList<QShortcut*> create_shortcuts();

        void update();
    };
}    // namespace hal
