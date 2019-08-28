#include "gui/graph_widget/graph_widget.h"

#include "netlist/gate.h"
#include "netlist/module.h"
#include "netlist/net.h"

#include "gui/graph_widget/contexts/dynamic_context.h"
#include "gui/graph_widget/graph_context_manager.h"
#include "gui/graph_widget/graph_graphics_view.h"
#include "gui/graph_widget/graph_layout_progress_widget.h"
#include "gui/graph_widget/graph_layout_spinner_widget.h"
#include "gui/graph_widget/graph_navigation_widget.h"
#include "gui/graph_widget/graphics_scene.h"
#include "gui/graph_widget/items/graphics_gate.h"
#include "gui/gui_globals.h"
#include "gui/overlay/dialog_overlay.h"
#include "gui/toolbar/toolbar.h"

#include <QDebug>
#include <QInputDialog>
#include <QKeyEvent>
#include <QToolButton>
#include <QVBoxLayout>
#include <QVariantAnimation>

graph_widget::graph_widget(QWidget* parent)
    : content_widget("Graph", parent), m_view(new graph_graphics_view(this)), m_context(nullptr), m_overlay(new dialog_overlay(this)), m_navigation_widget(new graph_navigation_widget(nullptr)),
      m_progress_widget(new graph_layout_progress_widget(this)), m_spinner_widget(new graph_layout_spinner_widget(this)), m_current_expansion(0)
{
    connect(m_navigation_widget, &graph_navigation_widget::navigation_requested, this, &graph_widget::handle_navigation_jump_requested);
    connect(m_navigation_widget, &graph_navigation_widget::close_requested, m_overlay, &dialog_overlay::hide);
    connect(m_navigation_widget, &graph_navigation_widget::close_requested, this, &graph_widget::reset_focus);

    connect(m_overlay, &dialog_overlay::clicked, m_overlay, &dialog_overlay::hide);

    connect(m_view, &graph_graphics_view::module_double_clicked, this, &graph_widget::handle_module_double_clicked);

    m_overlay->hide();
    m_overlay->set_widget(m_navigation_widget);
    m_spinner_widget->hide();
    m_content_layout->addWidget(m_view);

    m_view->setFrameStyle(QFrame::NoFrame);
    m_view->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    m_view->setRenderHint(QPainter::Antialiasing, false);
    m_view->setDragMode(QGraphicsView::RubberBandDrag);

    // debug: go to context 1; delete later
    //    debug_module_one();
}

graph_widget::graph_widget(graph_context* context, QWidget* parent)
    : content_widget("Graph", parent), m_view(new graph_graphics_view(this)), m_context(context), m_overlay(new dialog_overlay(this)), m_navigation_widget(new graph_navigation_widget(nullptr)),
      m_progress_widget(new graph_layout_progress_widget(this)), m_spinner_widget(new graph_layout_spinner_widget(this)), m_current_expansion(0)
{
    connect(m_navigation_widget, &graph_navigation_widget::navigation_requested, this, &graph_widget::handle_navigation_jump_requested);
    connect(m_navigation_widget, &graph_navigation_widget::close_requested, m_overlay, &dialog_overlay::hide);
    connect(m_navigation_widget, &graph_navigation_widget::close_requested, this, &graph_widget::reset_focus);

    connect(m_overlay, &dialog_overlay::clicked, m_overlay, &dialog_overlay::hide);

    connect(m_view, &graph_graphics_view::module_double_clicked, this, &graph_widget::handle_module_double_clicked);

    m_overlay->hide();
    m_overlay->set_widget(m_navigation_widget);
    m_spinner_widget->hide();
    m_content_layout->addWidget(m_view);

    m_view->setFrameStyle(QFrame::NoFrame);
    m_view->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    m_view->setRenderHint(QPainter::Antialiasing, false);
    m_view->setDragMode(QGraphicsView::RubberBandDrag);

    context->subscribe(this);
    change_context(context);
}

graph_context* graph_widget::get_context() const
{
    return m_context;
}

void graph_widget::handle_scene_available()
{
    m_view->setScene(m_context->scene());

    connect(m_overlay, &dialog_overlay::clicked, m_overlay, &dialog_overlay::hide);
    m_overlay->hide();
    //    m_progress_widget->stop();
    m_spinner_widget->hide();
    m_overlay->set_widget(m_navigation_widget);

    if (hasFocus())
        m_view->setFocus();

    // JUMP TO THE GATE
    // JUMP SHOULD BE HANDLED SEPARATELY
    //    if (item)
    //        m_graphics_widget->view()->ensureVisible(item);
}

void graph_widget::handle_scene_unavailable()
{
    m_view->setScene(nullptr);

    disconnect(m_overlay, &dialog_overlay::clicked, m_overlay, &dialog_overlay::hide);

    //m_progress_widget->set_direction(graph_layout_progress_widget::direction::right);
    //m_progress_widget->set_direction(graph_layout_progress_widget::direction::left);
    //m_overlay->set_widget(m_progress_widget);
    m_overlay->set_widget(m_spinner_widget);
    //m_progress_widget->start();

    if (m_overlay->isHidden())
        m_overlay->show();
}

