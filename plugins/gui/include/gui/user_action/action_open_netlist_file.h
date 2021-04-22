#pragma once
#include "user_action.h"

namespace hal
{
    /**
     * @ingroup user_action
     */
    class ActionOpenNetlistFile : public UserAction
    {
        QString mFilename;
    public:
        ActionOpenNetlistFile(const QString& filename_ = QString());
        QString tagname() const override;
        bool exec() override;
        void writeToXml(QXmlStreamWriter& xmlOut) const override;
        void readFromXml(QXmlStreamReader& xmlIn) override;
        void addToHash(QCryptographicHash& cryptoHash) const override;
    };

    /**
     * @ingroup user_action
     */
    class ActionOpenNetlistFileFactory : public UserActionFactory
    {
    public:
        ActionOpenNetlistFileFactory();
        UserAction* newAction() const;
        static ActionOpenNetlistFileFactory* sFactory;
    };
}
