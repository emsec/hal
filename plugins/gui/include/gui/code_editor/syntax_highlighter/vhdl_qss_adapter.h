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

#include <QTextCharFormat>
#include <QWidget>

namespace hal
{
    class VhdlQssAdapter : public QWidget
    {
        Q_OBJECT
        Q_PROPERTY(QColor keywordColor READ keywordColor WRITE setKeywordColor)
        Q_PROPERTY(QColor typeColor READ typeColor WRITE setTypeColor)
        Q_PROPERTY(QColor numberColor READ numberColor WRITE setNumberColor)
        Q_PROPERTY(QColor stringColor READ stringColor WRITE setStringColor)
        Q_PROPERTY(QColor commentColor READ commentColor WRITE setCommentColor)

    public:
        explicit VhdlQssAdapter(QWidget* parent = nullptr);
        static VhdlQssAdapter* instance();

        void repolish();

        QColor textColor() const;
        QColor keywordColor() const;
        QColor typeColor() const;
        QColor numberColor() const;
        QColor stringColor() const;
        QColor commentColor() const;

        void setTextColor(const QColor& color);
        void setKeywordColor(const QColor& color);
        void setTypeColor(const QColor& color);
        void setNumberColor(const QColor& color);
        void setStringColor(const QColor& color);
        void setCommentColor(const QColor& color);

        // LEFT PUBLIC INTENTIONALLY
        QTextCharFormat mKeywordFormat;
        QTextCharFormat mTypeFormat;
        QTextCharFormat mNumberFormat;
        QTextCharFormat mStringFormat;
        QTextCharFormat mCommentFormat;

    private:
        QColor mTextColor;
        QColor mKeywordColor;
        QColor mTypeColor;
        QColor mNumberColor;
        QColor mStringColor;
        QColor mCommentColor;
    };
}
