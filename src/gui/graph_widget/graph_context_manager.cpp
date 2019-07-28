#include "gui/graph_widget/graph_context_manager.h"

#include "gui/graph_widget/contexts/dynamic_context.h"
#include "gui/graph_widget/contexts/module_context.h"
#include "gui/graph_widget/layouters/standard_graph_layouter.h"
#include "gui/graph_widget/layouters/minimal_graph_layouter.h"
#include "gui/graph_widget/shaders/module_shader.h"
#include "gui/gui_globals.h"

static const int max_module_contexts = 10; // USE SETTINGS FOR THIS

graph_context_manager::graph_context_manager()
{

}

module_context* graph_context_manager::get_module_context(const u32 id)
{
    for (module_context* c : m_module_contexts)
        if (c->get_id() == id)
            return c;

    std::shared_ptr<module> m = g_netlist->get_module_by_id(id);
    if (!m)
        return nullptr;

    module_context* c = new module_context(m);
    m_module_contexts.append(c); // USE LRU
    return c;
}

dynamic_context* graph_context_manager::add_dynamic_context(const QString& name)
{
    dynamic_context* context = new dynamic_context(name);
    m_dynamic_contexts.append(context);
    return context;
}

dynamic_context* graph_context_manager::get_dynamic_context(const QString& name)
{
    for (dynamic_context* context : m_dynamic_contexts)
        if (context->name() == name)
            return context;

    return nullptr;
}

QStringList graph_context_manager::dynamic_context_list() const
{
    QStringList list;

    for (dynamic_context* context : m_dynamic_contexts)
        list.append(context->name());

    return list;
}

void graph_context_manager::handle_module_removed(const std::shared_ptr<module> m)
{
    // REMOVE MODULE CONTEXT
    for (int i = 0; i < m_module_contexts.size(); ++i)
        if (m_module_contexts[i]->get_id() == m->get_id())
        {
            delete m_module_contexts[i];
            m_module_contexts.remove(i);
            break;
        }

    // REMOVE MODULE FROM DYNAMIC CONTEXTS
    for (dynamic_context* context : m_dynamic_contexts)
        if (context->modules().contains(m->get_id()))
            context->remove(QSet<u32>{m->get_id()}, QSet<u32>(), QSet<u32>());

    // TRIGGER RESHADE FOR ALL CONTEXTS THAT RECURSIVELY CONTAIN THE MODULE
}

void graph_context_manager::handle_module_name_changed(const std::shared_ptr<module> m) const
{
    // UPDATE MODULE CONTEXTS
    for (module_context* context : m_module_contexts)
        if (context->modules().contains(m->get_id()))
            context->request_update();

    // UPDATE DYNAMIC CONTEXTS
    for (dynamic_context* context : m_dynamic_contexts)
        if (context->modules().contains(m->get_id()))
            context->request_update();

    // TRIGGER RESHADE FOR ALL CONTEXTS THAT RECURSIVELY CONTAIN THE MODULE
}

void graph_context_manager::handle_module_submodule_added(const std::shared_ptr<module> m, const u32 added_module) const
{
    for (module_context* context : m_module_contexts)
        if (context->get_id() == m->get_id())
        {
            context->add(QSet<u32>{added_module}, QSet<u32>(), QSet<u32>());
            break;
        }

    // TRIGGER RELAYOUT FOR ALL CONTEXTS THAT RECURSIVELY CONTAIN THE MODULE
    // TRIGGER RESHADE FOR ALL CONTEXTS THAT RECURSIVELY CONTAIN THE MODULE
}

void graph_context_manager::handle_module_submodule_removed(const std::shared_ptr<module> m, const u32 removed_module) const
{
    for (module_context* context : m_module_contexts)
        if (context->get_id() == m->get_id())
        {
            context->remove(QSet<u32>{removed_module}, QSet<u32>(), QSet<u32>());
            break;
        }

    // TRIGGER RELAYOUT FOR ALL CONTEXTS THAT RECURSIVELY CONTAIN THE MODULE
    // TRIGGER RESHADE FOR ALL CONTEXTS THAT RECURSIVELY CONTAIN THE MODULE
}

void graph_context_manager::handle_module_gate_assigned(const std::shared_ptr<module> m, const u32 inserted_gate) const
{
    for (module_context* context : m_module_contexts)
        if (context->get_id() == m->get_id())
        {
            context->add(QSet<u32>(), QSet<u32>{inserted_gate}, QSet<u32>());
            break;
        }

    // TRIGGER RELAYOUT FOR ALL CONTEXTS THAT RECURSIVELY CONTAIN THE MODULE
    // TRIGGER RESHADE FOR ALL CONTEXTS THAT RECURSIVELY CONTAIN THE MODULE
}

