#include "gui/graph_widget/contexts/graph_context.h"

#include "hal_core/netlist/module.h"

#include "gui/graph_widget/contexts/graph_context_subscriber.h"
#include "gui/graph_widget/graphics_scene.h"
#include "gui/graph_widget/layouters/layouter_task.h"
#include "gui/gui_globals.h"

#include <QVector>

namespace hal
{
    static const bool lazy_updates = false;

    GraphContext::GraphContext(const QString& name, QObject* parent)
        : QObject(parent),
          m_name(name),
          m_user_update_count(0),
          m_unapplied_changes(false),
          m_scene_update_required(false),
          m_scene_update_in_progress(false)
    {
        m_timestamp = QDateTime::currentDateTime();
    }

    void GraphContext::set_layouter(GraphLayouter* layouter)
    {
        assert(layouter);

        connect(layouter, qOverload<int>(&GraphLayouter::status_update), this, qOverload<int>(&GraphContext::handle_layouter_update), Qt::ConnectionType::QueuedConnection);
        connect(layouter, qOverload<const QString&>(&GraphLayouter::status_update), this, qOverload<const QString&>(&GraphContext::handle_layouter_update), Qt::ConnectionType::QueuedConnection);

        m_layouter = layouter;
    }

    void GraphContext::set_shader(GraphShader* shader)
    {
        assert(shader);

        m_shader = shader;
    }

    GraphContext::~GraphContext()
    {
        for (GraphContextSubscriber* subscriber : m_subscribers)
            subscriber->handle_context_about_to_be_deleted();
    }

    void GraphContext::subscribe(GraphContextSubscriber* const subscriber)
    {
        assert(subscriber);
        assert(!m_subscribers.contains(subscriber));

        m_subscribers.append(subscriber);
        update();
    }

    void GraphContext::unsubscribe(GraphContextSubscriber* const subscriber)
    {
        assert(subscriber);

        m_subscribers.removeOne(subscriber);
    }

    void GraphContext::begin_change()
    {
        ++m_user_update_count;
    }

    void GraphContext::end_change()
    {
        --m_user_update_count;
        if (m_user_update_count == 0)
        {
            evaluate_changes();
            update();
        }
    }

    void GraphContext::add(const QSet<u32>& modules, const QSet<u32>& gates, PlacementHint placement)
    {
        QSet<u32> new_modules = modules - m_modules;
        QSet<u32> new_gates   = gates - m_gates;

        QSet<u32> old_modules = m_removed_modules & modules;
        QSet<u32> old_gates   = m_removed_gates & gates;

        // if we have a placement hint for the added nodes, we ignore it and leave
        // the nodes where they are (i.e. we just deschedule their removal and not
        // re-run the placement algo on them)
        m_removed_modules -= old_modules;
        m_removed_gates -= old_gates;

        m_added_modules += new_modules;
        m_added_gates += new_gates;

        for (const u32 m : new_modules)
        {
            m_module_hints.insert(placement, m);
        }
        for (const u32 g : new_gates)
        {
            m_gate_hints.insert(placement, g);
        }

        if (m_user_update_count == 0)
        {
            evaluate_changes();
            update();
        }
    }

    void GraphContext::remove(const QSet<u32>& modules, const QSet<u32>& gates)
    {
        QSet<u32> old_modules = modules & m_modules;
        QSet<u32> old_gates   = gates & m_gates;

        m_removed_modules += old_modules;
        m_removed_gates += old_gates;

        m_added_modules -= modules;
        m_added_gates -= gates;

        // We don't update m_module_hints and m_gate_hints here. Keeping elements
        // in memory is less of an issue than finding the modules/gates _by value_
        // in the maps and removing them. At the end of apply_changes() the maps are
        // cleared anyway.

        if (m_user_update_count == 0)
        {
            evaluate_changes();
            update();
        }
    }

    void GraphContext::clear()
    {
        m_removed_modules = m_modules;
        m_removed_gates   = m_gates;

        m_added_modules.clear();
        m_added_gates.clear();

        m_module_hints.clear();
        m_gate_hints.clear();

        if (m_user_update_count == 0)
        {
            evaluate_changes();
            update();
        }
    }

    void GraphContext::fold_module_of_gate(const u32 id)
    {
        auto contained_gates = m_gates + m_added_gates - m_removed_gates;
        if (contained_gates.find(id) != contained_gates.end())
        {
            auto m = g_netlist->get_gate_by_id(id)->get_module();
            QSet<u32> gates;
            QSet<u32> modules;
            for (const auto& g : m->get_gates(nullptr, true))
            {
                gates.insert(g->get_id());
            }
            for (auto sm : m->get_submodules(nullptr, true))
            {
                modules.insert(sm->get_id());
            }
            begin_change();
            remove(modules, gates);
            add({m->get_id()}, {});
            end_change();
        }
    }

