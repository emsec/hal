#pragma once
#include "user_action.h"

namespace hal
{
    class ActionAddToSelection : public UserAction
    {
    public:
        ActionAddToSelection() {;}
        QString tagname() const override;
        void exec() override;
    };

    class ActionAddToSelectionFactory : public UserActionFactory
    {
    public:
        ActionAddToSelectionFactory();
        UserAction* newAction() const;
        static ActionAddToSelectionFactory* sFactory;
    };
}
