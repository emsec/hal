#include "gui/graph_widget/contexts/module_context.h"

module_context::module_context(const std::shared_ptr<module> m) :
    m_id(m->get_id())
{
    for (const std::shared_ptr<module>& s : m->get_submodules())
        m_modules.insert(s->get_id());

    for (const std::shared_ptr<gate>& g : m->get_gates())
        m_gates.insert(g->get_id());

    for (const std::shared_ptr<net>& n: m->get_internal_nets())
        m_nets.insert(n->get_id());

    m_layouter->add(m_modules, m_gates, m_nets);
    m_shader->add(m_modules, m_gates, m_nets);

    m_scene_update_required = true;
}

u32 module_context::get_id() const
{
    return m_id;
}
