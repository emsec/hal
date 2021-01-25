#include "gui/user_action/action_set_object_color.h"
#include "gui/grouping/grouping_manager_widget.h"
#include "gui/grouping/grouping_table_model.h"
#include "gui/gui_globals.h"

namespace hal
{
    ActionSetObjectColorFactory::ActionSetObjectColorFactory()
       : UserActionFactory("SetObjectColor") {;}

    ActionSetObjectColorFactory* ActionSetObjectColorFactory::sFactory = new ActionSetObjectColorFactory;

    UserAction* ActionSetObjectColorFactory::newAction() const
    {
        return new ActionSetObjectColor;
    }

    QString ActionSetObjectColor::tagname() const
    {
        return ActionSetObjectColorFactory::sFactory->tagname();
    }

    void ActionSetObjectColor::addToHash(QCryptographicHash& cryptoHash) const
    {
        cryptoHash.addData((char*)(&mColor),sizeof(mColor));
    }

    void ActionSetObjectColor::writeToXml(QXmlStreamWriter& xmlOut) const
    {
        xmlOut.writeTextElement("color", mColor.name(QColor::HexArgb));
    }

    void ActionSetObjectColor::readFromXml(QXmlStreamReader& xmlIn)
    {
        while (xmlIn.readNextStartElement())
        {
            if (xmlIn.name() == "color")
                mColor = QColor(xmlIn.readElementText());
        }
    }

    void ActionSetObjectColor::exec()
    {
        switch (mObject.type())
        {
        case UserActionObjectType::Grouping:
        {
            QColor oldCol = gContentManager->getGroupingManagerWidget()->getModel()->
                    recolorGrouping(mObject.id(), mColor);
            ActionSetObjectColor* act = new ActionSetObjectColor(oldCol);
            act->setObject(mObject);
            mUndoAction = act;
        }
            break;
        default:
            break;
        }

        UserAction::exec();
    }
}