void graph_context_manager::handle_module_gate_removed(const std::shared_ptr<module> m, const u32 removed_gate) const
{
    for (module_context* context : m_module_contexts)
        if (context->get_id() == m->get_id())
        {
            context->remove(QSet<u32>(), QSet<u32>{removed_gate}, QSet<u32>());
            break;
        }

    // TRIGGER RELAYOUT FOR ALL CONTEXTS THAT RECURSIVELY CONTAIN THE MODULE
    // TRIGGER RESHADE FOR ALL CONTEXTS THAT RECURSIVELY CONTAIN THE MODULE
}

void graph_context_manager::handle_gate_name_changed(const std::shared_ptr<gate> g) const
{
    // IF GATE IN CONTEXT REQUEST UPDATE
    for (module_context* context : m_module_contexts)
        if (context->gates().contains(g->get_id()))
            context->request_update();

    for (dynamic_context* context : m_dynamic_contexts)
        if (context->gates().contains(g->get_id()))
            context->request_update();

    // TRIGGER RESHADE FOR ALL CONTEXTS THAT RECURSIVELY CONTAIN THE MODULE
}

void graph_context_manager::handle_net_created(const std::shared_ptr<net> n) const
{
    Q_UNUSED(n)

    // CAN NETS BE CREATED WITH SRC AND DST INFORMATION ?
    // IF NO THIS EVENT DOESNT NEED TO BE HANDLED
}

void graph_context_manager::handle_net_removed(const std::shared_ptr<net> n) const
{
    // IF NET IS PART OF CONTEXT UPDATE
    for (module_context* context : m_module_contexts)
        if (context->nets().contains(n->get_id()))
            context->request_update();

    for (dynamic_context* context : m_dynamic_contexts)
        if (context->nets().contains(n->get_id()))
            context->request_update();

    // TRIGGER RESHADE FOR ALL CONTEXTS THAT RECURSIVELY CONTAIN THE MODULE
}

void graph_context_manager::handle_net_name_changed(const std::shared_ptr<net> n) const
{
    Q_UNUSED(n)

    // TRIGGER RESHADE FOR ALL CONTEXTS THAT RECURSIVELY CONTAIN THE MODULE
}

void graph_context_manager::handle_net_src_changed(const std::shared_ptr<net> n) const
{
    // IF NET IS PART OF CONTEXT UPDATE
    for (module_context* context : m_module_contexts)
        if (context->nets().contains(n->get_id()))
            context->request_update();

    for (dynamic_context* context : m_dynamic_contexts)
        if (context->nets().contains(n->get_id()))
            context->request_update();

    // TRIGGER RESHADE FOR ALL CONTEXTS THAT RECURSIVELY CONTAIN THE MODULE
}

void graph_context_manager::handle_net_dst_added(const std::shared_ptr<net> n, const u32 dst_gate_id) const
{
    Q_UNUSED(dst_gate_id)

    // IF NET OR DST GATE IS PART OF CONTEXT UPDATE
    for (module_context* context : m_module_contexts)
        if (context->nets().contains(n->get_id()) || context->gates().contains(dst_gate_id))
            context->request_update();

    for (dynamic_context* context : m_dynamic_contexts)
        if (context->nets().contains(n->get_id()) || context->gates().contains(dst_gate_id))
            context->request_update();

    // TRIGGER RESHADE FOR ALL CONTEXTS THAT RECURSIVELY CONTAIN THE MODULE
}

void graph_context_manager::handle_net_dst_removed(const std::shared_ptr<net> n, const u32 dst_gate_id) const
{
    Q_UNUSED(dst_gate_id)

    // IF NET IS PART OF CONTEXT UPDATE
    for (module_context* context : m_module_contexts)
        if (context->nets().contains(n->get_id()))
            context->request_update();

    for (dynamic_context* context : m_dynamic_contexts)
        if (context->nets().contains(n->get_id()))
            context->request_update();

    // TRIGGER RESHADE FOR ALL CONTEXTS THAT RECURSIVELY CONTAIN THE MODULE
}

graph_layouter* graph_context_manager::get_default_layouter(module_context* const context) const
{
    // USE SETTINGS + FACTORY
    //return new standard_graph_layouter(context);
    return new minimal_graph_layouter(context);
}

graph_layouter* graph_context_manager::get_default_layouter(dynamic_context* const context) const
{
    // USE SETTINGS + FACTORY
    return new standard_graph_layouter(context);
}

graph_shader* graph_context_manager::get_default_shader(module_context* const context) const
{
    // USE SETTINGS + FACTORY
    return new module_shader(context);
}

graph_shader* graph_context_manager::get_default_shader(dynamic_context* const context) const
{
    // USE SETTINGS + FACTORY
    return new module_shader(context);
}
