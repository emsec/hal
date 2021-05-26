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

#include "hal_core/utilities/log.h"

#include "gui/channel_manager/channel_item.h"

#include <QAbstractTableModel>
#include <QModelIndex>
#include <QVariant>

namespace hal
{
    /**
     * @ingroup logging
     * @brief Table model for log channels.
     *
     * This model handles incoming callbacks from the core's log manager. It creates new channels and updates existing ones and is
     * realized through a singleton pattern.
     * It is implemented as a ’standard’ Qt tablemodel overwriting the necessary functions. Please refer to the Qt documentation
     * for further details as to how to implement models.
     */
    class ChannelModel : public QAbstractTableModel
    {
        /**
         * Represents the meaning of each column in the model. The progressbar- and status- column
         * are not used yet and (therefore) not implemented.
         */
        enum class ColumnNumber
        {
            NameColumn        = 0,
            ProgressBarColumn = 1,
            StatusColumn      = 2
        };

        Q_OBJECT

    public:
        /**
         * Get the singleton instance of the model.
         * @return The channel model instance.
         */
        static ChannelModel* get_instance();

        /**
         * The destructor. Removes callback.
         */
        ~ChannelModel() override;

        /** @name Overwritten model functions
         */
        ///@{
        QVariant data(const QModelIndex& index, int role) const override;
        Qt::ItemFlags flags(const QModelIndex& index) const override;
        QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
        QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
        int rowCount(const QModelIndex& parent = QModelIndex()) const override;
        int columnCount(const QModelIndex& parent = QModelIndex()) const override;
        ///@}

        /**
         * Adds a new channel with the given name to its temporary channel list.
         *
         * @param name - The name of the soon to be channel.
         * @return The created channel item.
         */
        ChannelItem* add_channel(const QString name);

        /**
         * This function manages the callback from the logmanager. It is registered in the model's constructor and checks if the channel for the
         * message already exists. If yes, the message is added to channel. If not, the channel is created.
         *
         * @param t - The type of the message (debug, error,...).
         * @param channel_name - The channel of the message.
         * @param msg_text - The message itselft.
         */
        void handleLogmanagerCallback(const spdlog::level::level_enum& t, const std::string& channel_name, const std::string& msg_text);

    Q_SIGNALS:
        /**
         * This signal is emitted at the end of handleLogmanagerCallback (when a new message was received from the logmanager).
         *
         * @param t - The type of the message.
         * @param logger_name - The channel of the message.
         * @param msg - The message itself.
         */
        void updated(spdlog::level::level_enum t, const std::string& logger_name, std::string const& msg);

    private:
        explicit ChannelModel(QObject* parent = nullptr);

        QList<QString> mChannelToIgnore;
        QList<ChannelItem*> mPermanentItems;
        QList<ChannelItem*> mTemporaryItems;

        u64 mGuiCallbackId;
    };
}
