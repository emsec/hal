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

#ifndef RED_PIN_GATE_H
#define RED_PIN_GATE_H

#include "graph_widget/graphics_items/graphics_gate.h"

class gate;

class red_pin_gate : public graphics_gate
{
public:
    red_pin_gate(std::shared_ptr<gate> g);

    static void load_settings();

    virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = 0) Q_DECL_OVERRIDE;

    virtual QPointF get_input_pin_scene_position(QString type) override;
    virtual QPointF get_output_pin_scene_position(QString type) override;

private:
    //STATIC SECTION THAT EACH NODE SHARES (design purposes)

    static QFont s_name_font;
    static QFont s_pin_font;
    static QColor s_default_color;
    static QPen s_pen;

    static qreal s_drawn_pin_height;
    static qreal s_overall_pin_width;
    static qreal s_overall_pin_height;

    static qreal s_pin_name_rect_stretching;
    static qreal s_name_rect_stretching;

    //    static qreal s_name_left_spacing;
    //    static qreal s_name_right_spacing;

    //NORMAL SECTION ("logic" purposes)

    QVector<QPointF> m_input_pin_positions;
    QVector<QPointF> m_output_pin_positions;

    qreal m_input_pin_name_width;
    qreal m_output_pin_name_width;

    //FOR NOW, USE RECTS INSTEAD OF POINTS TO CALCULATE/DRAW TEXT(CHANGE WHEN OPTIMIZED)

    QRectF m_drawn_rect;
    QRectF m_name_rect;

    QVector<QRectF> m_input_pin_name_rects;
    QVector<QRectF> m_output_pin_name_rects;

    void format();
};

#endif    // RED_PIN_GATE_H
