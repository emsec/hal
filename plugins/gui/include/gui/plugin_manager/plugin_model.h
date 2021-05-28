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

#include "hal_core/defines.h"

#include "gui/plugin_manager/plugin_item.h"

#include <QAbstractItemModel>

namespace hal
{
    /**
     * @ingroup unused
     */
    class PluginModel : public QAbstractItemModel
    {
        Q_OBJECT

    public:
        explicit PluginModel(QObject* parent = nullptr);
        ~PluginModel();

        int rowCount(const QModelIndex& parent = QModelIndex()) const override;
        int columnCount(const QModelIndex& parent = QModelIndex()) const override;
        QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
        QModelIndex parent(const QModelIndex& child) const override;
        QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
        QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
        bool moveRows(const QModelIndex& sourceParent, int sourceRow, int count, const QModelIndex& destinationParent, int destinationChild) override;
        bool dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) override;
        Qt::DropActions supportedDropActions() const override;
        Qt::ItemFlags flags(const QModelIndex& index) const override;
        bool setData(const QModelIndex& index, const QVariant& value, int role) override;

        bool isValidIndex(const QModelIndex& idx);
        void pluginManagerCallback(bool is_load, std::string const& plugin_name, std::string const& plugin_path);
        void requestLoadPlugin(const QString& name, const QString& path);
        void requestUnloadPlugin(QModelIndexList idx);
        const QList<PluginItem> getPluginList();

    Q_SIGNALS:
        void loadPlugin(QString name, QString path);
        void unloadPlugin(QString name, QString path);
        void runPlugin(const QString& name);

    public Q_SLOTS:
        void handleLoadPlugin(QString name, QString path);
        void handleUnloadPlugin(QString name, QString path);
        void handleRunPluginTriggered(const QModelIndex& index);

    private:
        QStringList mColumns;
        QList<PluginItem> mItems;
        u64 mModelChangedCallbackId;
    };
}
