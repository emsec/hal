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

#include <QMutex>
#include <QObject>
#include <QPlainTextEdit>
#include <QQueue>
#include <QtCore/qreadwritelock.h>

namespace hal
{
    struct FilterItem;

    /**
     * @ingroup logging
     * @brief Appends log messages to the corresponding LoggerWidget.
     *
     * The LoggerMarshall outsources the logic of appending a log to the QPlaintTextEdit of the LoggerWidget it belongs to.
     * By calling appendLog the LoggerMarshall will transform the received log message to an html string with the color
     * of the respective type of the message (e.g. error (red), debug (yellow), etc.).
     */
    class LoggerMarshall : public QObject
    {
        Q_OBJECT

    public:
        /**
         * Constructor.
         *
         * @param edit - The QPlainTextEdit of the LoggerWidget this LoggerMarshall should belong to.
         * @param parent - The parent widget
         */
        explicit LoggerMarshall(QPlainTextEdit* edit, QObject* parent = nullptr);

    public Q_SLOTS:

        /**
         * Given a message type, a message and a filter this function wraps the message in an html string of the
         * respective color of the channel type, applies the filter and appends the final html string to the
         * QPlainTextEdit of the LoggerWidget this LoggerMarshall belongs to.
         *
         * @param log_type - The message type
         * @param msg - The message
         * @param filter - The filter
         */
        void appendLog(spdlog::level::level_enum log_type, QString const& msg);

    private:
        int mMaxLineCount;

        //will be deleted within the logger_widgets destrcutor (is the parent of the textedit)
        QPlainTextEdit* mEdit;
    };
}
