#include "gui/drop_marker/drop_marker.h"

#include <QPropertyAnimation>

namespace hal
{
    DropMarker::DropMarker(Qt::Orientation orientation, QWidget* parent) : QFrame(parent), mAnimation(new QPropertyAnimation(this)), mOrientation(orientation), mFixedWidth(0), mFixedHeight(0)
    {
        mAnimation->setTargetObject(this);
        mAnimation->setDuration(200);
        mAnimation->setStartValue(0);
        mAnimation->setEndValue(0);

        if (mOrientation == Qt::Horizontal)
            mAnimation->setPropertyName("fixedWidth");
        if (mOrientation == Qt::Vertical)
            mAnimation->setPropertyName("fixedHeight");
    }

    int DropMarker::fixedWidth()
    {
        return mFixedWidth;
    }

    int DropMarker::fixedHeight()
    {
        return mFixedHeight;
    }

    void DropMarker::setFixedWidth(int width)
    {
        mFixedWidth = width;

        setMinimumWidth(width);
        setMaximumWidth(width);
    }

    void DropMarker::setFixedHeight(int height)
    {
        mFixedHeight = height;

        setMinimumHeight(height);
        setMaximumHeight(height);
    }

    void DropMarker::setEndValue(int value)
    {
        mAnimation->setEndValue(value);
    }

    void DropMarker::expand()
    {
        mAnimation->setDirection(QPropertyAnimation::Forward);

        if (mAnimation->state() == QAbstractAnimation::Stopped)
            mAnimation->start();
    }

    void DropMarker::collapse()
    {
        mAnimation->setDirection(QPropertyAnimation::Backward);

        if (mAnimation->state() == QAbstractAnimation::Stopped)
            mAnimation->start();
    }

    void DropMarker::reset()
    {
        if (mAnimation->state() == QAbstractAnimation::Running)
            mAnimation->stop();

        if (mOrientation == Qt::Horizontal)
            setFixedWidth(0);
        if (mOrientation == Qt::Vertical)
            setFixedHeight(0);
    }
}
