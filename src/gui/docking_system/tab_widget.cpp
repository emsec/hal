#include "docking_system/tab_widget.h"
#include "content_frame/content_frame.h"
#include "content_widget/content_widget.h"
#include "docking_system/content_drag_relay.h"
#include <QApplication>
#include <QDebug>
#include <QDesktopWidget>
#include <QShortcut>
#include <QStyle>
namespace hal{
tab_widget::tab_widget(QWidget* parent)
    : hal_widget(parent), m_vertical_layout(new QVBoxLayout()), m_horizontal_layout(new QHBoxLayout()), m_dock_bar(new dock_bar(Qt::Horizontal, button_orientation::horizontal, this)),
      m_left_toolbar(new toolbar()), m_right_toolbar(new toolbar()), m_current_widget(nullptr), m_action_detach(new QAction(this))
{
    connect(m_action_detach, &QAction::triggered, this, &tab_widget::detach_current_widget);
    connect(content_drag_relay::instance(), &content_drag_relay::drag_start, this, &tab_widget::handle_drag_start);
    connect(content_drag_relay::instance(), &content_drag_relay::drag_end, this, &tab_widget::handle_drag_end);

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

void tab_widget::add(content_widget* widget, int index)
{
    widget->set_anchor(this);
    m_dock_bar->add_button(widget, index);
    if (m_current_widget == nullptr)
        open(widget);
    show();
}

void tab_widget::remove(content_widget* widget)
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

void tab_widget::detach(content_widget* widget)
{
    int index = m_dock_bar->index(widget);
    if (index != -1)
    {
        m_dock_bar->detach_button(widget);
        content_frame* frame = new content_frame(widget, false, nullptr);
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

void tab_widget::reattach(content_widget* widget)
{
    int index = m_dock_bar->index(widget);
    if (index != -1)
    {
        if (m_current_widget == nullptr)
            open(widget);
        m_dock_bar->reattach_button(widget);
        show();

        for (content_frame* frame : m_detached_frames)
        {
            if (frame->content() == widget)
                m_detached_frames.removeOne(frame);
        }
    }
}

void tab_widget::open(content_widget* widget)
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

void tab_widget::close(content_widget* widget)
{
    Q_UNUSED(widget)

    m_dock_bar->check_button(m_current_widget);
}

void tab_widget::handle_no_current_widget(int index)
{
    content_widget* widget = m_dock_bar->next_available_widget(index);
    if (widget)
        open(widget);
    else
    {
        hide();
        m_left_toolbar->clear();
    }
}

//does not delete the widgets, just removes
void tab_widget::remove_content()
{
    for (int i = m_dock_bar->count() - 1; i >= 0; i--)
    {
        content_widget* widget = m_dock_bar->widget_at(i);
        remove(widget);
    }

    m_dock_bar->update();
}

void tab_widget::detach_current_widget()
{
    if (m_current_widget)
        detach(m_current_widget);
}

void tab_widget::handle_drag_start()
{
    show();
}

void tab_widget::handle_drag_end()
{
    if (m_dock_bar->unused())
        hide();
}
}
