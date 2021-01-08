#pragma once
#include "user_action.h"

namespace hal
{
    class ActionRemoveFromSelection : public UserAction
    {
    public:
        ActionRemoveFromSelection() {;}
        QString tagname() const override;
        void exec() override;
    };

    class ActionRemoveFromSelectionFactory : public UserActionFactory
    {
    public:
        ActionRemoveFromSelectionFactory();
        UserAction* newAction() const;
        static ActionRemoveFromSelectionFactory* sFactory;
    };
}
