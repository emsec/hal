#pragma once
#include "user_action.h"

namespace hal
{
    class ActionCreateObject : public UserAction
    {
    public:
        ActionCreateObject(const QString& objName = QString());
        void writeToXml(QXmlStreamWriter& xmlOut) const override;
        void readFromXml(QXmlStreamReader& xmlIn) override;
        void exec() override;
        QString tagname() const override;
    private:
        QString mObjectName;
    };

    class ActionCreateObjectFactory : public UserActionFactory
    {
    public:
        ActionCreateObjectFactory();
        UserAction* newAction() const;
        static ActionCreateObjectFactory* sFactory;
    };
}
