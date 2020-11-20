#include "gui/notifications/notification.h"
#include "gui/graphics_effects/shadow_effect.h"
#include "gui/gui_globals.h"

#include <QApplication>
#include <QGraphicsEffect>
#include <QMouseEvent>
#include <QPropertyAnimation>

namespace hal
{
    Notification::Notification(QWidget* parent) : QFrame(parent)
    {
        hide();
        setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::WindowDoesNotAcceptFocus | Qt::ToolTip);
        setAttribute(Qt::WA_ShowWithoutActivating);
        //setAttribute(Qt::WA_TranslucentBackground);
        setFixedSize(280, 130);
        //setGraphicsEffect(new ShadowEffect(this));

        QGraphicsOpacityEffect* effect = new QGraphicsOpacityEffect(this);
        effect->setOpacity(0);
        setGraphicsEffect(effect);
        mAnimation = new QPropertyAnimation(effect, "opacity", this);
        //setWindowOpacity(0);
        //mAnimation = new QPropertyAnimation(this, "opacity", this);
        mAnimation->setDuration(1000);
        mAnimation->setStartValue(0);
        mAnimation->setEndValue(1);
        mAnimation->setEasingCurve(QEasingCurve::Linear);
    }

    qreal Notification::getOpacity()
    {
        return windowOpacity();
    }

    void Notification::setOpacity(qreal opacity)
    {
        setWindowOpacity(opacity);
    }

    void Notification::mousePressEvent(QMouseEvent* event)
    {
        Q_UNUSED(event)

        event->accept();
        Q_EMIT clicked();
        //fadeOut();
        cleanup();
    }

    void Notification::fadeIn()
    {
        show();
        QApplication::beep();
        mAnimation->start();
    }

    void Notification::fadeOut()
    {
        mAnimation->setDirection(QPropertyAnimation::Backward);
        connect(mAnimation, &QPropertyAnimation::finished, this, &Notification::cleanup);

        if (mAnimation->state() != QPropertyAnimation::Running)
            mAnimation->start();
    }

    void Notification::cleanup()
    {
        gNotificationManager->remove(this);
    }
}
