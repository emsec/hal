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

#include "core/log.h"
#include <QMutex>
#include <QObject>
#include <QPlainTextEdit>
#include <QQueue>
#include <QtCore/qreadwritelock.h>

namespace hal
{
    struct hal_filter_item;

    class hal_logger_marshall : public QObject
    {
        Q_OBJECT

    public:
        explicit hal_logger_marshall(QPlainTextEdit* edit, QObject* parent = 0);

        ~hal_logger_marshall();

    Q_SIGNALS:

    public Q_SLOTS:

        void append_log(spdlog::level::level_enum log_type, QString const& msg, hal_filter_item* filter);

        void highlight_current_line();

    private:
        int m_max_line_count;

        //will be deleted within the hal_logger_widgets destrcutor (is the parent of the textedit)
        QPlainTextEdit* m_edit;
    };
}
