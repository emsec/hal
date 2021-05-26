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

#include "gui/docking_system/dock_button.h"
#include "gui/widget/widget.h"

#include <QFrame>
#include <QList>
#include <QPoint>

namespace hal
{
    class ContentAnchor;
    class ContentWidget;

    /**
     * @ingroup docking
     * @brief Contains and handles DockButton%s
     *
     * This class is used by the splitter anchor to store and handle DockButton%s. It manages information
     * including, but not limited to, the start/end of a drag action and the arrangement of the buttons.
     */
    class DockBar : public QFrame
    {
        Q_OBJECT

    public:
        /**
         * The dockbar's constructor.
         *
         * @param orientation - The orientation of the dockbar. It is either vertical or horizontal.
         * @param b_orientation - The orientation of the dockbar's buttons. It splits the vertical option into vertical_up and vertical_down.
         * @param parent - The dockbar's parent.
         */
        DockBar(Qt::Orientation orientation, button_orientation b_orientation, QWidget* parent = nullptr);

        /**
         * Overwritten QWidget function to handle the drag system. It is used to initialize additional draggin information so
         * that the dockbar's button can react accordingly (such as collapsing).
         *
         * @param event - The QMouseEvent that provides the mouse position and the pressed button.
         */
        void mouseMoveEvent(QMouseEvent* event) override;

        /**
         * Overwritten QWidget function to handle drag actions. It is called when a drag action enters the dockbar.
         *
         * @param event - The QDragEnterEvent.
         */
        void dragEnterEvent(QDragEnterEvent* event) override;

        /**
         * Overwritten QWidget function to handle drag actions. It is callen when drag mime data is moved within the dockbar.
         * It primarily manages the moving animation of the dockbuttons.
         *
         * @param event - The QDragMoveEvent.
         */
        void dragMoveEvent(QDragMoveEvent* event) override;

        /**
         * Overwritten QWidget function to handle drag actions. It is called when a drag action leaves the dockbar.
         *
         * @param event - The QDragLeaveEvent.
         */
        void dragLeaveEvent(QDragLeaveEvent* event) override;

        /**
         * Overwritten QWidget function to handle drag actions. It is called when drag mime data is dropped within the dockbar.
         * It removes the mime data (in this case a content widget) from its old splitter anchor and adds it to the dockbar's own
         * splitter anchor.
         *
         * @param event - The QDropEvent.
         */
        void dropEvent(QDropEvent* event) override;

        /**
         * Overwritten QObject function to handle the drag system. It is installed on a dockbutton to catch its MousePressEvent
         * so that the drag start position and dragbutton can be set.
         *
         * @param watched - The dockbutton that sent the event.
         * @param event - The event that was emitted by the watched dockbutton.
         * @return - True if the event must not be propagated any further. False (mostly the default) otherwise.
         */
        bool eventFilter(QObject* watched, QEvent* event) override;

        /**
         * Overwritten QWidget function to get the dockbar's preferred size. It is calculated using
         * the dockbuttons it contains.
         *
         * @return The preferred size.
         */
        QSize sizeHint() const override;

        /**
         * Overwritten QWidget function to get the dockbar's preferred size. It is calculated using
         * the dockbuttons it contains.
         *
         * @return The minimum size the dockbar prefers.
         */
        QSize minimumSizeHint() const override;

        /**
         * Sets the dockbar's correspondig content anchor. Used by the managing classes SplitterAnchor
         * and TabWidget.
         *
         * @param anchor
         */
        void setAnchor(ContentAnchor* anchor);

        /**
         * Sets the autohide property of the dockbar. If it is set to true the dockbar hides itself when
         * it contains zero dockbuttons. It still appears when the drag action of a dockbutton from anywhere is
         * started.
         *
         * @param autohide - The boolean to set the autohide property.
         */
        void setAutohide(bool autohide);

        /**
         * Get the used status of the dockbar. Returns true if the dockbar is unused, meaning all buttons are hidden
         * (For example when all widget belonging to the corresponding content anchor-and therefore to the dockbar-
         *  are detached).
         *
         * @return The used status.
         */
        bool unused();

        /**
         * Get the number of dockbuttons in the dockbar.
         *
         * @return The number of dockbuttons.
         */
        int count();

        /**
         * Get the index of the correspondig dockbutton in the dockbar.
         *
         * @param widget - The widget
         * @return The index of the widget. Returns -1 if the widget has no corresponding button.
         */
        int index(ContentWidget* widget);

        /**
         * Calculates new button positions and moves them based on their orientation and available status.
         */
        void rearrangeButtons();

        /**
         * Used when dragging a dockbutton within the dockbar. It animates the dockbuttons so that the
         * dragged button can be moved between them.
         */
        void collapseButtons();

        /**
         * Adds a new button for the content widget at the given position to the dockbar.
         *
         * @param widget - The content widget for which a button is created and then connected.
         * @param index - The index to add the button.
         */
        void addButton(ContentWidget* widget, int index);

        /**
         * Removes the corresponding button for the given conten widget.
         *
         * @param widget - The content widget to remove the corresponding button.
         * @return True when a button for the widget is found and sucessfully removed. False otherwise.
         */
        bool removeButton(ContentWidget* widget);

        /**
         * Removes a button at the given index.
         *
         * @param index - The index at which a button should be removed.
         * @return True when the button is sucessfully removed. False when the index is out of bounds.
         */
        bool removeButton(int index);

        /**
         * Hides the corresponding button for the given widget when a detach action is performed.
         *
         * @param widget - The widget that was detached.
         */
        void detachButton(ContentWidget* widget);

        /**
         * Shows the corresponding button for the given widget when a reattach action is performed.
         * @param widget - The widget that is reattached.
         */
        void reattachButton(ContentWidget* widget);

        /**
         * Checks the corresponding dockbutton of the given content widget.
         *
         * @param widget - The content widget for which the corresponding dockbutton is checked.
         */
        void checkButton(ContentWidget* widget);

        /**
         * Unchecks the corresponding bockbutton of the given content widget.
         *
         * @param widget - The content widget for which the corresponding dockbutton is unchecked.
         */
        void uncheckButton(ContentWidget* widget);

        /**
         * Get the content widget at a given index.
         *
         * @param index - The index for the content widget.
         * @return The content widget at the index. If the index is out of bounds a nullpointer is returned.
         */
        ContentWidget* widgetAt(int index);

        /**
         * Get the next available widget starting at the index. A widget is available when it belongs to the corresponding
         * splitter anchor and is not detached. When the index is < 0 or within the boundaries the search is conducted from
         * "left to right" (index 0 -> number of bottoms). If the index is greater than the number of buttons the search is
         * conducted from the opposite direction.
         *
         * @param index - The startindex of the search.
         * @return The found content widget. If no available widget is found a nullpointer is returned.
         */
        ContentWidget* nextAvailableWidget(int index);

        /**
         * Removes all buttons from the dockbear. The corresponding content widgets are not deleted but hidden.
         */
        void clear();

    Q_SIGNALS:

        /**
         * Q_SIGNAL that can be emitted to interrupt the collapse animation.
         */
        void stopAnimations();

    public Q_SLOTS:

        /**
         * Shows the dockbar in case it was hidden. It is connected to the dragStart signal
         * of the content drag relay.
         */
        void handleDragStart();

        /**
         * Hides the dockbar when it contains no dockbuttons and the autohide property is set.
         * It is connected to the dragEnd signal of the content relay.
         */
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
