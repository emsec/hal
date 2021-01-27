#pragma once
#include "user_action.h"

namespace hal
{
    class Module;
    class GraphContext;

    class ActionFoldModule : public UserAction
    {
        u32 mContextId;
    public:
        ActionFoldModule(u32 moduleId = 0);
        QString tagname() const override;
        void exec() override;
        void setContextId(u32 id) { mContextId = id; }
    };

    class ActionFoldModuleFactory : public UserActionFactory
    {
    public:
        ActionFoldModuleFactory();
        UserAction* newAction() const;
        static ActionFoldModuleFactory* sFactory;
    };
}
