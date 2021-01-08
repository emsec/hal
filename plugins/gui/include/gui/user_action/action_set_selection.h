#pragma once
#include "user_action.h"
#include <QSet>

namespace hal
{
    class ActionSetSelection : public UserAction
    {
        QSet<u32> mModules;
        QSet<u32> mGates;
        QSet<u32> mNets;

        QString setToText(const QSet<u32>& set) const;
        QSet<u32> setFromText(const QString& s) const;
    public:
        ActionSetSelection(const QSet<u32>& mods = QSet<u32>(),
                           const QSet<u32>& gats = QSet<u32>(),
                           const QSet<u32>& nets = QSet<u32>())
            : mModules(mods), mGates(gats), mNets(nets) {;}
        QString tagname() const override;
        void writeToXml(QXmlStreamWriter& xmlOut) const override;
        void readFromXml(QXmlStreamReader& xmlIn) override;
        void exec() override;
    };

    class ActionSetSelectionFactory : public UserActionFactory
    {
    public:
        ActionSetSelectionFactory();
        UserAction* newAction() const;
        static ActionSetSelectionFactory* sFactory;
    };
}
