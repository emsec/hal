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

#ifndef GRAPH_NAVIGATION_MODEL_H
#define GRAPH_NAVIGATION_MODEL_H

#include "def.h"
#include "graph_navigation_widget/graph_navigation_item.h"
#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>

class graph_navigation_model : public QAbstractItemModel
{
    enum class ColumnNumber
    {
        NameColumn        = 0,
        IDColumn          = 1,
        ProgressBarColumn = 2,
        StatusColumn      = 3
    };

    enum class ModelConstant
    {
        MaxVariableTopLevelItems   = 30,
        NumberOfFixedTopLevelItems = 1,
        NumberOfFixedHALChildItems = 2,
    };

    Q_OBJECT

public:
    explicit graph_navigation_model(QObject* parent = 0);
    ~graph_navigation_model();

    QVariant data(const QModelIndex& index, int role) const Q_DECL_OVERRIDE;
    Qt::ItemFlags flags(const QModelIndex& index) const Q_DECL_OVERRIDE;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const Q_DECL_OVERRIDE;
    QModelIndex parent(const QModelIndex& index) const Q_DECL_OVERRIDE;
    int rowCount(const QModelIndex& parent = QModelIndex()) const Q_DECL_OVERRIDE;
    int columnCount(const QModelIndex& parent = QModelIndex()) const Q_DECL_OVERRIDE;

    QModelIndex* get_index_by_logger_name(const std::string& logger_name);
    QModelIndexList corresponding_indexes(const QList<u32>& gate_ids, const QList<u32>& net_ids, const QList<u32>& module_ids);
    graph_navigation_item* add_item(QString name, int id, const std::string& parent_item_logger_name = 0);
    //    void update_item_log(spdlog::level::level_enum t, const std::string &logger_name, std::string const &msg);
    //    void update_item_progress(const std::string &logger_name, int progress);
    //    void update_item_status(const std::string &logger_name, QString status);

    void update_modules();

private:
    void setupModelData();
    void match_ids_recursive(graph_navigation_item* item, QModelIndexList& list, const QList<u32>& gate_ids, const QList<u32>& net_ids, const QList<u32>& module_ids);
    graph_navigation_item* m_root_item;
    graph_navigation_item* m_top_level_item;
    graph_navigation_item* m_gate_item;
    graph_navigation_item* m_net_item;
    graph_navigation_item* m_module_item;
    graph_navigation_item* m_global_input_item;
    graph_navigation_item* m_global_output_item;
};

#endif    // GRAPH_NAVIGATION_MODEL_H
