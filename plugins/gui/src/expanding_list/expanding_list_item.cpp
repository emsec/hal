#include "gui/expanding_list/expanding_list_item.h"

#include "gui/expanding_list/expanding_list_button.h"

#include <QPropertyAnimation>
#include <QStyle>

namespace hal
{
    ExpandingListItem::ExpandingListItem(ExpandingListButton* parentButton, QWidget* parent)
        : QFrame(parent), mParentButton(parentButton), mCollapsedHeight(0), mExpandedHeight(0),
          //mAnimation(new QPropertyAnimation(this, "minimumHeight", this)),
          mAnimation(new QPropertyAnimation(this, "fixedHeight", this)), mExpanded(false), mFixedHeight(mParentButton->minimumHeight())
    {
        parentButton->setParent(this);
        parentButton->show();
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        setMinimumHeight(mParentButton->minimumHeight());
    }

    QSize ExpandingListItem::minimumSizeHint() const
    {
        return minimumSize();
    }

    QSize ExpandingListItem::sizeHint() const
    {
        return size();
    }

    void ExpandingListItem::resizeEvent(QResizeEvent* event)
    {
        int width = event->size().width();

        mParentButton->resize(width, mParentButton->height());

        for (ExpandingListButton* button : mChildButtons)
            button->resize(width, button->height());
    }

    bool ExpandingListItem::expanded()
    {
        return mExpanded;
    }

    int ExpandingListItem::fixedHeight()
    {
        return mFixedHeight;
    }

    bool ExpandingListItem::contains(ExpandingListButton* button)
    {
        if (button == mParentButton)
            return true;

        for (ExpandingListButton* child_button : mChildButtons)
            if (button == child_button)
                return true;

        return false;
    }

    ExpandingListButton* ExpandingListItem::parentButton()
    {
        return mParentButton;
    }

    void ExpandingListItem::appendChildButton(ExpandingListButton* button)
    {
        mChildButtons.append(button);
        button->setParent(this);
        button->set_type("child");
        button->show();
    }

    void ExpandingListItem::repolish()
    {
        QStyle* s = style();

        s->unpolish(this);
        s->polish(this);

        mParentButton->repolish();
        mParentButton->move(0, 0);
        mCollapsedHeight = mParentButton->height();
        int offset         = mCollapsedHeight;

        for (ExpandingListButton* button : mChildButtons)
        {
            button->repolish();
            button->move(0, offset);
            offset += button->height();
        }

        mExpandedHeight = offset;

        mAnimation->setDuration(200);
        //    mAnimation->setStartValue(QSize(width(), mCollapsedHeight));
        //    mAnimation->setEndValue(QSize(width(), mExpandedHeight));

        mAnimation->setStartValue(mCollapsedHeight);
        mAnimation->setEndValue(mExpandedHeight);

        setFixedHeight(mCollapsedHeight);
    }

    void ExpandingListItem::collapse()
    {
        for (ExpandingListButton* button : mChildButtons)
            button->setEnabled(false);

        mAnimation->setDirection(QPropertyAnimation::Backward);

        if (mAnimation->state() != QPropertyAnimation::Running)
            mAnimation->start();
    }

    void ExpandingListItem::expand()
    {
        for (ExpandingListButton* button : mChildButtons)
            button->setEnabled(true);

        mAnimation->setDirection(QPropertyAnimation::Forward);

        if (mAnimation->state() == QPropertyAnimation::Running)
        {
            mAnimation->pause();
            mAnimation->resume();
        }
        else
            mAnimation->start();
    }

    void ExpandingListItem::setExpanded(bool expanded)
    {
        mExpanded = expanded;
        repolish();
    }

    void ExpandingListItem::setFixedHeight(int height)
    {
        mFixedHeight = height;

        setMinimumHeight(height);
        setMaximumHeight(height);
    }
}
