#include "gui/user_action/action_reorder_object.h"

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
        return false;
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
