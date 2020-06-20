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

#include <QToolButton>

namespace hal
{
    class ContentWidget;

    enum class button_orientation
    {
        horizontal    = 0,
        vertical_up   = 1,
        vertical_down = 2
    };

    class DockButton : public QToolButton
    {
        Q_OBJECT

    public:
        DockButton(ContentWidget* widget, button_orientation orientation, QObject* eventFilter, QWidget* parent);

        void paintEvent(QPaintEvent* event) Q_DECL_OVERRIDE;

        void adjust_size();
        int relative_width();
        ContentWidget* widget();
        bool hidden();
        bool available();
        void set_available(bool available);
        void set_relative_height(int height);

        //overshadowed functions
        void hide();
        void show();

    public Q_SLOTS:
        void handle_clicked(bool checked);

    private:
        ContentWidget* m_widget;
        button_orientation m_orientation;
        int m_width;
        int m_height;
        int m_relative_width;
        int m_relative_height;
        int m_icon_size;
        int m_text_offset;
        int m_width_padding;
        int m_height_padding;
        bool m_hidden;
        bool m_available = true;
    };
}
