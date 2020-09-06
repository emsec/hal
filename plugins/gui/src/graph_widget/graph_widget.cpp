#include "gui/graph_widget/graph_widget.h"

#include "gui/content_manager/content_manager.h"
#include "gui/graph_tab_widget/graph_tab_widget.h"
#include "gui/graph_widget/contexts/graph_context.h"
#include "gui/graph_widget/graph_context_manager.h"
#include "gui/graph_widget/graph_graphics_view.h"
#include "gui/graph_widget/graph_layout_spinner_widget.h"
#include "gui/graph_widget/graph_navigation_widget_v2.h"
#include "gui/graph_widget/graphics_scene.h"
#include "gui/graph_widget/items/nodes/gates/graphics_gate.h"
#include "gui/graph_widget/items/nodes/modules/graphics_module.h"
#include "gui/gui_def.h"
#include "gui/gui_globals.h"
#include "gui/gui_utils/netlist.h"
#include "gui/overlay/dialog_overlay.h"
#include "gui/toolbar/toolbar.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/module.h"
#include "hal_core/netlist/net.h"
#include "hal_core/utilities/utils.h"

#include <QDebug>
#include <QInputDialog>
#include <QKeyEvent>
#include <QMessageBox>
#include <QTimer>
#include <QToolButton>
#include <QVBoxLayout>
#include <QVariantAnimation>

