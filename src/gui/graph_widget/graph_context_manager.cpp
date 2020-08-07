#include "gui/graph_widget/graph_context_manager.h"

#include "gui/graph_widget/contexts/graph_context.h"
#include "gui/graph_widget/layouters/physical_graph_layouter.h"
#include "gui/graph_widget/layouters/standard_graph_layouter.h"
#include "gui/graph_widget/shaders/module_shader.h"
#include "gui/gui_globals.h"
#include "netlist/gate.h"
#include "netlist/module.h"
#include "netlist/netlist.h"

#include <QDateTime>

namespace hal
{
    GraphContextManager::GraphContextManager()
    {
    }

    GraphContext* GraphContextManager::create_new_context(const QString& name)
    {
        GraphContext* context = new GraphContext(name);
        context->set_layouter(get_default_layouter(context));
        context->set_shader(get_default_shader(context));
        m_graph_contexts.append(context);
        Q_EMIT context_created(context);
        return context;
    }

    void GraphContextManager::rename_graph_context(GraphContext* ctx, const QString& new_name)
    {
        ctx->m_name = new_name;

        Q_EMIT context_renamed(ctx);
    }

    void GraphContextManager::delete_graph_context(GraphContext* ctx)
    {
        Q_EMIT deleting_context(ctx);
        m_graph_contexts.remove(m_graph_contexts.indexOf(ctx));
        delete ctx;
    }

    QVector<GraphContext*> GraphContextManager::get_contexts() const
    {
        return m_graph_contexts;
    }

    bool GraphContextManager::context_with_name_exists(const QString& name) const
    {
        for (const auto& ctx : m_graph_contexts)
        {
            if (ctx->name() == name)
            {
                return true;
            }
        }
        return false;
    }

    void GraphContextManager::handle_module_removed(Module* m)
    {
        for (GraphContext* context : m_graph_contexts)
            if (context->modules().contains(m->get_id()))
                context->remove({m->get_id()}, {});
    }

    void GraphContextManager::handle_module_name_changed(Module* m) const
    {
        for (GraphContext* context : m_graph_contexts)
            if (context->modules().contains(m->get_id()))
                context->schedule_scene_update();
    }

    void GraphContextManager::handle_module_color_changed(Module* m) const
    {
        auto gates = m->get_gates();
        QSet<u32> gateIDs;
        for (auto g : gates)
            gateIDs.insert(g->get_id());
        for (GraphContext* context : m_graph_contexts)
            if (context->modules().contains(m->get_id())    // contains module
                || context->gates().intersects(gateIDs))    // contains gate from module
                context->schedule_scene_update();
        // a context can contain a gate from a module if it is showing the module
        // or if it's showing a parent and the module is unfolded
    }

    void GraphContextManager::handle_module_submodule_added(Module* m, const u32 added_module) const
    {
        for (GraphContext* context : m_graph_contexts)
            if (context->is_showing_module(m->get_id(), {added_module}, {}, {}, {}))
                context->add({added_module}, {});
    }

    void GraphContextManager::handle_module_submodule_removed(Module* m, const u32 removed_module)
    {
        // FIXME this also triggers on module deletion (not only moving)
        // and collides with handle_module_removed
        for (GraphContext* context : m_graph_contexts)
            if (context->is_showing_module(m->get_id(), {}, {}, {removed_module}, {}))
            {
                context->remove({removed_module}, {});
                if (context->empty())
                {
                    delete_graph_context(context);
                }
            }
    }

    void GraphContextManager::handle_module_gate_assigned(Module* m, const u32 inserted_gate) const
    {
        for (GraphContext* context : m_graph_contexts)
            if (context->is_showing_module(m->get_id(), {}, {inserted_gate}, {}, {}))
                context->add({}, {inserted_gate});
    }

    void GraphContextManager::handle_module_gate_removed(Module* m, const u32 removed_gate)
    {
        for (GraphContext* context : m_graph_contexts)
        {
            if (context->is_showing_module(m->get_id(), {}, {}, {}, {removed_gate}))
            {
                context->remove({}, {removed_gate});
                if (context->empty())
                {
                    delete_graph_context(context);
                }
            }
            // if a module is unfolded, then the gate is not deleted from the view
            // but the color of the gate changes to its new parent's color
            else if (context->gates().contains(removed_gate))
                context->schedule_scene_update();
        }
    }

