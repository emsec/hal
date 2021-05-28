//  MIT License
//
//  Copyright (c) 2019 Ruhr University Bochum, Chair for Embedded Security. All Rights reserved.
//  Copyright (c) 2021 Max Planck Institute for Security and Privacy. All Rights reserved.
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

#include "gui/docking_system/dock_bar.h"
#include "gui/content_anchor/content_anchor.h"
#include "gui/widget/widget.h"
#include "gui/toolbar/toolbar.h"

#include <QAction>
#include <QLayout>
#include <QList>
#include <QStackedWidget>

class QShortcut;

namespace hal
{
    class ContentFrame;
    class ContentWidget;

    /**
     * @ingroup docking
     * @brief Adds specific ContentWidget%s to HALs main content area
     *
     * The TabWidget class provides with the SplitterAnchor the functionality to add content widgets
     * to hal's main content area on the "lowest" level.
     */
    class TabWidget : public Widget, public ContentAnchor
    {
        Q_OBJECT

    public:

        /**
         * The constructor.
         *
         * @param parent - The tab widget's parent.
         */
        explicit TabWidget(QWidget* parent = nullptr);

        /**
         * Adds the given content widget to the dockbar at the given position
         * and displays it if no current widget is set.
         *
         * @param widget - The widget to add to the dockbar.
         * @param index - The position.
         */
        virtual void add(ContentWidget* widget, int index = -1);

        /**
         * Removes the given content widget from the dockbar. If the widget was currently
         * displayed it is then hidden.
         *
         * @param widget - The content widget to remove.
         */
        virtual void remove(ContentWidget* widget);

        /**
         * Detaches the content widget and its corresponding dockbutton (hides it) from the dockbar
         * and displays the content widget in its own window.
         * @param widget
         */
        virtual void detach(ContentWidget* widget);

        /**
         * Reattches the given content widget and corresponding dockbutton (shows it) to the dockbar.
         *
         * @param widget - The content widget to reattach.
         */
        virtual void reattach(ContentWidget* widget);

        /**
         * Shows the given content widget in the tab widgets area. It sets up the new toolbar and shortcuts.
         *
         * @param widget - The widget to open.
         */
        virtual void open(ContentWidget* widget);

        /**
         * Closes the currently displayed widget.
         *
         * @param widget - This parameter is not used since always the active widget is closed.
         */
        virtual void close(ContentWidget* widget);

        /**
         * This function is used when the currently displayed widget is either removed or detached from the
         * tab widget. It sets the next available widget as the current one. If no available widget is found
         * the tab widget hides itself.
         *
         * @param index - The startposition to search for the next widget.
         */
        void handleNoCurrentWidget(int index);

        /**
         * Removes all buttons from the dockbar and therefore the widgets from the area. The corresponding
         * widgets are not destroyed but hidden.
         */
        void clear();

    public Q_SLOTS:

        /**
         * Detaches the currently displayed widget.
         */
        void detachCurrentWidget();

        /**
         * Shows its area in case it was hidden. It is connected to the dragStart signal
         * of the content drag relay.
         */
        void handleDragStart();

        /**
         * Hides the area when the tab widget's dockbar contains no dockbuttons.
         * It is connected to the dragEnd signal of the content relay.
         */
        void handleDragEnd();

    private:
        QVBoxLayout* mVerticalLayout;
        QHBoxLayout* mHorizontalLayout;
        DockBar* mDockBar;
        Toolbar* mLeftToolbar;
        Toolbar* mRightToolbar;
        ContentWidget* mCurrentWidget;
        QAction* mActionDetach;
        QList<ContentFrame*> mDetachedFrames;
        QList<QShortcut*> mActiveShortcuts;
    };
}
