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

#include <QList>
#include <QRegularExpression>

namespace hal
{
    /**
     * @ingroup logging
     * @brief Item to filter different message types.
     *
     * A filter for message types of the logger. Every message type is assigned one of the following rules: <br>
     * <b> ShowAll, HideAll, Process </b> <br>
     * The following message types are distinguished: <br>
     * <b> trace, debug, info, warning, error, critical, default </b> <br>
     * Moreover a FilterItem contains a keywords list. If the rule is set to <b>Process</b> the messages of the
     * respective type were scanned for these keywords. Such messages are only shown if they contain at
     * least one of these keywords.
     */
    struct FilterItem
    {
        enum class Rule
        {
            /// All messages of this type should be shown.
            ShowAll = 0,
            /// No messages of this type should be shown.
            HideAll = 1,
            /// Messages are only shown if they contain at least one keyword of the passed keyword list.
            Process = 2
        };

        /**
         * Constructor. Initializes the filter rules for all message types and stores the passed keyword list.
         *
         * @param trace - Filter rule for message type 'trace'
         * @param debug - Filter rule for message type 'debug'
         * @param info - Filter rule for message type 'info'
         * @param warning - Filter rule for message type 'warning'
         * @param error - Filter rule for message type 'error'
         * @param critical - Filter rule for message type 'critical'
         * @param default_rule - Filter rule for a default message type
         * @param keywords - The keyword list used for the <b>Process</b> filter rule
         * @param reg_ex - A regular expression
         */
        FilterItem(Rule trace, Rule debug, Rule info, Rule warning, Rule error, Rule critical, Rule default_rule, QStringList& keywords, QRegularExpression& reg_ex)
            : mTrace(trace), mDebug(debug), mInfo(info), mWarning(warning), mError(error), mCritical(critical), mDefault(default_rule), mKeywords(keywords), mRegEx(reg_ex)
        {
        }

        const Rule mTrace;

        const Rule mDebug;

        const Rule mInfo;

        const Rule mWarning;

        const Rule mError;

        const Rule mCritical;

        const Rule mDefault;

        const QStringList mKeywords;

        const QRegularExpression mRegEx;
    };
}
