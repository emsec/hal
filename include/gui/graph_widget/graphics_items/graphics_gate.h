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

#ifndef GRAPHICS_GATE_H
#define GRAPHICS_GATE_H

#include "graph_graphics_item.h"
#include <memory>

class gate;

class graphics_gate : public graph_graphics_item
{
public:
    graphics_gate(std::shared_ptr<gate> g);

    QRectF boundingRect() const Q_DECL_OVERRIDE;
    virtual QPainterPath shape() const Q_DECL_OVERRIDE;

    virtual QPointF get_input_pin_scene_position(QString type)  = 0;
    virtual QPointF get_output_pin_scene_position(QString type) = 0;

    //uncertain about this one, might not be necessary
    std::shared_ptr<gate> get_gate();

    qreal get_width();
    qreal get_height();

    std::string get_input_pin_type_at_position(size_t pos);
    std::string get_output_pin_type_at_position(size_t pos);

protected:
    qreal m_width;
    qreal m_height;

    QString m_name;
    QString m_type;

    QVector<QString> m_input_pins;
    QVector<QString> m_output_pins;

    std::shared_ptr<gate> m_gate;
};

#endif    // GRAPHICS_GATE_H
