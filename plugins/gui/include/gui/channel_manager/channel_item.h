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

#include "hal_core/utilities/log.h"

#include <QReadWriteLock>
#include <QString>
#include <QVariant>

#include <boost/circular_buffer.hpp>
namespace hal
{

    struct ChannelEntry
    {
        ChannelEntry(std::string msg, spdlog::level::level_enum msg_type) : mMsg(msg), mMsgType(msg_type)
        {
        }

        const std::string mMsg;    // USE QSTRING HERE
        const spdlog::level::level_enum mMsgType;
    };

    static const int sMaxEntries = 1000;

    struct LogChannel
    {
        //    LogChannel(const QString& name) : mName(name)
        //    {
        //    }

        const QString mName;

        ChannelEntry mEntries[sMaxEntries];

        int mEntryCount;
        int mFirstEntry;

        int mUnreadEntries;
        int mUnreadWarnings;
        int mUnreadErrors;
        int mUnreadSuccesses;
    };

    class ChannelItem
    {
    public:
        ChannelItem(QString name);

        QVariant data(int column) const;
        const QString name() const;
        const boost::circular_buffer<ChannelEntry*>* getBuffer() const;
        QReadWriteLock* getLock();

        void appendEntry(ChannelEntry* entry);

    private:
        const QString mName;
        boost::circular_buffer<ChannelEntry*> mLogEntries;
        QReadWriteLock mLock;

        int mObserver;
        int mUnread;
        int mUnreadWarnings;
        int mUnreadErrors;
        int mUnreadSuccesses;
    };

}
