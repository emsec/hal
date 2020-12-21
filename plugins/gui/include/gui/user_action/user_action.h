#pragma once
#include <QString>
#include "user_action_manager.h"

namespace hal
{
    class UserAction
    {
    public:
        virtual void exec();
        UserActionManager::UserActionType type() const { return mType; }
    protected:
        UserAction(UserActionManager::UserActionType type_);

        UserActionManager::UserActionType mType;
        UserAction *mUndoAction;
    };
}

