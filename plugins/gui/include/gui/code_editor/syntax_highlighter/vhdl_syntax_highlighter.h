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

#include <QRegularExpression>
#include <QSyntaxHighlighter>

namespace hal
{
    class VhdlSyntaxHighlighter : public QSyntaxHighlighter
    {
        Q_OBJECT

    public:
        explicit VhdlSyntaxHighlighter(QTextDocument* parent = nullptr);

    protected:
        void highlightBlock(const QString& text) Q_DECL_OVERRIDE;
        //void run_current_state(const QString &text, int offset = 0);

    private:
        //    QRegularExpression::PatternOptions m_pattern_options;
        //    QRegularExpression::MatchType m_match_type;
        //    QRegularExpression::MatchOptions m_match_options;
        //    QStringList m_valid_identifier;
        //    QStringList m_types;
        //    QStringList mKeywords;
        //    QHash<QString, QTextCharFormat> m_formatters;
        //
        //    QString m_current_state;
        //    QStack<QString> m_stack;
        //
        //    struct HighlightingRule
        //    {
        //        QRegularExpression expression;
        //        QStringList list;
        //        QString state;
        //    };
        //
        //    HighlightingRule make_rule(QString expression, QStringList list, QString state);
        //
        //    HighlightingRule m_keyword_rule;
        //    HighlightingRule m_type_rule;
        //    QHash<QString, QList<HighlightingRule>> m_rules;

        struct HighlightingRule
        {
            QRegularExpression mPattern;
            QTextCharFormat mFormat;
        };

        QVector<HighlightingRule> mHighlightingRules;

        QRegularExpression mSingleLineCommentExpression;
        QRegularExpression mCommentStartExpression;
        QRegularExpression mCommentEndExpression;
    };
}
