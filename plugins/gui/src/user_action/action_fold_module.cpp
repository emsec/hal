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

        GraphContext* ctx = mContextId
                ? gGraphContextManager->getContextById(mContextId)
                : gContentManager->getContextManagerWidget()->getCurrentContext();
        if (!ctx) return false;

        if (!ctx->foldModuleAction(mObject.id())) return false;
        ActionUnfoldModule* undo = new ActionUnfoldModule(mObject.id());
        undo->setContextId(mContextId);
        mUndoAction = undo;
        return UserAction::exec();
    }
}
