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
                if (pinGroup && pinGroup->size() > 1)
                {
                    auto oldIndex = pin->get_group().second;
                    auto result   = mod->move_pin_within_group(pinGroup, pin, mNewIndex);
                    if (result.is_error())
                    {
                        return false;
                    }

                    ActionReorderObject* undo = new ActionReorderObject(oldIndex);
                    undo->setObject(mObject);
                    undo->setParentObject(mParentObject);
                    //undo->setPinOrPingroupIdentifier(mPinOrPingroupIdentifier);
                    mUndoAction = undo;
                }
                else
                {
                    return false;
                }
            }
            break;
            default:
                return false;
        }

        return UserAction::exec();
    }

    QString ActionReorderObject::tagname() const
    {
        return ActionReorderObjectFactory::sFactory->tagname();
    }

}    // namespace hal
