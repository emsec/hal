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

#include "plugin_manager/plugin_item.h"
#include <QAbstractItemModel>
#include <def.h>

namespace hal
{
    class plugin_model : public QAbstractItemModel
    {
        Q_OBJECT

    public:
        explicit plugin_model(QObject* parent = 0);
        ~plugin_model();
        bool is_valid_index(const QModelIndex& idx);
        virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
        virtual int columnCount(const QModelIndex& parent = QModelIndex()) const;
        virtual QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const;
        virtual QModelIndex parent(const QModelIndex& child) const;
        virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
        virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;
        virtual bool moveRows(const QModelIndex& sourceParent, int sourceRow, int count, const QModelIndex& destinationParent, int destinationChild);
        virtual bool dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent);
        virtual Qt::DropActions supportedDropActions() const;
        virtual Qt::ItemFlags flags(const QModelIndex& index) const;
        virtual bool setData(const QModelIndex& index, const QVariant& value, int role);
        void plugin_manager_callback(bool is_load, std::string const& plugin_name, std::string const& plugin_path);
        void request_load_plugin(const QString& name, const QString& path);
        void request_unload_plugin(QModelIndexList idx);
        const QList<plugin_item> get_plugin_list();

    Q_SIGNALS:
        void load_plugin(QString name, QString path);
        void unload_plugin(QString name, QString path);
        void run_plugin(const QString& name);

    public Q_SLOTS:
        void handle_load_plugin(QString name, QString path);
        void handle_unload_plugin(QString name, QString path);
        void handle_run_plugin_triggered(const QModelIndex& index);

    private:
        QStringList m_columns;
        QList<plugin_item> m_items;
        u64 m_model_changed_callback_id;
    };
}
