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

#include <QWidget>

class QFrame;
class QScrollBar;

namespace hal
{

    /**
     * @ingroup python-editor
     * @brief The scrollbar of the CodeEditorMinimap.
     */
    class MinimapScrollbar : public QWidget
    {
        Q_OBJECT

    public:
        /**
         * Constructor.
         *
         * @param parent - The parent widget
         */
        MinimapScrollbar(QWidget* parent = nullptr);

        /**
         * Gets the minimal slider position.
         *
         * @returns the minimal slider position
         */
        int minimum() const;

        /**
         * Gets the maximal slider position.
         *
         * @returns the minimal slider position
         */
        int maximum() const;

        /**
         * Gets the current slider position value (i.e. the line number the top of the slider is on).
         *
         * @returns the current slide position value
         */
        int value() const;

        /**
         * Gets the height of the slider.
         *
         * @returns the height of the slider
         */
        int sliderHeight() const;

        /**
         * Gets the position of the slider.
         *
         * @returns the position of the slider
         */
        int sliderPosition() const;

        /**
         * Sets the possible range of the slider, i.e. the minimal and maximal vertical position the slider may have.
         *
         * @param minimum - The minimal position value the slider may have
         * @param maximum - The maximal position value the slider may have
         */
        void setRange(const int minimum, const int maximum);

        /**
         * Sets the position value (i.e. line number) of the slider.
         *
         * @param value - The new value the slider should set to
         */
        void setValue(const int value);

        /**
         * Adjusts the height of the slider.
         *
         * @param height - The new height of the slider
         */
        void setSliderHeight(const int height);

        /**
         * Sets the underlying QScrollBar for the minimap scrollbar. Since the CodeEditor and the CodeEditorMinimap have
         * to stay in sync, they share on Scrollbar to work on.
         *
         * @param scrollbar - The scrollbar to work on
         */
        void setScrollbar(QScrollBar* scrollbar);

    protected:
        /**
         * Handles the paint event for the minimap scrollbar.
         *
         * @param event - The paint event
         */
        void paintEvent(QPaintEvent* event) override;

        /**
         * Handles the resize event for the minimap scrollbar. Used to adjust the slider height if the height of the
         * minimap/code editor changes.
         *
         * @param event - The resize event
         */
        void resizeEvent(QResizeEvent* event) override;

        /**
         * Handles the mouse press event. Used (with the other mouse events) to enable the movement of the slider
         * by the mouse.
         *
         * @param event - The mouse press event.
         */
        void mousePressEvent(QMouseEvent* event) override;

        /**
         * Handles the mouse press event. Used (with the other mouse events) to enable the movement of the slider
         * by the mouse.
         *
         * @param event - The mouse event.
         */
        void mouseMoveEvent(QMouseEvent* event) override;

        /**
         * Handles the mouse release event. Used (with the other mouse events) to enable the movement of the slider
         * by the mouse.
         *
         * @param event - The mouse release event.
         */
        void mouseReleaseEvent(QMouseEvent* event) override;

        /**
         * Handles the event that the cursor leaves the widget.
         *
         * @param event - The leave event
         */
        void leaveEvent(QEvent* event) override;

    private:
        void adjustSliderToValue();

        QFrame* mSlider;

        int mMinimum;
        int mMaximum;
        int mValue;

        int mHandleLength;
        int mHandlePosition;

        bool mMousePressed;
        int mDragOffset;

        QScrollBar* mScrollbar;
    };
}
