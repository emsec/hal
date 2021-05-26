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

#include "gui/content_widget/content_widget.h"

#include <QWidget>

class QFrame;
class QHBoxLayout;
class QVBoxLayout;

namespace hal
{
    class DockBar;
    class Splitter;
    class SplitterAnchor;
    class TabWidget;

    enum class content_anchor
    {
        center = 0,
        left   = 1,
        right  = 2,
        bottom = 3
    };

    /**
     * @ingroup gui
     * @brief Manages the layout of all ContentWidget%s
     *
     * The ContentLayoutArea manages the main layout of the content widgets that appears after a file was opened.
     * It owns the hierarchy of the layout down to the Splitters/SplitterAnchor%s. In provides a function to move
     * content widgets to one of its content anchors.
     *
     * The ContentLayoutArea builds up the following layout hierarchy to achieve the desired
     * overall layout (in this order): <br>
     * <i>mTopLevelLayout</i> <ul>
     *   <li> <i>mSecondLevelLayout</i> <ul>
     *      <li> mLeftDock (the docker bar on the far left)
     *      <li> <i>mThirdLevelLayout</i> <ul>
     *          <li> <i>mFourthLevelLayout</i> <ul>
     *              <li> <i>mVerticalSplitter</i> <ul>
     *                  <li> <i>mHorizontalSplitter</i> <ul>
     *                      <li> mLeftSplitter (contains the content widgets docked and opened left)
     *                      <li> mTabWidget (the tab widget (including the top docker bar) at the center that only
     *                           contains the graph-views widget)
     *                      <li> mRightSplitter (contains the content widgets docked and opened right) </ul>
     *                  <li> mBottomSplitter(contains the content widgets docked and opened at the bottom) </ul> </ul> </ul>
     *      <li> mRightDock (the docker bar on the far right) </ul>
     *   <li> <i>mBottomContainer</i> <ul>
     *      <li> mLeftSpacer (an empty frame to create a margin at the left side of the bottom docker)
     *      <li> mBottomDock (the docker bar at the very bottom)
     *      <li> mRightSpacer (an empty frame to create a margin at the right side of the bottom docker) </ul> </ul>
     */
    class ContentLayoutArea : public QWidget
    {
        Q_OBJECT

    public:
        /**
         * Constructor.
         * Already builds up the layout hierarchy.
         *
         * @param parent - The parent widget
         */
        explicit ContentLayoutArea(QWidget* parent = nullptr);

        /**
         * Adds a content widget to the a specified content anchor at a specified position.
         *
         * @param widget - The content widget that should be added to the anchor
         * @param index - The position index this widget should have in the anchor it is moved to
         * @param anchor - The anchor the content widget should be moved to (i.e. the direction of the docker bar it
         *                 is moved to)
         */
        void addContent(ContentWidget* widget, int index, content_anchor anchor);

        /**
         * Not yet implemented.
         *
         * @param widget
         */
        void removeContent(ContentWidget* widget);

        /**
         * Removes all buttons from all docker bars and therefore all widgets.
         * Mark that the widgets are not destroyed but only hidden.
         */
        void clear();

        /**
         * Given the size of the MainWindow this function initializes the splitter sizes of the various splitters in the
         * layout hierarchy. They are chosen to provide reasonable proportions for default usage.
         *
         * @param size
         */
        void initSplitterSize(const QSize& size);

    public Q_SLOTS:
        /**
         * Q_SLOT to update the appearance of the left docker bar. If the docker bar contains no widget it will be hidden.
         */
        void updateLeftDockBar();

        /**
         * Q_SLOT to update the appearance of the right docker bar. If the docker bar contains no widget it will be hidden.
         */
        void updateRightDockBar();

        /**
         * Q_SLOT to update the appearance of the bottom docker bar. If the docker bar contains no widget it will be hidden.
         */
        void updateBottomDockBar();

        /**
         * Q_SLOT to handle that a docker button was dragged out of its bar. All hidden docker bars are temporarily shown
         * while dragging a docker button.
         */
        void handleDragStart();

        /**
         * Q_SLOT to handle the end of the docker button drag event (see handleDragStart). All hidden docker bars that
         * were temporarily shown while dragging a docker button are hidden again.
         */
        void handleDragEnd();

    private:
        QVBoxLayout* mTopLevelLayout;
        QHBoxLayout* mSecondLevelLayout;
        QVBoxLayout* mThirdLevelLayout;
        QHBoxLayout* mFourthLevelLayout;
        QVBoxLayout* mSplitterLayout;
        QHBoxLayout* mCentralLayout;
        QHBoxLayout* mSpacerLayout;

        Splitter* mVerticalSplitter;
        Splitter* mHorizontalSplitter;
        Splitter* mLeftSplitter;
        Splitter* mRightSplitter;
        Splitter* mBottomSplitter;

        DockBar* mLeftDock;
        DockBar* mRightDock;
        DockBar* mBottomDock;

        QWidget* mBottomContainer;
        QFrame* mLeftSpacer;
        QFrame* mRightSpacer;

        SplitterAnchor* mLeftAnchor;
        SplitterAnchor* mRightAnchor;
        SplitterAnchor* mBottomAnchor;

        TabWidget* mTabWidget;

        bool mWasBottomContainerHidden;
    };
}
