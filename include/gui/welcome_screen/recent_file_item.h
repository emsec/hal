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

class QHBoxLayout;
class QLabel;
class QPropertyAnimation;
class QVBoxLayout;
class QToolButton;

namespace hal
{
    class RecentFileItem : public QFrame
    {
        Q_OBJECT
        Q_PROPERTY(bool hover READ hover)
        Q_PROPERTY(bool disabled READ disabled)
        Q_PROPERTY(QString icon_path READ icon_path WRITE set_icon_path)
        Q_PROPERTY(QString icon_style READ icon_style WRITE set_icon_style)

    public:
        explicit RecentFileItem(const QString& file, QWidget* parent = nullptr);

        void enterEvent(QEvent* event) Q_DECL_OVERRIDE;
        void leaveEvent(QEvent* event) Q_DECL_OVERRIDE;
        void mousePressEvent(QMouseEvent* event) Q_DECL_OVERRIDE;

        virtual QSize sizeHint() const Q_DECL_OVERRIDE;
        virtual bool eventFilter(QObject* object, QEvent* event) Q_DECL_OVERRIDE;

        QString file();

        void repolish();

        bool hover();
        bool disabled();
        QString icon_path();
        QString icon_style();

        void set_hover_active(bool active);
        void set_disabled(bool disable);
        void set_icon_path(const QString& path);
        void set_icon_style(const QString& style);

    Q_SIGNALS:
        void remove_requested(RecentFileItem* item);

    private:
        QWidget* m_widget;
        QHBoxLayout* m_horizontal_layout;
        QLabel* m_icon_label;
        QVBoxLayout* m_vertical_layout;
        QLabel* m_name_label;
        QLabel* m_path_label;
        QPropertyAnimation* m_animation;
        QToolButton* m_remove_button;

        QString m_file;
        QString m_path;

        bool m_hover;
        bool m_disabled;

        QString m_icon_path;
        QString m_icon_style;

        //functions
        void handle_close_requested();
    };
}
