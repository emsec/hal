#include "gui/user_action/action_delete_object.h"

#include "gui/graph_widget/contexts/graph_context.h"
#include "gui/context_manager_widget/models/context_tree_model.h"
#include "gui/graph_widget/layout_locker.h"
#include "gui/grouping/grouping_manager_widget.h"
#include "gui/grouping/grouping_table_model.h"
#include "gui/gui_globals.h"
#include "gui/module_model/module_model.h"
#include "gui/module_model/module_color_manager.h"
#include "gui/user_action/action_add_items_to_object.h"
#include "gui/user_action/action_create_object.h"
#include "gui/user_action/action_set_object_color.h"
#include "gui/user_action/action_set_object_type.h"
#include "gui/user_action/user_action_compound.h"

namespace hal
{
    ActionDeleteObjectFactory::ActionDeleteObjectFactory() : UserActionFactory("DeleteObject")
    {
        ;
    }

    ActionDeleteObjectFactory* ActionDeleteObjectFactory::sFactory = new ActionDeleteObjectFactory;

    UserAction* ActionDeleteObjectFactory::newAction() const
    {
        return new ActionDeleteObject;
    }

    QString ActionDeleteObject::tagname() const
    {
        return ActionDeleteObjectFactory::sFactory->tagname();
    }

    bool ActionDeleteObject::exec()
    {
        Module* mod;
        Gate* gat;
        Net* net;
        GraphContext* ctx;
        ContextDirectory* ctxDir;

        LayoutLocker llock;

        switch (mObject.type())
        {
            case UserActionObjectType::PinGroup: {
                mod            = gNetlist->get_module_by_id(mParentObject.id());
                auto* pinGroup = mod->get_pin_group_by_id(mObject.id());
                if (mod != nullptr && pinGroup != nullptr)
                {
                    QSet<u32> pins;
                    for (const auto& pin : pinGroup->get_pins())
                    {
                        pins.insert(pin->get_id());
                    }
                    UserActionCompound* act = new UserActionCompound;
                    act->setUseCreatedObject();
                    ActionCreateObject* actCreate = new ActionCreateObject(UserActionObjectType::PinGroup, QString::fromStdString(pinGroup->get_name()));
                    actCreate->setParentObject(mParentObject);
                    act->addAction(actCreate);
                    act->addAction(new ActionAddItemsToObject(QSet<u32>(), QSet<u32>(), QSet<u32>(), pins));    //setting of obj/parentobj handled in compound
                    mUndoAction = act;
                    auto res    = mod->delete_pin_group(pinGroup);
                }
                else
                    return false;
            }
            break;
            case UserActionObjectType::Module:
                mod = gNetlist->get_module_by_id(mObject.id());
                if (mod)
                {
                    UserActionCompound* act = new UserActionCompound;
                    act->setUseCreatedObject();
                    ActionCreateObject* actCreate = new ActionCreateObject(UserActionObjectType::Module, QString::fromStdString(mod->get_name()));
                    actCreate->setParentId(mod->get_parent_module()->get_id());
                    act->addAction(actCreate);
                    act->addAction(new ActionSetObjectType(QString::fromStdString(mod->get_type())));
                    act->addAction(new ActionSetObjectColor(gNetlistRelay->getModuleColorManager()->moduleColor(mod->get_id())));
                    QSet<u32> mods, gats;
                    for (Gate* g : mod->get_gates())
                        gats.insert(g->get_id());
                    for (Module* sm : mod->get_submodules())
                        mods.insert(sm->get_id());
                    if (!mods.isEmpty() || !gats.isEmpty())
                        act->addAction(new ActionAddItemsToObject(mods, gats));
                    mUndoAction = act;
                    gNetlist->delete_module(mod);
                }
                else
                    return false;
                break;
            case UserActionObjectType::Gate:
                gat = gNetlist->get_gate_by_id(mObject.id());
                if (gat)
                    gNetlist->delete_gate(gat);
                else
                    return false;
                break;
            case UserActionObjectType::Net:
                net = gNetlist->get_net_by_id(mObject.id());
                if (net)
                    gNetlist->delete_net(net);
                else
                    return false;
                break;
            case UserActionObjectType::Grouping: {
                GroupingTableModel* grpModel = gContentManager->getGroupingManagerWidget()->getModel();
                for (int irow = 0; irow < grpModel->rowCount(); irow++)
                {
                    const GroupingTableEntry& grpEntry = grpModel->groupingAt(irow);
                    if (grpEntry.id() == mObject.id())
                    {
                        ActionCreateObject* undoCreate = new ActionCreateObject(UserActionObjectType::Grouping, grpEntry.name());
                        undoCreate->setObject(mObject);
                        QSet<u32> mods, gats, nets;
                        for (u32 id : grpEntry.grouping()->get_module_ids())
                            mods.insert(id);
                        for (u32 id : grpEntry.grouping()->get_gate_ids())
                            gats.insert(id);
                        for (u32 id : grpEntry.grouping()->get_net_ids())
                            nets.insert(id);
                        ActionSetObjectColor* undoColor = new ActionSetObjectColor(grpEntry.color());
                        undoColor->setObject(mObject);
                        ActionAddItemsToObject* undoPopulate = new ActionAddItemsToObject(mods, gats, nets);
                        undoPopulate->setObject(mObject);
                        UserActionCompound* act = new UserActionCompound;
                        act->setUseCreatedObject();
                        act->addAction(undoCreate);
                        act->addAction(undoColor);
                        act->addAction(undoPopulate);
                        mUndoAction = act;
                        grpModel->removeRows(irow);
                        break;
                    }
                }
            }
            break;
            case UserActionObjectType::ContextView:
                ctx = gGraphContextManager->getContextById(mObject.id());
                if (ctx)
                {
                    UserActionCompound* act = new UserActionCompound;
                    act->setUseCreatedObject();
                    ActionCreateObject* actCreate = new ActionCreateObject(UserActionObjectType::ContextView, ctx->name());
                    actCreate->setObject(UserActionObject(ctx->id(),UserActionObjectType::ContextView));
                    actCreate->setParentId(gGraphContextManager->getParentId(ctx->id(),false));
                    act->addAction(actCreate);
                    act->addAction(new ActionAddItemsToObject(ctx->modules(), ctx->gates()));
                    act->setParentObject(mParentObject);

                    mUndoAction = act;
                    gGraphContextManager->deleteGraphContext(ctx);
                }
                else
                    return false;
                break;
            case UserActionObjectType::ContextDir:
                ctxDir = gGraphContextManager->getDirectoryById(mObject.id());
                if (ctxDir)
                {
                    if (gGraphContextManager->getContextTreeModel()->getDirectory(ctxDir->id())->getChildCount() != 0) {
                        mUndoAction = nullptr;
                    } else {
                        UserActionCompound* act = new UserActionCompound;
                        act->setUseCreatedObject();
                        ActionCreateObject* actCreate = new ActionCreateObject(UserActionObjectType::ContextDir, ctxDir->name());
                        actCreate->setObject(UserActionObject(ctxDir->id(),UserActionObjectType::ContextDir));
                        actCreate->setParentId(gGraphContextManager->getParentId(ctxDir->id(),true));
                        act->addAction(actCreate);
                        act->addAction(new ActionAddItemsToObject({gNetlist->get_top_module()->get_id()}, {}));
                        act->setParentObject(mParentObject);

                        mUndoAction = act;
                    }
                    gGraphContextManager->deleteContextDirectory(ctxDir);
                }
                else
                    return false;
                break;
            default:
                return false;
        }
        return UserAction::exec();
    }
}    // namespace hal
