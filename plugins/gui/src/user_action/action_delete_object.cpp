#include "gui/user_action/action_add_items_to_object.h"
#include "gui/user_action/action_delete_object.h"
#include "gui/user_action/action_create_object.h"
#include "gui/user_action/action_set_object_color.h"
#include "gui/user_action/action_set_object_type.h"
#include "gui/user_action/user_action_compound.h"
#include "gui/grouping/grouping_manager_widget.h"
#include "gui/grouping/grouping_table_model.h"
#include "gui/module_model/module_model.h"
#include "gui/gui_globals.h"

namespace hal
{
    ActionDeleteObjectFactory::ActionDeleteObjectFactory()
       : UserActionFactory("DeleteObject") {;}

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
        Gate*   gat;
        Net*    net;
        GraphContext* ctx;
        switch (mObject.type()) {
        case UserActionObjectType::PinGroup:
        {
            return false;//tmp
        }
            break;
        case UserActionObjectType::Module:
            mod = gNetlist->get_module_by_id(mObject.id());
            if (mod)
            {
                UserActionCompound* act = new UserActionCompound;
                act->setUseCreatedObject();
                ActionCreateObject* actCreate =
                        new ActionCreateObject(UserActionObjectType::Module,
                                               QString::fromStdString(mod->get_name()));
                actCreate->setParentId(mod->get_parent_module()->get_id());
                act->addAction(actCreate);
                act->addAction(new ActionSetObjectType(QString::fromStdString(mod->get_type())));
                act->addAction(new ActionSetObjectColor(gNetlistRelay->getModuleModel()->moduleColor(mod->get_id())));
                QSet<u32> mods, gats;
                for (Gate* g : mod->get_gates())
                    gats.insert(g->get_id());
                for (Module* sm : mod->get_submodules())
                    mods.insert(sm->get_id());
                if (!mods.isEmpty() || !gats.isEmpty())
                    act->addAction(new ActionAddItemsToObject(mods,gats));
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
        case UserActionObjectType::Grouping:
        {
            GroupingTableModel* grpModel = gContentManager->getGroupingManagerWidget()->getModel();
            for (int irow=0; irow < grpModel->rowCount(); irow++)
            {
                const GroupingTableEntry& grpEntry = grpModel->groupingAt(irow);
                if (grpEntry.id() == mObject.id())
                {
                    ActionCreateObject* undoCreate =
                            new ActionCreateObject(UserActionObjectType::Grouping, grpEntry.name());
                    undoCreate->setObject(mObject);
                    QSet<u32> mods, gats, nets;
                    for (u32 id : grpEntry.grouping()->get_module_ids())
                        mods.insert(id);
                    for (u32 id : grpEntry.grouping()->get_gate_ids())
                        gats.insert(id);
                    for (u32 id : grpEntry.grouping()->get_net_ids())
                        nets.insert(id);
                    ActionSetObjectColor* undoColor =
                            new ActionSetObjectColor(grpEntry.color());
                    undoColor->setObject(mObject);
                    ActionAddItemsToObject* undoPopulate =
                            new ActionAddItemsToObject(mods, gats, nets);
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
        case UserActionObjectType::Context:
            ctx = gGraphContextManager->getContextById(mObject.id());
            if (ctx)
            {
                // TODO : Undo action
                gGraphContextManager->deleteGraphContext(ctx);
            }
            else
                return false;
            break;
        default:
            return false;
        }
        return UserAction::exec();
    }
}
