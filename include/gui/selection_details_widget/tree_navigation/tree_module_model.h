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
#include "netlist/event_system/gate_event_handler.h"
#include "netlist/event_system/net_event_handler.h"
#include "netlist/event_system/module_event_handler.h"
#include "gui_utils/sort.h"

#include <QAbstractItemModel>
#include <QFont>
#include <QIcon>
#include <QModelIndex>
#include <QVariant>

namespace hal
{
    class tree_ModuleItem;

    class tree_ModuleModel : public QAbstractItemModel
    {
        Q_OBJECT

    public:
        tree_ModuleModel(QObject* parent = 0);
        ~tree_ModuleModel();

        //information access
        QVariant data(const QModelIndex& index, int role) const Q_DECL_OVERRIDE;
        QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
        QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const Q_DECL_OVERRIDE;
        QModelIndex parent(const QModelIndex& index) const Q_DECL_OVERRIDE;
        Qt::ItemFlags flags(const QModelIndex& index) const Q_DECL_OVERRIDE;
        int rowCount(const QModelIndex& parent = QModelIndex()) const Q_DECL_OVERRIDE;
        int columnCount(const QModelIndex& parent = QModelIndex()) const Q_DECL_OVERRIDE;

        //this function provides the indexes for the navigation widget to handle a selection_changed for the view
        QModelIndexList get_corresponding_indexes(const QList<u32>& gate_ids, const QList<u32>& net_ids);

        void update(u32 module_id);

        static const int NAME_COLUMN = 0;
        static const int ID_COLUMN   = 1;
        static const int TYPE_COLUMN = 2;

    private:
        void setup_model_data();

        //helper functions to convert between index and item
        tree_ModuleItem* get_item(const QModelIndex& index) const;
        QModelIndex get_modelindex(tree_ModuleItem* item);
        QList<QModelIndex> get_modelindexes_for_row(tree_ModuleItem* item);

        void insert_item(tree_ModuleItem* parent, int position, tree_ModuleItem* item);
        void remove_item(tree_ModuleItem* item);

        void load_data_settings();

        tree_ModuleItem* m_root_item;
        tree_ModuleItem* m_gates_item;
        tree_ModuleItem* m_nets_item;

        QIcon m_design_icon;
        QFont m_structured_font;
    };
}
