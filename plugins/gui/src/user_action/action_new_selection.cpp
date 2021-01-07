#include "gui/user_action/action_new_selection.h"

namespace hal
{
    ActionNewSelectionFactory::ActionNewSelectionFactory()
        : UserActionFactory("NewSelection") {;}

    ActionNewSelectionFactory* ActionNewSelectionFactory::sFactory = new ActionNewSelectionFactory;

    UserAction* ActionNewSelectionFactory::newAction() const
    {
        return new ActionNewSelection;
    }

    QString ActionNewSelection::tagname() const
    {
        return ActionNewSelectionFactory::sFactory->tagname();
    }

}
