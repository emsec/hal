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

#include "content_widget/content_widget.h"

#include <QWidget>

class QFrame;
class QHBoxLayout;
class QVBoxLayout;

namespace hal
{
    class DockBar;
    class splitter;
    class SplitterAnchor;
    class TabWidget;

    enum class content_anchor
    {
        center = 0,
        left   = 1,
        right  = 2,
        bottom = 3
    };

    class ContentLayoutArea : public QWidget
    {
        Q_OBJECT

    public:
        explicit ContentLayoutArea(QWidget* parent = 0);
        void add_content(ContentWidget* widget, int index, content_anchor anchor);

        void init_splitter_size(const QSize& size);

    public Q_SLOTS:
        void update_left_dock_bar();
        void update_right_dock_bar();
        void update_bottom_dock_bar();

    private:
        QVBoxLayout* m_top_level_layout;
        QHBoxLayout* m_second_level_layout;
        QVBoxLayout* m_third_level_layout;
        QHBoxLayout* m_fourth_level_layout;
        QVBoxLayout* m_splitter_layout;
        QHBoxLayout* m_central_layout;
        QHBoxLayout* m_spacer_layout;

        splitter* m_vertical_splitter;
        splitter* m_horizontal_splitter;
        splitter* m_left_splitter;
        splitter* m_right_splitter;
        splitter* m_bottom_splitter;

        DockBar* m_left_dock;
        DockBar* m_right_dock;
        DockBar* m_bottom_dock;

        QWidget* m_bottom_container;
        QFrame* m_left_spacer;
        QFrame* m_right_spacer;

        SplitterAnchor* m_left_anchor;
        SplitterAnchor* m_right_anchor;
        SplitterAnchor* m_bottom_anchor;

        TabWidget* m_tab_widget;
    };
}
