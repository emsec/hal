#include "gui/user_action/user_action.h"

namespace hal
{
    UserActionFactory::UserActionFactory(const QString& nam)
    {
        UserActionManager::instance()->registerFactory(this);
    }

    UserAction::UserAction()
        : mWaitForReady(false),
          mUndoAction(nullptr) {;}

    void UserAction::exec() {
        UserActionManager::instance()->addExecutedAction(this);
    }

}
