/*
 * MIT License
 *
 * Copyright (c) 2019 Chair for Embedded Security, Ruhr-University Bochum
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef ITEMLESS_MODULE_MODEL_H
#define ITEMLESS_MODULE_MODEL_H

#include "def.h"

#include "module_model/module_item.h"

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>

class itemless_module_model : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit itemless_module_model(QObject* parent = nullptr);
    ~itemless_module_model();

    // MANDATORY
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const Q_DECL_OVERRIDE;
    QModelIndex parent(const QModelIndex& index) const Q_DECL_OVERRIDE;
    int rowCount(const QModelIndex& parent = QModelIndex()) const Q_DECL_OVERRIDE;
    int columnCount(const QModelIndex& parent = QModelIndex()) const Q_DECL_OVERRIDE;
    QVariant data(const QModelIndex& index, int role) const Q_DECL_OVERRIDE;

    // OPTIONAL
    Qt::ItemFlags flags(const QModelIndex& index) const Q_DECL_OVERRIDE;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;

    QModelIndex* get_index_by_logger_name(const std::string& logger_name);
    QModelIndexList corresponding_indexes(const QList<u32>& gate_ids, const QList<u32>& net_ids, const QList<u32>& module_ids);
    module_item* add_item(QString name, int id, const std::string& parent_item_logger_name = 0);

    void update_modules();

private:
    void setupModelData();
    void match_ids_recursive(module_item* item, QModelIndexList& list, const QList<u32>& gate_ids, const QList<u32>& net_ids, const QList<u32>& module_ids);
    module_item* m_root_item; // DO ITEMS EVEN MAKE SENSE ?
};

#endif // ITEMLESS_MODULE_MODEL_H
