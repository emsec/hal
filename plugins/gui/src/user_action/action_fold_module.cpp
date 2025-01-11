#include "gui/user_action/action_fold_module.h"
#include "gui/user_action/action_add_items_to_object.h"
#include "gui/user_action/action_remove_items_from_object.h"
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
        : mContextId(0), mPlacementHint(PlacementHint::Standard)
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

        Node moduleToFold(mObject.id(),Node::Module);
        NetLayoutPoint moduleAlreadyPositioned = ctx->getLayouter()->positonForNode(moduleToFold);
        if (!moduleAlreadyPositioned.isUndefined())
            return false; // nothing to do, module already folded

        QPoint polePosition;

        QSet<u32> gats;
        for (const auto& g : m->get_gates(nullptr, true))
        {
            u32 gid = g->get_id();
            NetLayoutPoint pos = ctx->getLayouter()->positonForNode(Node(gid,Node::Gate));
            if (pos.isUndefined()) continue;
            if (gats.isEmpty() || pos.x() < polePosition.x() || pos.y() < polePosition.y())
                polePosition = pos;
            gats.insert(gid);
        }

        QSet<u32> mods;
        for (auto sm : m->get_submodules(nullptr, true))
        {
            u32 mid = sm->get_id();
            NetLayoutPoint pos = ctx->getLayouter()->positonForNode(Node(mid,Node::Module));
            if (pos.isUndefined()) continue;
            if ( (gats.isEmpty()&&mods.isEmpty()) ||
                    pos.x() < polePosition.x() || pos.y() < polePosition.y())
                polePosition = pos;
            mods.insert(mid);
        }

        ActionRemoveItemsFromObject* actr = new ActionRemoveItemsFromObject(mods,gats);
        actr->setObject(UserActionObject(ctx->id(),UserActionObjectType::ContextView));
        addAction(actr);

        ActionAddItemsToObject* acta = new ActionAddItemsToObject({mObject.id()});
        acta->setObject(UserActionObject(ctx->id(),UserActionObjectType::ContextView));
        PlacementHint plc(PlacementHint::GridPosition);
        plc.addGridPosition(moduleToFold,polePosition);
        acta->setPlacementHint(plc);
        addAction(acta);

        return UserActionCompound::exec();
    }
}
