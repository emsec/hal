#pragma once

#include "user_action.h"
#include <QList>

namespace hal {
    class UserActionCompound : public UserAction
    {
    public:
        UserActionCompound();
        void addAction(UserAction* act);
        void exec() override;
    protected:
        QList<UserAction*> mActionList;
    };
}
