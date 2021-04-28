#pragma once
#include "user_action.h"

namespace hal
{
    /**
     * @ingroup user_action
     */
    class ActionDeleteObject : public UserAction
    {
    public:
        ActionDeleteObject() {;}
        bool exec() override;
        QString tagname() const override;
    };

    /**
     * @ingroup user_action
     */
    class ActionDeleteObjectFactory : public UserActionFactory
    {
    public:
        ActionDeleteObjectFactory();
        UserAction* newAction() const;
        static ActionDeleteObjectFactory* sFactory;
    };
}
