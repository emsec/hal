#include "gui/gui_globals.h"
#include "gui/user_action/action_unfold_module.h"
#include "gui/content_manager/content_manager.h"
#include "gui/graph_widget/contexts/graph_context.h"
#include "gui/context_manager_widget/context_manager_widget.h"
#include "gui/graph_tab_widget/graph_tab_widget.h"

namespace hal
{
    ActionUnfoldModule::ActionUnfoldModule()
      : UserAction(UserActionManager::UnfoldModule) {;}

    void ActionUnfoldModule::exec()
    {
        if (!mObjectId) return;

        auto m = gNetlist->get_module_by_id(mObjectId);
        if (m->get_gates().empty() && m->get_submodules().empty())
            return;

        GraphContext* currentContext =
                gContentManager->getContextManagerWidget()->getCurrentContext();
        if (!currentContext) return;

        execInternal(m,currentContext);
        UserAction::exec();
    }

    void ActionUnfoldModule::execInternal(Module *m, GraphContext* currentContext)
    {
        if (currentContext->gates().isEmpty() &&
                currentContext->modules() == QSet<u32>({mObjectId}))
        {
            currentContext->unfoldModule(mObjectId);
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

        for (const auto& ctx : gGraphContextManager->getContexts())
        {
            if ((ctx->gates().isEmpty() && ctx->modules() == QSet<u32>({mObjectId})) || (ctx->modules() == module_ids && ctx->gates() == gate_ids))
            {
                gContentManager->getGraphTabWidget()->showContext(ctx);
                return;
            }
        }

        auto ctx = gGraphContextManager->createNewContext(QString::fromStdString(m->get_name()));
        ctx->add(module_ids, gate_ids);
    }

    void ActionUnfoldModule::writeToXml(QXmlStreamWriter& xmlOut) const
    {
        xmlOut.writeTextElement("id", QString::number(mObjectId));
    }

    void ActionUnfoldModule::readFromXml(QXmlStreamReader& xmlIn)
    {
        while (xmlIn.readNextStartElement())
        {
            if (xmlIn.name() == "id")
                setObjectId(xmlIn.readElementText().toInt());
        }
    }
}
