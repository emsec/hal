#include "gui/user_action/user_action_compound.h"

namespace hal {
    UserActionCompound::UserActionCompound()
        : UserAction(UserActionManager::CompoundAction)
    {;}

    void UserActionCompound::addAction(UserAction* act)
    {
        mActionList.append(act);
    }

    void UserActionCompound::exec()
    {
        for (UserAction* act : mActionList)
            act->exec();
    }
}
