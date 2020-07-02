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

#include "widget/widget.h"

#include <QIcon>
#include <QList>

class QShortcut;
class QVBoxLayout;

namespace hal
{
    class ContentAnchor;
    class Toolbar;

    class ContentWidget : public Widget
    {
        Q_OBJECT
        Q_PROPERTY(QString icon_style READ icon_style WRITE set_icon_style)
        Q_PROPERTY(QString icon_path READ icon_path WRITE set_icon_path)

    public:
        explicit ContentWidget(QString name, QWidget* parent = nullptr);

        virtual void setup_toolbar(Toolbar* Toolbar);
        virtual QList<QShortcut*> create_shortcuts();

        void repolish();

        QString name();
        QIcon icon();

        void set_anchor(ContentAnchor* anchor);
        void set_icon(QIcon icon);
        void set_name(const QString& name);

        QString icon_style();
        QString icon_path();

        void set_icon_style(const QString& style);
        void set_icon_path(const QString& path);

    Q_SIGNALS:
        void removed();
        void detached();
        void reattached();
        void opened();
        void closed();
        void name_changed(const QString& name);

    public Q_SLOTS:
        void remove();
        void detach();
        void reattach();
        void open();
        void close();

    private:
        void closeEvent(QCloseEvent* event);

        QString m_name;
        QIcon m_icon;
        ContentAnchor* m_anchor = nullptr;
        int m_index_priority         = 0;

        QString m_icon_style;
        QString m_icon_path;

    protected:
        QVBoxLayout* m_content_layout;
    };
}
