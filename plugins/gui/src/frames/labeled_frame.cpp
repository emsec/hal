#include "gui/frames/labeled_frame.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>

namespace hal
{
    LabeledFrame::LabeledFrame(QWidget* parent)
        : QFrame(parent), mLayout(new QVBoxLayout), mHeader(new QFrame()), mHeaderLayout(new QHBoxLayout()), mLeftSpacer(new QFrame()), mLabel(new QLabel()), mRightSpacer(new QFrame())
    {
        setLayout(mLayout);
        mLayout->setContentsMargins(0, 0, 0, 0);
        mLayout->setSpacing(0);
        mLayout->setAlignment(Qt::AlignTop);

        mHeader->setObjectName("header");
        //mHeader->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
        mHeader->setLayout(mHeaderLayout);
        mLayout->addWidget(mHeader);
        //mLayout->setAlignment(mHeader, Qt::AlignTop);

        mHeaderLayout->setContentsMargins(0, 0, 0, 0);
        mHeaderLayout->setSpacing(0);

        mLeftSpacer->setObjectName("left-spacer");
        mLeftSpacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
        mHeaderLayout->addWidget(mLeftSpacer);

        mLabel->setObjectName("label");
        mLabel->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
        mHeaderLayout->addWidget(mLabel);

        mRightSpacer->setObjectName("right-spacer");
        mRightSpacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
        mHeaderLayout->addWidget(mRightSpacer);
    }

    void LabeledFrame::addContent(QWidget* content)
    {
        mLayout->addWidget(content);
    }
}
