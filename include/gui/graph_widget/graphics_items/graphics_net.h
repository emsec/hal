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

#ifndef GRAPHICS_NET_H
#define GRAPHICS_NET_H

#include "graph_widget/graphics_items/graphics_item.h"

#include <memory>

class net;

class graphics_net : public graphics_item
{
public:
    struct visuals
    {
        QColor color;

        // BOOL HIDE ?
        // DRAW TYPE ENUM
        // SPLIT MARKERS
    };

    static void load_settings();

    graphics_net(std::shared_ptr<net> n);

    QRectF boundingRect() const Q_DECL_OVERRIDE;
    virtual QPainterPath shape() const Q_DECL_OVERRIDE;

    virtual void finalize() = 0;

    std::shared_ptr<net> get_net();

protected:
    static QPen s_pen;
    static qreal s_stroke_width;

    QRectF m_rect;
    QPainterPath m_shape;

    std::shared_ptr<net> m_net;
};

#endif // GRAPHICS_NET_H
