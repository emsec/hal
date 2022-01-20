#include "gui/user_action/action_reorder_object.h"
#include "gui/gui_globals.h"

namespace hal
{
    //** ACTION_REORDER_OBJECT_FACTORY
    ActionReorderObjectFactory* ActionReorderObjectFactory::sFactory = new ActionReorderObjectFactory;

    ActionReorderObjectFactory::ActionReorderObjectFactory() : UserActionFactory("ReorderObject")
    {
    }

    UserAction *ActionReorderObjectFactory::newAction() const
    {
        return new ActionReorderObject;
    }

    //** ACTION_REORDER_OBJECT
    ActionReorderObject::ActionReorderObject(const int newIndex) : mNewIndex(newIndex)
    {
    }

    ActionReorderObject::ActionReorderObject(const int newIndex, QString pinOrPingroupId)
        : mNewIndex(newIndex), mPinOrPingroupIdentifier(pinOrPingroupId)
    {

    }

    bool ActionReorderObject::exec()
    {
        switch (mObject.type())
        {
            case UserActionObjectType::Pin:
            {
                auto mod = gNetlist->get_module_by_id(mObject.id());
                auto pin = mod ? mod->get_pin(mPinOrPingroupIdentifier.toStdString()) : nullptr;
                auto pinGroup = pin ? pin->get_group().first : nullptr;
                if(pinGroup && pinGroup->size() > 1)
                {
                    auto oldIndex = pin->get_group().second;
                    bool ret = mod->move_pin_within_group(pinGroup, pin, mNewIndex);
                    if(!ret)
                        return false;

                    ActionReorderObject* undo = new ActionReorderObject(oldIndex);
                    undo->setObject(mObject);
                    undo->setPinOrPingroupIdentifier(mPinOrPingroupIdentifier);
                    mUndoAction = undo;
                }
                else
                    return false;
                break;
            }
            default:
                return false;
        }

        return UserAction::exec();
    }

    QString ActionReorderObject::tagname() const
    {
        return ActionReorderObjectFactory::sFactory->tagname();
    }

    void ActionReorderObject::setPinOrPingroupIdentifier(QString currName)
    {
        mPinOrPingroupIdentifier = currName;
    }

}
