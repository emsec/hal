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

#ifndef LOGGER_QSS_ADAPTER_H
#define LOGGER_QSS_ADAPTER_H

#include <QWidget>
namespace hal{
class logger_qss_adapter : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QColor trace_color READ trace_color WRITE set_trace_color)
    Q_PROPERTY(QColor debug_color READ debug_color WRITE set_debug_color)
    Q_PROPERTY(QColor info_color READ info_color WRITE set_info_color)
    Q_PROPERTY(QColor warning_color READ warning_color WRITE set_warning_color)
    Q_PROPERTY(QColor error_color READ error_color WRITE set_error_color)
    Q_PROPERTY(QColor critical_color READ critical_color WRITE set_critical_color)
    Q_PROPERTY(QColor default_color READ default_color WRITE set_default_color)
    Q_PROPERTY(QColor marker_color READ marker_color WRITE set_marker_color)

    Q_PROPERTY(QColor trace_highlight READ trace_highlight WRITE set_trace_highlight)
    Q_PROPERTY(QColor debug_highlight READ debug_highlight WRITE set_debug_highlight)
    Q_PROPERTY(QColor info_highlight READ info_highlight WRITE set_info_highlight)
    Q_PROPERTY(QColor warning_highlight READ warning_highlight WRITE set_warning_highlight)
    Q_PROPERTY(QColor error_highlight READ error_highlight WRITE set_error_highlight)
    Q_PROPERTY(QColor critical_highlight READ critical_highlight WRITE set_critical_highlight)
    Q_PROPERTY(QColor default_highlight READ default_highlight WRITE set_default_highlight)

public:
    static logger_qss_adapter* instance();

    QColor trace_color() const;
    QColor debug_color() const;
    QColor info_color() const;
    QColor warning_color() const;
    QColor error_color() const;
    QColor critical_color() const;
    QColor default_color() const;
    QColor marker_color() const;

    QColor trace_highlight() const;
    QColor debug_highlight() const;
    QColor info_highlight() const;
    QColor warning_highlight() const;
    QColor error_highlight() const;
    QColor critical_highlight() const;
    QColor default_highlight() const;

    void set_trace_color(const QColor& color);
    void set_debug_color(const QColor& color);
    void set_info_color(const QColor& color);
    void set_warning_color(const QColor& color);
    void set_error_color(const QColor& color);
    void set_critical_color(const QColor& color);
    void set_default_color(const QColor& color);
    void set_marker_color(const QColor& color);

    void set_trace_highlight(const QColor& color);
    void set_debug_highlight(const QColor& color);
    void set_info_highlight(const QColor& color);
    void set_warning_highlight(const QColor& color);
    void set_error_highlight(const QColor& color);
    void set_critical_highlight(const QColor& color);
    void set_default_highlight(const QColor& color);


private:
    explicit logger_qss_adapter(QWidget* parent = nullptr);

    QColor m_trace_color;
    QColor m_debug_color;
    QColor m_info_color;
    QColor m_warning_color;
    QColor m_error_color;
    QColor m_critical_color;
    QColor m_default_color;
    QColor m_marker_color;

    QColor m_trace_highlight;
    QColor m_debug_highlight;
    QColor m_info_highlight;
    QColor m_warning_highlight;
    QColor m_error_highlight;
    QColor m_critical_highlight;
    QColor m_default_highlight;

};
}

#endif //LOGGER_QSS_ADAPTER_H
