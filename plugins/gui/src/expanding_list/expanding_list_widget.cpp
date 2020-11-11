#include "gui/expanding_list/expanding_list_widget.h"

#include "gui/expanding_list/expanding_list_button.h"
#include "gui/expanding_list/expanding_list_item.h"

#include <QFrame>
#include <QStyle>
#include <QVBoxLayout>

namespace hal
{
    ExpandingListWidget::ExpandingListWidget(QWidget* parent)
        : QScrollArea(parent), mContent(new QFrame()), mContentLayout(new QVBoxLayout()), mSpacer(new QFrame()), mSelectedButton(nullptr), mExtendedItem(nullptr), mOffset(0)
    {
        setFrameStyle(QFrame::NoFrame);
        setWidget(mContent);
        setWidgetResizable(true);
        mContent->setObjectName("content");
        mContent->setFrameStyle(QFrame::NoFrame);
        mContent->setLayout(mContentLayout);
        mContentLayout->setAlignment(Qt::AlignTop);
        mContentLayout->setContentsMargins(0, 0, 0, 0);
        mContentLayout->setSpacing(0);
        mSpacer->setObjectName("spacer");
        mSpacer->setFrameStyle(QFrame::NoFrame);
        mContentLayout->addWidget(mSpacer);
    }

    void ExpandingListWidget::appendItem(ExpandingListButton* button, ExpandingListButton* parentButton)
    {
        if (parentButton)
        {
            for (ExpandingListItem* item : mItems)
                if (parentButton == item->parentButton())
                    item->appendChildButton(button);
        }
        else
        {
            ExpandingListItem* item = new ExpandingListItem(button);
            mItems.append(item);
            mContentLayout->addWidget(item);
        }
        connect(button, &ExpandingListButton::clicked, this, &ExpandingListWidget::handleClicked);
    }

    void ExpandingListWidget::selectButton(ExpandingListButton* button)
    {
        if (button == mSelectedButton)
            return;

        if (!button)
        {
            if (mSelectedButton)
            {
                mSelectedButton->setSelected(false);
                mSelectedButton = nullptr;
            }

            if (mExtendedItem)
            {
                mExtendedItem->setExpanded(false);
                mExtendedItem->collapse();
                mExtendedItem = nullptr;
            }
            return;
        }

        for (ExpandingListItem* item : mItems)
            if (item->contains(button))
            {
                if (mSelectedButton)
                    mSelectedButton->setSelected(false);

                mSelectedButton = button;
                mSelectedButton->setSelected(true);

                if (item != mExtendedItem)
                {
                    if (mExtendedItem)
                    {
                        mExtendedItem->setExpanded(false);
                        mExtendedItem->collapse();
                    }

                    mExtendedItem = item;
                    mExtendedItem->setExpanded(true);
                    mExtendedItem->expand();
                }
            }

        Q_EMIT buttonSelected(button);
    }

    void ExpandingListWidget::selectItem(int index)
    {
        if (index < 0 || index >= mItems.size())
            return;

        selectButton(mItems.at(index)->parentButton());
    }

    void ExpandingListWidget::repolish()
    {
        QStyle* s = style();

        s->unpolish(this);
        s->polish(this);

        for (ExpandingListItem* item : mItems)
            item->repolish();
    }

    void ExpandingListWidget::handleClicked()
    {
        QObject* obj                  = sender();
        ExpandingListButton* button = static_cast<ExpandingListButton*>(obj);
        selectButton(button);
    }
}
