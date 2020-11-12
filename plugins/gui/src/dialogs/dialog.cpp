#include "gui/dialogs/dialog.h"

#include <QGraphicsOpacityEffect>
#include <QPainter>
#include <QParallelAnimationGroup>
#include <QPropertyAnimation>

namespace hal
{
    Dialog::Dialog(QWidget* parent) : QFrame(parent),
        mEffect(new QGraphicsOpacityEffect(this)), // SEEMS TO WORK MORE RELIABLY THAN WINDOWOPACITY
        mXAnimation(new QPropertyAnimation(this, "xOffset", this)),
        mYAnimation(new QPropertyAnimation(this, "yOffset", this)),
        mOpacityAnimation(new QPropertyAnimation(mEffect, "opacity", this)),
        mAnimationGroup(new QParallelAnimationGroup(this))
    {
        hide();

        mEffect->setOpacity(0);
        setGraphicsEffect(mEffect);

        mXAnimation->setDuration(200);
        mXAnimation->setStartValue(0);
        mXAnimation->setEndValue(0);
        mXAnimation->setEasingCurve(QEasingCurve::OutQuad);

        mYAnimation->setDuration(200);
        mYAnimation->setStartValue(-30);
        mYAnimation->setEndValue(0);
        mYAnimation->setEasingCurve(QEasingCurve::OutQuad);

        mOpacityAnimation->setDuration(300);
        mOpacityAnimation->setStartValue(0);
        mOpacityAnimation->setEndValue(1);
        mOpacityAnimation->setEasingCurve(QEasingCurve::InQuad);

        mAnimationGroup->addAnimation(mXAnimation);
        mAnimationGroup->addAnimation(mYAnimation);
        mAnimationGroup->addAnimation(mOpacityAnimation);

        connect(mAnimationGroup, &QParallelAnimationGroup::finished, this, &Dialog::fadeInFinished);
    }

    void Dialog::fadeIn()
    {
        show();
        mAnimationGroup->start();

        fadeInStarted();
    }

    int Dialog::xOffset() const
    {
        return mXOffset;
    }

    int Dialog::yOffset() const
    {
        return mYOffset;
    }

    void Dialog::setXOffset(const int offset)
    {
        mXOffset = offset;

        Q_EMIT offsetChanged();
    }

    void Dialog::setYOffset(const int offset)
    {
        mYOffset = offset;

        Q_EMIT offsetChanged();
    }

    void Dialog::fadeInStarted()
    {
        // INTENTIONALLY EMPTY
    }

    void Dialog::fadeInFinished()
    {
        // INTENTIONALLY EMPTY
    }
}
