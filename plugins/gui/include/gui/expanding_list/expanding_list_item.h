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

#include <QFrame>
#include <QList>
#include <QResizeEvent>

class QPropertyAnimation;

namespace hal
{
    class ExpandingListButton;

    /**
     * @ingroup settings
     * @brief Wrapper for ExpandingListButton%s.
     *
     * The ExpandingListItem class is a wrapper class for the ExpandingListButton%s on the first
     * level of the ExpandingListWidget. It adds the ability to add child buttons -that can either
     * be hidden or expanded- to the wrapped object.
     */
    class ExpandingListItem : public QFrame
    {
        Q_OBJECT
        Q_PROPERTY(bool expanded READ expanded)
        Q_PROPERTY(int fixedHeight READ fixedHeight WRITE setFixedHeight)

    public:
        /**
         * The constructor.
         *
         * @param but - The button to wrap.
         * @param parent - The parent widget.
         */
        ExpandingListItem(ExpandingListButton* but, QWidget* parent = nullptr);

        /**
         * Overwritten Qt-function. Returns the recommended minimum size for the item.
         *
         * @return The recommended minimum size.
         */
        virtual QSize minimumSizeHint() const override;

        /**
         * Overwritten Qt-function. Returns the preferred  size for the item.
         *
         * @return The preferred size.
         */
        virtual QSize sizeHint() const override;

        /**
         * Overwritten Qt-functions. This function is called when a resize event occurs.
         * It resizes this item's wrapped button as well as all its children.
         *
         * @param event - The resize event that contains the event's information.
         */
        virtual void resizeEvent(QResizeEvent* event) override;

        /** @name Q_PROPERTY READ Functions
          */
        ///@{

        /**
         * Get the expanded property of this items.
         *
         * @return True if the item's children are expanded. False otherwise.
         */
        bool expanded() const;

        /**
         * Get the the fixed height property of this item. It is initially set to the
         * minimum height of the wrapped button.
         *
         * @return The item's fixed height.
         */
        int fixedHeight() const;
        ///@}

        /**
         * Gets the ExpandingListButton that belongs to this ExpandingListItem
         *
         * @returns the ExpandingListButton
         */
        ExpandingListButton* button() const;

        /**
         * Applies the style that is currently set through the (globally set) stylesheet
         * to itself and all its child buttons. It also calculates the potentially new
         * expanded and collapsed height and sets these values in its animation object.
         */
        void repolish();

        /**
         * Disables all child buttons and starts the animation to collapse them all
         * (so that only the wrapped button is displayed).
         */
        void collapse();

        /**
         * Enables all child buttons and starts the animation to expand them all
         * (so that these are displayed below the wrapped button).
         */
        void expand();

        /** @name Q_PROPERTY WRITE Function
          */
        ///@{

        /**
         * Sets the fixed height property of this item. This function is called in the
         * repolish function and the fixed height is set to the height this item has when
         * it is collapsed.
         *
         * @param height - The new fixed height.
         */
        void setFixedHeight(int height);
        ///@}

    private:
        ExpandingListButton* mButton;

        int mCollapsedHeight;
        int mExpandedHeight;

        bool mExpanded;
        int mFixedHeight;
    };
}
