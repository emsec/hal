#include "gui/graph_widget/graph_widget.h"

#include "netlist/gate.h"
#include "netlist/module.h"
#include "netlist/net.h"

#include "gui/graph_tab_widget/graph_tab_widget.h"
#include "gui/graph_widget/contexts/graph_context.h"
#include "gui/graph_widget/graph_context_manager.h"
#include "gui/graph_widget/graph_graphics_view.h"
#include "gui/graph_widget/graph_layout_spinner_widget.h"
#include "gui/graph_widget/graph_navigation_widget.h"
#include "gui/graph_widget/graphics_scene.h"
#include "gui/graph_widget/items/nodes/gates/graphics_gate.h"
#include "gui/graph_widget/items/nodes/modules/graphics_module.h"
#include "gui/gui_def.h"
#include "gui/gui_globals.h"
#include "gui/gui_utils/netlist.h"
#include "gui/hal_content_manager/hal_content_manager.h"
#include "gui/overlay/dialog_overlay.h"
#include "gui/toolbar/toolbar.h"

#include <QDebug>
#include <QInputDialog>
#include <QKeyEvent>
#include <QMessageBox>
#include <QToolButton>
#include <QVBoxLayout>
#include <QVariantAnimation>
#include <QTimer>

graph_widget::graph_widget(graph_context* context, QWidget* parent)
    : content_widget("Graph", parent), m_view(new graph_graphics_view(this)), m_context(context), m_overlay(new dialog_overlay(this)), m_navigation_widget(new graph_navigation_widget(nullptr)),
      m_spinner_widget(new graph_layout_spinner_widget(this)), m_current_expansion(0)
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

    m_context->subscribe(this);

    if (!m_context->scene_update_in_progress())
    {
        m_view->setScene(m_context->scene());
        m_view->centerOn(0, 0);
    }
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
    m_spinner_widget->hide();
    m_overlay->set_widget(m_navigation_widget);

    if (hasFocus())
        m_view->setFocus();
}

void graph_widget::handle_scene_unavailable()
{
    m_view->setScene(nullptr);

    disconnect(m_overlay, &dialog_overlay::clicked, m_overlay, &dialog_overlay::hide);

    m_overlay->set_widget(m_spinner_widget);

    if (m_overlay->isHidden())
        m_overlay->show();
}

void graph_widget::handle_context_about_to_be_deleted()
{
    m_view->setScene(nullptr);
    m_context = nullptr;
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

    if (m_context->scene_update_in_progress())
        return;

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
        case Qt::Key_Z:
        {
            if (event->modifiers() & Qt::ControlModifier)    // modifiers are set as bitmasks
            {
            }
            break;
        }
        case Qt::Key_Escape:
        {
            g_selection_relay.clear_and_update();
            break;
        }
        default:
            break;
    }
}

