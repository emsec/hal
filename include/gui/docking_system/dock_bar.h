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

#ifndef DOCK_BAR_H
#define DOCK_BAR_H

#include "docking_system/dock_button.h"
#include "hal_widget/hal_widget.h"

#include <QFrame>
#include <QList>
#include <QPoint>

namespace hal{
class hal_content_anchor;
class content_widget;

class dock_bar : public QFrame
{
    Q_OBJECT

public:
    dock_bar(Qt::Orientation orientation, button_orientation b_orientation, QWidget* parent = 0);

    void mouseMoveEvent(QMouseEvent* event) Q_DECL_OVERRIDE;
    void dragEnterEvent(QDragEnterEvent* event) Q_DECL_OVERRIDE;
    void dragMoveEvent(QDragMoveEvent* event) Q_DECL_OVERRIDE;
    void dragLeaveEvent(QDragLeaveEvent* event) Q_DECL_OVERRIDE;
    void dropEvent(QDropEvent* event) Q_DECL_OVERRIDE;
    bool eventFilter(QObject* watched, QEvent* event) Q_DECL_OVERRIDE;
    QSize sizeHint() const Q_DECL_OVERRIDE;
    QSize minimumSizeHint() const Q_DECL_OVERRIDE;

    void set_anchor(hal_content_anchor* anchor);
    void set_autohide(bool autohide);
    bool unused();
    int count();
    int index(content_widget* widget);
    void rearrange_buttons();
    void collapse_buttons();
    void add_button(content_widget* widget, int index);
    bool remove_button(content_widget* widget);
    bool remove_button(int index);

    void detach_button(content_widget* widget);
    void reattach_button(content_widget* widget);
    void check_button(content_widget* widget);
    void uncheck_button(content_widget* widget);
    content_widget* widget_at(int index);
    content_widget* next_available_widget(int index);

Q_SIGNALS:
    void stop_animations();

public Q_SLOTS:
    void handle_drag_start();
    void handle_drag_end();

private:
    hal_content_anchor* m_anchor;
    QList<dock_button*> m_buttons;
    button_orientation m_button_orientation;
    int m_button_offset  = 0;
    int m_button_spacing = 1;
    bool m_autohide      = true;

    static QPoint s_drag_start_position;
    static dock_button* s_drag_button;
    static int s_begin_drop_range;
    static int s_end_drop_range;
    static int s_drop_spacing;
    static dock_button* s_move_marker;
};
}

#endif    // DOCK_BAR_H
