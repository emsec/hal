#pragma once
#include "user_action.h"
#include <QString>

namespace hal
{
    class ActionRenameObject : public UserAction
    {
        QString mNewName;
        u32 mNetId;
        enum PortType { NoPort, Input, Output } mPortType;
    public:
        ActionRenameObject(const QString& name=QString())
            : mNewName(name), mNetId(0), mPortType(NoPort) {;}
        bool exec() override;
        QString tagname() const override;
        void writeToXml(QXmlStreamWriter& xmlOut) const override;
        void readFromXml(QXmlStreamReader& xmlIn) override;
        void addToHash(QCryptographicHash& cryptoHash) const override;
        void setInputNetId(u32 id)  { mPortType=Input;  mNetId=id; }
        void setOutputNetId(u32 id) { mPortType=Output; mNetId=id; }
    };

    class ActionRenameObjectFactory : public UserActionFactory
    {
    public:
        ActionRenameObjectFactory();
        UserAction* newAction() const;
        static ActionRenameObjectFactory* sFactory;
    };
}
