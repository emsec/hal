#include "gui/user_action/action_selection_to_object.h"
#include "gui/gui_globals.h"
#include "hal_core/netlist/grouping.h"

namespace hal
{
    ActionSelectionToObjectFactory::ActionSelectionToObjectFactory()
        : UserActionFactory("SelectionToObject") {;}

    ActionSelectionToObjectFactory* ActionSelectionToObjectFactory::sFactory = new ActionSelectionToObjectFactory;

    UserAction* ActionSelectionToObjectFactory::newAction() const
    {
        return new ActionSelectionToObject;
    }

    QString ActionSelectionToObject::tagname() const
    {
        return ActionSelectionToObjectFactory::sFactory->tagname();
    }

    void ActionSelectionToObject::exec()
    {
        switch (mObject.type())
        {
        case UserActionObjectType::Grouping:
        {
            Grouping* grp = gNetlist->get_grouping_by_id(mObject.id());
            if (!grp) return;
            for (u32 mid : gSelectionRelay->selectedModulesList())
            {
                Module* m = gNetlist->get_module_by_id(mid);
                if (m)
                {
                    Grouping* mg = m->get_grouping();
                    if (mg) mg->remove_module(m);
                    grp->assign_module(m);
                }
            }
            for (u32 gid : gSelectionRelay->selectedGatesList())
            {
                Gate* g = gNetlist->get_gate_by_id(gid);
                if (g)
                {
                    Grouping* gg = g->get_grouping();
                    if (gg) gg->remove_gate(g);
                    grp->assign_gate(g);
                }
            }
            for (u32 nid : gSelectionRelay->selectedNetsList())
            {
                Net* n = gNetlist->get_net_by_id(nid);
                if (n)
                {
                    Grouping* ng = n->get_grouping();
                    if (ng) ng->remove_net(n);
                    grp->assign_net(n);
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
