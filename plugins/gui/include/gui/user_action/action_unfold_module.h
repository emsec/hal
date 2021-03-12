#pragma once
#include "user_action.h"

namespace hal
{
    class Module;
    class GraphContext;

    class ActionUnfoldModule : public UserAction
    {
        u32 mContextId;
        PlacementHint mPlacementHint;
    public:
        ActionUnfoldModule(u32 moduleId = 0);
        QString tagname() const override;
        bool exec() override;
        void setContextId(u32 id) { mContextId = id; }
        void setObject(const UserActionObject& obj) override;
        void setPlacementHint(PlacementHint hint) { mPlacementHint = hint; }
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
