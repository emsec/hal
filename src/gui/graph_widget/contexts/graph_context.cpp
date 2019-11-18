#include "gui/graph_widget/contexts/graph_context.h"

#include "netlist/module.h"

#include "gui/graph_widget/contexts/graph_context_subscriber.h"
#include "gui/graph_widget/graphics_scene.h"
#include "gui/graph_widget/layouters/layouter_task.h"
#include "gui/gui_globals.h"

static const bool lazy_updates = false;

graph_context::graph_context(const QString& name, QObject* parent)
    : QObject(parent),
      m_name(name),
      m_user_update_count(0),
      m_unapplied_changes(false),
      m_scene_update_required(false),
      m_scene_update_in_progress(false)
{
}

void graph_context::set_layouter(graph_layouter* layouter)
{
    assert(layouter);

    connect(layouter, qOverload<int>(&graph_layouter::status_update), this, qOverload<int>(&graph_context::handle_layouter_update), Qt::ConnectionType::QueuedConnection);
    connect(layouter, qOverload<const QString&>(&graph_layouter::status_update), this, qOverload<const QString&>(&graph_context::handle_layouter_update), Qt::ConnectionType::QueuedConnection);

    m_layouter = layouter;
}

void graph_context::set_shader(graph_shader* shader)
{
    assert(shader);

    m_shader = shader;
}

graph_context::~graph_context()
{
    for (graph_context_subscriber* subscriber : m_subscribers)
        subscriber->handle_context_about_to_be_deleted();
}

void graph_context::subscribe(graph_context_subscriber* const subscriber)
{
    assert(subscriber);
    assert(!m_subscribers.contains(subscriber));

    m_subscribers.append(subscriber);
    update();
}

void graph_context::unsubscribe(graph_context_subscriber* const subscriber)
{
    assert(subscriber);

    m_subscribers.removeOne(subscriber);
}

void graph_context::begin_change()
{
    ++m_user_update_count;
}

void graph_context::end_change()
{
    --m_user_update_count;
    if (m_user_update_count == 0)
    {
        evaluate_changes();
        update();
    }
}

void graph_context::add(const QSet<u32>& modules, const QSet<u32>& gates)
{
    QSet<u32> new_modules = modules - m_modules;
    QSet<u32> new_gates   = gates - m_gates;

    QSet<u32> old_modules = m_removed_modules & modules;
    QSet<u32> old_gates   = m_removed_gates & gates;

    m_removed_modules -= old_modules;
    m_removed_gates -= old_gates;

    m_added_modules += new_modules;
    m_added_gates += new_gates;

    if (m_user_update_count == 0)
    {
        evaluate_changes();
        update();
    }
}

void graph_context::remove(const QSet<u32>& modules, const QSet<u32>& gates)
{
    QSet<u32> old_modules = modules & m_modules;
    QSet<u32> old_gates   = gates & m_gates;

    m_removed_modules += old_modules;
    m_removed_gates += old_gates;

    m_added_modules -= modules;
    m_added_gates -= gates;

    if (m_user_update_count == 0)
    {
        evaluate_changes();
        update();
    }
}

void graph_context::clear()
{
    m_removed_modules = m_modules;
    m_removed_gates   = m_gates;

    m_added_modules.clear();
    m_added_gates.clear();

    if (m_user_update_count == 0)
    {
        evaluate_changes();
        update();
    }
}

void graph_context::fold_module_of_gate(const u32 id)
{
    auto contained_gates = m_gates + m_added_gates - m_removed_gates;
    if (contained_gates.find(id) != contained_gates.end())
    {
        auto m = g_netlist->get_gate_by_id(id)->get_module();
        QSet<u32> gates;
        QSet<u32> modules;
        for (const auto& g : m->get_gates(DONT_CARE, DONT_CARE, true))
        {
            gates.insert(g->get_id());
        }
        for (const auto& sm : m->get_submodules(DONT_CARE, true))
        {
            modules.insert(sm->get_id());
        }
        begin_change();
        remove(modules, gates);
        add({m->get_id()}, {});
        end_change();
    }
}

