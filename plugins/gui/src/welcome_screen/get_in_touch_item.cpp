#include "gui/welcome_screen/get_in_touch_item.h"

#include "gui/gui_utils/graphics.h"

#include <QEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QPropertyAnimation>
#include <QStyle>
#include <QVBoxLayout>

namespace hal
{
    GetInTouchItem::GetInTouchItem(const QString& title, const QString& mDescription, QWidget* parent)
        : QFrame(parent), mHorizontalLayout(new QHBoxLayout()), mIconLabel(new QLabel()), mVerticalLayout(new QVBoxLayout()), mTitleLabel(new QLabel()), mDescriptionLabel(new QLabel()),
          mAnimation(new QPropertyAnimation(this)), mHover(false)
    {
        mHorizontalLayout->setContentsMargins(0, 0, 0, 0);
        mHorizontalLayout->setSpacing(0);

        mIconLabel->setObjectName("icon-label");
        mIconLabel->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

        mVerticalLayout->setContentsMargins(0, 0, 0, 0);
        mVerticalLayout->setSpacing(0);

        mTitleLabel->setObjectName("title-label");
        mTitleLabel->setText(title);

        mDescriptionLabel->setObjectName("mDescription-label");
        mDescriptionLabel->setText(mDescription);
        mDescriptionLabel->setWordWrap(true);

        setLayout(mHorizontalLayout);
        mHorizontalLayout->addWidget(mIconLabel);
        mHorizontalLayout->setAlignment(mIconLabel, Qt::AlignTop);
        mHorizontalLayout->addLayout(mVerticalLayout);
        mVerticalLayout->addWidget(mTitleLabel);
        mVerticalLayout->addWidget(mDescriptionLabel);

        ensurePolished();
        repolish();
    }

    void GetInTouchItem::enterEvent(QEvent* event)
    {
        Q_UNUSED(event)

        mHover = true;
        repolish();
    }

    void GetInTouchItem::leaveEvent(QEvent* event)
    {
        Q_UNUSED(event)

        mHover = false;
        repolish();
    }

    void GetInTouchItem::mousePressEvent(QMouseEvent* event)
    {
        Q_UNUSED(event)

        event->accept();
        Q_EMIT clicked();
    }

    void GetInTouchItem::repolish()
    {
        QStyle* s = style();

        s->unpolish(this);
        s->polish(this);

        s->unpolish(mIconLabel);
        s->polish(mIconLabel);

        s->unpolish(mTitleLabel);
        s->polish(mTitleLabel);

        s->unpolish(mDescriptionLabel);
        s->polish(mDescriptionLabel);

        if (!mIconPath.isEmpty())
            mIconLabel->setPixmap(gui_utility::getStyledSvgIcon(mIconStyle, mIconPath).pixmap(QSize(17, 17)));
    }

    bool GetInTouchItem::hover()
    {
        return mHover;
    }

    QString GetInTouchItem::iconPath()
    {
        return mIconPath;
    }

    QString GetInTouchItem::iconStyle()
    {
        return mIconStyle;
    }

    void GetInTouchItem::setHoverActive(bool active)
    {
        mHover = active;
    }

    void GetInTouchItem::setIconPath(const QString& path)
    {
        mIconPath = path;
    }

    void GetInTouchItem::setIconStyle(const QString& style)
    {
        mIconStyle = style;
    }
}
