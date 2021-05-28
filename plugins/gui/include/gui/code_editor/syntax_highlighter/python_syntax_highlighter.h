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

#include <QRegularExpression>
#include <QSyntaxHighlighter>
#include <QTextCharFormat>

namespace hal
{
    /**
     * @ingroup python
     * @brief A syntax highlighter that fits for python code.
     *
     * The following sections are distinguished and their color can be
     * configured in the qss file: <ul>
     *   <li> Python Keywords (e.g. if, and, break, class, True, ...)
     *   <li> Operators (e.g. =, +, -, ==, &, <<, ...)
     *   <li> Brackets ( round brackets (), curly brackets {} and square brackets [])
     *   <li> Single Quoted String ('string')
     *   <li> Double Quoted String ("string")
     *   <li> Keyword: self
     *   <li> Numbers (e.g. 42, 3.5, 0xDEADBEAF, 0765)
     *   <li> Comments (single- and multiline)
     * </ul>
     */
    class PythonSyntaxHighlighter : public QSyntaxHighlighter
    {
        Q_OBJECT

    public:
        /**
         * Constructor. Initializes the syntax highlight patterns.
         *
         * @param parent - The parent text document
         */
        explicit PythonSyntaxHighlighter(QTextDocument* parent = nullptr);

    protected:
        /**
         * Highlights the given text block.
         *
         * @param text - the text block to highlight
         */
        void highlightBlock(const QString& text) override;

    private:
        int closeMultilineComment(const QString& text, const QRegularExpression& delimiter, const int offset = 0);

        struct HighlightingRule
        {
            QRegularExpression mPattern;
            QTextCharFormat mFormat;
        };

        QVector<HighlightingRule> mHighlightingRules;

        QRegularExpression mMultiLineCommentDelimiter;
        QRegularExpression mTrippleSingleQuote;
        QRegularExpression mTrippleDoubleQuote;
    };
}
