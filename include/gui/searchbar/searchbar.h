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

class QLabel;
class QLineEdit;
class QHBoxLayout;
class QPushButton;
class QToolButton;

namespace hal{

class searchbar : public QFrame
{
    Q_OBJECT
    Q_PROPERTY(QString search_icon READ search_icon WRITE set_search_icon)
    Q_PROPERTY(QString search_icon_style READ search_icon_style WRITE set_search_icon_style)
    Q_PROPERTY(QString clear_icon READ clear_icon WRITE set_clear_icon)
    Q_PROPERTY(QString clear_icon_style READ clear_icon_style WRITE set_clear_icon_style)

public:
    searchbar(QWidget* parent = 0);

    QString search_icon() const;
    QString search_icon_style() const;
    QString clear_icon() const;
    QString clear_icon_style() const;

    void set_search_icon(const QString& icon);
    void set_search_icon_style(const QString& style);
    void set_clear_icon(const QString& icon);
    void set_clear_icon_style(const QString& style);

    void set_placeholder_text(const QString& text);
    void set_mode_button_text(const QString& text);

    void clear();
    QString get_current_text();

    void repolish();

Q_SIGNALS:
    void text_edited(const QString& text);
    void return_pressed();
    void mode_clicked();

public Q_SLOTS:
    void handle_text_edited(const QString& text);
    void handle_return_pressed();
    void handle_mode_clicked();

private:
    QHBoxLayout* m_layout;

    QLabel* m_search_icon_label;
    QLineEdit* m_line_edit;
    QLabel* m_clear_icon_label;
    QPushButton* m_mode_button;
    QToolButton* m_down_button;
    QToolButton* m_up_button;

    QString m_search_icon;
    QString m_search_icon_style;
    QString m_clear_icon;
    QString m_clear_icon_style;
};
}
