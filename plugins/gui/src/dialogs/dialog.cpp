#include "gui/dialogs/dialog.h"

#include <QGraphicsOpacityEffect>
#include <QPainter>
#include <QParallelAnimationGroup>
#include <QPropertyAnimation>

namespace hal
{
    Dialog::Dialog(QWidget* parent) : QFrame(parent),
        m_effect(new QGraphicsOpacityEffect(this)), // SEEMS TO WORK MORE RELIABLY THAN WINDOWOPACITY
        m_x_animation(new QPropertyAnimation(this, "x_offset", this)),
        m_y_animation(new QPropertyAnimation(this, "y_offset", this)),
        m_opacity_animation(new QPropertyAnimation(m_effect, "opacity", this)),
        m_animation_group(new QParallelAnimationGroup(this))
    {
        hide();

        m_effect->setOpacity(0);
        setGraphicsEffect(m_effect);

        m_x_animation->setDuration(200);
        m_x_animation->setStartValue(0);
        m_x_animation->setEndValue(0);
        m_x_animation->setEasingCurve(QEasingCurve::OutQuad);

        m_y_animation->setDuration(200);
        m_y_animation->setStartValue(-30);
        m_y_animation->setEndValue(0);
        m_y_animation->setEasingCurve(QEasingCurve::OutQuad);

        m_opacity_animation->setDuration(300);
        m_opacity_animation->setStartValue(0);
        m_opacity_animation->setEndValue(1);
        m_opacity_animation->setEasingCurve(QEasingCurve::InQuad);

        m_animation_group->addAnimation(m_x_animation);
        m_animation_group->addAnimation(m_y_animation);
        m_animation_group->addAnimation(m_opacity_animation);

        connect(m_animation_group, &QParallelAnimationGroup::finished, this, &Dialog::fade_in_finished);
    }

    void Dialog::fade_in()
    {
        show();
        m_animation_group->start();

        fade_in_started();
    }

    int Dialog::x_offset() const
    {
        return m_x_offset;
    }

    int Dialog::y_offset() const
    {
        return m_y_offset;
    }

    void Dialog::set_x_offset(const int offset)
    {
        m_x_offset = offset;

        Q_EMIT offset_changed();
    }

    void Dialog::set_y_offset(const int offset)
    {
        m_y_offset = offset;

        Q_EMIT offset_changed();
    }

    void Dialog::fade_in_started()
    {
        // INTENTIONALLY EMPTY
    }

    void Dialog::fade_in_finished()
    {
        // INTENTIONALLY EMPTY
    }
}
