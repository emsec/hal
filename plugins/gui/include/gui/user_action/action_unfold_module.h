#pragma once
#include "user_action.h"

namespace hal
{
    class Module;
    class GraphContext;

    class ActionUnfoldModule : public UserAction
    {
    public:
        ActionUnfoldModule();
        void exec() override;
        virtual void writeToXml(QXmlStreamWriter& xmlOut) const override;
        virtual void readFromXml(QXmlStreamReader& xmlIn) override;
    private:
        void execInternal(Module* m, GraphContext* currentContext);
    };
}
