#include "notifications/notification.h"
#include "graphics_effects/shadow_effect.h"
#include "gui_globals.h"

#include <QApplication>
#include <QGraphicsEffect>
#include <QMouseEvent>
#include <QPropertyAnimation>

notification::notification(QWidget* parent) : QFrame(parent)
{
    hide();
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::WindowDoesNotAcceptFocus | Qt::ToolTip);
    setAttribute(Qt::WA_ShowWithoutActivating);
    //setAttribute(Qt::WA_TranslucentBackground);
    setFixedSize(280, 130);
    //setGraphicsEffect(new shadow_effect(this));

    QGraphicsOpacityEffect* effect = new QGraphicsOpacityEffect(this);
    effect->setOpacity(0);
    setGraphicsEffect(effect);
    m_animation = new QPropertyAnimation(effect, "opacity", this);
    //setWindowOpacity(0);
    //m_animation = new QPropertyAnimation(this, "opacity", this);
    m_animation->setDuration(1000);
    m_animation->setStartValue(0);
    m_animation->setEndValue(1);
    m_animation->setEasingCurve(QEasingCurve::Linear);
}

qreal notification::get_opacity()
{
    return windowOpacity();
}

void notification::set_opacity(qreal opacity)
{
    setWindowOpacity(opacity);
}

void notification::mousePressEvent(QMouseEvent* event)
{
    Q_UNUSED(event)

    event->accept();
    Q_EMIT clicked();
    //fade_out();
    cleanup();
}

void notification::fade_in()
{
    show();
    QApplication::beep();
    m_animation->start();
}

void notification::fade_out()
{
    m_animation->setDirection(QPropertyAnimation::Backward);
    connect(m_animation, &QPropertyAnimation::finished, this, &notification::cleanup);

    if (m_animation->state() != QPropertyAnimation::Running)
        m_animation->start();
}

void notification::cleanup()
{
    g_notification_manager->remove(this);
}
