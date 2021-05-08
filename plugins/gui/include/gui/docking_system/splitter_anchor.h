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

#include "gui/content_anchor/content_anchor.h"

#include <QList>
#include <QObject>

namespace hal
{
    class DockBar;
    class Splitter;
    class ContentFrame;
    class ContentWidget;

    /**
     * @ingroup docking
     * @brief Adds specific ContentWidget%s to HALs main content area
     *
     * The SplitterAnchor class provides with the TabWidget the functionality to add ContentWidget%s to hal's main content area on the "lowest"
     * level as well as the area (via the Splitter) to do so. It is the bridge between the DockBar and its corresponding area (the splitter)
     * and manages/synchronizes the information between them.
     */
    class SplitterAnchor : public QObject, public ContentAnchor
    {
        Q_OBJECT

    public:
        /**
         *The splitter anchor's constructor.
         *
         * @param DockBar - The splitter ancor's dockbar.
         * @param Splitter - The actual splitter to represent the dockbar's area.
         * @param parent - The parent of the splitter anchor.
         */
        SplitterAnchor(DockBar* DockBar, Splitter* Splitter, QObject* parent = nullptr);

        /**
         * Adds a contentwidget at the given position to the splitter area and tells the dockbar to
         * add a button at the given position that holds the content widget's data.
         *
         * @param widget - The content widget to be added to the docking system.
         * @param index - The index at which the content widget has to be added.
         */
        virtual void add(ContentWidget* widget, int index = -1) override;

        /**
         * Removes (in this case hides, not deletes) the content widget from this area.
         * The dockbutton is also removed from the dockbar.
         *
         * @param widget - The content widget that shall be removed.
         */
        virtual void remove(ContentWidget* widget) override;

        /**
         * This function detaches a widget and its button from its area and dockbar (hides them)
         * when the action "detach" is triggered in the content frame. A new content
         * frame is created and the widget is displayed on the same level as hal as a seperate window.
         *
         * @param widget - The widget to be detached.
         */
        virtual void detach(ContentWidget* widget) override;

        /**
         * Reattaches the given widget back to the splitter area and dockbar (shows them again) when
         * the action "reattach" is triggered in the content frame. A new content frame is created and
         * then placed back into the area. Thereafter the content widget's corresponding button is shown again.
         *
         * @param widget - The widget that is reattached.
         */
        virtual void reattach(ContentWidget* widget) override;

        /**
         * Shows the given widget.
         *
         * @param widget - The widget to be displayed.
         */
        virtual void open(ContentWidget* widget) override;

        /**
         * Hides the given widget.
         *
         * @param widget - The widget to hide.
         */
        virtual void close(ContentWidget* widget) override;

        /**
         * Get the number of widgets currently associated with the dockbar.
         *
         * @return The number of widgets.
         */
        int count();

        /**
         * Removes all buttons from the dockbar and therefore the widgets from the area. The corresponding
         * widgets are not destroyed but hidden.
         */
        void clear();

    Q_SIGNALS:

        /**
         * Q_SIGNAL that is emitted when a widget is either added, removed, attached or reattched.
         */
        void contentChanged();

    private:
        DockBar* mDockBar;
        Splitter* mSplitter;
        QList<ContentFrame*> mDetachedFrames;
    };
}
