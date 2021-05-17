#include "gui/expanding_list/expanding_list_item.h"

#include "gui/expanding_list/expanding_list_button.h"

#include <QStyle>

namespace hal
{
    ExpandingListItem::ExpandingListItem(ExpandingListButton* but, QWidget* parent)
        : QFrame(parent), mButton(but), mCollapsedHeight(0), mExpandedHeight(0),
          mExpanded(false), mFixedHeight(mButton->minimumHeight())
    {
        mButton->setParent(this);
        mButton->show();
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        setMinimumHeight(mButton->minimumHeight());
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

        mButton->resize(width, mButton->height());
    }

    bool ExpandingListItem::expanded() const
    {
        return mExpanded;
    }

    int ExpandingListItem::fixedHeight() const
    {
        return mFixedHeight;
    }


    ExpandingListButton* ExpandingListItem::button() const
    {
        return mButton;
    }

    void ExpandingListItem::repolish()
    {
        QStyle* s = style();

        s->unpolish(this);
        s->polish(this);

        mButton->repolish();
        mButton->move(0, 0);
        mCollapsedHeight = mButton->height();
        int offset       = mCollapsedHeight;

        mExpandedHeight = offset;

        setFixedHeight(mCollapsedHeight);
    }

    void ExpandingListItem::collapse()
    {
        mExpanded = false;
        hide();
    }

    void ExpandingListItem::expand()
    {
        mExpanded = true;
        show();
        repolish();
    }

    void ExpandingListItem::setFixedHeight(int height)
    {
        mFixedHeight = height;

        setMinimumHeight(height);
        setMaximumHeight(height);
    }
}
