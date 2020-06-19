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
//  SOFTWARE

#pragma once

#include <QGraphicsObject>

namespace hal
{
    class node_drag_shadow : public QGraphicsObject
    {
        Q_OBJECT

    public:
        enum class drag_cue
        {
            movable = 0,
            swappable = 1,
            rejected = 2
        };

        node_drag_shadow();

        void start(const QPointF& posF, const QSizeF& sizeF);
        void stop();

        qreal width() const;
        qreal height() const;
        QSizeF size() const;

        void set_width(const qreal width);
        void set_height(const qreal height);

        void set_visual_cue(const drag_cue cue);

        static void set_lod(const qreal lod);
        static void load_settings();

    protected:
        void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;
        QRectF boundingRect() const override;
        QPainterPath shape() const override;

    private:
        static qreal s_lod;
        static QPen s_pen;
        static QColor s_color_pen[];
        static QColor s_color_solid[];
        static QColor s_color_translucent[];

        drag_cue m_cue;

        qreal m_width;
        qreal m_height;
    };
}