void graph_widget::handle_context_about_to_be_deleted()
{
    m_view->setScene(nullptr);
    m_context = nullptr;

    // SHOW SOME KIND OF "NO CONTEXT SELECTED" WIDGET
    // UPDATE OTHER DATA, LIKE TOOLBUTTONS
}

void graph_widget::handle_status_update(const int percent)
{
    Q_UNUSED(percent)
}

void graph_widget::handle_status_update(const QString& message)
{
    Q_UNUSED(message)
}

void graph_widget::keyPressEvent(QKeyEvent* event)
{
    if (!m_context)
        return;

    if (!m_context->available())
        return;

    //if (m_context && m_context->available())

    switch (event->key())
    {
        case Qt::Key_Left:
        {
            handle_navigation_left_request();
            break;
        }
        case Qt::Key_Right:
        {
            handle_navigation_right_request();
            break;
        }
        case Qt::Key_Up:
        {
            handle_navigation_up_request();
            break;
        }
        case Qt::Key_Down:
        {
            handle_navigation_down_request();
            break;
        }
        case Qt::Key_Backspace:
        {
            handle_history_step_back_request();
            break;
        }
        default:
            break;
    }
}

void graph_widget::handle_navigation_jump_requested(const u32 via_net, const u32 to_gate)
{
    setFocus();
    // ASSERT INPUTS ARE VALID ?
    auto n = g_netlist->get_net_by_id(via_net);
    auto g = g_netlist->get_gate_by_id(to_gate);

    if (!g || !n)
        return;

    if (!m_context->gates().contains(to_gate))
    {
        add_context_to_history();
        m_context->add({}, {to_gate}, {});
    }
    else
    {
        m_overlay->hide();
        //if (hasFocus())
        m_view->setFocus();
    }

    // SELECT IN RELAY
    g_selection_relay.clear();
    g_selection_relay.m_selected_gates.insert(to_gate);
    g_selection_relay.m_focus_type = selection_relay::item_type::gate;
    g_selection_relay.m_focus_id   = to_gate;
    g_selection_relay.m_subfocus   = selection_relay::subfocus::none;

    u32 cnt = 0;
    for (const auto& pin : g->get_input_pin_types())
    {
        if (g->get_fan_in_net(pin) == n)    // input net
        {
            g_selection_relay.m_subfocus       = selection_relay::subfocus::left;
            g_selection_relay.m_subfocus_index = cnt;
            break;
        }
        cnt++;
    }
    if (g_selection_relay.m_subfocus == selection_relay::subfocus::none)
    {
        cnt = 0;
        for (const auto& pin : g->get_output_pin_types())
        {
            if (g->get_fan_out_net(pin) == n)    // input net
            {
                g_selection_relay.m_subfocus       = selection_relay::subfocus::right;
                g_selection_relay.m_subfocus_index = cnt;
                break;
            }
            cnt++;
        }
    }

    g_selection_relay.relay_selection_changed(nullptr);

    // JUMP TO THE GATE
    ensure_gate_visible(to_gate);
}

void graph_widget::handle_module_double_clicked(const u32 id)
{
    // CONNECT DIRECTLY TO HANDLE ???
    // MAYBE ADDITIONAL CODE NECESSARY HERE...
    handle_enter_module_requested(id);
}

// ADD SOUND OR ERROR MESSAGE TO FAILED NAVIGATION ATTEMPTS
void graph_widget::handle_navigation_left_request()
{
    switch (g_selection_relay.m_focus_type)
    {
        case selection_relay::item_type::none:
        {
            return;
        }
        case selection_relay::item_type::gate:
        {
            std::shared_ptr<gate> g = g_netlist->get_gate_by_id(g_selection_relay.m_focus_id);

            if (!g)
                return;

            if (g_selection_relay.m_subfocus == selection_relay::subfocus::left)
            {
                std::string pin_type   = g->get_input_pin_types()[g_selection_relay.m_subfocus_index];
                std::shared_ptr<net> n = g->get_fan_in_net(pin_type);

                if (!n)
                    return;

                if (n->get_src().gate == nullptr)
                {
                    g_selection_relay.clear();
                    g_selection_relay.m_selected_nets.insert(n->get_id());
                    g_selection_relay.m_focus_type = selection_relay::item_type::net;
                    g_selection_relay.m_focus_id   = n->get_id();
                    g_selection_relay.relay_selection_changed(nullptr);
                }
                else
                {
                    handle_navigation_jump_requested(n->get_id(), n->get_src().get_gate()->get_id());
                }
            }
            else if (g->get_input_pin_types().size())
            {
                g_selection_relay.m_subfocus       = selection_relay::subfocus::left;
                g_selection_relay.m_subfocus_index = 0;

                g_selection_relay.relay_subfocus_changed(nullptr);
            }

            return;
        }
        case selection_relay::item_type::net:
        {
            std::shared_ptr<net> n = g_netlist->get_net_by_id(g_selection_relay.m_focus_id);

            if (!n)
                return;

            if (n->get_src().gate != nullptr)
            {
                handle_navigation_jump_requested(n->get_id(), n->get_src().get_gate()->get_id());
            }

            return;
        }
        case selection_relay::item_type::module:
        {
            return;
        }
    }
}

