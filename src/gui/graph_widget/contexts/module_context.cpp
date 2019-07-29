#include "gui/graph_widget/contexts/module_context.h"

#include "gui/gui_globals.h"

module_context::module_context(const std::shared_ptr<const module> m) : graph_context(context_type::module, g_graph_context_manager.get_default_layouter(this), g_graph_context_manager.get_default_shader(this)),
    m_id(m->get_id())
{
    for (const std::shared_ptr<module>& s : m->get_submodules())
        m_modules.insert(s->get_id());

    for (const std::shared_ptr<gate>& g : m->get_gates())
        m_gates.insert(g->get_id());

    for (const std::shared_ptr<net>& n: m->get_internal_nets())
        m_nets.insert(n->get_id());

    // DEBUG CODE
    for (const std::shared_ptr<gate>& g : m->get_gates())
    {
        for (std::shared_ptr<net> n : g->get_fan_in_nets())
            if (n->is_unrouted())
                m_nets.insert(n->get_id());

        for (std::shared_ptr<net> n : g->get_fan_out_nets())
            if (n->is_unrouted())
                m_nets.insert(n->get_id());
    }

    m_layouter->add(m_modules, m_gates, m_nets);
    m_shader->add(m_modules, m_gates, m_nets);

    m_scene_update_required = true;
}

u32 module_context::get_id() const
{
    return m_id;
}

//const QSet<u32>& module_context::modules() const
//{
//    return m_modules;
//}

//const QSet<u32>& module_context::gates() const
//{
//    return m_gates;
//}

const QSet<u32>& module_context::internal_nets() const
{
    return m_internal_nets;
}

const QSet<u32>& module_context::global_io_nets() const
{
    return m_global_io_nets;
}

const QSet<u32>& module_context::local_io_nets() const
{
    return m_local_io_nets;
}