    void GraphContext::unfold_module(const u32 id)
    {
        auto contained_modules = m_modules + m_added_modules - m_removed_modules;

        if (contained_modules.find(id) != contained_modules.end())
        {
            auto m = g_netlist->get_module_by_id(id);
            QSet<u32> gates;
            QSet<u32> modules;
            for (const auto& g : m->get_gates())
            {
                gates.insert(g->get_id());
            }
            for (auto sm : m->get_submodules())
            {
                modules.insert(sm->get_id());
            }

            // That would unfold the empty module into nothing, meaning there would
            // be no way back
            assert(!gates.empty() || !modules.empty());

            begin_change();
            remove({id}, {});
            add(modules, gates);
            end_change();
        }
    }

    bool GraphContext::empty() const
    {
        return m_gates.empty() && m_modules.empty();
    }

    bool GraphContext::is_showing_module(const u32 id) const
    {
        return is_showing_module(id, {}, {}, {}, {});
    }

    bool GraphContext::is_showing_module(const u32 id, const QSet<u32>& minus_modules, const QSet<u32>& minus_gates, const QSet<u32>& plus_modules, const QSet<u32>& plus_gates) const
    {
        // There are all sorts of problems when we allow this, since now any empty
        // module thinks that it is every other empty module. Blocking this,
        // however, essentially means that we must destroy all empty contexts:
        // With the block in place, a context won't recognize it's module anymore
        // once the last gate has removed, so adding a new gate won't update the
        // context.
        if (empty())
        {
            return false;
        }

        auto m = g_netlist->get_module_by_id(id);
        // TODO deduplicate
        QSet<u32> gates;
        QSet<u32> modules;
        for (const auto& g : m->get_gates())
        {
            gates.insert(g->get_id());
        }
        for (auto sm : m->get_submodules())
        {
            modules.insert(sm->get_id());
        }
        // qDebug() << "GATES" << gates;
        // qDebug() << "MINUS_GATES" << minus_gates;
        // qDebug() << "PLUS_GATES" << plus_gates;
        // qDebug() << "MGATES" << m_gates;
        return (m_gates - m_removed_gates) + m_added_gates == (gates - minus_gates) + plus_gates && (m_modules - m_removed_modules) + m_added_modules == (modules - minus_modules) + plus_modules;
    }

    bool GraphContext::is_showing_net_source(const u32 net_id) const
    {
        auto net = g_netlist->get_net_by_id(net_id);
        auto src_pins = net->get_sources();

        for(auto pin : src_pins)
        {
            if(pin->get_gate() != nullptr)
            {
                if(m_gates.contains(pin->get_gate()->get_id()))
                    return true;
            }
        }

        return false;
    }

    bool GraphContext::is_showing_net_destination(const u32 net_id) const
    {
        auto net = g_netlist->get_net_by_id(net_id);
        auto dst_pins = net->get_destinations();

        for(auto pin : dst_pins)
        {
            if(pin->get_gate() != nullptr)
            {
                if(m_gates.contains(pin->get_gate()->get_id()))
                    return true;
            }
        }

        return false;
    }

    const QSet<u32>& GraphContext::modules() const
    {
        return m_modules;
    }

    const QSet<u32>& GraphContext::gates() const
    {
        return m_gates;
    }

    const QSet<u32>& GraphContext::nets() const
    {
        return m_nets;
    }

    GraphicsScene* GraphContext::scene()
    {
        return m_layouter->scene();
    }

    QString GraphContext::name() const
    {
        return m_name;
    }

    bool GraphContext::scene_update_in_progress() const
    {
        return m_scene_update_in_progress;
    }

    void GraphContext::schedule_scene_update()
    {
        m_scene_update_required = true;

        if (lazy_updates)
            if (m_subscribers.empty())
                return;

        update();
    }

    Node GraphContext::node_for_gate(const u32 id) const
    {
        if (m_gates.contains(id))
            return Node(id, Node::Gate);

        Gate* g = g_netlist->get_gate_by_id(id);

        if (!g)
            return Node();

        Module* m = g->get_module();

        while (m)
        {
            if (m_modules.contains(m->get_id()))
                return Node(m->get_id(), Node::Module);
            m = m->get_parent_module();
        }

        return Node();
    }

