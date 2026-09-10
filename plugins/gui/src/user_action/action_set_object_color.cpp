#include "gui/user_action/action_set_object_color.h"
#include "gui/grouping/grouping_manager_widget.h"
#include "gui/grouping/grouping_table_model.h"
#include "gui/module_model/module_color_manager.h"
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
            if (xmlIn.name() == QString("color"))
                mColor = QColor(xmlIn.readElementText());
        }
    }

    bool ActionSetObjectColor::exec()
    {
        QColor oldColor;

        switch (mObject.type())
        {
        case UserActionObjectType::Module:
            oldColor = gNetlistRelay->getModuleColorManager()->setModuleColor(mObject.id(),mColor);

            // Set module color will fire moduleColorChanged event.
            // However, gGraphContextManager is not in the receiver list and has to be updated manually
            gGraphContextManager->handleModuleColorChanged(gNetlist->get_module_by_id(mObject.id()));
            break;
        case UserActionObjectType::Grouping:
            oldColor = gContentManager->getGroupingManagerWidget()->getModel()->
                    recolorGrouping(mObject.id(), mColor);
            break;
        default:
            return false;
        }
        if (oldColor.isValid())
        {
            ActionSetObjectColor* act = new ActionSetObjectColor(oldColor);
            act->setObject(mObject);
            mUndoAction = act;
        }

        return UserAction::exec();
    }
}
