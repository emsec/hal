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
        ChannelEntry(std::string msg, spdlog::level::level_enum msg_type) : m_msg(msg), m_msg_type(msg_type)
        {
        }

        const std::string m_msg;    // USE QSTRING HERE
        const spdlog::level::level_enum m_msg_type;
    };

    static const int max_entries = 1000;

    struct LogChannel
    {
        //    LogChannel(const QString& name) : m_name(name)
        //    {
        //    }

        const QString m_name;

        ChannelEntry m_entries[max_entries];

        int m_entry_count;
        int m_first_entry;

        int unread_entries;
        int unread_warnings;
        int unread_errors;
        int unread_successes;
    };

    class ChannelItem
    {
    public:
        ChannelItem(QString name);

        QVariant data(int column) const;
        const QString name() const;
        const boost::circular_buffer<ChannelEntry*>* get_buffer() const;
        QReadWriteLock* get_lock();

        void append_entry(ChannelEntry* entry);

    private:
        const QString m_name;
        boost::circular_buffer<ChannelEntry*> m_log_entries;
        QReadWriteLock m_lock;

        int m_observer;
        int m_unread;
        int m_unread_warnings;
        int m_unread_errors;
        int m_unread_successes;
    };

}
