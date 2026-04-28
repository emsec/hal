#include "gui/user_action/action_set_object_data.h"
#include "gui/gui_globals.h"
#include "gui/netlist_relay/netlist_relay.h"

namespace hal
{

    ActionSetObjectDataFactory::ActionSetObjectDataFactory() : UserActionFactory("SetObjectData")
    {}
    ActionSetObjectDataFactory* ActionSetObjectDataFactory::sFactory = new ActionSetObjectDataFactory;

    UserAction *ActionSetObjectDataFactory::newAction() const
    {
        return new ActionSetObjectData;
    }

    ActionSetObjectData::ActionSetObjectData(ActionType act, const DataTableEntry& dataEntry)
        : mAct(act), mDataEntry(dataEntry)
    {;}

    /*
        return DataTableEntry::fromContainer(container, mDataEntry.category.toStdString(), mDataEntry.key.toStdString());
        return DataTableEntry::fromContainer(container, mOldCat.toStdString(), mOldKey.toStdString());
    */

    bool ActionSetObjectData::exec()
    {
        if (mDataEntry.isNull()) return false;

        //for now, this action only supports setting the data of gates, modules and nets (DataContainer)
        DataContainer* container = nullptr;
        switch(mObject.type())
        {
            case UserActionObjectType::Gate:
                container = dynamic_cast<DataContainer*>(gNetlist->get_gate_by_id(mObject.id()));
                break;
            case UserActionObjectType::Module:
                container = dynamic_cast<DataContainer*>(gNetlist->get_module_by_id(mObject.id()));
                break;
            case UserActionObjectType::Net:
                container = dynamic_cast<DataContainer*>(gNetlist->get_net_by_id(mObject.id()));
                break;
            default:
                return false;
        }

        ActionSetObjectData* undoAction = nullptr; // undo action will allways be of same type as action

        std::string searchCategory = (mAct == MoveAction) ? mOldCat.toStdString() : mDataEntry.category.toStdString();
        std::string searchKey      = (mAct == MoveAction) ? mOldKey.toStdString() : mDataEntry.key.toStdString();
        DataTableEntry currentEntry = DataTableEntry::fromContainer(container, searchCategory, searchKey);

        //construct appropriate undo action
        switch (mAct)
        {
            case UnknownAction:
                return false;
            case DeleteAction:
                if (currentEntry.isNull()) return false;
                undoAction = new ActionSetObjectData(CreateAction, currentEntry);
                break;
            case CreateAction:
                undoAction = new ActionSetObjectData(DeleteAction, mDataEntry);
                break;
            case ModifyAction:
                if (currentEntry.isNull())
                    // does not exist yet, it is a create
                    undoAction = new ActionSetObjectData(DeleteAction, mDataEntry);
                else
                    undoAction = new ActionSetObjectData(ModifyAction, currentEntry);
                break;
            case MoveAction:
                if (currentEntry.isNull()) return false;
                undoAction = new ActionSetObjectData(MoveAction, currentEntry);
                undoAction->setChangeKeyAndOrCategory(mDataEntry.category, mDataEntry.key);
                break;
        }

        if (undoAction)
        {
            undoAction->setObject(mObject);
            mUndoAction = undoAction;
        }

        // modify container
        switch (mAct)
        {
            case DeleteAction:
                container->delete_data(mDataEntry.category.toStdString(), mDataEntry.key.toStdString());
                break;
            case CreateAction:
            case ModifyAction:
                mDataEntry.toContainer(container);
                break;
            case MoveAction:
                container->delete_data(mOldCat.toStdString(), mOldKey.toStdString());
                mDataEntry.toContainer(container);
                break;
            default:
                break;
        }

        if (mObject.type() == UserActionObjectType::Gate)
            if (Gate* gate = gNetlist->get_gate_by_id(mObject.id()))
                Q_EMIT gNetlistRelay->gateBooleanFunctionChanged(gate);

        return UserAction::exec();
    }

    QString ActionSetObjectData::tagname() const
    {
        return ActionSetObjectDataFactory::sFactory->tagname();
    }

    void ActionSetObjectData::writeToXml(QXmlStreamWriter &xmlOut) const
    {
        const QStringList actionTypeToString = {"unknown", "delete", "create", "modify", "move"};
        xmlOut.writeTextElement("action",   actionTypeToString[mAct]);
        xmlOut.writeTextElement("category", mDataEntry.category);
        xmlOut.writeTextElement("key",      mDataEntry.key);
        xmlOut.writeTextElement("type",     mDataEntry.dataType);
        xmlOut.writeTextElement("value",    mDataEntry.value);
        if(mAct == MoveAction)
        {
            xmlOut.writeTextElement("oldCategory", mOldCat);
            xmlOut.writeTextElement("oldKey", mOldKey);
        }
    }

    void ActionSetObjectData::readFromXml(QXmlStreamReader &xmlIn)
    {
        const QStringList actionTypeToString = {"unknown", "delete", "create", "modify", "move"};
        while(xmlIn.readNextStartElement())
        {
            if(xmlIn.name() == "category")
                mDataEntry.category = xmlIn.readElementText();
            else if(xmlIn.name() == "key")
                mDataEntry.key      = xmlIn.readElementText();
            else if(xmlIn.name() == "type")
                mDataEntry.dataType = xmlIn.readElementText();
            else if(xmlIn.name() == "value")
                mDataEntry.value    = xmlIn.readElementText();
            else if(xmlIn.name() == "oldCategory")
                mOldCat = xmlIn.readElementText();
            else if(xmlIn.name() == "oldKey")
                mOldKey = xmlIn.readElementText();
            else if (xmlIn.name() == "action")
            {
                QString tmpAct = xmlIn.readElementText();
                int i = actionTypeToString.indexOf(tmpAct);
                mAct = i < 0 ? UnknownAction : (ActionType) i;
            }
        }
    }

    void ActionSetObjectData::addToHash(QCryptographicHash &cryptoHash) const
    {
        char ctype = (char) mAct;
        cryptoHash.addData(&ctype, 1);
        cryptoHash.addData("cat", 3);
        cryptoHash.addData(mDataEntry.category.toUtf8());
        cryptoHash.addData("key", 3);
        cryptoHash.addData(mDataEntry.key.toUtf8());
        cryptoHash.addData("type", 4);
        cryptoHash.addData(mDataEntry.dataType.toUtf8());
        cryptoHash.addData("val", 3);
        cryptoHash.addData(mDataEntry.value.toUtf8());
        if(mAct == MoveAction)
        {
            cryptoHash.addData("oldCat", 6);
            cryptoHash.addData(mOldCat.toUtf8());
            cryptoHash.addData("oldKey", 6);
            cryptoHash.addData(mOldKey.toUtf8());
        }
    }

    void ActionSetObjectData::setChangeKeyAndOrCategory(QString oldCategory, QString oldKey)
    {
        mAct = MoveAction;
        mOldKey = oldKey;
        mOldCat = oldCategory;
    }

}
