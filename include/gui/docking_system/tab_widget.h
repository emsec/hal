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

#include "docking_system/dock_bar.h"
#include "hal_content_anchor/hal_content_anchor.h"
#include "hal_widget/hal_widget.h"
#include "toolbar/toolbar.h"

#include <QAction>
#include <QLayout>
#include <QList>
#include <QStackedWidget>

class QShortcut;

namespace hal{
class content_frame;
class content_widget;

class tab_widget : public hal_widget, public hal_content_anchor
{
    Q_OBJECT

public:
    explicit tab_widget(QWidget* parent = nullptr);
    virtual void add(content_widget* widget, int index = -1);
    virtual void remove(content_widget* widget);
    virtual void detach(content_widget* widget);
    virtual void reattach(content_widget* widget);
    virtual void open(content_widget* widget);
    virtual void close(content_widget* widget);

    void handle_no_current_widget(int index);
    void remove_content();

public Q_SLOTS:
    void detach_current_widget();
    void handle_drag_start();
    void handle_drag_end();

private:
    QVBoxLayout* m_vertical_layout;
    QHBoxLayout* m_horizontal_layout;
    dock_bar* m_dock_bar;
    toolbar* m_left_toolbar;
    toolbar* m_right_toolbar;
    content_widget* m_current_widget;
    QAction* m_action_detach;
    QList<content_frame*> m_detached_frames;
    QList<QShortcut*> m_active_shortcuts;
};
}
