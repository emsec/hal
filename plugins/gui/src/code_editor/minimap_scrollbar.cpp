#include "gui/code_editor/minimap_scrollbar.h"

#include <QFrame>
#include <QMouseEvent>
#include <QPainter>
#include <QScrollBar>

#include <cmath>
#include <math.h>

namespace hal
{
    MinimapScrollbar::MinimapScrollbar(QWidget* parent) : QWidget(parent),
          mSlider(new QFrame(this)),
          mMinimum(0),
          mMaximum(100),
          mValue(0),
          mHandleLength(0),
          mHandlePosition(0),
          mMousePressed(false),
          mDragOffset(0),
          mScrollbar(nullptr)
    {
        mSlider->setObjectName("slider");
        mSlider->show();
    }

    int MinimapScrollbar::minimum() const
    {
        return mMinimum;
    }

    int MinimapScrollbar::maximum() const
    {
        return mMaximum;
    }

    int MinimapScrollbar::value() const
    {
        return mValue;
    }

    int MinimapScrollbar::sliderHeight() const
    {
        return mSlider->height();
    }

    int MinimapScrollbar::sliderPosition() const
    {
        return mSlider->pos().y();
    }

    //void MinimapScrollbar::set_minimum(const int minimum)
    //{
    //    mMinimum = minimum;

    //    if (mValue < minimum)
    //        mValue = minimum;

    //    adjustSliderToValue();
    //}

    //void MinimapScrollbar::set_maximum(const int maximum)
    //{
    //    mMaximum = maximum;

    //    if (mValue > maximum)
    //        mValue = maximum;

    //    adjustSliderToValue();
    //}

    void MinimapScrollbar::setRange(const int minimum, const int maximum)
    {
        mMinimum = minimum;
        mMaximum = maximum;

        // MAYBE REDUNDANT, LEFT FOR COMPLETENESS
        if (mValue < minimum)
            mValue = minimum;

        else if (mValue > maximum)
            mValue = maximum;

        adjustSliderToValue();
    }

    void MinimapScrollbar::setValue(const int value)
    {
        if (mValue != value)
        {
            if (value < mMinimum)
                mValue = mMinimum;

            else if (value > mMaximum)
                mValue = mMaximum;

            else
                mValue = value;

            adjustSliderToValue();
        }
    }

    void MinimapScrollbar::setSliderHeight(int height)
    {
        mSlider->resize(mSlider->width(), height);
        adjustSliderToValue();
    }

    void MinimapScrollbar::setScrollbar(QScrollBar* scrollbar)
    {
        mScrollbar = scrollbar;
    }

    void MinimapScrollbar::paintEvent(QPaintEvent* event)
    {
        Q_UNUSED(event);
        // LEFT EMPTY INTENTIONALLY FOR TRANSPARENT BACKGROUND
    }

    void MinimapScrollbar::resizeEvent(QResizeEvent* event)
    {
        //QWidget::resizeEvent(event); // UNNECESSARY ?
        Q_UNUSED(event)

        mSlider->resize(width(), mSlider->height());
        adjustSliderToValue();
    }

    void MinimapScrollbar::mousePressEvent(QMouseEvent* event)
    {
        if (mSlider->geometry().contains(event->pos()))
        {
            mMousePressed = true;
            mDragOffset   = event->pos().y() - mSlider->geometry().top();
            event->accept();
            return;
        }
        else
            event->ignore();
    }

    void MinimapScrollbar::mouseMoveEvent(QMouseEvent* event)
    {
        if (!mScrollbar)
            return;

        if (mMousePressed)
        {
            event->accept();    // UNSURE

            int y = event->pos().y() - mDragOffset;

            if (y < 0)
                mScrollbar->setValue(mMinimum);

            else if (y > height() - mSlider->height())
                mScrollbar->setValue(mMaximum);

            else
            {
                int available_pixel = height() - mSlider->height();
                if (available_pixel <= 0)
                    return;

                int value_range = mMaximum - mMinimum;
                if (value_range <= 0)
                    return;

                double pixel_per_value = double(available_pixel) / double(value_range);
                mScrollbar->setValue(std::round(y / pixel_per_value));
            }
        }
    }

    void MinimapScrollbar::mouseReleaseEvent(QMouseEvent* event)
    {
        mMousePressed = false;
        event->accept();
    }

    void MinimapScrollbar::leaveEvent(QEvent* event)
    {
        mMousePressed = false;
        QWidget::leaveEvent(event);
    }

    void MinimapScrollbar::adjustSliderToValue()
    {
        int available_pixel = height() - mSlider->height();
        if (available_pixel <= 0)
            return;

        int value_range = mMaximum - mMinimum;
        if (value_range <= 0)
            return;

        double pixel_per_value = double(available_pixel) / double(value_range);
        int y                  = std::round(pixel_per_value * mValue);

        int max = height() - mSlider->height();
        if (y > max)
            y = max;

        mSlider->move(0, y);
    }
}
