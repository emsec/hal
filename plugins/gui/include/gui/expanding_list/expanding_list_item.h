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
#include <QList>
#include <QResizeEvent>

class QPropertyAnimation;

namespace hal
{
    class ExpandingListButton;

    class ExpandingListItem : public QFrame
    {
        Q_OBJECT
        Q_PROPERTY(bool expanded READ expanded)
        Q_PROPERTY(int fixedHeight READ fixedHeight WRITE setFixedHeight)

    public:
        ExpandingListItem(ExpandingListButton* parentButton, QWidget* parent = 0);

        virtual QSize minimumSizeHint() const Q_DECL_OVERRIDE;
        virtual QSize sizeHint() const Q_DECL_OVERRIDE;
        virtual void resizeEvent(QResizeEvent* event) Q_DECL_OVERRIDE;

        bool expanded();
        int fixedHeight();
        bool contains(ExpandingListButton* button);
        ExpandingListButton* parentButton();
        void appendChildButton(ExpandingListButton* button);

        void repolish();

        void collapse();
        void expand();

        void setExpanded(bool expanded);
        void setFixedHeight(int height);

    private:
        ExpandingListButton* mParentButton;
        QList<ExpandingListButton*> mChildButtons;

        int mCollapsedHeight;
        int mExpandedHeight;

        QPropertyAnimation* mAnimation;

        bool mExpanded;
        int mFixedHeight;
    };
}
