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

#ifndef GRAPH_NAVIGATION_ITEM_H
#define GRAPH_NAVIGATION_ITEM_H

#include "def.h"
#include <QList>
#include <QString>
#include <QVariant>

class graph_navigation_item : QObject
{
    Q_OBJECT

public:
    enum class item_type
    {
        ignore    = 0,
        net       = 1,
        gate      = 2,
        module = 3
    };

    graph_navigation_item(QString name, int id, item_type type, graph_navigation_item* parent_item = 0);
    ~graph_navigation_item();

    void append_child(graph_navigation_item* child);
    void prepend_child(graph_navigation_item* child);
    void insert_child(int row, graph_navigation_item* child);
    void remove_child(graph_navigation_item* child);

    graph_navigation_item* child(int row);
    int childCount() const;
    QVariant data(int column) const;
    int row() const;

    graph_navigation_item* parent_item();
    QString name();
    u32 id();
    item_type type();

private:
    graph_navigation_item* m_parent_item;
    QList<graph_navigation_item*> m_child_items;

    QString m_name;
    u32 m_id;
    item_type m_type;
};

Q_DECLARE_METATYPE(graph_navigation_item::item_type)

#endif    // GRAPH_NAVIGATION_ITEM_H
