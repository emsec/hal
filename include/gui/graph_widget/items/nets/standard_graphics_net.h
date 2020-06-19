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

#include "gui/graph_widget/items/nets/graphics_net.h"

#include <QLineF>
#include <QVector>

namespace hal
{
    class Net;

    class standard_graphics_net : public graphics_net
    {
    public:
        struct h_line
        {
            qreal small_x;
            qreal big_x;
            qreal y;
        };

        struct v_line
        {
            qreal x;
            qreal small_y;
            qreal big_y;
        };

        struct lines
        {
            void append_h_line(const qreal small_x, const qreal big_x, const qreal y);
            void append_v_line(const qreal x, const qreal small_y, const qreal big_y);

            void merge_lines();

        private:
            QVector<h_line> h_lines;
            QVector<v_line> v_lines;

            friend standard_graphics_net;
        };

        static void load_settings();
        static void update_alpha();

        standard_graphics_net(const std::shared_ptr<const Net> n, const lines& l);

        void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

    private:
        static qreal s_alpha;

        static qreal s_wire_length;

        static qreal s_left_arrow_offset;
        static qreal s_right_arrow_offset;

        static qreal s_arrow_left_x_shift;
        static qreal s_arrow_right_x_shift;
        static qreal s_arrow_side_length;

        static qreal s_arrow_width;
        static qreal s_arrow_height;

        static QPainterPath s_arrow;

        static qreal s_split_radius;

        QVector<QLineF> m_lines;
        QVector<QPointF> m_splits;
    };
}
