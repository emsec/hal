#include "gui/user_action/action_set_object_type.h"

#include "gui/gui_globals.h"
#include "hal_core/netlist/gate_library/enums/pin_type.h"
#include "hal_core/netlist/module.h"
#include "hal_core/utilities/enums.h"

namespace hal
{
    ActionSetObjectTypeFactory::ActionSetObjectTypeFactory() : UserActionFactory("SetObjectType")
    {
        ;
    }

    ActionSetObjectTypeFactory* ActionSetObjectTypeFactory::sFactory = new ActionSetObjectTypeFactory;

    UserAction* ActionSetObjectTypeFactory::newAction() const
    {
        return new ActionSetObjectType;
    }

    QString ActionSetObjectType::tagname() const
    {
        return ActionSetObjectTypeFactory::sFactory->tagname();
    }

    void ActionSetObjectType::addToHash(QCryptographicHash& cryptoHash) const
    {
        cryptoHash.addData(mObjectType.toUtf8());
    }

    void ActionSetObjectType::writeToXml(QXmlStreamWriter& xmlOut) const
    {
        xmlOut.writeTextElement("type", mObjectType);
    }

    void ActionSetObjectType::readFromXml(QXmlStreamReader& xmlIn)
    {
        while (xmlIn.readNextStartElement())
        {
            if (xmlIn.name() == "type")
                mObjectType = xmlIn.readElementText();
        }
    }

    bool ActionSetObjectType::exec()
    {
        QString oldType;
        Module* mod;
        switch (mObject.type())
        {
            /* TODO PIN
            case UserActionObjectType::Pin:    //only possible to change module pin types
            {
                //should also check if its a valid type? (enum_from_string default value doesnt help
                //since there is no default pintype value)
                if (mObjectType.isEmpty())
                    return false;

                mod = gNetlist->get_module_by_id(mParentObject.id());
                if (!mod)
                    return false;

                auto* pin = mod->get_pin_by_id(mObject.id());
                if (pin == nullptr)
                {
                    return false;
                }

                oldType = QString::fromStdString(enum_to_string(pin->get_type()));

                if (mod->set_pin_type(pin, enum_from_string<PinType>(mObjectType.toStdString(), PinType::none)) == false)
                {
                    return false;
                }
            }
            break;
            */
            case UserActionObjectType::Module:
                mod = gNetlist->get_module_by_id(mObject.id());
                if (mod != nullptr)
                {
                    oldType = QString::fromStdString(mod->get_type());
                    mod->set_type(mObjectType.toStdString());
                }
                else
                    return false;
                break;
            default:
                return false;
        }
        mUndoAction = new ActionSetObjectType(oldType);
        mUndoAction->setObject(mObject);
        mUndoAction->setParentObject(mParentObject);
        return UserAction::exec();
    }
}    // namespace hal
