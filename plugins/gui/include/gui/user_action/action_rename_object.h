#pragma once
#include "user_action.h"
#include <QString>

namespace hal
{
    class ActionRenameObject : public UserAction
    {
        QString mNewName;
    public:
        ActionRenameObject(const QString& name=QString())
            : mNewName(name) {;}
        void exec() override;
        QString tagname() const override;
        void writeToXml(QXmlStreamWriter& xmlOut) const override;
        void readFromXml(QXmlStreamReader& xmlIn) override;
        void addToHash(QCryptographicHash& cryptoHash) const override;
    };

    class ActionRenameObjectFactory : public UserActionFactory
    {
    public:
        ActionRenameObjectFactory();
        UserAction* newAction() const;
        static ActionRenameObjectFactory* sFactory;
    };
}
