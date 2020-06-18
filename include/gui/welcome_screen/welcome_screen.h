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

#ifndef WELCOME_SCREEN_H
#define WELCOME_SCREEN_H

#include "QFrame"
namespace hal{
class get_in_touch_widget;
class open_file_widget;
class recent_files_widget;
class labeled_frame;

class QHBoxLayout;

class welcome_screen : public QFrame
{
    Q_OBJECT

public:
    explicit welcome_screen(QWidget* parent = nullptr);

    void repolish();

    //    void intro();
    //    void reset();

private:
    QHBoxLayout* m_layout;

    labeled_frame* m_recent_files_frame;
    labeled_frame* m_open_file_frame;
    labeled_frame* m_get_in_touch_frame;

    recent_files_widget* m_recent_files_widget;
    open_file_widget* m_open_file_widget;
    get_in_touch_widget* m_get_in_touch_widget;
};
}

#endif    // WELCOME_SCREEN_H