void graph_context::unfold_module(const u32 id)
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
        for (const auto& sm : m->get_submodules())
        {
            modules.insert(sm->get_id());
        }
        begin_change();
        remove({id}, {});
        add(modules, gates);
        end_change();
    }
}

const QSet<u32>& graph_context::modules() const
{
    return m_modules;
}

const QSet<u32>& graph_context::gates() const
{
    return m_gates;
}

const QSet<u32>& graph_context::nets() const
{
    return m_nets;
}

graphics_scene* graph_context::scene()
{
    return m_layouter->scene();
}

QString graph_context::name() const
{
    return m_name;
}

bool graph_context::scene_update_in_progress() const
{
    return m_scene_update_in_progress;
}

void graph_context::schedule_scene_update()
{
    m_scene_update_required = true;

    if (lazy_updates)
        if (m_subscribers.empty())
            return;

    update();
}

bool graph_context::node_for_gate(hal::node& node, const u32 id) const
{
    if (m_gates.contains(id))
    {
        node.id   = id;
        node.type = hal::node_type::gate;
        return true;
    }

    std::shared_ptr<gate> g = g_netlist->get_gate_by_id(id);

    if (!g)
        return false;

    std::shared_ptr<module> m = g->get_module();

    while (m)
    {
        if (m_modules.contains(m->get_id()))
        {
            node.id   = m->get_id();
            node.type = hal::node_type::module;
            return true;
        }

        m = m->get_parent_module();
    }

    return false;
}

graph_layouter* graph_context::debug_get_layouter() const
{
    return m_layouter;
}

void graph_context::handle_layouter_update(const int percent)
{
    for (graph_context_subscriber* s : m_subscribers)
        s->handle_status_update(percent);
}

void graph_context::handle_layouter_update(const QString& message)
{
    for (graph_context_subscriber* s : m_subscribers)
        s->handle_status_update(message);
}

void graph_context::handle_layouter_finished()
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

        for (graph_context_subscriber* s : m_subscribers)
            s->handle_scene_available();
    }
}

void graph_context::evaluate_changes()
{
    if (!m_added_gates.isEmpty() || !m_removed_gates.isEmpty() || !m_added_modules.isEmpty() || !m_removed_modules.isEmpty())
        m_unapplied_changes = true;
}

void graph_context::update()
{
    if (m_scene_update_in_progress)
        return;

    if (m_unapplied_changes)
        apply_changes();

    if (m_scene_update_required)
        start_scene_update();
}

void graph_context::apply_changes()
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
        for (const auto& net : g->get_fan_in_nets())
        {
            m_nets.insert(net->get_id());
        }
        for (const auto& net : g->get_fan_out_nets())
        {
            m_nets.insert(net->get_id());
        }
    }
    for (const auto& id : m_modules)
    {
        auto m = g_netlist->get_module_by_id(id);
        for (const auto& net : m->get_input_nets())
        {
            m_nets.insert(net->get_id());
        }
        for (const auto& net : m->get_output_nets())
        {
            m_nets.insert(net->get_id());
        }
    }

    m_layouter->add(m_added_modules, m_added_gates, m_nets);
    m_shader->add(m_added_modules, m_added_gates, m_nets);

    m_added_modules.clear();
    m_added_gates.clear();

    m_removed_modules.clear();
    m_removed_gates.clear();

    m_unapplied_changes     = false;
    m_scene_update_required = true;
}

void graph_context::start_scene_update()
{
    m_scene_update_required = false;
    m_scene_update_in_progress = true;

    for (graph_context_subscriber* s : m_subscribers)
        s->handle_scene_unavailable();

    m_layouter->scene()->disconnect_all();

    layouter_task* task = new layouter_task(m_layouter);
    connect(task, &layouter_task::finished, this, &graph_context::handle_layouter_finished, Qt::ConnectionType::QueuedConnection);
    g_thread_pool->queue_task(task);
}
