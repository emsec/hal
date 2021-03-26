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

#include <QScrollArea>
#include <QHash>
#include <QList>

class QFrame;
class QVBoxLayout;

namespace hal
{
    class ExpandingListButton;
    class ExpandingListItem;

    class ExpandingListGroup : public QList<ExpandingListItem*>
    {
    public:
        bool mCollapsed;
        ExpandingListGroup() : mCollapsed(false) {;}
        void toggleCollapsed(ExpandingListButton* exceptSelected);
    };

    class ExpandingListWidget : public QScrollArea
    {
        Q_OBJECT

    public:
        ExpandingListWidget(QWidget* parent = 0);

        //    bool eventFilter(QObject* object, QEvent* event) override;

        void appendItem(ExpandingListButton* button, const QString& groupName = QString());
        void selectButton(ExpandingListButton* button);
        void selectFirstItem();

        void repolish();
        bool hasGroup(const QString& groupName) const;

    Q_SIGNALS:
        void buttonSelected(ExpandingListButton* button);

    public Q_SLOTS:
        void handleClicked();

    private:
        QFrame* mContent;
        QVBoxLayout* mContentLayout;
        QFrame* mSpacer;

        QMap<QString,ExpandingListItem*> mItemMap;

        ExpandingListButton* mSelectedButton;

        int mItemWidth;
        int mOffset;
        QHash<QString,ExpandingListGroup> mButtonGroup;
    };
}
