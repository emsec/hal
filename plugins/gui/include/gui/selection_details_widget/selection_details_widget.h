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
#include "gui/selection_details_widget/tree_navigation/selection_tree_item.h"
#include "hal_core/def.h"

class QTableWidget;
class QStackedWidget;
class QSplitter;
class QListWidget;
class QLabel;

namespace hal
{
    class Searchbar;
    class SelectionTreeView;
    class ModuleDetailsWidget;
    class GateDetailsWidget;
    class NetDetailsWidget;

    class SelectionDetailsWidget : public ContentWidget
    {
        Q_OBJECT

    public:
        SelectionDetailsWidget(QWidget* parent = 0);
        void clear();

    Q_SIGNALS:
        void triggerHighlight(QVector<const SelectionTreeItem*> highlight);

    public Q_SLOTS:
        void handle_selection_update(void* sender);
        void handleTreeSelection(const SelectionTreeItem* sti);
        QList<QShortcut*> create_shortcuts() Q_DECL_OVERRIDE;

    private:
        void singleSelectionInternal(const SelectionTreeItem* sti);

        QSplitter*           m_splitter;
        SelectionTreeView*   m_selectionTreeView;
        QWidget*             m_selectionDetails;
        unsigned int         m_numberSelectedItems;

        QStackedWidget* m_stacked_widget;

        QWidget* m_empty_widget;
        GateDetailsWidget* m_gate_details;
        NetDetailsWidget* m_net_details;
        ModuleDetailsWidget* m_module_details;
        QLabel* m_item_deleted_label;

        Searchbar* m_searchbar;

        void toggle_searchbar();
    };
}
