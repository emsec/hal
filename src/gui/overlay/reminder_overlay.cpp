#include "overlay/reminder_overlay.h"
#include "main_window/main_window.h"

#include <QGraphicsOpacityEffect>
#include <QLabel>
#include <QPixmap>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QVBoxLayout>

namespace hal
{
    ReminderOverlay::ReminderOverlay(QWidget* parent) : Overlay(parent), m_layout(new QVBoxLayout()), m_image_label(new QLabel()), m_text_label(new QLabel()), m_button(new QPushButton())
    {
        connect(m_button, &QPushButton::clicked, this, &ReminderOverlay::self_destruct);

        m_layout->setContentsMargins(0, 0, 0, 0);
        m_layout->setSpacing(0);
        m_layout->setAlignment(Qt::AlignHCenter);

        m_image_label->setObjectName("image-label");

        m_text_label->setObjectName("text-label");
        m_text_label->setAlignment(Qt::AlignCenter);
        m_text_label->setText("Please turn on screen capturing now");
        m_text_label->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

        m_button->setObjectName("button");
        m_button->setText("Done");

        setLayout(m_layout);
        m_layout->addWidget(m_image_label);
        m_layout->setAlignment(m_image_label, Qt::AlignHCenter);
        m_layout->addWidget(m_text_label);
        m_layout->addWidget(m_button);
        m_layout->setAlignment(m_button, Qt::AlignRight);

        m_image_label->ensurePolished();
        m_image_label->setPixmap(QPixmap(":/images/hal").scaled(m_image_label->width(), m_image_label->height()));
    }

    void ReminderOverlay::self_destruct()
    {
        m_button->setEnabled(false);

        QGraphicsOpacityEffect* eff1 = new QGraphicsOpacityEffect(this);
        m_image_label->setGraphicsEffect(eff1);
        QPropertyAnimation* a1 = new QPropertyAnimation(eff1, "opacity", this);
        a1->setDuration(400);
        a1->setStartValue(1);
        a1->setEndValue(0);
        a1->setEasingCurve(QEasingCurve::OutCubic);
        a1->start();

        QGraphicsOpacityEffect* eff2 = new QGraphicsOpacityEffect(this);
        m_text_label->setGraphicsEffect(eff2);
        QPropertyAnimation* a2 = new QPropertyAnimation(eff2, "opacity", this);
        a2->setDuration(400);
        a2->setStartValue(1);
        a2->setEndValue(0);
        a2->setEasingCurve(QEasingCurve::OutCubic);
        a2->start();

        QGraphicsOpacityEffect* eff3 = new QGraphicsOpacityEffect(this);
        m_button->setGraphicsEffect(eff3);
        QPropertyAnimation* a3 = new QPropertyAnimation(eff3, "opacity", this);
        a3->setDuration(400);
        a3->setStartValue(1);
        a3->setEndValue(0);
        a3->setEasingCurve(QEasingCurve::OutCubic);
        a3->start();

        QGraphicsOpacityEffect* eff4 = new QGraphicsOpacityEffect(this);
        this->setGraphicsEffect(eff4);
        QPropertyAnimation* a4 = new QPropertyAnimation(eff4, "opacity", this);
        a4->setDuration(800);
        a4->setStartValue(1);
        a4->setEndValue(0);
        a4->setEasingCurve(QEasingCurve::OutCubic);
        connect(a4, &QPropertyAnimation::finished, this, &QObject::deleteLater);
        a4->start();
    }
}
