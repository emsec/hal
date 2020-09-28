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

    class OldStandardGraphicsNet : public GraphicsNet
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
            QVector<h_line> h_lines;
            QVector<v_line> v_lines;

    //        void remove_zero_length_lines();
    //        void fix_order();
    //        void move(qreal x, qreal y);
        };

        static void load_settings();
        static void update_alpha();

        //StandardGraphicsNet(Net* n, const lines& l);
        OldStandardGraphicsNet(Net* n, lines& l, bool complete = true);

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

        QVector<QLineF> m_output_lines;
        QVector<QLineF> m_other_lines;
        QVector<QPointF> m_splits;

        bool m_complete;
    };
}
