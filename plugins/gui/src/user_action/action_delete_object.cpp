#include "gui/user_action/action_add_items_to_object.h"
#include "gui/user_action/action_delete_object.h"
#include "gui/user_action/action_create_object.h"
#include "gui/user_action/action_set_object_color.h"
#include "gui/user_action/user_action_compound.h"
#include "gui/grouping/grouping_manager_widget.h"
#include "gui/grouping/grouping_table_model.h"
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

    void ActionDeleteObject::exec()
    {
        switch (mObject.type()) {
        case UserActionObjectType::Module:
        {
            Module* m = gNetlist->get_module_by_id(mObject.id());
            if (!m) return;
            gNetlist->delete_module(m);
        }
            break;
        case UserActionObjectType::Gate:
        {
            Gate* g = gNetlist->get_gate_by_id(mObject.id());
            if (!g) return;
            gNetlist->delete_gate(g);
        }
            break;
        case UserActionObjectType::Net:
        {
            Net* n = gNetlist->get_net_by_id(mObject.id());
            if (!n) return;
            gNetlist->delete_net(n);
        }
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
        default:
            break;
        }
        UserAction::exec();
    }
}
