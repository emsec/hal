#pragma once
#include "user_action.h"

namespace hal
{
    /**
     * @ingroup gui
     */
    class ActionCreateObject : public UserAction
    {
        QString mObjectName;
        u32 mParentId;
    public:
        ActionCreateObject(UserActionObjectType::ObjectType type=UserActionObjectType::None,
                           const QString& objName = QString());
        bool exec() override;
        QString tagname() const override;
        void writeToXml(QXmlStreamWriter& xmlOut) const override;
        void readFromXml(QXmlStreamReader& xmlIn) override;
        void addToHash(QCryptographicHash& cryptoHash) const override;
        void setParentId(u32 pid) {mParentId = pid;}
    };

    /**
     * @ingroup gui
     */
    class ActionCreateObjectFactory : public UserActionFactory
    {
    public:
        ActionCreateObjectFactory();
        UserAction* newAction() const;
        static ActionCreateObjectFactory* sFactory;
    };
}
