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

#ifndef SCHEDULED_PLUGIN_ITEM_H
#define SCHEDULED_PLUGIN_ITEM_H

#include <QFrame>

class QHBoxLayout;
class QLabel;
class QPropertyAnimation;
class QVBoxLayout;

namespace hal{

class scheduled_plugin_item : public QFrame
{
    Q_OBJECT
    Q_PROPERTY(bool hover READ hover)
    Q_PROPERTY(QString icon_path READ icon_path WRITE set_icon_path)
    Q_PROPERTY(QString icon_style READ icon_style WRITE set_icon_style)

public:
    explicit scheduled_plugin_item(const QString& name, QWidget* parent = nullptr);

    void enterEvent(QEvent* event) Q_DECL_OVERRIDE;
    void leaveEvent(QEvent* event) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent* event) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent* event) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent* event) Q_DECL_OVERRIDE;

    void repolish();

    bool hover();
    QString icon_path();
    QString icon_style();

    QString name();

    void set_hover_active(bool active);
    void set_icon_path(const QString& path);
    void set_icon_style(const QString& style);

Q_SIGNALS:
    void clicked(scheduled_plugin_item*);
    void drag_started(scheduled_plugin_item*);
    void removed(scheduled_plugin_item*);

private:
    static bool s_drag_in_progress;
    static QPoint s_drag_start_position;

    void exec_drag();

    QHBoxLayout* m_layout;
    QLabel* m_label;

    bool m_hover;

    QString m_name;

    QString m_icon_path;
    QString m_icon_style;
};
}

#endif    // SCHEDULED_PLUGIN_ITEM_H
