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

#ifndef STANDARD_GRAPHICS_GATE_H
#define STANDARD_GRAPHICS_GATE_H

#include "graphics_gate.h"

class minimal_graphics_gate : public graphics_gate
{
public:
    static void load_settings();

    minimal_graphics_gate(std::shared_ptr<gate> g);

    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = 0) Q_DECL_OVERRIDE;

    QPainterPath shape() const Q_DECL_OVERRIDE;

    QPointF get_input_scene_position(const u32 net_id, const QString& pin_type) const Q_DECL_OVERRIDE;
    QPointF get_output_scene_position(const u32 net_id, const QString& pin_type) const Q_DECL_OVERRIDE;

    virtual void set_visuals(const visuals& v) Q_DECL_OVERRIDE;

private:
    static QPen s_pen;

    static QFont s_name_font;
    static QFont s_type_font;
    static QFont s_pin_font;

    static qreal s_name_font_height;
    static qreal s_type_font_height;

    static qreal s_pin_inner_horizontal_spacing;
    static qreal s_pin_outer_horizontal_spacing;

    static qreal s_pin_inner_vertical_spacing;
    static qreal s_pin_outer_vertical_spacing;
    static qreal s_pin_upper_vertical_spacing;
    static qreal s_pin_lower_vertical_spacing;

    static qreal s_pin_font_height;
    static qreal s_pin_font_ascent;
    static qreal s_pin_font_descent;
    static qreal s_pin_font_baseline;

    static qreal s_inner_name_type_spacing;
    static qreal s_outer_name_type_spacing;

    void format();

    QPointF m_name_position;
    QPointF m_type_position;

    QVector<QPointF> m_input_pin_positions;
    QVector<QPointF> m_output_pin_positions;
};

#endif    // STANDARD_GRAPHICS_GATE_H
