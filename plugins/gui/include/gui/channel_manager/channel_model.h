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

#include "gui/channel_manager/channel_item.h"
#include "hal_core/utilities/log.h"
#include <QAbstractTableModel>
#include <QModelIndex>
#include <QVariant>
#include <boost/circular_buffer.hpp>
namespace hal
{
    class ChannelModel : public QAbstractTableModel
    {
        enum class ColumnNumber
        {
            NameColumn        = 0,
            ProgressBarColumn = 1,
            StatusColumn      = 2
        };

        Q_OBJECT

    public:
        static ChannelModel* get_instance();
        ~ChannelModel();

        QVariant data(const QModelIndex& index, int role) const Q_DECL_OVERRIDE;
        Qt::ItemFlags flags(const QModelIndex& index) const Q_DECL_OVERRIDE;
        QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
        QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const Q_DECL_OVERRIDE;
        int rowCount(const QModelIndex& parent = QModelIndex()) const Q_DECL_OVERRIDE;
        int columnCount(const QModelIndex& parent = QModelIndex()) const Q_DECL_OVERRIDE;

        ChannelItem* add_channel(const QString name);
        void handleLogmanagerCallback(const spdlog::level::level_enum& t, const std::string& channel_name, const std::string& msg_text);

    Q_SIGNALS:
        void updated(spdlog::level::level_enum t, const std::string& logger_name, std::string const& msg);

    private:
        explicit ChannelModel(QObject* parent = 0);

        QList<QString> mChannelToIgnore;
        QList<ChannelItem*> mPermanentItems;
        boost::circular_buffer<ChannelItem*> mTemporaryItems;

        u64 mGuiCallbackId;
    };
}
