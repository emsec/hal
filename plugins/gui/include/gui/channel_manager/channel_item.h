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

#include <QList>
#include <QReadWriteLock>
#include <QString>
#include <QVariant>

namespace hal
{
    /**
     * @ingroup logging
     * @brief The ChannelEntry struct is used by the ChannelItem class to store a single entry.
     *
     * An entry consists of the message itself and and its type (error, debug, etc.).
     */
    struct ChannelEntry
    {
        /**
         * The channel entry's "constructor"
         *
         * @param msg - The message
         * @param msg_type - The message's type
         */
        ChannelEntry(std::string msg, spdlog::level::level_enum msg_type) : mMsg(msg), mMsgType(msg_type)
        {
        }

        const std::string mMsg;
        const spdlog::level::level_enum mMsgType;
    };

    static const int sMaxEntries = 1000;

    /**
     * @ingroup logging
     * @brief This class represents a channel in the channel model. It primarily holds the channel
     * name and its entries.
     */
    class ChannelItem
    {
    public:
        /**
         * The constructor
         * @param name - The name of the channel.
         */
        ChannelItem(QString name);

        /**
         * Get the column specific data of the item. The channel logically consists of only one column that holds the name.
         * This function is the same as name() for column = 0.
         *
         * @param column - The column for which data is requested.
         * @return The data of the column.
         */
        QVariant data(int column) const;

        /**
         * Get the channel's name.
         *
         * @return The name of the channel.
         */
        const QString name() const;

        /**
         * Get the List of all buffered messages.
         *
         * @return The List that contains the channel entries.
         */
        const QList<ChannelEntry*>* getList() const;

        /**
         * Get the lock of the channel.
         *
         * @return The channel's lock.
         */
        QReadWriteLock* getLock();

        /**
         * Append a given entry to the end of the message list. If the list's size exceeds the maximum number
         * of allowed entries, the first entry in the list is removed.
         *
         * @param entry - The entry to append to the list.
         */
        void appendEntry(ChannelEntry* entry);

    private:
        const QString mName;
        QList<ChannelEntry*> mLogEntries;
        QReadWriteLock mLock;

        int mObserver;
        int mUnread;
        int mUnreadWarnings;
        int mUnreadErrors;
        int mUnreadSuccesses;
    };

}
