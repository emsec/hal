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

#include "gui/selection_details_widget/tree_navigation/selection_tree_item.h"
#include "gui/selection_details_widget/tree_navigation/selection_tree_model.h"
#include "gui/selection_details_widget/tree_navigation/selection_tree_proxy.h"

#include <QTreeView>
#include <QWidget>

namespace hal
{
    class SelectionTreeView : public QTreeView
    {
        Q_OBJECT

    Q_SIGNALS:
        void triggerSelection(const SelectionTreeItem* sti);

    public Q_SLOTS:
        void handleFilterTextChanged(const QString& filter_text);
 
    protected:
        virtual void currentChanged(const QModelIndex& current, const QModelIndex& previous) Q_DECL_OVERRIDE;

    public:
        SelectionTreeView(QWidget* parent = nullptr);
        void setDefaultColumnWidth();
        void clearHide();
        void populate(bool mVisible);
        SelectionTreeItem* itemFromIndex(const QModelIndex& index = QModelIndex()) const;

    private Q_SLOTS:
        void handleCustomContextMenuRequested(const QPoint& point);

    private:
        SelectionTreeModel* mSelectionTreeModel;
        SelectionTreeProxyModel* mSelectionTreeProxyModel;
    };
}    // namespace hal
