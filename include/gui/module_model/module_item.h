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

#ifndef MODULE_ITEM_H
#define MODULE_ITEM_H

#include "def.h"

#include <QColor>
#include <QList>
#include <QString>
#include <QVariant>

class module_item
{
public:
    module_item(const QString& name, const int id);
    ~module_item();

    void insert_child(int row, module_item* child);
    void remove_child(module_item* child);

    void append_child(module_item* child);
    void prepend_child(module_item* child);

    module_item* parent();
    module_item* child(int row);

    const module_item* const_parent() const;
    const module_item* const_child(int row) const;

    int childCount() const;
    QVariant data(int column) const;
    int row() const;

    QString name() const;
    u32 id() const;
    QColor color() const;

    void set_parent(module_item* parent);
    void set_name(const QString& name);
    void set_color(const QColor& color);

private:
    module_item* m_parent;
    QList<module_item*> m_child_items;

    QString m_name;
    u32 m_id;

    QColor m_color;
};

#endif // MODULE_ITEM_H
