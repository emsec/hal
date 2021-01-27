#include "gui/user_action/action_unfold_module.h"
#include "gui/gui_globals.h"
#include "gui/content_manager/content_manager.h"
#include "gui/context_manager_widget/context_manager_widget.h"
#include "gui/graph_tab_widget/graph_tab_widget.h"
#include "gui/graph_widget/contexts/graph_context.h"
#include "gui/user_action/action_fold_module.h"

namespace hal
{
    ActionUnfoldModuleFactory::ActionUnfoldModuleFactory()
        : UserActionFactory("UnfoldModule") {;}

    ActionUnfoldModuleFactory* ActionUnfoldModuleFactory::sFactory = new ActionUnfoldModuleFactory;

    UserAction* ActionUnfoldModuleFactory::newAction() const
    {
        return new ActionUnfoldModule;
    }

    QString ActionUnfoldModule::tagname() const
    {
        return ActionUnfoldModuleFactory::sFactory->tagname();
    }

    ActionUnfoldModule::ActionUnfoldModule(u32 moduleId)
        : mContextId(0)
    {
        if (moduleId)
            setObject(UserActionObject(moduleId,UserActionObjectType::Module));
    }

    void ActionUnfoldModule::exec()
    {
        if (mObject.type() != UserActionObjectType::Module) return;

        Module* m = gNetlist->get_module_by_id(mObject.id());
        if (m->get_gates().empty() && m->get_submodules().empty())
            return;

        GraphContext* currentContext = mContextId
                ? gGraphContextManager->getContextById(mContextId)
                : gContentManager->getContextManagerWidget()->getCurrentContext();
        if (!currentContext) return;

        ActionFoldModule* undo = new ActionFoldModule(mObject.id());
        undo->setContextId(mContextId);
        mUndoAction = undo;
        execInternal(m,currentContext);
        UserAction::exec();
    }

    void ActionUnfoldModule::execInternal(Module *m, GraphContext* currentContext)
    {
        if (currentContext->gates().isEmpty() &&
                currentContext->modules() == QSet<u32>({mObject.id()}))
        {
            currentContext->unfoldModule(mObject.id());
            return;
        }

        QSet<u32> gats;
        QSet<u32> mods;
        for (const auto& g : m->get_gates())
        {
            gats.insert(g->get_id());
        }
        for (auto sm : m->get_submodules())
        {
            mods.insert(sm->get_id());
        }

        for (const auto& ctx : gGraphContextManager->getContexts())
        {
            if ((ctx->gates().isEmpty() && ctx->modules() == QSet<u32>({mObject.id()})) ||
                    (ctx->modules() == mods && ctx->gates() == gats))
            {
                gContentManager->getGraphTabWidget()->showContext(ctx);
                return;
            }
        }

        auto ctx = gGraphContextManager->createNewContext(QString::fromStdString(m->get_name()));
        ctx->add(mods, gats);
    }
}
