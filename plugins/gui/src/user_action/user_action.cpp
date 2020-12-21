#include "gui/user_action/user_action.h"

namespace hal
{
    UserAction::UserAction(UserActionManager::UserActionType type_)
        : mType(type_), mUndoAction(nullptr) {;}

    void UserAction::exec() {
        UserActionManager::instance()->addExecutedAction(this);
    }

}
