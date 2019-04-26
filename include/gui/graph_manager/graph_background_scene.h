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

#ifndef GRAPH_BACKGROUND_SCENE_H
#define GRAPH_BACKGROUND_SCENE_H

#include <QGraphicsScene>

#include <QPainter>
#include <QVarLengthArray>

class graph_background_scene : public QGraphicsScene
{
    Q_OBJECT
public:
    graph_background_scene(QObject* parent = 0);

    void update_max_width(qreal width);

    void update_max_height(qreal height);

    void apply_layout();

    bool is_grid_visible();

protected:
    virtual void drawBackground(QPainter* painter, const QRectF& rect);

private Q_SLOTS:

    void set_draw_grid_false();

    void set_draw_grid_true();

private:
    qreal m_max_width, m_max_height;

    bool m_draw_grid;
};

#endif    // GRAPH_BACKGROUND_SCENE_H
