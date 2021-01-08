#include "gui/user_action/action_add_to_selection.h"
#include "gui/gui_globals.h"

namespace hal
{
    ActionAddToSelectionFactory::ActionAddToSelectionFactory()
        : UserActionFactory("AddToSelection") {;}

    ActionAddToSelectionFactory* ActionAddToSelectionFactory::sFactory = new ActionAddToSelectionFactory;

    UserAction* ActionAddToSelectionFactory::newAction() const
    {
        return new ActionAddToSelection;
    }

    QString ActionAddToSelection::tagname() const
    {
        return ActionAddToSelectionFactory::sFactory->tagname();
    }

    void ActionAddToSelection::exec()
    {
        switch (mObject.type()) {
        case UserActionObjectType::Module:
            gSelectionRelay->addModule(mObject.id());
            break;
        case UserActionObjectType::Gate:
            gSelectionRelay->addGate(mObject.id());
            break;
        case UserActionObjectType::Net:
            gSelectionRelay->addNet(mObject.id());
            break;
        default:
            break;
        }
        UserAction::exec();
    }
}