    GraphLayouter* GraphContext::debug_get_layouter() const
    {
        return m_layouter;
    }

    void GraphContext::handle_layouter_update(const int percent)
    {
        for (GraphContextSubscriber* s : m_subscribers)
            s->handle_status_update(percent);
    }

    void GraphContext::handle_layouter_update(const QString& message)
    {
        for (GraphContextSubscriber* s : m_subscribers)
            s->handle_status_update(message);
    }

    void GraphContext::handle_layouter_finished()
    {
        if (m_unapplied_changes)
            apply_changes();

        if (m_scene_update_required)
        {
            start_scene_update();
        }
        else
        {
            m_shader->update();
            m_layouter->scene()->update_visuals(m_shader->get_shading());

            m_scene_update_in_progress = false;

            m_layouter->scene()->connect_all();

            for (GraphContextSubscriber* s : m_subscribers)
                s->handle_scene_available();
        }
    }

    void GraphContext::evaluate_changes()
    {
        if (!m_added_gates.isEmpty() || !m_removed_gates.isEmpty() || !m_added_modules.isEmpty() || !m_removed_modules.isEmpty())
            m_unapplied_changes = true;
    }

    void GraphContext::update()
    {
        if (m_scene_update_in_progress)
            return;

        if (m_unapplied_changes)
            apply_changes();

        if (m_scene_update_required)
            start_scene_update();
    }

    void GraphContext::apply_changes()
    {
        m_modules -= m_removed_modules;
        m_gates -= m_removed_gates;

        m_modules += m_added_modules;
        m_gates += m_added_gates;

        m_layouter->remove(m_removed_modules, m_removed_gates, m_nets);
        m_shader->remove(m_removed_modules, m_removed_gates, m_nets);

        m_nets.clear();
        for (const auto& id : m_gates)
        {
            auto g = g_netlist->get_gate_by_id(id);
            for (auto net : g->get_fan_in_nets())
            {
                //if(!net->is_unrouted() || net->is_global_input_net() || net->is_global_output_net())
                    m_nets.insert(net->get_id());
            }
            for (auto net : g->get_fan_out_nets())
            {
                //if(!net->is_unrouted() || net->is_global_input_net() || net->is_global_output_net())
                    m_nets.insert(net->get_id());
            }
        }
        for (const auto& id : m_modules)
        {
            auto m = g_netlist->get_module_by_id(id);
            for (auto net : m->get_input_nets())
            {
                m_nets.insert(net->get_id());
            }
            for (auto net : m->get_output_nets())
            {
                m_nets.insert(net->get_id());
            }
        }

        // number of placement hints is small, not performance critical
        QVector<PlacementHint> queued_hints;
        queued_hints.append(m_module_hints.uniqueKeys().toVector());
        // we can't use QSet for enum class
        for (auto h : m_gate_hints.uniqueKeys())
        {
            if (!queued_hints.contains(h))
            {
                queued_hints.append(h);
            }
        }

        for (PlacementHint h : queued_hints)
        {
            // call the placer once for each placement hint

            QSet<u32> modules_for_hint = QSet<u32>::fromList(m_module_hints.values(h));
            modules_for_hint &= m_added_modules; // may contain obsolete entries that we must filter out
            QSet<u32> gates_for_hint = QSet<u32>::fromList(m_gate_hints.values(h));
            gates_for_hint &= m_added_gates;
            m_layouter->add(modules_for_hint, gates_for_hint, m_nets, h);
        }

        m_shader->add(m_added_modules, m_added_gates, m_nets);

        m_added_modules.clear();
        m_added_gates.clear();

        m_removed_modules.clear();
        m_removed_gates.clear();

        m_module_hints.clear();
        m_gate_hints.clear();

        m_unapplied_changes     = false;
        m_scene_update_required = true;
    }

    void GraphContext::start_scene_update()
    {
        m_scene_update_required = false;
        m_scene_update_in_progress = true;

        for (GraphContextSubscriber* s : m_subscribers)
            s->handle_scene_unavailable();

        m_layouter->scene()->disconnect_all();

    //    LayouterTask* task = new LayouterTask(m_layouter);
    //    connect(task, &LayouterTask::finished, this, &GraphContext::handle_layouter_finished, Qt::ConnectionType::QueuedConnection);
    //    g_thread_pool->queue_task(task);

        m_layouter->layout();
        handle_layouter_finished();
    }

    QDateTime GraphContext::get_timestamp() const
    {
        return m_timestamp;
    }
}
