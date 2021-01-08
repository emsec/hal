#pragma once
#include "user_action.h"
#include "gui/selection_relay/selection_relay.h"

namespace hal
{
    class ActionSetFocus : public UserAction
    {
        SelectionRelay::Subfocus mSubfocus;
        u32 mSubfocusIndex;
    public:
        ActionSetFocus(SelectionRelay::Subfocus sfoc = SelectionRelay::Subfocus::None, u32 sfinx = 0)
            : mSubfocus(sfoc), mSubfocusIndex(sfinx) {;}
        void writeToXml(QXmlStreamWriter& xmlOut) const override;
        void readFromXml(QXmlStreamReader& xmlIn) override;
        void exec() override;
        QString tagname() const override;
    };

    class ActionSetFocusFactory : public UserActionFactory
    {
    public:
        ActionSetFocusFactory();
        UserAction* newAction() const;
        static ActionSetFocusFactory* sFactory;
    };
}
