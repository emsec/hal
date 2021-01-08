#include "gui/user_action/action_delete_object.h"
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
            Grouping* grp = gNetlist->get_grouping_by_id(mObject.id());
            if (!grp) return;
            gNetlist->delete_grouping(grp);
        }
            break;
        default:
            break;
        }
        UserAction::exec();
    }
}
