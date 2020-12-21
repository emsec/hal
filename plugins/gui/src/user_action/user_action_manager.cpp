#include "gui/user_action/user_action_manager.h"

namespace hal
{
    UserActionManager* UserActionManager::inst = nullptr;

    UserActionManager::UserActionManager(QObject *parent) : QObject(parent)
    {

    }

    void UserActionManager::addExecutedAction(UserAction* act)
    {
        mActionHistory.append(act);
    }

    UserActionManager* UserActionManager::instance()
    {
        if (!inst) inst = new UserActionManager;
        return inst;
    }
}
