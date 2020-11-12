#include "gui/docking_system/splitter_anchor.h"
#include "gui/content_frame/content_frame.h"
#include "gui/content_widget/content_widget.h"
#include "gui/docking_system/content_drag_relay.h"
#include "gui/docking_system/dock_bar.h"
#include "gui/splitter/splitter.h"
#include <QApplication>
#include <QDesktopWidget>
#include <QStyle>

namespace hal
{
    SplitterAnchor::SplitterAnchor(DockBar* DockBar, Splitter* Splitter, QObject* parent) : QObject(parent), mDockBar(DockBar), mSplitter(Splitter)
    {
        connect(ContentDragRelay::instance(), &ContentDragRelay::dragStart, mDockBar, &DockBar::handleDragStart);
        connect(ContentDragRelay::instance(), &ContentDragRelay::dragEnd, mDockBar, &DockBar::handleDragEnd);

        mDockBar->setAnchor(this);
    }

    void SplitterAnchor::add(ContentWidget* widget, int index)
    {
        widget->setAnchor(this);
        ContentFrame* frame = new ContentFrame(widget, true, nullptr);
        frame->hide();
        mSplitter->insertWidget(index, frame);
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

    void SplitterAnchor::detach(ContentWidget* widget)
    {
        widget->hide();
        widget->setParent(nullptr);
        ContentFrame* frame = new ContentFrame(widget, false, nullptr);
        frame->setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, frame->size(), qApp->desktop()->availableGeometry()));
        frame->show();

        if (mSplitter->unused())
            mSplitter->hide();

        mDockBar->detachButton(widget);

        Q_EMIT contentChanged();
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

    int SplitterAnchor::count()
    {
        return mDockBar->count();
    }

    void SplitterAnchor::removeContent()
    {
        for (int i = mDockBar->count() - 1; i >= 0; i--)
        {
            ContentWidget* widget = mDockBar->widgetAt(i);
            remove(widget);
        }

        mDockBar->update();
    }
}
