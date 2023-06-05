#include "gui/user_action/action_reorder_object.h"

#include "gui/gui_globals.h"

namespace hal
{
    //** ACTION_REORDER_OBJECT_FACTORY
    ActionReorderObjectFactory* ActionReorderObjectFactory::sFactory = new ActionReorderObjectFactory;

    ActionReorderObjectFactory::ActionReorderObjectFactory() : UserActionFactory("ReorderObject")
    {
    }

    UserAction* ActionReorderObjectFactory::newAction() const
    {
        return new ActionReorderObject;
    }

    //** ACTION_REORDER_OBJECT
    ActionReorderObject::ActionReorderObject(const int newIndex) : mNewIndex(newIndex)
    {
    }

    bool ActionReorderObject::exec()
    {
        int oldIndex = -1;
        switch (mObject.type())
        {
            case UserActionObjectType::Pin: {
                auto mod = gNetlist->get_module_by_id(mParentObject.id());
                if (!mod)
                {
                    return false;
                }
                auto pin = mod->get_pin_by_id(mObject.id());
                if (pin == nullptr)
                {
                    return false;
                }
                auto pinGroup = pin->get_group().first;
                if (pinGroup)
                {
                    if (pinGroup->size() > 1)
                    {
                        oldIndex = pin->get_group().second;
                        auto result   = mod->move_pin_within_group(pinGroup, pin, mNewIndex);
                        if (result.is_error())
                        {
                            return false;
                        }
                    }
                }
                else
                {
                    return false;
                }
            }
            break;
        case UserActionObjectType::PinGroup: {
                auto mod = gNetlist->get_module_by_id(mParentObject.id());
                if(!mod)
                    return false;

                auto pinGroup = mod->get_pin_group_by_id(mObject.id());
                if(!pinGroup)
                    return false;

                // todo: perhaps provide aa function that returns the index of a group within the module? Similar to pin-index within group
                auto groups = mod->get_pin_groups();
                for(int i = 0; i < groups.size(); i++)
                    if(pinGroup == groups.at(i))
                        oldIndex = i;

                auto result = mod->move_pin_group(pinGroup, mNewIndex);
                if(result.is_error())
                    return false;
            }
            break;
            default:
                return false;
        }
        if(oldIndex != -1)
        {
            ActionReorderObject* undo = new ActionReorderObject(oldIndex);
            undo->setObject(mObject);
            undo->setParentObject(mParentObject);
            mUndoAction = undo;
        }
        return UserAction::exec();
    }

    QString ActionReorderObject::tagname() const
    {
        return ActionReorderObjectFactory::sFactory->tagname();
    }

    void ActionReorderObject::writeToXml(QXmlStreamWriter& xmlOut) const
    {
        writeParentObjectToXml(xmlOut);
        xmlOut.writeTextElement("index", QString::number(mNewIndex));
    }

    void ActionReorderObject::readFromXml(QXmlStreamReader& xmlIn)
    {
        while (xmlIn.readNextStartElement())
        {
            readParentObjectFromXml(xmlIn);
            if (xmlIn.name() == "index")
                //check with bool (toInt(bool, base)) if conversions failed?
                mNewIndex = xmlIn.readElementText().toInt();
        }
    }

    void ActionReorderObject::addToHash(QCryptographicHash& cryptoHash) const
    {
        cryptoHash.addData(QByteArray::number(mNewIndex));
    }

}    // namespace hal
