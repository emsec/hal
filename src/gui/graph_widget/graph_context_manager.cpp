#include "gui/graph_widget/graph_context_manager.h"

#include "gui/graph_widget/contexts/cone_context.h"
#include "gui/graph_widget/contexts/dynamic_context.h"
#include "gui/graph_widget/contexts/module_context.h"
#include "gui/graph_widget/layouters/minimal_graph_layouter.h"
#include "gui/graph_widget/layouters/standard_graph_layouter.h"
#include "gui/graph_widget/layouters/standard_graph_layouter_v2.h"
#include "gui/graph_widget/shaders/module_shader.h"
#include "gui/gui_globals.h"

static const int max_module_contexts = 10;    // USE SETTINGS FOR THIS

graph_context_manager::graph_context_manager() : m_top(nullptr)
{
}

dynamic_context* graph_context_manager::add_dynamic_context(const QString& name)
{
    for (dynamic_context* context : m_dynamic_contexts)
    {
        if (context->name() == name)
        {
            return nullptr;
        }
    }

    dynamic_context* context = new dynamic_context(name);
    m_dynamic_contexts.append(context);
    Q_EMIT context_created(context);
    return context;
}

dynamic_context* graph_context_manager::get_dynamic_context(const QString& name)
{
    for (dynamic_context* context : m_dynamic_contexts)
        if (context->name() == name)
            return context;

    return nullptr;
}

bool graph_context_manager::rename_dynamic_context(const QString& old_name, const QString& new_name)
{
    dynamic_context* to_change = nullptr;
    for (dynamic_context* context : m_dynamic_contexts)
    {
        if (context->name() != old_name && context->name() == new_name)
        {
            return false;
        }
        if (context->name() == old_name)
        {
            to_change = context;
        }
    }
    if (to_change == nullptr)
        return false;

    to_change->m_name = new_name;

    Q_EMIT context_renamed(to_change);

    return true;
}

bool graph_context_manager::remove_dynamic_context(const QString& name)
{
    for (int i = 0; i < m_dynamic_contexts.size(); ++i)
    {
        auto context = m_dynamic_contexts[i];
        if (m_dynamic_contexts[i]->name() == name)
        {
            m_dynamic_contexts.remove(i);
            Q_EMIT context_removed(context);
            return true;
        }
    }
    return false;
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
    // REMOVE MODULE FROM DYNAMIC CONTEXTS
    for (dynamic_context* context : m_dynamic_contexts)
        if (context->modules().contains(m->get_id()))
            context->remove(QSet<u32>{m->get_id()}, QSet<u32>(), QSet<u32>());

    // TRIGGER RESHADE FOR ALL CONTEXTS THAT RECURSIVELY CONTAIN THE MODULE
}

void graph_context_manager::handle_module_name_changed(const std::shared_ptr<module> m) const
{
    // UPDATE DYNAMIC CONTEXTS
    for (dynamic_context* context : m_dynamic_contexts)
        if (context->modules().contains(m->get_id()))
            context->request_update();

    // TRIGGER RESHADE FOR ALL CONTEXTS THAT RECURSIVELY CONTAIN THE MODULE
}

void graph_context_manager::handle_module_submodule_added(const std::shared_ptr<module> m, const u32 added_module) const
{
    // TRIGGER RELAYOUT FOR ALL CONTEXTS THAT RECURSIVELY CONTAIN THE MODULE
    // TRIGGER RESHADE FOR ALL CONTEXTS THAT RECURSIVELY CONTAIN THE MODULE
}

void graph_context_manager::handle_module_submodule_removed(const std::shared_ptr<module> m, const u32 removed_module) const
{
    // TRIGGER RELAYOUT FOR ALL CONTEXTS THAT RECURSIVELY CONTAIN THE MODULE
    // TRIGGER RESHADE FOR ALL CONTEXTS THAT RECURSIVELY CONTAIN THE MODULE
}

void graph_context_manager::handle_module_gate_assigned(const std::shared_ptr<module> m, const u32 inserted_gate) const
{

    // TRIGGER RELAYOUT FOR ALL CONTEXTS THAT RECURSIVELY CONTAIN THE MODULE
    // TRIGGER RESHADE FOR ALL CONTEXTS THAT RECURSIVELY CONTAIN THE MODULE
}

void graph_context_manager::handle_module_gate_removed(const std::shared_ptr<module> m, const u32 removed_gate) const
{
    // TRIGGER RELAYOUT FOR ALL CONTEXTS THAT RECURSIVELY CONTAIN THE MODULE
    // TRIGGER RESHADE FOR ALL CONTEXTS THAT RECURSIVELY CONTAIN THE MODULE
}

void graph_context_manager::handle_gate_name_changed(const std::shared_ptr<gate> g) const
{
    // IF GATE IN CONTEXT REQUEST UPDATE
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
    for (dynamic_context* context : m_dynamic_contexts)
        if (context->nets().contains(n->get_id()))
            context->request_update();

    // TRIGGER RESHADE FOR ALL CONTEXTS THAT RECURSIVELY CONTAIN THE MODULE
}

void graph_context_manager::handle_net_dst_added(const std::shared_ptr<net> n, const u32 dst_gate_id) const
{
    Q_UNUSED(dst_gate_id)

    // IF NET OR DST GATE IS PART OF CONTEXT UPDATE
    for (dynamic_context* context : m_dynamic_contexts)
        if (context->nets().contains(n->get_id()) || context->gates().contains(dst_gate_id))
            context->request_update();

    // TRIGGER RESHADE FOR ALL CONTEXTS THAT RECURSIVELY CONTAIN THE MODULE
}

void graph_context_manager::handle_net_dst_removed(const std::shared_ptr<net> n, const u32 dst_gate_id) const
{
    Q_UNUSED(dst_gate_id)

    // IF NET IS PART OF CONTEXT UPDATE
    for (dynamic_context* context : m_dynamic_contexts)
        if (context->nets().contains(n->get_id()))
            context->request_update();

    // TRIGGER RESHADE FOR ALL CONTEXTS THAT RECURSIVELY CONTAIN THE MODULE
}

graph_layouter* graph_context_manager::get_default_layouter(module_context* const context) const
{
    // USE SETTINGS + FACTORY
    return new standard_graph_layouter(context);
    //return new standard_graph_layouter_v2(context);
    //return new minimal_graph_layouter(context);
}

graph_layouter* graph_context_manager::get_default_layouter(cone_context* const context) const
{
    // USE SETTINGS + FACTORY
    return new standard_graph_layouter(context);
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

graph_shader* graph_context_manager::get_default_shader(cone_context* const context) const
{
    // USE SETTINGS + FACTORY
    return new module_shader(context);
}

graph_shader* graph_context_manager::get_default_shader(dynamic_context* const context) const
{
    // USE SETTINGS + FACTORY
    return new module_shader(context);
}

graph_context* graph_context_manager::get_context()
{
    return m_top;
}

void graph_context_manager::create_top_context()
{
    m_top = g_graph_context_manager.add_dynamic_context(QString::fromStdString(g_netlist->get_top_module()->get_name()));
    QSet<u32> global_nets;
    for (auto& net : g_netlist->get_global_inout_nets())
    {
        global_nets.insert(net->get_id());
    }
    for (auto& net : g_netlist->get_global_input_nets())
    {
        global_nets.insert(net->get_id());
    }
    for (auto& net : g_netlist->get_global_output_nets())
    {
        global_nets.insert(net->get_id());
    }
    m_top->add({1}, {}, global_nets);
}
