#include "gui/docking_system/tab_widget.h"
#include "gui/content_frame/content_frame.h"
#include "gui/content_widget/content_widget.h"
#include "gui/docking_system/content_drag_relay.h"
#include <QApplication>
#include <QDebug>
#include <QDesktopWidget>
#include <QShortcut>
#include <QStyle>

namespace hal
{
    TabWidget::TabWidget(QWidget* parent)
        : Widget(parent), mVerticalLayout(new QVBoxLayout()), mHorizontalLayout(new QHBoxLayout()), mDockBar(new DockBar(Qt::Horizontal, button_orientation::horizontal, this)),
          mLeftToolbar(new Toolbar()), mRightToolbar(new Toolbar()), mCurrentWidget(nullptr), mActionDetach(new QAction(this))
    {
        connect(mActionDetach, &QAction::triggered, this, &TabWidget::detachCurrentWidget);
        connect(ContentDragRelay::instance(), &ContentDragRelay::dragStart, this, &TabWidget::handleDragStart);
        connect(ContentDragRelay::instance(), &ContentDragRelay::dragEnd, this, &TabWidget::handleDragEnd);

        setFocusPolicy(Qt::FocusPolicy::StrongFocus);

        mVerticalLayout->setContentsMargins(0, 0, 0, 0);
        mVerticalLayout->setSpacing(0);

        mHorizontalLayout->setContentsMargins(0, 0, 0, 0);
        mHorizontalLayout->setSpacing(0);

        mDockBar->setAutohide(false);
        mDockBar->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        //    mDockBar->setFixedHeight(24);    // use settings

        //    mToolbar->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
        //    mToolbar->setFixedHeight(24);    // use settings

        mLeftToolbar->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
        mLeftToolbar->setIconSize(QSize(18, 18));
        mRightToolbar->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        mRightToolbar->setIconSize(QSize(18, 18));

        //    m_content_toolbar->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
        //    m_container_toolbar->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::MinimumExpanding);

        mActionDetach->setText("Detach");
        mRightToolbar->addAction(mActionDetach);
        mDockBar->setAnchor(this);

        setLayout(mVerticalLayout);
        mVerticalLayout->addLayout(mHorizontalLayout);
        mVerticalLayout->setAlignment(mHorizontalLayout, Qt::AlignTop);
        mHorizontalLayout->addWidget(mDockBar);
        mHorizontalLayout->addWidget(mLeftToolbar);
        mHorizontalLayout->addWidget(mRightToolbar);

        hide();
    }

    void TabWidget::add(ContentWidget* widget, int index)
    {
        widget->setAnchor(this);
        mDockBar->addButton(widget, index);
        if (mCurrentWidget == nullptr)
            open(widget);
        show();
    }

    void TabWidget::remove(ContentWidget* widget)
    {
        int index = mDockBar->index(widget);
        if (index != -1)
        {
            mDockBar->removeButton(index);

            if (widget == mCurrentWidget)
            {
                widget->hide();
                widget->setParent(nullptr);
                mCurrentWidget = nullptr;
                handleNoCurrentWidget(index);
            }
            widget->setAnchor(nullptr);
        }
    }

    void TabWidget::detach(ContentWidget* widget)
    {
        int index = mDockBar->index(widget);
        if (index != -1)
        {
            mDockBar->detachButton(widget);
            ContentFrame* frame = new ContentFrame(widget, false, nullptr);
            mDetachedFrames.append(frame);
            frame->setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, frame->size(), qApp->desktop()->availableGeometry()));
            frame->show();

            if (widget == mCurrentWidget)
            {
                mCurrentWidget = nullptr;
                handleNoCurrentWidget(index);
            }
        }
    }

    void TabWidget::reattach(ContentWidget* widget)
    {
        int index = mDockBar->index(widget);
        if (index != -1)
        {
            if (mCurrentWidget == nullptr)
                open(widget);
            mDockBar->reattachButton(widget);
            show();

            for (ContentFrame* frame : mDetachedFrames)
            {
                if (frame->content() == widget)
                    mDetachedFrames.removeOne(frame);
            }
        }
    }

    void TabWidget::open(ContentWidget* widget)
    {
        int index = mDockBar->index(widget);
        if (index == -1)
            return;

        if (mCurrentWidget)
        {
            mCurrentWidget->hide();
            mCurrentWidget->setParent(nullptr);
            mDockBar->uncheckButton(mCurrentWidget);
        }
        mLeftToolbar->clear();
        mVerticalLayout->addWidget(widget);
        widget->show();
        widget->setupToolbar(mLeftToolbar);
        mDockBar->checkButton(widget);
        mCurrentWidget = widget;

        for (QShortcut* r : mActiveShortcuts)
        {
            delete r;
        }

        mActiveShortcuts.clear();

        for (QShortcut* s : widget->createShortcuts())
        {
            s->setParent(this);
            s->setContext(Qt::WidgetWithChildrenShortcut);
            s->setEnabled(true);
            mActiveShortcuts.append(s);
        }
    }

    void TabWidget::close(ContentWidget* widget)
    {
        Q_UNUSED(widget)

        mDockBar->checkButton(mCurrentWidget);
    }

    void TabWidget::handleNoCurrentWidget(int index)
    {
        ContentWidget* widget = mDockBar->nextAvailableWidget(index);
        if (widget)
            open(widget);
        else
        {
            hide();
            mLeftToolbar->clear();
        }
    }

    //does not delete the widgets, just removes
    void TabWidget::removeContent()
    {
        for (int i = mDockBar->count() - 1; i >= 0; i--)
        {
            ContentWidget* widget = mDockBar->widgetAt(i);
            remove(widget);
        }

        mDockBar->update();
    }

    void TabWidget::detachCurrentWidget()
    {
        if (mCurrentWidget)
            detach(mCurrentWidget);
    }

    void TabWidget::handleDragStart()
    {
        show();
    }

    void TabWidget::handleDragEnd()
    {
        if (mDockBar->unused())
            hide();
    }
}
