#pragma once
#include "user_action.h"

namespace hal
{
    /**
     * @ingroup gui
     */
    class ActionDeleteObject : public UserAction
    {
    public:
        ActionDeleteObject() {;}
        bool exec() override;
        QString tagname() const override;
    };

    /**
     * @ingroup gui
     */
    class ActionDeleteObjectFactory : public UserActionFactory
    {
    public:
        ActionDeleteObjectFactory();
        UserAction* newAction() const;
        static ActionDeleteObjectFactory* sFactory;
    };
}
