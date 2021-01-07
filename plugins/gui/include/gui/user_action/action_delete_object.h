#pragma once
#include "user_action.h"

namespace hal
{
    class ActionDeleteObject : public UserAction
    {
    public:
        ActionDeleteObject() {;}
        void writeToXml(QXmlStreamWriter& xmlOut) const override;
        void readFromXml(QXmlStreamReader& xmlIn) override;
        void exec() override;
        QString tagname() const override;
    };

    class ActionDeleteObjectFactory : public UserActionFactory
    {
    public:
        ActionDeleteObjectFactory();
        UserAction* newAction() const;
        static ActionDeleteObjectFactory* sFactory;
    };
}
