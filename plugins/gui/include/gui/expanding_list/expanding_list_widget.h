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

#include <QScrollArea>
#include <QHash>
#include <QMap>
#include <QString>
#include <QList>

class QFrame;
class QVBoxLayout;

namespace hal
{
    class ExpandingListButton;
    class ExpandingListItem;

    /**
     * @ingroup settings
     * @brief A helper class to group related list items (buttons).
     */
    class ExpandingListGroup : public QList<ExpandingListItem*>
    {
    public:
        bool mCollapsed;
        /**
         * The constructor.
         */
        ExpandingListGroup() : mCollapsed(false) {;}

        /**
         * Expands the given list button if it is collapsed. Collapses it otherwise.
         *
         * @param exceptSelected - The button to toggle.
         */
        void toggleCollapsed(ExpandingListButton* exceptSelected);
    };

    /**
     * @ingroup settings
     * @brief A selection menu with a hierarchic structure.
     *
     * The ExpandingListWidget class is a list in which each top-level-item can be expanded
     * to display other items below it (e.g. a flattened tree with 2 levels). This widget
     * primarily functions as a navigation or section menu (as can be seen in the settings).
     * Each of the list's item can either be selected or not, and only one item can be
     * selected at a time (thus showing which section is currently displayed).
     */
    class ExpandingListWidget : public QScrollArea
    {
        Q_OBJECT

    public:

        /**
         * The constructor.
         *
         * @param parent - The widget's parent.
         */
        ExpandingListWidget(QWidget* parent = nullptr);

        /**
         * Appends a button to the list. If an optional parent button is specified, the button
         * is added as a child to the parent and then hidden from the user. When the user clicks
         * on the top-level parent button, all its children appear right below it.
         *
         * @param button - The button to add to the list.
         * @param groupName - Name of group to which the button is added (optional).
         */
        void appendItem(ExpandingListButton* button, const QString& groupName = QString());

        /**
         * Selects a specific button within the list. It collapses or expands all necessary
         * items depending on the situation (which button was selected before).
         *
         * @param button - The button to select.
         */
        void selectButton(ExpandingListButton* button);

        /**
         * Select the first ExpandingListItem of this widget.
         */
        void selectFirstItem();

        /**
         * Selects a specific item within the list. An item is a wrapper for a top-level button
         * so that children can be added (a normal button can not have children).
         *
         * @param index - The index of the item to select.
         */
        void selectItem(int index);

        /**
         * Applies the currently set style (e.g. stylesheet) to itself and all of its items.
         */
        void repolish();

        /**
         * Returns <b>true</b> iff this ExpandingListWidget has a ExpandingListGroup with the specified name.
         *
         * @param groupName - The name of the ExpandingListGroup to search for
         * @returns <b>true</b> iff the the group exists
         */
        bool hasGroup(const QString& groupName) const;

    Q_SIGNALS:
        /**
         * Q_SIGNAL that is emitted when a new button is selected.
         *
         * @param button - The newly selected button.
         */
        void buttonSelected(ExpandingListButton* button);

    public Q_SLOTS:

        /**
         * Handler method to catch the clicked signal of the buttons.
         */
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
