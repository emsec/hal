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

    class ContentLayoutArea : public QWidget
    {
        Q_OBJECT

    public:
        explicit ContentLayoutArea(QWidget* parent = 0);
        void addContent(ContentWidget* widget, int index, content_anchor anchor);

        void initSplitterSize(const QSize& size);

    public Q_SLOTS:
        void updateLeftDockBar();
        void updateRightDockBar();
        void updateBottomDockBar();

        //needed to fix a bug where the bottom dockbar does not reappear after dragging every widget out of it
        void handleDragStart();
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
