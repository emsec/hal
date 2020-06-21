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

#include <QFrame>

class QAction;
class QVBoxLayout;

namespace hal
{
    class hal_window_layout_container;
    class hal_window_toolbar;
    class Overlay;
    class workspace; // TEMP NAME ?

    class hal_window : public QFrame
    {
        Q_OBJECT

    public:
        explicit hal_window(QWidget* parent = nullptr);

        void lock();
        void unlock();

        void standard_view();
        void special_view(QWidget* widget);

        void repolish();

        hal_window_toolbar* get_toolbar();
        Overlay* get_Overlay();

    protected:
        //void paintEvent(QPaintEvent* event) Q_DECL_OVERRIDE;
        //bool event(QEvent* event) Q_DECL_OVERRIDE;
        void closeEvent(QCloseEvent* event) Q_DECL_OVERRIDE;
    //    void changeEvent(QEvent* event) Q_DECL_OVERRIDE;
    //    void resizeEvent(QResizeEvent* event) Q_DECL_OVERRIDE;

    private:
        //void rearrange();

        QVBoxLayout* m_outer_layout;
        hal_window_layout_container* m_layout_container;
        QVBoxLayout* m_inner_layout;
        hal_window_toolbar* m_toolbar;
        workspace* m_workspace;

        Overlay* m_Overlay;
        QGraphicsEffect* m_effect;
    };
}
