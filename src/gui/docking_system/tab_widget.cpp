#include "docking_system/tab_widget.h"
#include "content_frame/content_frame.h"
#include "content_widget/content_widget.h"
#include "docking_system/content_drag_relay.h"
#include <QApplication>
#include <QDebug>
#include <QDesktopWidget>
#include <QShortcut>
#include <QStyle>

namespace hal
{
    TabWidget::TabWidget(QWidget* parent)
        : Widget(parent), m_vertical_layout(new QVBoxLayout()), m_horizontal_layout(new QHBoxLayout()), m_dock_bar(new DockBar(Qt::Horizontal, button_orientation::horizontal, this)),
          m_left_toolbar(new Toolbar()), m_right_toolbar(new Toolbar()), m_current_widget(nullptr), m_action_detach(new QAction(this))
    {
        connect(m_action_detach, &QAction::triggered, this, &TabWidget::detach_current_widget);
        connect(ContentDragRelay::instance(), &ContentDragRelay::drag_start, this, &TabWidget::handle_drag_start);
        connect(ContentDragRelay::instance(), &ContentDragRelay::drag_end, this, &TabWidget::handle_drag_end);

        setFocusPolicy(Qt::FocusPolicy::StrongFocus);

        m_vertical_layout->setContentsMargins(0, 0, 0, 0);
        m_vertical_layout->setSpacing(0);

        m_horizontal_layout->setContentsMargins(0, 0, 0, 0);
        m_horizontal_layout->setSpacing(0);

        m_dock_bar->set_autohide(false);
        m_dock_bar->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        //    m_dock_bar->setFixedHeight(24);    // use settings

        //    m_toolbar->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
        //    m_toolbar->setFixedHeight(24);    // use settings

        m_left_toolbar->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
        m_left_toolbar->setIconSize(QSize(18, 18));
        m_right_toolbar->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        m_right_toolbar->setIconSize(QSize(18, 18));

        //    m_content_toolbar->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
        //    m_container_toolbar->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::MinimumExpanding);

        m_action_detach->setText("Detach");
        m_right_toolbar->addAction(m_action_detach);
        m_dock_bar->set_anchor(this);

        setLayout(m_vertical_layout);
        m_vertical_layout->addLayout(m_horizontal_layout);
        m_vertical_layout->setAlignment(m_horizontal_layout, Qt::AlignTop);
        m_horizontal_layout->addWidget(m_dock_bar);
        m_horizontal_layout->addWidget(m_left_toolbar);
        m_horizontal_layout->addWidget(m_right_toolbar);

        hide();
    }

    void TabWidget::add(ContentWidget* widget, int index)
    {
        widget->set_anchor(this);
        m_dock_bar->add_button(widget, index);
        if (m_current_widget == nullptr)
            open(widget);
        show();
    }

    void TabWidget::remove(ContentWidget* widget)
    {
        int index = m_dock_bar->index(widget);
        if (index != -1)
        {
            m_dock_bar->remove_button(index);

            if (widget == m_current_widget)
            {
                widget->hide();
                widget->setParent(nullptr);
                m_current_widget = nullptr;
                handle_no_current_widget(index);
            }
            widget->set_anchor(nullptr);
        }
    }

    void TabWidget::detach(ContentWidget* widget)
    {
        int index = m_dock_bar->index(widget);
        if (index != -1)
        {
            m_dock_bar->detach_button(widget);
            ContentFrame* frame = new ContentFrame(widget, false, nullptr);
            m_detached_frames.append(frame);
            frame->setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, frame->size(), qApp->desktop()->availableGeometry()));
            frame->show();

            if (widget == m_current_widget)
            {
                m_current_widget = nullptr;
                handle_no_current_widget(index);
            }
        }
    }

    void TabWidget::reattach(ContentWidget* widget)
    {
        int index = m_dock_bar->index(widget);
        if (index != -1)
        {
            if (m_current_widget == nullptr)
                open(widget);
            m_dock_bar->reattach_button(widget);
            show();

            for (ContentFrame* frame : m_detached_frames)
            {
                if (frame->content() == widget)
                    m_detached_frames.removeOne(frame);
            }
        }
    }

    void TabWidget::open(ContentWidget* widget)
    {
        int index = m_dock_bar->index(widget);
        if (index == -1)
            return;

        if (m_current_widget)
        {
            m_current_widget->hide();
            m_current_widget->setParent(nullptr);
            m_dock_bar->uncheck_button(m_current_widget);
        }
        m_left_toolbar->clear();
        m_vertical_layout->addWidget(widget);
        widget->show();
        widget->setup_toolbar(m_left_toolbar);
        m_dock_bar->check_button(widget);
        m_current_widget = widget;

        for (QShortcut* r : m_active_shortcuts)
        {
            delete r;
        }

        m_active_shortcuts.clear();

        for (QShortcut* s : widget->create_shortcuts())
        {
            s->setParent(this);
            s->setContext(Qt::WidgetWithChildrenShortcut);
            s->setEnabled(true);
            m_active_shortcuts.append(s);
        }
    }

    void TabWidget::close(ContentWidget* widget)
    {
        Q_UNUSED(widget)

        m_dock_bar->check_button(m_current_widget);
    }

    void TabWidget::handle_no_current_widget(int index)
    {
        ContentWidget* widget = m_dock_bar->next_available_widget(index);
        if (widget)
            open(widget);
        else
        {
            hide();
            m_left_toolbar->clear();
        }
    }

    //does not delete the widgets, just removes
    void TabWidget::remove_content()
    {
        for (int i = m_dock_bar->count() - 1; i >= 0; i--)
        {
            ContentWidget* widget = m_dock_bar->widget_at(i);
            remove(widget);
        }

        m_dock_bar->update();
    }

    void TabWidget::detach_current_widget()
    {
        if (m_current_widget)
            detach(m_current_widget);
    }

    void TabWidget::handle_drag_start()
    {
        show();
    }

    void TabWidget::handle_drag_end()
    {
        if (m_dock_bar->unused())
            hide();
    }
}
