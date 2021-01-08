#pragma once
#include "user_action.h"

namespace hal
{
    class Module;
    class GraphContext;

    class ActionUnfoldModule : public UserAction
    {
    public:
        ActionUnfoldModule() {;}
        QString tagname() const override;
        void exec() override;
    private:
        void execInternal(Module* m, GraphContext* currentContext);
    };

    class ActionUnfoldModuleFactory : public UserActionFactory
    {
    public:
        ActionUnfoldModuleFactory();
        UserAction* newAction() const;
        static ActionUnfoldModuleFactory* sFactory;
    };
}
