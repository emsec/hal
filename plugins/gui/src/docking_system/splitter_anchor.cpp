#include "gui/content_frame/content_frame.h"
#include "gui/content_widget/content_widget.h"
#include "gui/docking_system/content_drag_relay.h"
#include "gui/docking_system/dock_bar.h"
#include "gui/docking_system/splitter_anchor.h"
#include "gui/hal_qt_compat/hal_qt_compat.h"
#include "gui/splitter/splitter.h"
#include <QApplication>
#include <QStyle>

namespace hal
{
    SplitterAnchor::SplitterAnchor(ContentLayout::Position apos, DockBar* dockBar, Splitter* splitter, QObject* parent) : QObject(parent), mSplitter(splitter)
    {
        mDockBar = dockBar;
        mAnchorPosition = apos;
        connect(ContentDragRelay::instance(), &ContentDragRelay::dragStart, mDockBar, &DockBar::handleDragStart);
        connect(ContentDragRelay::instance(), &ContentDragRelay::dragEnd, mDockBar, &DockBar::handleDragEnd);

        mDockBar->setAnchor(this);
    }

    void SplitterAnchor::add(ContentWidget* widget, int index)
    {
        // detached widgets with hidden button are not present in splitter widget
        int splitterIndex = index;
        for (int i=0; i<index; i++)
            if (mDockBar->isHidden(i))
                --splitterIndex;
        widget->setAnchor(this);
        ContentFrame* frame = new ContentFrame(widget, true, nullptr);
        frame->hide();
        mSplitter->insertWidget(splitterIndex, frame);
        mDockBar->addButton(widget, index);

        Q_EMIT contentChanged();
    }

    void SplitterAnchor::remove(ContentWidget* widget)
    {
        widget->setAnchor(nullptr);
        widget->hide();
        widget->setParent(nullptr);
        mDockBar->removeButton(widget);

        Q_EMIT contentChanged();
    }

    ContentFrame *SplitterAnchor::detach(ContentWidget* widget)
    {
        widget->hide();
        widget->setParent(nullptr);
        ContentFrame* frame = new ContentFrame(widget, false, nullptr);
        frame->setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, frame->size(), QtCompat::desktopGeometry()));
        frame->show();

        if (mSplitter->unused())
            mSplitter->hide();

        mDockBar->detachButton(widget);

        Q_EMIT contentChanged();
        return frame;
     }

    void SplitterAnchor::reattach(ContentWidget* widget)
    {
        int index            = mDockBar->index(widget);
        ContentFrame* frame = new ContentFrame(widget, true, nullptr);
        frame->hide();
        mSplitter->insertWidget(index, frame);
        mDockBar->reattachButton(widget);

        Q_EMIT contentChanged();
    }

    void SplitterAnchor::open(ContentWidget* widget)
    {
        for (int i = 0; i < mSplitter->count(); i++)
        {
            if (static_cast<ContentFrame*>(mSplitter->widget(i))->content() == widget)
            {
                static_cast<ContentFrame*>(mSplitter->widget(i))->show();
                mSplitter->show();
                break;
            }
        }
        mDockBar->checkButton(widget);
    }

    void SplitterAnchor::close(ContentWidget* widget)
    {
        for (int i = 0; i < mSplitter->count(); i++)
        {
            if (static_cast<ContentFrame*>(mSplitter->widget(i))->content() == widget)
            {
                static_cast<ContentFrame*>(mSplitter->widget(i))->hide();
                if (mSplitter->unused())
                    mSplitter->hide();
                break;
            }
        }
        mDockBar->uncheckButton(widget);
    }

    void SplitterAnchor::clear()
    {
        /*
        for (int i = 0; i < mDockBar->count(); ++i)
        {
            ContentWidget* widget = mDockBar->widgetAt(0);
            remove(widget);
        }

        mDockBar->update();
        */

        mDockBar->clear();
    }
}
