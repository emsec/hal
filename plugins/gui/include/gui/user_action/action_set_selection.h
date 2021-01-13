#pragma once
#include "user_action.h"
#include <QSet>

namespace hal
{
    class ActionSetSelection : public UserAction
    {
        friend class SelectionRelay;

        QSet<u32> mModules;
        QSet<u32> mGates;
        QSet<u32> mNets;

        QSet<u32> mPreviousModules;
        QSet<u32> mPreviousGates;
        QSet<u32> mPreviousNets;

        QString setToText(const QSet<u32>& set) const;
        QSet<u32> setFromText(const QString& s) const;
    public:
        ActionSetSelection() {;}

        QString tagname() const override;
        void writeToXml(QXmlStreamWriter& xmlOut) const override;
        void readFromXml(QXmlStreamReader& xmlIn) override;
        void exec() override;
        void addToHash(QCryptographicHash& cryptoHash) const override;
        bool hasModifications() const;
    };

    class ActionSetSelectionFactory : public UserActionFactory
    {
    public:
        ActionSetSelectionFactory();
        UserAction* newAction() const;
        static ActionSetSelectionFactory* sFactory;
    };
}
