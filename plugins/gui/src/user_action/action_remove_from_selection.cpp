#include "gui/user_action/action_remove_from_selection.h"
#include "gui/gui_globals.h"

namespace hal
{
    ActionRemoveFromSelectionFactory::ActionRemoveFromSelectionFactory()
        : UserActionFactory("RemoveFromSelection") {;}

    ActionRemoveFromSelectionFactory* ActionRemoveFromSelectionFactory::sFactory = new ActionRemoveFromSelectionFactory;

    UserAction* ActionRemoveFromSelectionFactory::newAction() const
    {
        return new ActionRemoveFromSelection;
    }

    QString ActionRemoveFromSelection::tagname() const
    {
        return ActionRemoveFromSelectionFactory::sFactory->tagname();
    }

    void ActionRemoveFromSelection::exec()
    {
        switch (mObject.type()) {
        case UserActionObjectType::Module:
            gSelectionRelay->removeModule(mObject.id());
            break;
        case UserActionObjectType::Gate:
            gSelectionRelay->removeGate(mObject.id());
            break;
        case UserActionObjectType::Net:
            gSelectionRelay->removeNet(mObject.id());
            break;
        default:
            break;
        }
        UserAction::exec();
    }
}
