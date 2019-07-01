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

#ifndef TREE_NAVIGATION_MODEL_H
#define TREE_NAVIGATION_MODEL_H

#include "def.h"
#include "netlist/event_system/gate_event_handler.h"
#include "netlist/event_system/net_event_handler.h"
#include "netlist/event_system/module_event_handler.h"
#include <QAbstractItemModel>
#include <QFont>
#include <QIcon>
#include <QModelIndex>
#include <QVariant>

class tree_navigation_item;

class tree_navigation_model : public QAbstractItemModel
{
public:
    tree_navigation_model(QObject* parent = 0);
    ~tree_navigation_model();

    //information access
    QVariant data(const QModelIndex& index, int role) const Q_DECL_OVERRIDE;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const Q_DECL_OVERRIDE;
    QModelIndex parent(const QModelIndex& index) const Q_DECL_OVERRIDE;
    Qt::ItemFlags flags(const QModelIndex& index) const Q_DECL_OVERRIDE;
    int rowCount(const QModelIndex& parent = QModelIndex()) const Q_DECL_OVERRIDE;
    int columnCount(const QModelIndex& parent = QModelIndex()) const Q_DECL_OVERRIDE;

    //this function provides the indexes for the navigation widget to handle a selection_changed for the view
    QModelIndexList get_corresponding_indexes(const QList<u32>& gate_ids, const QList<u32>& net_ids, const QList<u32>& module_ids);

    void handle_module_created(std::shared_ptr<module> m);
    void handle_module_gate_assigned(std::shared_ptr<module> m, u32 assigned_gate);
    void handle_module_gate_removed(std::shared_ptr<module> m, u32 removed_gate);
    void handle_module_name_changed(std::shared_ptr<module> m);
    void handle_module_removed(std::shared_ptr<module> m);

    void handle_gate_created(std::shared_ptr<gate> g);
    void handle_gate_removed(std::shared_ptr<gate> g);
    void handle_gate_name_changed(std::shared_ptr<gate> g);

    void handle_net_created(std::shared_ptr<net> n);
    void handle_net_removed(std::shared_ptr<net> n);
    void handle_net_name_changed(std::shared_ptr<net> n);



    static const int NAME_COLUMN = 0;
    static const int ID_COLUMN   = 1;
    static const int TYPE_COLUMN = 2;

private:
    void setup_model_data();

    //helper functions to convert between index and item
    tree_navigation_item* get_item(const QModelIndex& index) const;
    QModelIndex get_modelindex(tree_navigation_item* item);
    QList<QModelIndex> get_modelindexes_for_row(tree_navigation_item* item);

    void insert_item(tree_navigation_item* parent, int position, tree_navigation_item* item);
    void remove_item(tree_navigation_item* item);

    //these items are fixed
    tree_navigation_item* m_root_item;
    tree_navigation_item* m_top_design_item;
    tree_navigation_item* m_gates_item;
    tree_navigation_item* m_nets_item;
    tree_navigation_item* m_global_input_nets_item;
    tree_navigation_item* m_global_output_nets_item;
    tree_navigation_item* m_global_inout_nets_item;
    tree_navigation_item* m_modules_item;

    QIcon m_design_icon;
    QFont m_structured_font;
    void load_data_settings();
};
#endif    // TREE_NAVIGATION_MODEL_H
