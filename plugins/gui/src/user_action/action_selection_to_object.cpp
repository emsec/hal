#include "gui/user_action/action_selection_to_object.h"

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

}
