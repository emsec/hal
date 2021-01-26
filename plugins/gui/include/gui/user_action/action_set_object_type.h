#pragma once
#include "user_action.h"

namespace hal
{
    class ActionSetObjectType : public UserAction
    {
        QString mObjectType;
    public:
        ActionSetObjectType(const QString& type=QString()) : mObjectType(type) {;}
        void exec() override;
        QString tagname() const override;
        void writeToXml(QXmlStreamWriter& xmlOut) const override;
        void readFromXml(QXmlStreamReader& xmlIn) override;
        void addToHash(QCryptographicHash& cryptoHash) const override;
    };

    class ActionSetObjectTypeFactory : public UserActionFactory
    {
    public:
        ActionSetObjectTypeFactory();
        UserAction* newAction() const;
        static ActionSetObjectTypeFactory* sFactory;
    };
}