void graph_widget::handle_navigation_right_request()
{
    switch (g_selection_relay.m_focus_type)
    {
        case selection_relay::item_type::none:
        {
            return;
        }
        case selection_relay::item_type::gate:
        {
            std::shared_ptr<gate> g = g_netlist->get_gate_by_id(g_selection_relay.m_focus_id);

            if (!g)
                return;

            if (g_selection_relay.m_subfocus == selection_relay::subfocus::right)
            {
                auto n = g->get_fan_out_net(g->get_output_pin_types()[g_selection_relay.m_subfocus_index]);
                if (n->get_num_of_dsts() == 0)
                {
                    g_selection_relay.clear();
                    g_selection_relay.m_selected_nets.insert(n->get_id());
                    g_selection_relay.m_focus_type = selection_relay::item_type::net;
                    g_selection_relay.m_focus_id   = n->get_id();
                    g_selection_relay.relay_selection_changed(nullptr);
                }
                else if (n->get_num_of_dsts() == 1)
                {
                    handle_navigation_jump_requested(n->get_id(), n->get_dsts()[0].get_gate()->get_id());
                }
                else
                {
                    m_navigation_widget->setup();
                    m_navigation_widget->setFocus();
                    m_overlay->show();
                }
            }
            else if (g->get_output_pin_types().size())
            {
                g_selection_relay.m_subfocus       = selection_relay::subfocus::right;
                g_selection_relay.m_subfocus_index = 0;

                g_selection_relay.relay_subfocus_changed(nullptr);
            }

            return;
        }
        case selection_relay::item_type::net:
        {
            std::shared_ptr<net> n = g_netlist->get_net_by_id(g_selection_relay.m_focus_id);

            if (!n)
                return;

            if (n->get_num_of_dsts() == 1)
            {
                handle_navigation_jump_requested(n->get_id(), n->get_dsts()[0].get_gate()->get_id());
            }
            else
            {
                m_navigation_widget->setup();
                m_navigation_widget->setFocus();
                m_overlay->show();
            }

            return;
        }
        case selection_relay::item_type::module:
        {
            return;
        }
    }
}

void graph_widget::handle_navigation_up_request()
{
    if (g_selection_relay.m_focus_type == selection_relay::item_type::gate)
        if (m_context->gates().contains(g_selection_relay.m_focus_id))
            g_selection_relay.navigate_up();
}

void graph_widget::handle_navigation_down_request()
{
    if (g_selection_relay.m_focus_type == selection_relay::item_type::gate)
        if (m_context->gates().contains(g_selection_relay.m_focus_id))
            g_selection_relay.navigate_down();
}

void graph_widget::handle_history_step_back_request()
{
    if (m_context_history.empty())
        return;
    auto entry = m_context_history.back();
    m_context_history.pop_back();

    m_context->begin_change();
    m_context->clear();
    m_context->add(entry.m_modules, entry.m_gates, {});
    m_context->end_change();
}

void graph_widget::handle_enter_module_requested(const u32 id)
{
    add_context_to_history();
    m_context->begin_change();
    m_context->clear();
    auto m = g_netlist->get_module_by_id(id);

    QSet<u32> modules, gates;
    for (const auto& x : m->get_submodules())
        modules.insert(x->get_id());
    for (const auto& x : m->get_gates())
        gates.insert(x->get_id());
    m_context->add(modules, gates, {});
    m_context->end_change();
}

void graph_widget::ensure_gate_visible(const u32 gate)
{
    if (m_context->update_in_progress())
        return;

    const graphics_gate* itm = m_context->scene()->get_gate_item(gate);

    auto anim = new QVariantAnimation();
    anim->setDuration(1000);

    QPointF center = m_view->mapToScene(QRect(0, 0, m_view->viewport()->width(), m_view->viewport()->height())).boundingRect().center();
    anim->setStartValue(center);
    anim->setEndValue(itm->pos());

    connect(anim, &QVariantAnimation::valueChanged, [=](const QVariant& value) { m_view->centerOn(value.toPoint()); });

    anim->start();
}

void graph_widget::add_context_to_history()
{
    context_history_entry entry;

    entry.m_modules = m_context->modules();
    entry.m_gates   = m_context->gates();

    m_context_history.push_back(entry);

    while (m_context_history.size() > 10)
    {
        m_context_history.pop_front();
    }
}

void graph_widget::change_context(graph_context* const context)
{
    m_context_history.clear();
    assert(context);

    if (m_context)
        m_context->unsubscribe(this);

    m_context = context;
    m_context->subscribe(this);

    if (!m_context->update_in_progress())
    {
        m_view->setScene(m_context->scene());
        m_view->centerOn(0, 0);
    }
}

void graph_widget::reset_focus()
{
    m_view->setFocus();
}

graph_graphics_view* graph_widget::view()
{
    return m_view;
}
