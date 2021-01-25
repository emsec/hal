#include "gui/gui_globals.h"
#include "gui/user_action/action_rename_object.h"
#include "gui/grouping/grouping_manager_widget.h"
#include "gui/grouping/grouping_table_model.h"

namespace hal
{
    ActionRenameObjectFactory::ActionRenameObjectFactory()
       : UserActionFactory("RenameObject") {;}

    ActionRenameObjectFactory* ActionRenameObjectFactory::sFactory = new ActionRenameObjectFactory;

    UserAction* ActionRenameObjectFactory::newAction() const
    {
        return new ActionRenameObject;
    }

    QString ActionRenameObject::tagname() const
    {
        return ActionRenameObjectFactory::sFactory->tagname();
    }

    void ActionRenameObject::addToHash(QCryptographicHash& cryptoHash) const
    {
        cryptoHash.addData(mNewName.toUtf8());
    }

    void ActionRenameObject::writeToXml(QXmlStreamWriter& xmlOut) const
    {
        xmlOut.writeTextElement("name", mNewName);
    }

    void ActionRenameObject::readFromXml(QXmlStreamReader& xmlIn)
    {
        while (xmlIn.readNextStartElement())
        {
            if (xmlIn.name() == "name")
                mNewName = xmlIn.readElementText();
        }
    }

    void ActionRenameObject::exec()
    {
        QString oldName;
        switch (mObject.type()) {
        case UserActionObjectType::Grouping:
            oldName = gContentManager->getGroupingManagerWidget()->getModel()->
                    renameGrouping(mObject.id(),mNewName);
            break;
        default:
            break;
        }
        mUndoAction = new ActionRenameObject(oldName);
        mUndoAction->setObject(mObject);
        UserAction::exec();
    }
}
