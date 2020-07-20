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
    class PythonQssAdapter : public QWidget
    {
        Q_OBJECT
        Q_PROPERTY(QColor text_color READ text_color WRITE set_text_color)
        Q_PROPERTY(QColor keyword_color READ keyword_color WRITE set_keyword_color)
        Q_PROPERTY(QColor operator_color READ operator_color WRITE set_operator_color)
        Q_PROPERTY(QColor brace_color READ brace_color WRITE set_brace_color)
        Q_PROPERTY(QColor defclass_color READ defclass_color WRITE set_defclass_color)
        Q_PROPERTY(QColor self_color READ self_color WRITE set_self_color)
        Q_PROPERTY(QColor number_color READ number_color WRITE set_number_color)
        Q_PROPERTY(QColor single_quoted_string_color READ single_quoted_string_color WRITE set_single_quoted_string_color)
        Q_PROPERTY(QColor double_quoted_string_color READ double_quoted_string_color WRITE set_double_quoted_string_color)
        Q_PROPERTY(QColor comment_color READ comment_color WRITE set_comment_color)

    public:
        explicit PythonQssAdapter(QWidget* parent = nullptr);
        ~PythonQssAdapter();    // DEBUG CODE, DELETE LATER
        static PythonQssAdapter* instance();

        void repolish();

        QColor text_color() const;
        QColor keyword_color() const;
        QColor operator_color() const;
        QColor brace_color() const;
        QColor defclass_color() const;
        QColor self_color() const;
        QColor number_color() const;
        QColor single_quoted_string_color() const;
        QColor double_quoted_string_color() const;
        QColor comment_color() const;

        void set_text_color(const QColor& color);
        void set_keyword_color(const QColor& color);
        void set_operator_color(const QColor& color);
        void set_brace_color(const QColor& color);
        void set_defclass_color(const QColor& color);
        void set_self_color(const QColor& color);
        void set_number_color(const QColor& color);
        void set_single_quoted_string_color(const QColor& color);
        void set_double_quoted_string_color(const QColor& color);
        void set_comment_color(const QColor& color);

        // LEFT PUBLIC INTENTIONALLY
        QTextCharFormat m_keyword_format;
        QTextCharFormat m_operator_format;
        QTextCharFormat m_brace_format;
        QTextCharFormat m_defclass_format;
        QTextCharFormat m_self_format;
        QTextCharFormat m_number_format;
        QTextCharFormat m_single_quoted_string_format;
        QTextCharFormat m_double_quoted_string_format;
        QTextCharFormat m_comment_format;

    private:
        QColor m_text_color;
        QColor m_keyword_color;
        QColor m_operator_color;
        QColor m_brace_color;
        QColor m_defclass_color;
        QColor m_self_color;
        QColor m_number_color;
        QColor m_single_quoted_string_color;
        QColor m_double_quoted_string_color;
        QColor m_comment_color;
    };
}
