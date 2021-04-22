#pragma once
#include "user_action.h"
#include <QSet>

namespace hal
{
    /**
     * @ingroup user_action
     */
    class ActionSetSelectionFocus : public UserAction
    {
        friend class SelectionRelay;

        QSet<u32> mModules;
        QSet<u32> mGates;
        QSet<u32> mNets;

        SelectionRelay::Subfocus mSubfocus;
        u32 mSubfocusIndex;

    public:
        ActionSetSelectionFocus() {;}

        QString tagname() const override;
        void writeToXml(QXmlStreamWriter& xmlOut) const override;
        void readFromXml(QXmlStreamReader& xmlIn) override;
        bool exec() override;
        void addToHash(QCryptographicHash& cryptoHash) const override;
        bool hasModifications() const;
        void setObject(const UserActionObject &obj) override;
    };

    /**
     * @ingroup user_action
     */
    class ActionSetSelectionFocusFactory : public UserActionFactory
    {
    public:
        ActionSetSelectionFocusFactory();
        UserAction* newAction() const;
        static ActionSetSelectionFocusFactory* sFactory;
    };
}
