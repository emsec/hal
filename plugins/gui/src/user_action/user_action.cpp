#include "gui/user_action/user_action.h"

namespace hal
{
    UserAction::UserAction(UserActionManager::UserActionType type_)
        : mType(type_), mObjectId(0),
          mWaitForReady(false),
          mUndoAction(nullptr) {;}

    void UserAction::exec() {
        UserActionManager::instance()->addExecutedAction(this);
    }

}
