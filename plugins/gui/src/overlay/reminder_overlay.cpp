#include "gui/overlay/reminder_overlay.h"
#include "gui/main_window/main_window.h"

#include <QGraphicsOpacityEffect>
#include <QLabel>
#include <QPixmap>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QVBoxLayout>

namespace hal
{
    ReminderOverlay::ReminderOverlay(QWidget* parent) : Overlay(parent), mLayout(new QVBoxLayout()), mImageLabel(new QLabel()), mTextLabel(new QLabel()), mButton(new QPushButton())
    {
        connect(mButton, &QPushButton::clicked, this, &ReminderOverlay::selfDestruct);

        mLayout->setContentsMargins(0, 0, 0, 0);
        mLayout->setSpacing(0);
        mLayout->setAlignment(Qt::AlignHCenter);

        mImageLabel->setObjectName("image-label");

        mTextLabel->setObjectName("text-label");
        mTextLabel->setAlignment(Qt::AlignCenter);
        mTextLabel->setText("Please turn on screen capturing now");
        mTextLabel->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

        mButton->setObjectName("button");
        mButton->setText("Done");

        setLayout(mLayout);
        mLayout->addWidget(mImageLabel);
        mLayout->setAlignment(mImageLabel, Qt::AlignHCenter);
        mLayout->addWidget(mTextLabel);
        mLayout->addWidget(mButton);
        mLayout->setAlignment(mButton, Qt::AlignRight);

        mImageLabel->ensurePolished();
        mImageLabel->setPixmap(QPixmap(":/images/hal").scaled(mImageLabel->width(), mImageLabel->height()));
    }

    void ReminderOverlay::selfDestruct()
    {
        mButton->setEnabled(false);

        QGraphicsOpacityEffect* eff1 = new QGraphicsOpacityEffect(this);
        mImageLabel->setGraphicsEffect(eff1);
        QPropertyAnimation* a1 = new QPropertyAnimation(eff1, "opacity", this);
        a1->setDuration(400);
        a1->setStartValue(1);
        a1->setEndValue(0);
        a1->setEasingCurve(QEasingCurve::OutCubic);
        a1->start();

        QGraphicsOpacityEffect* eff2 = new QGraphicsOpacityEffect(this);
        mTextLabel->setGraphicsEffect(eff2);
        QPropertyAnimation* a2 = new QPropertyAnimation(eff2, "opacity", this);
        a2->setDuration(400);
        a2->setStartValue(1);
        a2->setEndValue(0);
        a2->setEasingCurve(QEasingCurve::OutCubic);
        a2->start();

        QGraphicsOpacityEffect* eff3 = new QGraphicsOpacityEffect(this);
        mButton->setGraphicsEffect(eff3);
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
