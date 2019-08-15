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

#ifndef __HAL_GRAPH_MODEL_T_H__
#define __HAL_GRAPH_MODEL_T_H__

#include "old_graph_model_item.h"
#include "netlist/gate.h"
#include "netlist/net.h"
#include "netlist/netlist.h"
#include "old_tree_model_item.h"
#include <QAbstractItemModel>
#include <QList>
#include <QModelIndex>
#include <QObject>
#include <QStringList>
#include <QVariant>

class old_graph_tree_model : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit old_graph_tree_model(QObject* parent = 0);

    ~old_graph_tree_model();

    /*the standard functions needed for a basic, non-editable treemodel*/
    QVariant data(const QModelIndex& index, int role) const Q_DECL_OVERRIDE;    //Verursacht Error
    Qt::ItemFlags flags(const QModelIndex& index) const Q_DECL_OVERRIDE;

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;

    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const Q_DECL_OVERRIDE;

    QModelIndex parent(const QModelIndex& index) const Q_DECL_OVERRIDE;

    int rowCount(const QModelIndex& parent = QModelIndex()) const Q_DECL_OVERRIDE;

    int columnCount(const QModelIndex& parent = QModelIndex()) const Q_DECL_OVERRIDE;

    /*this function needs to be called when the graph is loaded from a file to fill the model, maps the graph into the model*/
    void setupModelData(std::shared_ptr<netlist> g);

private:
    /*some stuff needed for the basic treemodel*/
    old_tree_model_item* rootItem;

    QList<old_tree_model_item> items;

    QStringList m_columns;

    /*a function that helps to setup the model-data*/
    void add_net_set_to_a_parent_item(std::set<std::shared_ptr<net>> t, old_tree_model_item* parent);
};

#endif
