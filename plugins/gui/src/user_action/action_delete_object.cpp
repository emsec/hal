#include "gui/user_action/action_delete_object.h"

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

}
