#include "gui/user_action/action_set_object_data.h"
#include "gui/gui_globals.h"

namespace hal
{

    ActionSetObjectDataFactory::ActionSetObjectDataFactory() : UserActionFactory("SetObjectData")
    {}
    ActionSetObjectDataFactory* ActionSetObjectDataFactory::sFactory = new ActionSetObjectDataFactory;

    UserAction *ActionSetObjectDataFactory::newAction() const
    {
        return new ActionSetObjectData;
    }

    ActionSetObjectData::ActionSetObjectData(QString category, QString key, QString type, QString val)
    : mKey(key), mCat(category), mType(type), mVal(val), mOldKey(""), mOldCat(""), mKeyOrCatModified(false)
    {
    }

    bool ActionSetObjectData::exec()
    {
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
            default: break;
        }

        if(!container)
            return false;

        //implicit modifying of an existing entry, but it must be deleted beforehand and undo action must be constructed in an appropriate way
        if(mKeyOrCatModified)
        {
            if(!container->has_data(mOldCat.toStdString(), mOldKey.toStdString()))//additional security check
                return false;

            //construct appropriate undo action
            auto data = container->get_data(mOldCat.toStdString(), mOldKey.toStdString());
            mUndoAction = new ActionSetObjectData(mOldCat, mOldKey, QString::fromStdString(std::get<0>(data)), QString::fromStdString(std::get<1>(data)));
            mUndoAction->setObject(mObject);
            static_cast<ActionSetObjectData*>(mUndoAction)->setChangeKeyAndOrCategory(mCat, mKey);

            //delete old, add new
            container->delete_data(mOldCat.toStdString(), mOldKey.toStdString());
            container->set_data(mCat.toStdString(), mKey.toStdString(), mType.toStdString(), mVal.toStdString());
        }
        //check if the data to set already exists (undo action is then "change back")
        else if(container->has_data(mCat.toStdString(), mKey.toStdString()))
        {
            auto data = container->get_data(mCat.toStdString(), mKey.toStdString());
            mUndoAction = new ActionSetObjectData(mCat, mKey, QString::fromStdString(std::get<0>(data)), QString::fromStdString(std::get<1>(data)));
            mUndoAction->setObject(mObject);
            container->set_data(mCat.toStdString(), mKey.toStdString(), mType.toStdString(), mVal.toStdString());
        }
        else //if the data does not already exists -> it is new data -> undo action deletes it (yet to come)
        {
        }

        return UserAction::exec();
    }

    QString ActionSetObjectData::tagname() const
    {
        return ActionSetObjectDataFactory::sFactory->tagname();
    }

    void ActionSetObjectData::writeToXml(QXmlStreamWriter &xmlOut) const
    {
        xmlOut.writeTextElement("category", mCat);
        xmlOut.writeTextElement("key", mKey);
        xmlOut.writeTextElement("type", mType);
        xmlOut.writeTextElement("value", mVal);
        if(mKeyOrCatModified)
        {
            xmlOut.writeTextElement("oldCategory", mOldCat);
            xmlOut.writeTextElement("oldKey", mOldKey);
        }
    }

    void ActionSetObjectData::readFromXml(QXmlStreamReader &xmlIn)
    {
        while(xmlIn.readNextStartElement())
        {
            if(xmlIn.name() == "category")
                mCat = xmlIn.readElementText();
            else if(xmlIn.name() == "key")
                mKey = xmlIn.readElementText();
            else if(xmlIn.name() == "type")
                mType = xmlIn.readElementText();
            else if(xmlIn.name() == "value")
                mVal = xmlIn.readElementText();
            else if(xmlIn.name() == "oldCategory")
            {
                mKeyOrCatModified = true;
                mOldCat = xmlIn.readElementText();
            }
            else if(xmlIn.name() == "oldKey")
            {
                mKeyOrCatModified = true;
                mOldKey = xmlIn.readElementText();
            }
        }
    }

    void ActionSetObjectData::addToHash(QCryptographicHash &cryptoHash) const
    {
        cryptoHash.addData("cat", 3);
        cryptoHash.addData(mCat.toUtf8());
        cryptoHash.addData("key", 3);
        cryptoHash.addData(mKey.toUtf8());
        cryptoHash.addData("type", 4);
        cryptoHash.addData(mType.toUtf8());
        cryptoHash.addData("val", 3);
        cryptoHash.addData(mVal.toUtf8());
        if(mKeyOrCatModified)
        {
            cryptoHash.addData("oldCat", 6);
            cryptoHash.addData(mOldCat.toUtf8());
            cryptoHash.addData("oldKey", 6);
            cryptoHash.addData(mOldKey.toUtf8());
        }
    }

    void ActionSetObjectData::setChangeKeyAndOrCategory(QString oldCategory, QString oldKey)
    {
        mOldKey = oldKey;
        mOldCat = oldCategory;
        mKeyOrCatModified = true;
    }

}
