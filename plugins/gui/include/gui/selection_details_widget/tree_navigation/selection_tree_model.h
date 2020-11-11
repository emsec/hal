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
#include "hal_core/netlist/event_system/gate_event_handler.h"
#include "hal_core/netlist/event_system/net_event_handler.h"
#include "hal_core/netlist/event_system/module_event_handler.h"
#include "gui/gui_utils/sort.h"

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QFont>
#include <QIcon>
#include <QVariant>

namespace hal
{
    class SelectionTreeItem;
    class SelectionTreeItemModule;
    class SelectionTreeItemRoot;

    class SelectionTreeModel : public QAbstractItemModel
    {
        Q_OBJECT

    public:
        SelectionTreeModel(QObject* parent = 0);
        ~SelectionTreeModel();

        //information access
        QVariant data(const QModelIndex& index, int role) const Q_DECL_OVERRIDE;
        QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
        QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const Q_DECL_OVERRIDE;
        QModelIndex parent(const QModelIndex& index) const Q_DECL_OVERRIDE;
        Qt::ItemFlags flags(const QModelIndex& index) const Q_DECL_OVERRIDE;
        int rowCount(const QModelIndex& parent = QModelIndex()) const Q_DECL_OVERRIDE;
        int columnCount(const QModelIndex& parent = QModelIndex()) const Q_DECL_OVERRIDE;

        void fetchSelection(bool hasEntries);

        //helper functions to convert between index and item
        QModelIndex indexFromItem(SelectionTreeItem* item) const;

        void suppressedByFilter(QList<u32>& modIds, QList<u32>& gatIds, QList<u32>& netIds,
                                const QRegularExpression& regex) const;

        static const int sNameColumn = 0;
        static const int sIdColumn   = 1;
        static const int sTypeColumn = 2;
        static const int sMaxColumn  = 3;


    private:

        void moduleRecursion(SelectionTreeItemModule* modItem);
        bool doNotDisturb(const QModelIndex& inx = QModelIndex()) const;
        SelectionTreeItem* itemFromIndex(const QModelIndex& index) const;


        SelectionTreeItemRoot* mRootItem;

        /// avoid calls while model is under reconstruction
        int mDoNotDisturb;
    };

    class SelectionTreeModelDisposer : public QObject
    {
        Q_OBJECT
    public:
        SelectionTreeModelDisposer(SelectionTreeItemRoot* stim, QObject* parent=nullptr);

    public Q_SLOTS:
        void dispose();

    private:
        SelectionTreeItemRoot* mRootItem;
    };


}
