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

#ifndef OLD_GRAPHICS_ITEM_QSS_ADAPTER
#define OLD_GRAPHICS_ITEM_QSS_ADAPTER

#include <QWidget>
#include <QColor>

class old_graphics_item_qss_adapter : public QWidget
{
    Q_OBJECT

public:
    static old_graphics_item_qss_adapter* instance();

    QColor gate_default_color() const;
    QColor gate_font_color() const;
    QColor gate_selected_color() const;

    QColor net_default_color() const;
    QColor net_selected_color() const;

    void set_gate_default_color(const QColor& color);
    void set_gate_font_color(const QColor& color);
    void set_gate_selected_color(const QColor& color);

    void set_net_default_color(const QColor& color);
    void set_net_selected_color(const QColor& color);

private:
    explicit old_graphics_item_qss_adapter(QWidget* parent = nullptr);

    QColor m_gate_default_color;
    QColor m_gate_font_color;
    QColor m_gate_selected_color;

    QColor m_net_default_color;
    QColor m_net_selected_color;


};

#endif //OLD_GRAPHICS_ITEM_QSS_ADAPTER
