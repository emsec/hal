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

#ifndef GRAPH_GRAPHICS_ITEM_H
#define GRAPH_GRAPHICS_ITEM_H

#include <QColor>
#include <QGraphicsItem>
#include <QString>

enum class item_class
{
    gate = 0,
    net  = 1,
};

class graph_graphics_item : public QGraphicsItem
{
public:
    static void load_settings();
    static void set_lod(qreal lod);

    graph_graphics_item();

    item_class get_class();
    void set_color(QColor& color);

protected:
    static qreal s_lod;
    static QColor s_selection_color;

    void mousePressEvent(QGraphicsSceneMouseEvent* event) Q_DECL_OVERRIDE;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) Q_DECL_OVERRIDE;

    QColor m_color;

    int m_id;

    item_class m_class;
};

#endif    // GRAPH_GRAPHICS_ITEM_H
