#include "gui/settings/settings_display.h"

#include <QFrame>
#include <QVBoxLayout>

namespace hal
{
    SettingsDisplay::SettingsDisplay(QWidget* parent)
        : QScrollArea(parent), mContent(new QFrame), mContentLayout(new QVBoxLayout()), mSubContent(new QFrame()), mSubContentLayout(new QVBoxLayout())
    {
        mContent->setFrameStyle(QFrame::NoFrame);

        mContentLayout->setAlignment(Qt::AlignCenter);
        mContentLayout->setContentsMargins(0, 0, 0, 0);
        mContentLayout->setSpacing(0);
        mContent->setLayout(mContentLayout);

        mSubContent->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
        mSubContent->setFrameStyle(QFrame::NoFrame);
        mContentLayout->addWidget(mSubContent);

        mSubContentLayout->setAlignment(Qt::AlignLeft);
        mSubContentLayout->setContentsMargins(0, 0, 0, 0);
        mSubContentLayout->setSpacing(0);
        mSubContent->setLayout(mSubContentLayout);

        setWidget(mContent);
    }

    void SettingsDisplay::addWidget(QWidget* widget)
    {
        //widget->hide();
        //mSubContentLayout->addWidget(widget);
        widget->setParent(0);
        widget->show();
    }
}
