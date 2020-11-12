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

#include "gui/widget/widget.h"

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
        Q_PROPERTY(QString iconStyle READ iconStyle WRITE setIconStyle)
        Q_PROPERTY(QString iconPath READ iconPath WRITE setIconPath)

    public:
        explicit ContentWidget(QString name, QWidget* parent = nullptr);

        virtual void setupToolbar(Toolbar* Toolbar);
        virtual QList<QShortcut*> createShortcuts();

        void repolish();

        QString name();
        QIcon icon();

        void setAnchor(ContentAnchor* anchor);
        void setIcon(QIcon icon);
        void set_name(const QString& name);

        QString iconStyle();
        QString iconPath();

        void setIconStyle(const QString& style);
        void setIconPath(const QString& path);

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

        QString mName;
        QIcon mIcon;
        ContentAnchor* mAnchor = nullptr;
        int mIndexPriority         = 0;

        QString mIconStyle;
        QString mIconPath;

    protected:
        QVBoxLayout* mContentLayout;
    };
}
