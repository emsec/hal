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
    splitter_anchor::splitter_anchor(dock_bar* dock_bar, splitter* splitter, QObject* parent) : QObject(parent), m_dock_bar(dock_bar), m_splitter(splitter)
    {
        connect(content_drag_relay::instance(), &content_drag_relay::drag_start, m_dock_bar, &dock_bar::handle_drag_start);
        connect(content_drag_relay::instance(), &content_drag_relay::drag_end, m_dock_bar, &dock_bar::handle_drag_end);

        m_dock_bar->set_anchor(this);
    }

    void splitter_anchor::add(content_widget* widget, int index)
    {
        widget->set_anchor(this);
        content_frame* frame = new content_frame(widget, true, nullptr);
        frame->hide();
        m_splitter->insertWidget(index, frame);
        m_dock_bar->add_button(widget, index);

        Q_EMIT content_changed();
    }

    void splitter_anchor::remove(content_widget* widget)
    {
        widget->set_anchor(nullptr);
        widget->hide();
        widget->setParent(nullptr);
        m_dock_bar->remove_button(widget);

        Q_EMIT content_changed();
    }

    void splitter_anchor::detach(content_widget* widget)
    {
        widget->hide();
        widget->setParent(nullptr);
        content_frame* frame = new content_frame(widget, false, nullptr);
        frame->setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, frame->size(), qApp->desktop()->availableGeometry()));
        frame->show();

        if (m_splitter->unused())
            m_splitter->hide();

        m_dock_bar->detach_button(widget);

        Q_EMIT content_changed();
    }

    void splitter_anchor::reattach(content_widget* widget)
    {
        int index            = m_dock_bar->index(widget);
        content_frame* frame = new content_frame(widget, true, nullptr);
        frame->hide();
        m_splitter->insertWidget(index, frame);
        m_dock_bar->reattach_button(widget);

        Q_EMIT content_changed();
    }

    void splitter_anchor::open(content_widget* widget)
    {
        for (int i = 0; i < m_splitter->count(); i++)
        {
            if (static_cast<content_frame*>(m_splitter->widget(i))->content() == widget)
            {
                static_cast<content_frame*>(m_splitter->widget(i))->show();
                m_splitter->show();
                break;
            }
        }
        m_dock_bar->check_button(widget);
    }

    void splitter_anchor::close(content_widget* widget)
    {
        for (int i = 0; i < m_splitter->count(); i++)
        {
            if (static_cast<content_frame*>(m_splitter->widget(i))->content() == widget)
            {
                static_cast<content_frame*>(m_splitter->widget(i))->hide();
                if (m_splitter->unused())
                    m_splitter->hide();
                break;
            }
        }
        m_dock_bar->uncheck_button(widget);
    }

    int splitter_anchor::count()
    {
        return m_dock_bar->count();
    }

    void splitter_anchor::remove_content()
    {
        for (int i = m_dock_bar->count() - 1; i >= 0; i--)
        {
            content_widget* widget = m_dock_bar->widget_at(i);
            remove(widget);
        }

        m_dock_bar->update();
    }
}
