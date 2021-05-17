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

class QHBoxLayout;
class QLabel;

namespace hal
{
    class ExpandingListWidget;

    /**
     * @ingroup settings
     * @brief One button of the ExpandingListWidget.
     *
     * An ExpandingListButton can be added to an ExpandingListWidget and is then
     * displayed as an item. Its primary feature is to be either selected or not.
     */
    class ExpandingListButton : public QFrame
    {
        Q_OBJECT
        Q_PROPERTY(int level READ level)
        Q_PROPERTY(bool hover READ hover)
        Q_PROPERTY(bool selected READ selected)
        Q_PROPERTY(QString iconStyle READ iconStyle WRITE setIconStyle)

    public:
        /**
         * The constructor.
         *
         * @param levl - The level this button is at
         * @param parent - The button's parent.
         */
        ExpandingListButton(int levl, QWidget* parent = 0);

        /**
         * Overwritten qt function that is triggered when the mouse enters the button.
         * This function is used to set the hover state (which is used to style the
         * button in the stylesheet(s)).
         *
         * @param event - The associated event.
         */
        void enterEvent(QEvent* event) override;

        /**
         * Overwritten qt function that is triggered when the mouse leaves the button.
         * This function is used to unset the hover state (which is used to style the
         * button in the stylesheet(s)).
         *
         * @param event - The associated event.
         */
        void leaveEvent(QEvent* event) override;

        /**
         * Overwritten qt function that is triggerd when the mouse is pressed inside the
         * button's boundaries. It emitts the clicked signal so that the ExpandingListWidget
         * can catch it (or any other widget that might want to know it)
         *
         * @param event - The associated event.
         */
        void mousePressEvent(QMouseEvent* event) override;

        /** @name Q_PROPERTY READ Functions
          */
        ///@{

        /**
         * Gets the level the this ExpandingListButton is at.
         *
         * @returns the level
         */
	    int level() const;

        /**
         * Get the button's hover state.
         *
         * @return True if the mouse is currently over the button. False otherwise.
         */
        bool hover() const;

        /**
         * Get the button's selected state.
         *
         * @return True if the button is currently selected. False otherwise.
         */
        bool selected() const;

        /**
         * Get the icon stlye of the button.
         *
         * @return The button's icon style.
         */
        QString iconStyle() const;
        ///@}


        /** @name Q_PROPERTY WRITE Functions
          */
        ///@{

        /**
         * Set or unset the button's selected state.
         *
         * @param selected - True to set the state, false to unset the state.
         */
        void setSelected(bool selected);

        /**
         * Sets the button's icon style.
         *
         * @param style - The style.
         */
        void setIconStyle(const QString& style);

        /**
         * Sets the icon to a default one with the given label text.
         *
         * @param text - The label text
         */
	    void setDefaultIcon(const QString& text);

        /**
         * Sets the path of the button's icon.
         *
         * @param path - The icon's path.
         */
        void setIconPath(const QString& path);
        ///@}

        /**
         * Sets the button's text.
         *
         * @param text - The soon to be displayed Text.
         */
        void setText(const QString& text);

        /**
         * Gets the text this ExpandingListButton displays
         *
         * @returns the ExpandingListButton%s text
         */
        QString text() const;

        /**
         * Applies the style that is currently set through the (globally set) stylesheet.
         */
        void repolish();

    Q_SIGNALS:

        /**
         * Q_SIGNAL that is emitted when the button is clicked.
         */
        void clicked();

    private:
        QHBoxLayout* mLayout;
        QFrame* mLeftBorder;
        QLabel* mIconLabel;
        QLabel* mTextLabel;
        QFrame* mRightBorder;

        int mLevel;
        bool mHover;
        bool mSelected;
        QString mIconStyle;
        QString mIconPath;

        static QHash<QString,QString> sIconMap;
        static QHash<QString,QString> defaultIconMap();
    };
}
