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

#ifndef EXPANDING_LIST_ITEM_H
#define EXPANDING_LIST_ITEM_H

#include <QFrame>
#include <QList>
#include <QResizeEvent>

class QPropertyAnimation;

namespace hal{
class expanding_list_button;

class expanding_list_item : public QFrame
{
    Q_OBJECT
    Q_PROPERTY(bool expanded READ expanded)
    Q_PROPERTY(int fixed_height READ fixed_height WRITE set_fixed_height)

public:
    expanding_list_item(expanding_list_button* parent_button, QWidget* parent = 0);

    virtual QSize minimumSizeHint() const Q_DECL_OVERRIDE;
    virtual QSize sizeHint() const Q_DECL_OVERRIDE;
    virtual void resizeEvent(QResizeEvent* event) Q_DECL_OVERRIDE;

    bool expanded();
    int fixed_height();
    bool contains(expanding_list_button* button);
    expanding_list_button* parent_button();
    void append_child_button(expanding_list_button* button);

    void repolish();

    void collapse();
    void expand();

    void set_expanded(bool expanded);
    void set_fixed_height(int height);

private:
    expanding_list_button* m_parent_button;
    QList<expanding_list_button*> m_child_buttons;

    int m_collapsed_height;
    int m_expanded_height;

    QPropertyAnimation* m_animation;

    bool m_expanded;
    int m_fixed_height;
};
}

#endif    // EXPANDING_LIST_ITEM_H
