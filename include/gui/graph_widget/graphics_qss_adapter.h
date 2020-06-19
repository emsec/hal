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

#include <QWidget>

namespace hal
{
    class graphics_qss_adapter : public QWidget
    {
        Q_OBJECT

        Q_PROPERTY(QColor gate_base_color READ gate_base_color WRITE set_gate_base_color)
        Q_PROPERTY(QColor net_base_color READ net_base_color WRITE set_net_base_color)

        Q_PROPERTY(QColor gate_selection_color READ gate_selection_color WRITE set_gate_selection_color)
        Q_PROPERTY(QColor net_selection_color READ net_selection_color WRITE set_net_selection_color)

        Q_PROPERTY(QFont gate_name_font READ gate_name_font WRITE set_gate_name_font)
        Q_PROPERTY(QFont gate_type_font READ gate_type_font WRITE set_gate_type_font)
        Q_PROPERTY(QFont gate_pin_font READ gate_pin_font WRITE set_gate_pin_font)

        Q_PROPERTY(QFont net_font READ net_font WRITE set_net_font)

        Q_PROPERTY(QColor grid_base_line_color READ grid_base_line_color WRITE set_grid_base_line_color)
        Q_PROPERTY(QColor grid_cluster_line_color READ grid_cluster_line_color WRITE set_grid_cluster_line_color)

        Q_PROPERTY(QColor grid_base_dot_color READ grid_base_dot_color WRITE set_grid_base_dot_color)
        Q_PROPERTY(QColor grid_cluster_dot_color READ grid_cluster_dot_color WRITE set_grid_cluster_dot_color)

    public:
        explicit graphics_qss_adapter(QWidget* parent = nullptr);

        void repolish();

        QColor gate_base_color() const;
        QColor net_base_color() const;

        // GATE BACKGROUND COLOR
        // GATE OUTLINE COLOR
        // ...

        QColor gate_selection_color() const;
        QColor net_selection_color() const;

        QFont gate_name_font() const;
        QFont gate_type_font() const;
        QFont gate_pin_font() const;

        QFont net_font() const;

        QColor grid_base_line_color() const;
        QColor grid_cluster_line_color() const;

        QColor grid_base_dot_color() const;
        QColor grid_cluster_dot_color() const;

        void set_gate_base_color(const QColor& color);
        void set_net_base_color(const QColor& color);

        void set_gate_selection_color(const QColor& color);
        void set_net_selection_color(const QColor& color);

        void set_gate_name_font(const QFont& font);
        void set_gate_type_font(const QFont& font);
        void set_gate_pin_font(const QFont& font);

        void set_net_font(const QFont& font);

        void set_grid_base_line_color(const QColor& color);
        void set_grid_cluster_line_color(const QColor& color);

        void set_grid_base_dot_color(const QColor& color);
        void set_grid_cluster_dot_color(const QColor& color);

    private:
        QColor m_gate_base_color;
        QColor m_net_base_color;

        QColor m_gate_selection_color;
        QColor m_net_selection_color;

        QFont m_gate_name_font;
        QFont m_gate_type_font;
        QFont m_gate_pin_font;

        QFont m_net_font;

        QColor m_grid_base_line_color;
        QColor m_grid_cluster_line_color;

        QColor m_grid_base_dot_color;
        QColor m_grid_cluster_dot_color;
    };
}
