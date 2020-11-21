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

#include "gui/docking_system/dock_button.h"
#include "gui/widget/widget.h"

#include <QFrame>
#include <QList>
#include <QPoint>

namespace hal
{
    class ContentAnchor;
    class ContentWidget;

    class DockBar : public QFrame
    {
        Q_OBJECT

    public:
        DockBar(Qt::Orientation orientation, button_orientation b_orientation, QWidget* parent = 0);

        void mouseMoveEvent(QMouseEvent* event) Q_DECL_OVERRIDE;
        void dragEnterEvent(QDragEnterEvent* event) Q_DECL_OVERRIDE;
        void dragMoveEvent(QDragMoveEvent* event) Q_DECL_OVERRIDE;
        void dragLeaveEvent(QDragLeaveEvent* event) Q_DECL_OVERRIDE;
        void dropEvent(QDropEvent* event) Q_DECL_OVERRIDE;
        bool eventFilter(QObject* watched, QEvent* event) Q_DECL_OVERRIDE;
        QSize sizeHint() const Q_DECL_OVERRIDE;
        QSize minimumSizeHint() const Q_DECL_OVERRIDE;

        void setAnchor(ContentAnchor* anchor);
        void setAutohide(bool autohide);
        bool unused();
        int count();
        int index(ContentWidget* widget);
        void rearrangeButtons();
        void collapseButtons();
        void addButton(ContentWidget* widget, int index);
        bool removeButton(ContentWidget* widget);
        bool removeButton(int index);

        void detachButton(ContentWidget* widget);
        void reattachButton(ContentWidget* widget);
        void checkButton(ContentWidget* widget);
        void uncheckButton(ContentWidget* widget);
        ContentWidget* widgetAt(int index);
        ContentWidget* nextAvailableWidget(int index);

    Q_SIGNALS:
        void stopAnimations();

    public Q_SLOTS:
        void handleDragStart();
        void handleDragEnd();

    private:
        ContentAnchor* mAnchor;
        QList<DockButton*> mButtons;
        button_orientation mButtonOrientation;
        int mButtonOffset  = 0;
        int mButtonSpacing = 1;
        bool mAutohide      = true;
        Qt::Orientation mOrientation;

        static QPoint sDragStartPosition;
        static DockButton* sDragButton;
        static int sBeginDropRange;
        static int sEndDropRange;
        static int sDropSpacing;
        static DockButton* sMoveMarker;
    };
}
