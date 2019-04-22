//  MIT License
//
//  Copyright (c) 2019 Marc Fyrbiak
//  Copyright (c) 2019 Sebastian Wallat
//  Copyright (c) 2019 Max Hoffmann
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

#ifndef PYTHON_CONSOLE_QSS_ADAPTER
#define PYTHON_CONSOLE_QSS_ADAPTER

#include <QWidget>
#include <QColor>

class python_console_qss_adapter : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QColor standard_color READ standard_color WRITE set_standard_color)
    Q_PROPERTY(QColor error_color READ error_color WRITE set_error_color)
    Q_PROPERTY(QColor promt_color READ promt_color WRITE set_promt_color)

public:

    static python_console_qss_adapter* instance();

    QColor standard_color() const;
    QColor error_color() const;
    QColor promt_color() const;

    void set_standard_color(const QColor& color);
    void set_error_color(const QColor& color);
    void set_promt_color(const QColor& color);

private:
    explicit python_console_qss_adapter(QWidget* parent = nullptr);

    QColor m_standard_color;
    QColor m_error_color;
    QColor m_promt_color;

};

#endif //PYTHON_CONSOLE_QSS_ADAPTER
