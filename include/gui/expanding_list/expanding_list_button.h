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

#ifndef EXPANDING_LIST_BUTTON_H
#define EXPANDING_LIST_BUTTON_H

#include <QFrame>

class QHBoxLayout;
class QLabel;

namespace hal{
class expanding_list_widget;


class expanding_list_button : public QFrame
{
    Q_OBJECT
    Q_PROPERTY(bool hover READ hover)
    Q_PROPERTY(bool selected READ selected)
    Q_PROPERTY(QString type READ type)
    Q_PROPERTY(QString icon_style READ icon_style WRITE set_icon_style)

public:
    expanding_list_button(QWidget* parent = 0);

    void enterEvent(QEvent* event) Q_DECL_OVERRIDE;
    void leaveEvent(QEvent* event) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent* event) Q_DECL_OVERRIDE;

    bool hover();
    bool selected();
    QString type();
    QString icon_style();

    void set_selected(bool selected);
    void set_type(const QString& type);
    void set_icon_style(const QString& style);

    void set_icon_path(const QString& path);
    void set_text(const QString& text);

    void repolish();

Q_SIGNALS:
    void clicked();

private:
    QHBoxLayout* m_layout;
    QFrame* m_left_border;
    QLabel* m_icon_label;
    QLabel* m_text_label;
    QFrame* m_right_border;

    bool m_hover;
    bool m_selected;
    QString m_type;
    QString m_icon_style;
    QString m_icon_path;
};
}

#endif    // EXPANDING_LIST_BUTTON_H
