#pragma once
#include "user_action.h"
#include "gui/gui_def.h"
#include <QSet>

namespace hal
{
    class ActionAddItemsToObject : public UserAction
    {
        QSet<u32> mModules;
        QSet<u32> mGates;
        QSet<u32> mNets;
        PlacementHint mPlacementHint;
        Node mPlacementOrigin;
    public:
        ActionAddItemsToObject(const QSet<u32>& mods = QSet<u32>(),
                               const QSet<u32>& gats = QSet<u32>(),
                               const QSet<u32>& nets = QSet<u32>())
            : mModules(mods), mGates(gats), mNets(nets),
              mPlacementHint(PlacementHint::Standard)
        {;}
        bool exec() override;
        void setPlacementHint(PlacementHint hint) { mPlacementHint = hint; }
        QString tagname() const override;
        void writeToXml(QXmlStreamWriter& xmlOut) const override;
        void readFromXml(QXmlStreamReader& xmlIn) override;
        void addToHash(QCryptographicHash& cryptoHash) const override;
    };

    class ActionAddItemsToObjectFactory : public UserActionFactory
    {
    public:
        ActionAddItemsToObjectFactory();
        UserAction* newAction() const;
        static ActionAddItemsToObjectFactory* sFactory;
    };
}
