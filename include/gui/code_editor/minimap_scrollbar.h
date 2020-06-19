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

#include <QWidget>

class QFrame;
class QScrollBar;

namespace hal
{

    class minimap_scrollbar : public QWidget
    {
        Q_OBJECT

    public:
        minimap_scrollbar(QWidget* parent = nullptr);

        int minimum() const;
        int maximum() const;
        int value() const;
        int slider_height() const;
        int slider_position() const;

        //    void set_minimum(const int minimum);
        //    void set_maximum(const int maximum);
        void set_range(const int minimum, const int maximum);
        void set_value(const int value);
        void set_slider_height(const int height);

        void set_scrollbar(QScrollBar* scrollbar);

    protected:
        void paintEvent(QPaintEvent* event) Q_DECL_OVERRIDE;
        void resizeEvent(QResizeEvent* event) Q_DECL_OVERRIDE;

        void mousePressEvent(QMouseEvent* event) Q_DECL_OVERRIDE;
        void mouseMoveEvent(QMouseEvent* event) Q_DECL_OVERRIDE;
        void mouseReleaseEvent(QMouseEvent* event) Q_DECL_OVERRIDE;
        void leaveEvent(QEvent* event) Q_DECL_OVERRIDE;

    private:
        void adjust_slider_to_value();

        QFrame* m_slider;

        int m_minimum;
        int m_maximum;
        int m_value;

        int m_handle_length;
        int m_handle_position;

        bool m_mouse_pressed;
        int m_drag_offset;

        QScrollBar* m_scrollbar;
    };
}
