#include "gui/user_action/action_set_focus.h"
#include "gui/gui_globals.h"

namespace hal
{
    ActionSetFocusFactory::ActionSetFocusFactory()
       : UserActionFactory("SetFocus") {;}

    ActionSetFocusFactory* ActionSetFocusFactory::sFactory = new ActionSetFocusFactory;

    UserAction* ActionSetFocusFactory::newAction() const
    {
        return new ActionSetFocus;
    }

    QString ActionSetFocus::tagname() const
    {
        return ActionSetFocusFactory::sFactory->tagname();
    }

    void ActionSetFocus::writeToXml(QXmlStreamWriter& xmlOut) const
    {
        switch (mSubfocus)
        {
        case SelectionRelay::Subfocus::Left:
            xmlOut.writeAttribute("subfocus","Left");
            xmlOut.writeAttribute("subfocusIndex", QString::number(mSubfocusIndex));
            break;
        case SelectionRelay::Subfocus::Right:
            xmlOut.writeAttribute("subfocus","Right");
            xmlOut.writeAttribute("subfocusIndex", QString::number(mSubfocusIndex));
            break;
        default:
            break;
        }
    }

    void ActionSetFocus::readFromXml(QXmlStreamReader& xmlIn)
    {
        QStringRef sfocAttribute = xmlIn.attributes().value("subfocus");
        if (sfocAttribute.isNull() || sfocAttribute.isEmpty()) return;
        if (sfocAttribute == "Left")
        {
            mSubfocus = SelectionRelay::Subfocus::Left;
            mSubfocusIndex = xmlIn.attributes().value("subfocusIndex").toInt();
        }
        else if (sfocAttribute == "Right")
        {
            mSubfocus = SelectionRelay::Subfocus::Right;
            mSubfocusIndex = xmlIn.attributes().value("subfocusIndex").toInt();
        }
    }

    void ActionSetFocus::exec()
    {
        gSelectionRelay->setFocusDirect(UserActionObjectType::toItemType(mObject.type()),
                                  mObject.id(),mSubfocus,mSubfocusIndex);
        UserAction::exec();
    }
}
