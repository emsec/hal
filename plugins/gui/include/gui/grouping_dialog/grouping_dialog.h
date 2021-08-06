//  MIT License
//
//  Copyright (c) 2019 Ruhr University Bochum, Chair for Embedded Security. All Rights reserved.
//  Copyright (c) 2021 Max Planck Institute for Security and Privacy. All Rights reserved.
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

#include "gui/grouping/grouping_table_model.h"
#include "gui/grouping/grouping_proxy_model.h"
#include "gui/searchbar/searchbar.h"

#include <QDialog>
#include <QTableView>
#include <QDialogButtonBox>

namespace hal {

    class GroupingDialog : public QDialog
    {
        Q_OBJECT
    public:
        GroupingDialog(QWidget* parent=nullptr);
        QString groupName() const { return mGroupName; }
    private Q_SLOTS:
        void handleNewGroupingClicked();
        void handleToggleSearchbar();
        void handleFilterTextChanged(const QString& text);
        void handleDoubleClicked(const QModelIndex& index);
        void handleSelectionChanged();
    private:
        QString getGroupName(const QModelIndex& proxyIndex);

        GroupingTableModel* mGroupingTableModel;
        QTableView* mGroupingTableView;
        GroupingProxyModel* mProxyModel;
        QDialogButtonBox* mButtonBox;
        Searchbar* mSearchbar;

        QString mGroupName;
    };
}
