#include "docking_system/splitter_anchor.h"
#include "content_frame/content_frame.h"
#include "content_widget/content_widget.h"
#include "docking_system/content_drag_relay.h"
#include "docking_system/dock_bar.h"
#include "splitter/splitter.h"
#include <QApplication>
#include <QDesktopWidget>
#include <QStyle>

namespace hal
{
    SplitterAnchor::SplitterAnchor(DockBar* DockBar, splitter* splitter, QObject* parent) : QObject(parent), m_dock_bar(DockBar), m_splitter(splitter)
    {
        connect(ContentDragRelay::instance(), &ContentDragRelay::drag_start, m_dock_bar, &DockBar::handle_drag_start);
        connect(ContentDragRelay::instance(), &ContentDragRelay::drag_end, m_dock_bar, &DockBar::handle_drag_end);

        m_dock_bar->set_anchor(this);
    }

    void SplitterAnchor::add(ContentWidget* widget, int index)
    {
        widget->set_anchor(this);
        ContentFrame* frame = new ContentFrame(widget, true, nullptr);
        frame->hide();
        m_splitter->insertWidget(index, frame);
        m_dock_bar->add_button(widget, index);

        Q_EMIT content_changed();
    }

    void SplitterAnchor::remove(ContentWidget* widget)
    {
        widget->set_anchor(nullptr);
        widget->hide();
        widget->setParent(nullptr);
        m_dock_bar->remove_button(widget);

        Q_EMIT content_changed();
    }

    void SplitterAnchor::detach(ContentWidget* widget)
    {
        widget->hide();
        widget->setParent(nullptr);
        ContentFrame* frame = new ContentFrame(widget, false, nullptr);
        frame->setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, frame->size(), qApp->desktop()->availableGeometry()));
        frame->show();

        if (m_splitter->unused())
            m_splitter->hide();

        m_dock_bar->detach_button(widget);

        Q_EMIT content_changed();
    }

    void SplitterAnchor::reattach(ContentWidget* widget)
    {
        int index            = m_dock_bar->index(widget);
        ContentFrame* frame = new ContentFrame(widget, true, nullptr);
        frame->hide();
        m_splitter->insertWidget(index, frame);
        m_dock_bar->reattach_button(widget);

        Q_EMIT content_changed();
    }

    void SplitterAnchor::open(ContentWidget* widget)
    {
        for (int i = 0; i < m_splitter->count(); i++)
        {
            if (static_cast<ContentFrame*>(m_splitter->widget(i))->content() == widget)
            {
                static_cast<ContentFrame*>(m_splitter->widget(i))->show();
                m_splitter->show();
                break;
            }
        }
        m_dock_bar->check_button(widget);
    }

    void SplitterAnchor::close(ContentWidget* widget)
    {
        for (int i = 0; i < m_splitter->count(); i++)
        {
            if (static_cast<ContentFrame*>(m_splitter->widget(i))->content() == widget)
            {
                static_cast<ContentFrame*>(m_splitter->widget(i))->hide();
                if (m_splitter->unused())
                    m_splitter->hide();
                break;
            }
        }
        m_dock_bar->uncheck_button(widget);
    }

    int SplitterAnchor::count()
    {
        return m_dock_bar->count();
    }

    void SplitterAnchor::remove_content()
    {
        for (int i = m_dock_bar->count() - 1; i >= 0; i--)
        {
            ContentWidget* widget = m_dock_bar->widget_at(i);
            remove(widget);
        }

        m_dock_bar->update();
    }
}
