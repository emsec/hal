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

#ifndef NO_SCHEDULED_PLUGINS_WIDGET_H
#define NO_SCHEDULED_PLUGINS_WIDGET_H

#include <QFrame>
#include <QLabel>

class QVBoxLayout;

namespace hal{

class no_scheduled_plugins_widget : public QFrame
{
    Q_OBJECT
    Q_PROPERTY(bool drag_active READ drag_active)
    Q_PROPERTY(QString icon_path READ icon_path WRITE set_icon_path)
    Q_PROPERTY(QString icon_style READ icon_style WRITE set_icon_style)

public:
    explicit no_scheduled_plugins_widget(QWidget* parent = nullptr);

    void dragEnterEvent(QDragEnterEvent* event) Q_DECL_OVERRIDE;
    void dragLeaveEvent(QDragLeaveEvent* event) Q_DECL_OVERRIDE;
    void dropEvent(QDropEvent* event) Q_DECL_OVERRIDE;

    bool drag_active();
    QString icon_path();
    QString icon_style();

    void set_icon_path(const QString& path);
    void set_icon_style(const QString& style);

    void repolish();

Q_SIGNALS:
    void append_plugin(const QString& name);

private:
    QVBoxLayout* m_layout;
    QLabel* m_text_label;
    QLabel* m_icon_label;

    QString m_icon_path;
    QString m_icon_style;

    bool m_drag_active;
};
}

#endif    // NO_SCHEDULED_PLUGINS_WIDGET_H
