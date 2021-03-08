#include "gui/user_action/action_fold_module.h"
#include "gui/gui_globals.h"
#include "gui/content_manager/content_manager.h"
#include "gui/context_manager_widget/context_manager_widget.h"
#include "gui/graph_tab_widget/graph_tab_widget.h"
#include "gui/graph_widget/contexts/graph_context.h"
#include "gui/user_action/action_unfold_module.h"

namespace hal
{
    ActionFoldModuleFactory::ActionFoldModuleFactory()
        : UserActionFactory("FoldModule") {;}

    ActionFoldModuleFactory* ActionFoldModuleFactory::sFactory = new ActionFoldModuleFactory;

    UserAction* ActionFoldModuleFactory::newAction() const
    {
        return new ActionFoldModule;
    }

    ActionFoldModule::ActionFoldModule(u32 moduleId)
        : mContextId(0)
    {
        if (moduleId)
            setObject(UserActionObject(moduleId,UserActionObjectType::Module));
    }

    QString ActionFoldModule::tagname() const
    {
        return ActionFoldModuleFactory::sFactory->tagname();
    }

    bool ActionFoldModule::exec()
    {
        if (mObject.type() != UserActionObjectType::Module) return false;
        Module* m = gNetlist->get_module_by_id(mObject.id());
        if (!m) return false;

        GraphContext* ctx = mContextId
                ? gGraphContextManager->getContextById(mContextId)
                : gContentManager->getContextManagerWidget()->getCurrentContext();
        if (!ctx) return false;

        if (!ctx->foldModuleAction(mObject.id())) return false;
        ActionUnfoldModule* undo = new ActionUnfoldModule(mObject.id());
        undo->setContextId(mContextId);
        PlacementHint plc(PlacementHint::GridPosition);
        for (const Gate* g : m->get_gates())
        {
            Node nd(g->get_id(),Node::Gate);
            auto it = ctx->getLayouter()->nodeToPositionMap().find(nd);
            if (it!=ctx->getLayouter()->nodeToPositionMap().end())
                plc.addGridPosition(nd,it.value());
        }
        for (auto sm : m->get_submodules())
        {
            Node nd(sm->get_id(),Node::Module);
            auto it = ctx->getLayouter()->nodeToPositionMap().find(nd);
            if (it!=ctx->getLayouter()->nodeToPositionMap().end())
                plc.addGridPosition(nd,it.value());
        }
        undo->setPlacementHint(plc);
        mUndoAction = undo;
        return UserAction::exec();
    }
}