namespace hal
{
    GraphWidget::GraphWidget(GraphContext* context, QWidget* parent)
        : ContentWidget("Graph", parent), m_view(new GraphGraphicsView(this)), m_context(context), m_Overlay(new DialogOverlay(this)), m_navigation_widget_v2(new GraphNavigationWidgetV2(nullptr)),
          m_spinner_widget(new GraphLayoutSpinnerWidget(this)), m_current_expansion(0)
    {
        connect(m_navigation_widget_v2, &GraphNavigationWidgetV2::navigation_requested, this, &GraphWidget::handle_navigation_jump_requested);
        connect(m_navigation_widget_v2, &GraphNavigationWidgetV2::close_requested, m_Overlay, &DialogOverlay::hide);
        connect(m_navigation_widget_v2, &GraphNavigationWidgetV2::close_requested, this, &GraphWidget::reset_focus);

        connect(m_Overlay, &DialogOverlay::clicked, m_Overlay, &DialogOverlay::hide);

        connect(m_view, &GraphGraphicsView::module_double_clicked, this, &GraphWidget::handle_module_double_clicked);

        m_Overlay->hide();
        m_Overlay->set_widget(m_navigation_widget_v2);
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

    GraphContext* GraphWidget::get_context() const
    {
        return m_context;
    }

    void GraphWidget::handle_scene_available()
    {
        m_view->setScene(m_context->scene());

        connect(m_Overlay, &DialogOverlay::clicked, m_Overlay, &DialogOverlay::hide);

        m_Overlay->hide();
        m_spinner_widget->hide();
        m_Overlay->set_widget(m_navigation_widget_v2);

        if (hasFocus())
            m_view->setFocus();
    }

    void GraphWidget::handle_scene_unavailable()
    {
        m_view->setScene(nullptr);

        disconnect(m_Overlay, &DialogOverlay::clicked, m_Overlay, &DialogOverlay::hide);

        m_Overlay->set_widget(m_spinner_widget);

        if (m_Overlay->isHidden())
            m_Overlay->show();
    }

    void GraphWidget::handle_context_about_to_be_deleted()
    {
        m_view->setScene(nullptr);
        m_context = nullptr;
    }

    void GraphWidget::handle_status_update(const int percent)
    {
        Q_UNUSED(percent)
    }

    void GraphWidget::handle_status_update(const QString& message)
    {
        Q_UNUSED(message)
    }

    void GraphWidget::keyPressEvent(QKeyEvent* event)
    {
        if (!m_context)
            return;

        if (m_context->scene_update_in_progress())
            return;

        switch (event->key())
        {
            case Qt::Key_Left: {
                handle_navigation_left_request();
                break;
            }
            case Qt::Key_Right: {
                handle_navigation_right_request();
                break;
            }
            case Qt::Key_Up: {
                handle_navigation_up_request();
                break;
            }
            case Qt::Key_Down: {
                handle_navigation_down_request();
                break;
            }
            case Qt::Key_Z: {
                if (event->modifiers() & Qt::ControlModifier)    // modifiers are set as bitmasks
                {
                }
                break;
            }
            case Qt::Key_Escape: {
                g_selection_relay->clear_and_update();
                break;
            }
            default:
                break;
        }
    }

    void GraphWidget::substitute_by_visible_modules(const QSet<u32>& gates,
                                                    const QSet<u32>& modules,
                                                    QSet<u32>& target_gates,
                                                    QSet<u32>& target_modules,
                                                    QSet<u32>& remove_gates,
                                                    QSet<u32>& remove_modules) const
    {
        // EXPAND SELECTION AND CONTEXT UP THE HIERARCHY TREE

        for (auto& mid : modules)
        {
            auto m           = g_netlist->get_module_by_id(mid);
            QSet<u32> common = gui_utility::parent_modules(m) & m_context->modules();
            if (common.empty())
            {
                // we can select the module
                target_modules.insert(mid);
            }
            else
            {
                // we must select the respective parent module instead
                // (this "common" set only has one element)
                assert(common.size() == 1);
                target_modules += common;
            }
        }

        for (auto& gid : gates)
        {
            auto g           = g_netlist->get_gate_by_id(gid);
            QSet<u32> common = gui_utility::parent_modules(g) & m_context->modules();
            if (common.empty())
            {
                target_gates.insert(gid);
            }
            else
            {
                // At this stage, "common" could contain multiple elements because
                // we might have inserted a parent module where its child  module is
                // already visible. This is cleaned up later.
                target_modules += common;
            }
        }

        // PRUNE SELECTION AND CONTEXT DOWN THE HIERARCHY TREE

        // discard (and if required schedule for removal) all modules whose
        // parent modules we'll be showing
        QSet<u32> new_module_set = m_context->modules() + target_modules;
        for (auto& mid : m_context->modules())
        {
            auto m = g_netlist->get_module_by_id(mid);
            if (gui_utility::parent_modules(m).intersects(new_module_set))
            {
                remove_modules.insert(mid);
            }
        }
        auto it = target_modules.constBegin();
        while (it != target_modules.constEnd())
        {
            auto m = g_netlist->get_module_by_id(*it);
            if (gui_utility::parent_modules(m).intersects(new_module_set))
            {
                it = target_modules.erase(it);
            }
            else
            {
                ++it;
            }
        }

        // discard (and if required schedule for removal) all gates whose
        // parent modules we'll be showing
        new_module_set = (m_context->modules() - remove_modules) + target_modules;
        for (auto& gid : m_context->gates())
        {
            auto g = g_netlist->get_gate_by_id(gid);
            if (gui_utility::parent_modules(g).intersects(new_module_set))
            {
                remove_gates.insert(gid);
            }
        }
        it = target_gates.constBegin();
        while (it != target_gates.constEnd())
        {
            auto g = g_netlist->get_gate_by_id(*it);
            if (gui_utility::parent_modules(g).intersects(new_module_set))
            {
                it = target_gates.erase(it);
            }
            else
            {
                ++it;
            }
        }
        // qDebug() << "-----------";
        // qDebug() << "requested gates" << gates;
        // qDebug() << "requested modules" << modules;
        // qDebug() << "target gates" << target_gates;
        // qDebug() << "target modules" << target_modules;
        // qDebug() << "remove gates" << remove_gates;
        // qDebug() << "remove modules" << remove_modules;
    }

    void GraphWidget::set_modified_if_module()
    {
        // if our name matches that of a module, add the "modified" label and
        // optionally a number if a "modified"-labeled context already exists
        for (auto m : g_netlist->get_modules())
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
                    for (const auto& ctx : g_graph_context_manager->get_contexts())
                    {
                        if (ctx->name() == new_name)
                        {
                            found = true;
                            break;
                        }
                    }
                    if (!found)
                    {
                        g_graph_context_manager->rename_graph_context(m_context, new_name);
                        break;
                    }
                }
                break;
            }
        }
    }

    void GraphWidget::handle_navigation_jump_requested(const hal::node origin, const u32 via_net, const QSet<u32>& to_gates, const QSet<u32>& to_modules)
    {
        bool bail_animation = false;

        setFocus();

        // ASSERT INPUTS ARE VALID
        auto n = g_netlist->get_net_by_id(via_net);
        if (!n || (to_gates.empty() && to_modules.empty()))
        {
            // prevent stuck navigation widget
            m_Overlay->hide();
            m_view->setFocus();
            return;
        }

        // Substitute all gates by their modules if we're showing them.
        // This avoids ripping gates out of their already visible modules.
        QSet<u32> final_modules, remove_modules;
        QSet<u32> final_gates, remove_gates;
        substitute_by_visible_modules(to_gates, to_modules, final_gates, final_modules, remove_gates, remove_modules);

        // find out which gates and modules we still need to add to the context
        // (this makes the cone view work)
        QSet<u32> nonvisible_gates   = final_gates - m_context->gates();
        QSet<u32> nonvisible_modules = final_modules - m_context->modules();

        // if we don't have all gates and modules, we need to add them
        if (!nonvisible_gates.empty() || !nonvisible_modules.empty())
        {
            // display the "modified" label if we're showing a module context
            set_modified_if_module();

            // hint the layouter at the direction we're navigating in
            // (so the cone view nicely extends to the right or left)
            // either they're all inputs or all outputs, so just check the first one

            std::vector<Net*> in_nets;
            if (to_gates.empty())
            {
                in_nets = g_netlist->get_module_by_id(*to_modules.constBegin())->get_input_nets();
            }
            else
            {
                in_nets = utils::to_vector(g_netlist->get_gate_by_id(*to_gates.begin())->get_fan_in_nets());
            }
            bool netIsInput               = std::find(in_nets.begin(), in_nets.end(), n) != in_nets.cend();
            hal::placement_mode placement = netIsInput ? hal::placement_mode::prefer_right : hal::placement_mode::prefer_left;

            // add all new gates and modules
            m_context->begin_change();
            m_context->remove(remove_modules, remove_gates);
            m_context->add(nonvisible_modules, nonvisible_gates, hal::placement_hint{placement, origin});
            m_context->end_change();

            // FIXME find out how to do this properly
            // If we have added any gates, the scene may have resized. In that case, the animation can be erratic,
            // so we set a flag here that we can't run the animation. See end of this method for more details.
            bail_animation = true;
        }
        else
        {
            // if we don't need to add anything, we're done here

            m_Overlay->hide();
            //if (hasFocus())
            m_view->setFocus();
        }

        // SELECT IN RELAY
        g_selection_relay->clear();
        g_selection_relay->m_selected_gates   = final_gates;
        g_selection_relay->m_selected_modules = final_modules;

        // TODO implement subselections on modules, then add a case for when the
        // selection is only one module (instead of one gate)

        if (final_gates.size() == 1 && final_modules.empty())
        {
            // subfocus only possible when just one gate selected
            u32 gid = *final_gates.begin();
            auto g  = g_netlist->get_gate_by_id(gid);

            g_selection_relay->m_focus_type = SelectionRelay::item_type::gate;
            g_selection_relay->m_focus_id   = gid;
            g_selection_relay->m_subfocus   = SelectionRelay::subfocus::none;

            u32 cnt = 0;
            // TODO simplify (we do actually know if we're navigating left or right)
            for (const auto& pin : g->get_input_pins())
            {
                if (g->get_fan_in_net(pin) == n)    // input net
                {
                    g_selection_relay->m_subfocus       = SelectionRelay::subfocus::left;
                    g_selection_relay->m_subfocus_index = cnt;
                    break;
                }
                cnt++;
            }
            if (g_selection_relay->m_subfocus == SelectionRelay::subfocus::none)
            {
                cnt = 0;
                for (const auto& pin : g->get_output_pins())
                {
                    if (g->get_fan_out_net(pin) == n)    // input net
                    {
                        g_selection_relay->m_subfocus       = SelectionRelay::subfocus::right;
                        g_selection_relay->m_subfocus_index = cnt;
                        break;
                    }
                    cnt++;
                }
            }
        }
        else if (final_modules.size() == 1 && final_gates.empty())
        {
            // subfocus only possible when just one module selected
            u32 mid = *final_modules.begin();
            auto m  = g_netlist->get_module_by_id(mid);

            g_selection_relay->m_focus_type = SelectionRelay::item_type::module;
            g_selection_relay->m_focus_id   = mid;
            g_selection_relay->m_subfocus   = SelectionRelay::subfocus::none;

            u32 cnt = 0;
            // FIXME this is super hacky because currently we have no way of
            // properly indexing port names on modules (since no order is guaranteed
            // on the port names (different to pin names in gates), but our GUI
            // wants integer indexes)
            // (what we use here is the fact that GraphicsModule builds its port
            // list by traversing m->get_input_nets(), so we just use that order and
            // hope nobody touches that implementation)

            // TODO simplify (we do actually know if we're navigating left or right)
            for (const auto& inet : m->get_input_nets())
            {
                if (inet == n)    // input net
                {
                    g_selection_relay->m_subfocus       = SelectionRelay::subfocus::left;
                    g_selection_relay->m_subfocus_index = cnt;
                    break;
                }
                cnt++;
            }
            if (g_selection_relay->m_subfocus == SelectionRelay::subfocus::none)
            {
                cnt = 0;
                for (const auto& inet : m->get_output_nets())
                {
                    if (inet == n)    // input net
                    {
                        g_selection_relay->m_subfocus       = SelectionRelay::subfocus::right;
                        g_selection_relay->m_subfocus_index = cnt;
                        break;
                    }
                    cnt++;
                }
            }
        }

        g_selection_relay->relay_selection_changed(nullptr);

        // FIXME If the scene has been resized during this method, the animation triggered by
        // ensure_gates_visible is broken. Thus, if that is the case, we bail out here and not
        // trigger the animation.
        if (bail_animation)
            return;

        // JUMP TO THE GATES AND MODULES
        ensure_items_visible(final_gates, final_modules);
    }

    void GraphWidget::handle_module_double_clicked(const u32 id)
    {
        // CONNECT DIRECTLY TO HANDLE ???
        // MAYBE ADDITIONAL CODE NECESSARY HERE...
        handle_enter_module_requested(id);
    }

    // ADD SOUND OR ERROR MESSAGE TO FAILED NAVIGATION ATTEMPTS
    void GraphWidget::handle_navigation_left_request()
    {
        switch (g_selection_relay->m_focus_type)
        {
            case SelectionRelay::item_type::none: {
                return;
            }
            case SelectionRelay::item_type::gate: {
                Gate* g = g_netlist->get_gate_by_id(g_selection_relay->m_focus_id);

                if (!g)
                    return;

                if (g_selection_relay->m_subfocus == SelectionRelay::subfocus::left)
                {
                    std::string pin_type = g->get_input_pins()[g_selection_relay->m_subfocus_index];
                    Net* n               = g->get_fan_in_net(pin_type);

                    if (!n)
                        return;

                    if (n->get_num_of_sources() == 0)
                    {
                        g_selection_relay->clear();
                        g_selection_relay->m_selected_nets.insert(n->get_id());
                        g_selection_relay->m_focus_type = SelectionRelay::item_type::net;
                        g_selection_relay->m_focus_id   = n->get_id();
                        g_selection_relay->relay_selection_changed(nullptr);
                    }
                    else if (n->get_num_of_sources() == 1)
                    {
                        handle_navigation_jump_requested(hal::node{hal::node_type::gate, g->get_id()}, n->get_id(), {n->get_source()->get_gate()->get_id()}, {});
                    }
                    else
                    {
                        m_navigation_widget_v2->setup(false);
                        m_navigation_widget_v2->setFocus();
                        m_Overlay->show();
                    }
                }
                else if (g->get_input_pins().size())
                {
                    g_selection_relay->m_subfocus       = SelectionRelay::subfocus::left;
                    g_selection_relay->m_subfocus_index = 0;

                    g_selection_relay->relay_subfocus_changed(nullptr);
                }

                return;
            }
            case SelectionRelay::item_type::net: {
                Net* n = g_netlist->get_net_by_id(g_selection_relay->m_focus_id);

                if (!n)
                    return;

                if (n->get_num_of_sources() == 0)
                    return;

                if (n->get_num_of_sources() == 1)
                {
                    handle_navigation_jump_requested(hal::node{hal::node_type::gate, 0}, n->get_id(), {n->get_sources()[0]->get_gate()->get_id()}, {});
                }
                else
                {
                    m_navigation_widget_v2->setup(false);
                    m_navigation_widget_v2->setFocus();
                    m_Overlay->show();
                }

                return;
            }
            case SelectionRelay::item_type::module: {
                Module* m = g_netlist->get_module_by_id(g_selection_relay->m_focus_id);

                if (!m)
                    return;

                if (g_selection_relay->m_subfocus == SelectionRelay::subfocus::left)
                {
                    // FIXME this is super hacky because currently we have no way of
                    // properly indexing port names on modules (since no order is guaranteed
                    // on the port names (different to pin names in gates), but our GUI
                    // wants integer indexes)
                    // (what we use here is the fact that GraphicsModule builds its port
                    // list by traversing m->get_input_nets(), so we just use that order and
                    // hope nobody touches that implementation)
                    auto nets = m->get_input_nets();
                    auto it   = nets.begin();
                    if (g_selection_relay->m_subfocus_index > 0)
                        std::advance(it, g_selection_relay->m_subfocus_index);
                    auto n = *it;
                    if (n->get_num_of_sources() == 0)
                    {
                        g_selection_relay->clear();
                        g_selection_relay->m_selected_nets.insert(n->get_id());
                        g_selection_relay->m_focus_type = SelectionRelay::item_type::net;
                        g_selection_relay->m_focus_id   = n->get_id();
                        g_selection_relay->relay_selection_changed(nullptr);
                    }
                    else if (n->get_num_of_sources() == 1)
                    {
                        handle_navigation_jump_requested(hal::node{hal::node_type::module, m->get_id()}, n->get_id(), {n->get_sources()[0]->get_gate()->get_id()}, {});
                    }
                    else
                    {
                        m_navigation_widget_v2->setup(false);
                        m_navigation_widget_v2->setFocus();
                        m_Overlay->show();
                    }
                }
                else if (m->get_input_nets().size())
                {
                    g_selection_relay->m_subfocus       = SelectionRelay::subfocus::left;
                    g_selection_relay->m_subfocus_index = 0;

                    g_selection_relay->relay_subfocus_changed(nullptr);
                }

                return;
            }
        }
    }

    void GraphWidget::handle_navigation_right_request()
    {
        switch (g_selection_relay->m_focus_type)
        {
            case SelectionRelay::item_type::none: {
                return;
            }
            case SelectionRelay::item_type::gate: {
                Gate* g = g_netlist->get_gate_by_id(g_selection_relay->m_focus_id);

                if (!g)
                    return;

                if (g_selection_relay->m_subfocus == SelectionRelay::subfocus::right)
                {
                    auto n = g->get_fan_out_net(g->get_output_pins()[g_selection_relay->m_subfocus_index]);
                    if (n->get_num_of_destinations() == 0)
                    {
                        g_selection_relay->clear();
                        g_selection_relay->m_selected_nets.insert(n->get_id());
                        g_selection_relay->m_focus_type = SelectionRelay::item_type::net;
                        g_selection_relay->m_focus_id   = n->get_id();
                        g_selection_relay->relay_selection_changed(nullptr);
                    }
                    else if (n->get_num_of_destinations() == 1)
                    {
                        handle_navigation_jump_requested(hal::node{hal::node_type::gate, g->get_id()}, n->get_id(), {n->get_destinations()[0]->get_gate()->get_id()}, {});
                    }
                    else
                    {
                        m_navigation_widget_v2->setup(true);
                        m_navigation_widget_v2->setFocus();
                        m_Overlay->show();
                    }
                }
                else if (g->get_output_pins().size())
                {
                    g_selection_relay->m_subfocus       = SelectionRelay::subfocus::right;
                    g_selection_relay->m_subfocus_index = 0;

                    g_selection_relay->relay_subfocus_changed(nullptr);
                }

                return;
            }
            case SelectionRelay::item_type::net: {
                Net* n = g_netlist->get_net_by_id(g_selection_relay->m_focus_id);

                if (!n)
                    return;

                if (n->get_num_of_destinations() == 0)
                    return;

                if (n->get_num_of_destinations() == 1)
                {
                    handle_navigation_jump_requested(hal::node{hal::node_type::gate, 0}, n->get_id(), {n->get_destinations()[0]->get_gate()->get_id()}, {});
                }
                else
                {
                    m_navigation_widget_v2->setup(true);
                    m_navigation_widget_v2->setFocus();
                    m_Overlay->show();
                }

                return;
            }
            case SelectionRelay::item_type::module: {
                Module* m = g_netlist->get_module_by_id(g_selection_relay->m_focus_id);

                if (!m)
                    return;

                if (g_selection_relay->m_subfocus == SelectionRelay::subfocus::right)
                {
                    // FIXME this is super hacky because currently we have no way of
                    // properly indexing port names on modules (since no order is guaranteed
                    // on the port names (different to pin names in gates), but our GUI
                    // wants integer indexes)
                    // (what we use here is the fact that GraphicsModule builds its port
                    // list by traversing m->get_input_nets(), so we just use that order and
                    // hope nobody touches that implementation)
                    auto nets = m->get_output_nets();
                    auto it   = nets.begin();
                    if (g_selection_relay->m_subfocus_index > 0)
                        std::advance(it, g_selection_relay->m_subfocus_index);
                    auto n = *it;
                    if (n->get_num_of_destinations() == 0)
                    {
                        g_selection_relay->clear();
                        g_selection_relay->m_selected_nets.insert(n->get_id());
                        g_selection_relay->m_focus_type = SelectionRelay::item_type::net;
                        g_selection_relay->m_focus_id   = n->get_id();
                        g_selection_relay->relay_selection_changed(nullptr);
                    }
                    else if (n->get_num_of_destinations() == 1)
                    {
                        handle_navigation_jump_requested(hal::node{hal::node_type::module, m->get_id()}, n->get_id(), {n->get_destinations()[0]->get_gate()->get_id()}, {});
                    }
                    else
                    {
                        m_navigation_widget_v2->setup(true);
                        m_navigation_widget_v2->setFocus();
                        m_Overlay->show();
                    }
                }
                else if (m->get_output_nets().size())
                {
                    g_selection_relay->m_subfocus       = SelectionRelay::subfocus::right;
                    g_selection_relay->m_subfocus_index = 0;

                    g_selection_relay->relay_subfocus_changed(nullptr);
                }
            }
        }
    }

    void GraphWidget::handle_navigation_up_request()
    {
        // FIXME this is ugly
        if ((g_selection_relay->m_focus_type == SelectionRelay::item_type::gate && m_context->gates().contains(g_selection_relay->m_focus_id))
            || (g_selection_relay->m_focus_type == SelectionRelay::item_type::module && m_context->modules().contains(g_selection_relay->m_focus_id)))
            g_selection_relay->navigate_up();
    }

    void GraphWidget::handle_navigation_down_request()
    {
        // FIXME this is ugly
        if ((g_selection_relay->m_focus_type == SelectionRelay::item_type::gate && m_context->gates().contains(g_selection_relay->m_focus_id))
            || (g_selection_relay->m_focus_type == SelectionRelay::item_type::module && m_context->modules().contains(g_selection_relay->m_focus_id)))
            g_selection_relay->navigate_down();
    }

    void GraphWidget::handle_enter_module_requested(const u32 id)
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
        for (auto sm : m->get_submodules())
        {
            module_ids.insert(sm->get_id());
        }

        for (const auto& ctx : g_graph_context_manager->get_contexts())
        {
            if ((ctx->gates().isEmpty() && ctx->modules() == QSet<u32>({id})) || (ctx->modules() == module_ids && ctx->gates() == gate_ids))
            {
                g_content_manager->get_graph_tab_widget()->show_context(ctx);
                return;
            }
        }

        auto ctx = g_graph_context_manager->create_new_context(QString::fromStdString(m->get_name()));
        ctx->add(module_ids, gate_ids);
    }

    void GraphWidget::ensure_items_visible(const QSet<u32>& gates, const QSet<u32>& modules)
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
            auto rect = m_context->scene()->get_ModuleItem(id)->sceneBoundingRect();

            min_x = std::min(min_x, static_cast<int>(rect.left()));
            max_x = std::max(max_x, static_cast<int>(rect.right()));
            min_y = std::min(min_y, static_cast<int>(rect.top()));
            max_y = std::max(max_y, static_cast<int>(rect.bottom()));
        }

        auto targetRect = QRectF(min_x, min_y, max_x - min_x, max_y - min_y).marginsAdded(QMarginsF(20, 20, 20, 20));

        // FIXME This breaks as soon as the layouter call that preceded the call to this function
        // changed the scene size. If that happens, mapToScene thinks that the view is looking at (0,0)
        // and the animation jumps to (0,0) before moving to the correct target.
        auto currentRect = m_view->mapToScene(m_view->viewport()->geometry()).boundingRect();    // this has incorrect coordinates

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

    void GraphWidget::ensure_selection_visible()
    {
        if (m_context->scene_update_in_progress())
            return;

        if (m_context->gates().contains(g_selection_relay->m_selected_gates) == false || m_context->nets().contains(g_selection_relay->m_selected_nets) == false
            || m_context->modules().contains(g_selection_relay->m_selected_modules) == false)
            return;

        int min_x = INT_MAX;
        int min_y = INT_MAX;
        int max_x = INT_MIN;
        int max_y = INT_MIN;

        for (auto id : g_selection_relay->m_selected_gates)
        {
            auto rect = m_context->scene()->get_gate_item(id)->sceneBoundingRect();

            min_x = std::min(min_x, static_cast<int>(rect.left()));
            max_x = std::max(max_x, static_cast<int>(rect.right()));
            min_y = std::min(min_y, static_cast<int>(rect.top()));
            max_y = std::max(max_y, static_cast<int>(rect.bottom()));
        }

        for (auto id : g_selection_relay->m_selected_nets)
        {
            auto rect = m_context->scene()->get_net_item(id)->sceneBoundingRect();

            min_x = std::min(min_x, static_cast<int>(rect.left()));
            max_x = std::max(max_x, static_cast<int>(rect.right()));
            min_y = std::min(min_y, static_cast<int>(rect.top()));
            max_y = std::max(max_y, static_cast<int>(rect.bottom()));
        }

        for (auto id : g_selection_relay->m_selected_modules)
        {
            auto rect = m_context->scene()->get_ModuleItem(id)->sceneBoundingRect();

            min_x = std::min(min_x, static_cast<int>(rect.left()));
            max_x = std::max(max_x, static_cast<int>(rect.right()));
            min_y = std::min(min_y, static_cast<int>(rect.top()));
            max_y = std::max(max_y, static_cast<int>(rect.bottom()));
        }

        auto targetRect = QRectF(min_x, min_y, max_x - min_x, max_y - min_y).marginsAdded(QMarginsF(20, 20, 20, 20));

        // FIXME This breaks as soon as the layouter call that preceded the call to this function
        // changed the scene size. If that happens, mapToScene thinks that the view is looking at (0,0)
        // and the animation jumps to (0,0) before moving to the correct target.
        auto currentRect = m_view->mapToScene(m_view->viewport()->geometry()).boundingRect();    // this has incorrect coordinates

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

    void GraphWidget::reset_focus()
    {
        m_view->setFocus();
    }

    GraphGraphicsView* GraphWidget::view()
    {
        return m_view;
    }
}    // namespace hal