    void GraphContextManager::handle_gate_name_changed(Gate* g) const
    {
        for (GraphContext* context : m_graph_contexts)
            if (context->gates().contains(g->get_id()))
                context->schedule_scene_update();
    }

    void GraphContextManager::handle_net_created(Net* n) const
    {
        Q_UNUSED(n)

        // CAN NETS BE CREATED WITH SRC AND DST INFORMATION ?
        // IF NOT THIS EVENT DOESNT NEED TO BE HANDLED
    }

    void GraphContextManager::handle_net_removed(Net* n) const
    {
        for (GraphContext* context : m_graph_contexts)
            if (context->nets().contains(n->get_id()))
                context->schedule_scene_update();
    }

    void GraphContextManager::handle_net_name_changed(Net* n) const
    {
        Q_UNUSED(n)

        // TRIGGER RESHADE FOR ALL CONTEXTS THAT RECURSIVELY CONTAIN THE NET
    }

    void GraphContextManager::handle_net_source_added(Net* n, const u32 src_gate_id) const
    {
        for (GraphContext* context : m_graph_contexts)
        {
            if (context->nets().contains(n->get_id()) || context->gates().contains(src_gate_id))
            {
                // forcibly apply changes since nets need to be recalculated
                context->apply_changes();
                context->schedule_scene_update();
            }
        }
    }

    void GraphContextManager::handle_net_source_removed(Net* n, const u32 src_gate_id) const
    {
        UNUSED(src_gate_id);

        for (GraphContext* context : m_graph_contexts)
        {
            if (context->nets().contains(n->get_id()))
            {
                // forcibly apply changes since nets need to be recalculated
                context->apply_changes();
                context->schedule_scene_update();
            }
        }
    }

    void GraphContextManager::handle_net_destination_added(Net* n, const u32 dst_gate_id) const
    {
        for (GraphContext* context : m_graph_contexts)
        {
            if (context->nets().contains(n->get_id()) || context->gates().contains(dst_gate_id))
            {
                // forcibly apply changes since nets need to be recalculated
                context->apply_changes();
                context->schedule_scene_update();
            }
        }
    }

    void GraphContextManager::handle_net_destination_removed(Net* n, const u32 dst_gate_id) const
    {
        UNUSED(dst_gate_id);

        for (GraphContext* context : m_graph_contexts)
        {
            if (context->nets().contains(n->get_id()))
            {
                // forcibly apply changes since nets need to be recalculated
                context->apply_changes();
                context->schedule_scene_update();
            }
        }
    }

    void GraphContextManager::handle_marked_global_input(u32 net_id)
    {
        for (GraphContext* context : m_graph_contexts)
        {
            if (context->nets().contains(net_id) || context->is_showing_net_destination(net_id))
            {
                context->apply_changes();
                context->schedule_scene_update();
            }
        }
    }

    void GraphContextManager::handle_marked_global_output(u32 net_id)
    {
        for (GraphContext* context : m_graph_contexts)
        {
            if (context->nets().contains(net_id) || context->is_showing_net_source(net_id))
            {
                context->apply_changes();
                context->schedule_scene_update();
            }
        }
    }

    void GraphContextManager::handle_unmarked_global_input(u32 net_id)
    {
        for (GraphContext* context : m_graph_contexts)
            if (context->nets().contains(net_id))
            {
                context->apply_changes();
                context->schedule_scene_update();
            }
    }

    void GraphContextManager::handle_unmarked_global_output(u32 net_id)
    {
        for (GraphContext* context : m_graph_contexts)
            if (context->nets().contains(net_id))
            {
                context->apply_changes();
                context->schedule_scene_update();
            }
    }

    GraphLayouter* GraphContextManager::get_default_layouter(GraphContext* const context) const
    {
        // USE SETTINGS + FACTORY
        return new StandardGraphLayouter(context);
    }

    GraphShader* GraphContextManager::get_default_shader(GraphContext* const context) const
    {
        // USE SETTINGS + FACTORY
        return new ModuleShader(context);
    }
}