void graph_widget::handle_navigation_jump_requested(const hal::node origin, const u32 via_net, const QSet<u32>& to_gates)
{
    bool bail_animation = false;

    setFocus();

    // ASSERT INPUTS ARE VALID ?
    auto n = g_netlist->get_net_by_id(via_net);
    if (!n || to_gates.isEmpty())
        return;
    std::unordered_set<std::shared_ptr<gate>> gate_ptrs;
    for (u32 id : to_gates)
    {
        auto g = g_netlist->get_gate_by_id(id);
        if (!g)
            return;
        gate_ptrs.insert(g);
    }

    // filter out any gates for which we need to select the module alternatively
    // (because we're already showing that module and we would rip the gate out
    // of the module otherwise)

    // TODO encapsulate this, then move it to the left/right jump handlers and
    // allow passing of modules into the current method (this step is required
    // to implement the more flexible cone view navigation)
    QSet<u32> common_modules;
    std::unordered_set<std::shared_ptr<gate>> filtered_gate_ptrs;
    QSet<u32> filtered_to_gates;
    for (auto& g : gate_ptrs)
    {
        QSet<u32> common = gui_utility::parent_modules(g) & m_context->modules();
        if (common.empty())
        {
            // we can safely select this gate
            filtered_gate_ptrs.insert(g);
            filtered_to_gates.insert(g->get_id());
        }
        else
        {
            // we must select the module instead
            // (this "common" set only has one element)
            common_modules += common;
        }
    }

    if (!m_context->gates().contains(filtered_to_gates))
    {   
        for (const auto& m : g_netlist->get_modules())
        {
            if (m->get_name() == m_context->name().toStdString())
            {
                u32 cnt = 0;
                while (true)
                {
                    ++cnt;
                    QString new_name = m_context->name() + " modified";
                    if (cnt > 1)
                    {
                        new_name += " (" + QString::number(cnt) + ")";
                    }
                    bool found = false;
                    for (const auto& ctx : g_graph_context_manager.get_contexts())
                    {
                        if (ctx->name() == new_name)
                        {
                            found = true;
                            break;
                        }
                    }
                    if (!found)
                    {
                        g_graph_context_manager.rename_graph_context(m_context, new_name);
                        break;
                    }
                }
                break;
            }
        }

        // If we don't have all the gates in the current context, we need to
        // insert them

        auto in_nets = gate_ptrs.begin()->get()->get_fan_in_nets(); // either they're all inputs or all outputs, so just check the first one
        bool netIsInput = in_nets.find(n) != in_nets.cend();
        hal::placement_mode placement = netIsInput ? hal::placement_mode::prefer_right : hal::placement_mode::prefer_left;
        m_context->add({}, to_gates, hal::placement_hint{placement, origin});

        // If we have added any gates, the scene may have resized. In that case, the animation can be erratic,
        // so we set a flag here that we can't run the animation. See end of this method for more details.
        bail_animation = true;
    }
    else
    {
        m_overlay->hide();
        //if (hasFocus())
        m_view->setFocus();
    }

    // SELECT IN RELAY
    g_selection_relay.clear();
    g_selection_relay.m_selected_gates = filtered_to_gates;
    g_selection_relay.m_selected_modules = common_modules;

    // TODO implement subselections on modules, then add a case for when the
    // selection is only one module (instead of one gate)

    if (filtered_to_gates.size() == 1)
    {
        // subfocus only possible when just one gate selected

        auto g = *filtered_gate_ptrs.begin();
        g_selection_relay.m_focus_type = selection_relay::item_type::gate;
        g_selection_relay.m_focus_id   = g->get_id();
        g_selection_relay.m_subfocus   = selection_relay::subfocus::none;

        u32 cnt = 0;
        for (const auto& pin : g->get_input_pins())
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
            for (const auto& pin : g->get_output_pins())
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
    }

    g_selection_relay.relay_selection_changed(nullptr);

    // FIXME If the scene has been resized during this method, the animation triggered by
    // ensure_gates_visible is broken. Thus, if that is the case, we bail out here and not
    // trigger the animation.
    if (bail_animation)
        return;

    // JUMP TO THE GATES AND MODULES
    ensure_items_visible(filtered_to_gates, common_modules);
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
                std::string pin_type   = g->get_input_pins()[g_selection_relay.m_subfocus_index];
                std::shared_ptr<net> n = g->get_fan_in_net(pin_type);

                if (!n)
                    return;

                if (n->get_num_of_sources() == 0)
                {
                    g_selection_relay.clear();
                    g_selection_relay.m_selected_nets.insert(n->get_id());
                    g_selection_relay.m_focus_type = selection_relay::item_type::net;
                    g_selection_relay.m_focus_id   = n->get_id();
                    g_selection_relay.relay_selection_changed(nullptr);
                }
                else if (n->get_num_of_sources() == 1)
                {
                    handle_navigation_jump_requested(hal::node{hal::node_type::gate, g->get_id()}, n->get_id(), {n->get_source().get_gate()->get_id()});
                }
                else
                {
                    m_navigation_widget->setup(false);
                    m_navigation_widget->setFocus();
                    m_overlay->show();
                }
            }
            else if (g->get_input_pins().size())
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

            if (n->get_num_of_sources() == 0)
                return;

            if (n->get_num_of_sources() == 1)
            {
                handle_navigation_jump_requested(hal::node{hal::node_type::gate, 0}, n->get_id(), {n->get_sources()[0].get_gate()->get_id()});
            }
            else
            {
                m_navigation_widget->setup(false);
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
                auto n = g->get_fan_out_net(g->get_output_pins()[g_selection_relay.m_subfocus_index]);
                if (n->get_num_of_destinations() == 0)
                {
                    g_selection_relay.clear();
                    g_selection_relay.m_selected_nets.insert(n->get_id());
                    g_selection_relay.m_focus_type = selection_relay::item_type::net;
                    g_selection_relay.m_focus_id   = n->get_id();
                    g_selection_relay.relay_selection_changed(nullptr);
                }
                else if (n->get_num_of_destinations() == 1)
                {
                    handle_navigation_jump_requested(hal::node{hal::node_type::gate, g->get_id()}, n->get_id(), {n->get_destinations()[0].get_gate()->get_id()});
                }
                else
                {
                    m_navigation_widget->setup(true);
                    m_navigation_widget->setFocus();
                    m_overlay->show();
                }
            }
            else if (g->get_output_pins().size())
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

            if (n->get_num_of_destinations() == 0)
                return;

            if (n->get_num_of_destinations() == 1)
            {
                handle_navigation_jump_requested(hal::node{hal::node_type::gate, 0}, n->get_id(), {n->get_destinations()[0].get_gate()->get_id()});
            }
            else
            {
                m_navigation_widget->setup(true);
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

void graph_widget::handle_enter_module_requested(const u32 id)
{
    auto m = g_netlist->get_module_by_id(id);
    if (m->get_gates().empty() && m->get_submodules().empty())
    {
        QMessageBox msg;
        msg.setText("This module is empty.\nYou can't enter it.");
        msg.setWindowTitle("Error");
        msg.exec();
        return;
        // We would otherwise allow creation of a context with no gates, which
        // is bad because that context won't react to any updates since empty
        // contexts can't infer their corresponding module from their contents
    }

    if (m_context->gates().isEmpty() && m_context->modules() == QSet<u32>({id}))
    {
        m_context->unfold_module(id);
        return;
    }

    QSet<u32> gate_ids;
    QSet<u32> module_ids;
    for (const auto& g : m->get_gates())
    {
        gate_ids.insert(g->get_id());
    }
    for (const auto& sm : m->get_submodules())
    {
        module_ids.insert(sm->get_id());
    }

    for (const auto& ctx : g_graph_context_manager.get_contexts())
    {
        if ((ctx->gates().isEmpty() && ctx->modules() == QSet<u32>({id})) || (ctx->modules() == module_ids && ctx->gates() == gate_ids))
        {
            g_content_manager->get_graph_tab_widget()->show_context(ctx);
            return;
        }
    }

    auto ctx = g_graph_context_manager.create_new_context(QString::fromStdString(m->get_name()));
    ctx->add(module_ids, gate_ids);
}

void graph_widget::ensure_items_visible(const QSet<u32>& gates, const QSet<u32>& modules)
{
    if (m_context->scene_update_in_progress())
        return;

    int min_x = INT_MAX;
    int min_y = INT_MAX;
    int max_x = INT_MIN;
    int max_y = INT_MIN;

    for (auto id : gates)
    {
        auto rect = m_context->scene()->get_gate_item(id)->sceneBoundingRect();

        min_x = std::min(min_x, static_cast<int>(rect.left()));
        max_x = std::max(max_x, static_cast<int>(rect.right()));
        min_y = std::min(min_y, static_cast<int>(rect.top()));
        max_y = std::max(max_y, static_cast<int>(rect.bottom()));
    }

    // TODO clean up redundancy
    for (auto id : modules)
    {
        auto rect = m_context->scene()->get_module_item(id)->sceneBoundingRect();

        min_x = std::min(min_x, static_cast<int>(rect.left()));
        max_x = std::max(max_x, static_cast<int>(rect.right()));
        min_y = std::min(min_y, static_cast<int>(rect.top()));
        max_y = std::max(max_y, static_cast<int>(rect.bottom()));
    }

    auto targetRect = QRectF(min_x, min_y, max_x-min_x, max_y-min_y).marginsAdded(QMarginsF(20,20,20,20));

    // FIXME This breaks as soon as the layouter call that preceded the call to this function
    // changed the scene size. If that happens, mapToScene thinks that the view is looking at (0,0)
    // and the animation jumps to (0,0) before moving to the correct target.
    auto currentRect = m_view->mapToScene(m_view->viewport()->geometry()).boundingRect(); // this has incorrect coordinates

    auto centerFix = targetRect.center();
    targetRect.setWidth(std::max(targetRect.width(), currentRect.width()));
    targetRect.setHeight(std::max(targetRect.height(), currentRect.height()));
    targetRect.moveCenter(centerFix);

    //qDebug() << currentRect;

    auto anim = new QVariantAnimation();
    anim->setDuration(1000);
    anim->setStartValue(currentRect);
    anim->setEndValue(targetRect);
    // FIXME fitInView miscalculates the scale required to actually fit the rect into the viewport,
    // so that every time fitInView is called this will cause the scene to scale down by a very tiny amount.
    connect(anim, &QVariantAnimation::valueChanged, [=](const QVariant& value) { m_view->fitInView(value.toRectF(), Qt::KeepAspectRatio); });

    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void graph_widget::reset_focus()
{
    m_view->setFocus();
}

graph_graphics_view* graph_widget::view()
{
    return m_view;
}
