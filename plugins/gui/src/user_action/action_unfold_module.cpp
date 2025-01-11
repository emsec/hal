#include "gui/user_action/action_unfold_module.h"
#include "gui/gui_globals.h"
#include "gui/content_manager/content_manager.h"
#include "gui/context_manager_widget/context_manager_widget.h"
#include "gui/graph_tab_widget/graph_tab_widget.h"
#include "gui/graph_widget/contexts/graph_context.h"
#include "gui/user_action/action_fold_module.h"
#include <QDebug>

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
        : mContextId(0), mPlacementHint(PlacementHint::Standard)
    {
        if (moduleId)
            setObject(UserActionObject(moduleId,UserActionObjectType::Module));
    }

    void ActionUnfoldModule::setObject(const UserActionObject &obj)
    {
        switch (obj.type())
        {
        case UserActionObjectType::None:
        case UserActionObjectType::Module:
            UserAction::setObject(obj);
            break;
        case UserActionObjectType::ContextView:
            mContextId = obj.id();
            break;
        default:
            qDebug() << "ActionUnfoldModule warning: invalid object type" << obj.type();
            break;
        }
    }

    bool ActionUnfoldModule::exec()
    {
        if (mObject.type() != UserActionObjectType::Module) return false;

        Module* m = gNetlist->get_module_by_id(mObject.id());
        if (m->get_gates().empty() && m->get_submodules().empty())
            return false;

        GraphContext* ctx = mContextId
                ? gGraphContextManager->getContextById(mContextId)
                : gContentManager->getContextManagerWidget()->getCurrentContext();
        if (!ctx) return false;

        ActionFoldModule* undo = new ActionFoldModule(mObject.id());
        undo->setContextId(mContextId);
        PlacementHint plc(PlacementHint::GridPosition);
        Node nd(mObject.id(),Node::Module);
        NetLayoutPoint pos =  ctx->getLayouter()->positonForNode(nd);
        if (!pos.isUndefined())
        {
            plc.addGridPosition(nd,pos);
            undo->setPlacementHint(plc);
        }
        mUndoAction = undo;
        execInternal(m,ctx);
        return UserAction::exec();
    }

    void ActionUnfoldModule::execInternal(Module *m, GraphContext* currentContext)
    {
        // only module shown in context
        if (currentContext->gates().isEmpty() &&
                currentContext->modules() == QSet<u32>({mObject.id()}))
        {
            currentContext->unfoldModule(mObject.id(),mPlacementHint);
            return;
        }

        // module to unfold and other boxes shown
        if (currentContext->modules().contains(mObject.id()))
        {
            currentContext->unfoldModule(mObject.id(),mPlacementHint);
            return;
        }

        QSet<u32> gats;
        QSet<u32> mods;
        for (const Gate* g : m->get_gates())
        {
            gats.insert(g->get_id());
        }
        for (const Module* sm : m->get_submodules())
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
        ctx->add(mods, gats, mPlacementHint);
    }
}
