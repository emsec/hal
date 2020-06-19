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
namespace hal{
class vhdl_qss_adapter : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QColor keyword_color READ keyword_color WRITE set_keyword_color)
    Q_PROPERTY(QColor type_color READ type_color WRITE set_type_color)
    Q_PROPERTY(QColor number_color READ number_color WRITE set_number_color)
    Q_PROPERTY(QColor string_color READ string_color WRITE set_string_color)
    Q_PROPERTY(QColor comment_color READ comment_color WRITE set_comment_color)

public:
    explicit vhdl_qss_adapter(QWidget* parent = nullptr);
    static vhdl_qss_adapter* instance();

    void repolish();

    QColor text_color() const;
    QColor keyword_color() const;
    QColor type_color() const;
    QColor number_color() const;
    QColor string_color() const;
    QColor comment_color() const;

    void set_text_color(const QColor& color);
    void set_keyword_color(const QColor& color);
    void set_type_color(const QColor& color);
    void set_number_color(const QColor& color);
    void set_string_color(const QColor& color);
    void set_comment_color(const QColor& color);

    // LEFT PUBLIC INTENTIONALLY
    QTextCharFormat m_keyword_format;
    QTextCharFormat m_type_format;
    QTextCharFormat m_number_format;
    QTextCharFormat m_string_format;
    QTextCharFormat m_comment_format;

private:
    QColor m_text_color;
    QColor m_keyword_color;
    QColor m_type_color;
    QColor m_number_color;
    QColor m_string_color;
    QColor m_comment_color;
};
}
