#include "gui/user_action/user_action_compound.h"

namespace hal {
    UserActionCompoundFactory::UserActionCompoundFactory()
        : UserActionFactory("Compound") {;}

    UserActionCompoundFactory* UserActionCompoundFactory::sFactory = new UserActionCompoundFactory;

    UserAction* UserActionCompoundFactory::newAction() const
    {
        return new UserActionCompound;
    }

    QString UserActionCompound::tagname() const
    {
        return UserActionCompoundFactory::sFactory->tagname();
    }

    UserActionCompound::UserActionCompound()
        : mUseCreatedObject(false)
    {;}

    void UserActionCompound::addAction(UserAction* act)
    {
        mActionList.append(act);
    }

    void UserActionCompound::exec()
    {
        bool first = true;
        for (UserAction* act : mActionList)
        {
            if (mUseCreatedObject && !first)
                act->setObject(object());
            act->exec();
            if (mUseCreatedObject && first)
                setObject(act->object());
            first = false;
        }
    }

    void UserActionCompound::writeToXml(QXmlStreamWriter& xmlOut) const
    {
        xmlOut.writeStartElement("actions");
        xmlOut.writeAttribute("useCreatedObject",mUseCreatedObject?"true":"false");
        for (UserAction* act: mActionList)
            act->writeToXml(xmlOut);
        xmlOut.writeEndElement();
    }

    void UserActionCompound::readFromXml(QXmlStreamReader& xmlIn)
    {
        bool parseActions = false;
        while (!xmlIn.atEnd())
        {
            if (xmlIn.readNext())
            {
                if (xmlIn.isStartElement())
                {
                    if (xmlIn.name() == "actions")
                    {
                        parseActions = true;
                        mUseCreatedObject = (xmlIn.attributes().value("useCreatedObject")=="true");
                    }
                    else if (parseActions)
                    {
                        UserAction* act = UserActionManager::instance()->getParsedAction(xmlIn);
                        if (act) mActionList.append(act);
                    }
                }
                else if (xmlIn.isEndElement())
                {
                    if (xmlIn.name() == "actions")
                    {
                        parseActions = false;
                        return;
                    }
                }
            }
        }
    }
}
