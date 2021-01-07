#pragma once
#include "user_action.h"

namespace hal
{
    class ActionSelectionToObject : public UserAction
    {
    public:
        ActionSelectionToObject() {;}
        QString tagname() const override;
        void writeToXml(QXmlStreamWriter& xmlOut) const override;
        void readFromXml(QXmlStreamReader& xmlIn) override;
        void exec() override;
    };

    class ActionSelectionToObjectFactory : public UserActionFactory
    {
    public:
        ActionSelectionToObjectFactory();
        UserAction* newAction() const;
        static ActionSelectionToObjectFactory* sFactory;
    };

}
