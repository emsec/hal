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

#include <QToolButton>

namespace hal
{
    class ContentWidget;

    /**
     * @ingroup docking
     * This enum represents the orientation of a DockButton. A DockButton is either
     * placed in horizontal or vertical DockBar. Depending on this orientation the width, height
     * and text is displayed accordingly.
     */
    enum class button_orientation
    {
        horizontal    = 0,
        vertical_up   = 1,
        vertical_down = 2
    };

    /**
     * @ingroup docking
     * @brief A button of in the DockerBar
     *
     * The DockButton is placed and displayed in a DockBar to determine the position and appearance
     * of a content widget. It computes and holds necessary information for the docking system such as
     * its size, the associated content widget as well as his hidden/show status.
     */
    class DockButton : public QToolButton
    {
        Q_OBJECT
        Q_PROPERTY(QColor textColor READ textColor WRITE setTextColor)
        Q_PROPERTY(QColor checkedColor READ checkedColor WRITE setCheckedColor)
        Q_PROPERTY(QColor hoverColor READ hoverColor WRITE setHoverColor)

    public:
        /**
         * The constructor.
         *
         * @param widget - The associated content widget that is displayed in the corresponding area of the dockbutton.
         * @param orientation - The dockbutton's orientation. This is set depending if the dockbar its placed in is horizontal or vertical.
         * @param eventFilter - The eventfilter to be installed on the dockbutton. Mainly used by the dockbar to set a button as the current dragbutton.
         * @param parent -  The dockbutton's parent.
         */
        DockButton(ContentWidget* widget, button_orientation orientation, QObject* eventFilter, QWidget* parent);

        /**
         * Overwritten qt function.
         *
         * @param event - The paint event.
         */
        void paintEvent(QPaintEvent* event) override;

        /** @name Q_PROPERTY READ Functions
          */
        ///@{
        /**
         * Returns the text color
         */
        QColor textColor();

        /**
         * Returns the checked color
         */
        QColor checkedColor();

        /**
         * Returns the hover color
         */
        QColor hoverColor();
        ///@}

        /** @name Q_PROPERTY WRITE Functions
          */
        ///@{

        /**
         * Set the text color
         * @param color -  The new color.
         */
        void setTextColor(const QColor& color);

        /**
         * Set the checked color
         * @param color -  The new color.
         */
        void setCheckedColor(const QColor& color);

        /**
         * Set the hover color
         * @param color -  The new color.
         */
        void setHoverColor(const QColor& color);
        ///@}

        /**
         * Calculates its size and sets the height and width as fixed values.
         */
        void adjustSize();

        /**
         * Get the relative width of the button. This value is used in the adjustSize() calculation.
         *
         * @return The relative width.
         */
        int relativeWidth();

        /**
         * Get the associated content widget.
         *
         * @return The associated content widget.
         */
        ContentWidget* widget();

        /**
         * Get the visible status of the associated content widget wether it is currently shown or hidden.
         * Does the same as QWidget::isHidden() when combined with the overriden hide() method.
         *
         * @return The visible status of the widget.
         */
        bool hidden();

        /**
         * Get the available status of the dockbutton. This status is set to false when the dockbutton is dragged.
         *
         * @return The available status of the dockbutton.
         */
        bool available();

        /**
         * Sets the available status of the button. This status is to be set to false at the start of the buttondragging and
         * set to true at the end of it.
         *
         * @param available - The new available status of the widget.
         */
        void setAvailable(bool available);

        /**
         * Sets the relative height.
         *
         * @param height - The ’new’ height.
         */
        void setRelativeHeight(int height);

        //overshadowed functions

        /**
         * Overrides the  QWidget's hide method. Does the same as QWidget::hide().
         */
        void hide();

        /**
         * Overrides the QWidgets show method. Does the same as QWidget::show().
         */
        void show();

    public Q_SLOTS:

        /**
         * The main purpose of this function is to be connected to the buttons's own clicked signal.
         * When the button is checked and therefore mChecked = true (remember, it is a toolbutton), the
         * associated widget is opened and therefore visible. Otherwise it is closed.
         *
         * @param mChecked - The boolean wether the dockbutton is checked or not checked.
         */
        void handleClicked(bool mChecked);

    private:
        ContentWidget* mWidget;
        button_orientation mOrientation;
        int mWidth;
        int mHeight;
        int mRelativeWidth;
        int mRelativeHeight;
        int mIconSize;
        int mTextOffset;
        int mWidthPadding;
        int mHeightPadding;
        bool mHidden;
        bool mAvailable = true;

        QColor mTextColor;
        QColor mCheckedColor;
        QColor mHoverColor;
    };
}
