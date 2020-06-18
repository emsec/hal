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

#ifndef CONTENT_FRAME_H
#define CONTENT_FRAME_H

#include <QLabel>
#include <QLayout>
namespace hal{

class content_widget;
class toolbar;

class content_frame : public QWidget
{
    Q_OBJECT

public:
    explicit content_frame(content_widget* widget, bool attached, QWidget* parent = nullptr);

    void childEvent(QChildEvent* event) Q_DECL_OVERRIDE;

    content_widget* content();

public Q_SLOTS:
    void detach_widget();
    void reattach_widget();

private:
    void closeEvent(QCloseEvent* event) Q_DECL_OVERRIDE;

    QVBoxLayout* m_vertical_layout;
    QHBoxLayout* m_horizontal_layout;
    toolbar* m_left_toolbar;
    toolbar* m_right_toolbar;
    content_widget* m_widget;
    QLabel* m_name_label;

    QString m_detach_icon_path;
    QString m_detach_icon_style;
};
}

#endif    // CONTENT_FRAME_H
