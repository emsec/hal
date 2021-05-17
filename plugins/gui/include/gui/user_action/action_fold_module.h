#pragma once
#include "user_action.h"

namespace hal
{
    class Module;
    class GraphContext;

    /**
     * @ingroup user_action
     */
    class ActionFoldModule : public UserAction
    {
        u32 mContextId;
        PlacementHint mPlacementHint;
    public:
        ActionFoldModule(u32 moduleId = 0);
        QString tagname() const override;
        bool exec() override;
        void setContextId(u32 id) { mContextId = id; }
        void setPlacementHint(PlacementHint hint) { mPlacementHint = hint; }
    };

    /**
     * @ingroup user_action
     */
    class ActionFoldModuleFactory : public UserActionFactory
    {
    public:
        ActionFoldModuleFactory();
        UserAction* newAction() const;
        static ActionFoldModuleFactory* sFactory;
    };
}
