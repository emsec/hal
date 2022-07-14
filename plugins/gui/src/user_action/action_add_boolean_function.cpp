#include "gui/user_action/action_add_boolean_function.h"
#include"gui/gui_globals.h"

namespace hal
{
    ActionAddBooleanFunctionFactory::ActionAddBooleanFunctionFactory() : UserActionFactory("AddBooleanFunction")
    {

    }

    ActionAddBooleanFunctionFactory* ActionAddBooleanFunctionFactory::sFactory = new ActionAddBooleanFunctionFactory;

    UserAction *ActionAddBooleanFunctionFactory::newAction() const
    {
        return new ActionAddBooleanFunction;
    }

    ActionAddBooleanFunction::ActionAddBooleanFunction(QString booleanFuncName, BooleanFunction func, u32 gateID)
        : mName(booleanFuncName), mFunction(func)
    {
        setObject(UserActionObject(gateID, UserActionObjectType::Gate));
    }

    bool ActionAddBooleanFunction::exec()
    {
        auto gate = gNetlist->get_gate_by_id(mObject.id());
        if(!gate)
            return false;

        auto bf = gate->get_boolean_function(mName.toStdString());
        mUndoAction = !bf.is_empty() ? new ActionAddBooleanFunction(mName, bf, mObject.id()) : nullptr;

        if(!gate->add_boolean_function(mName.toStdString(), mFunction))
                return false;

        return UserAction::exec();
    }

    QString ActionAddBooleanFunction::tagname() const
    {
        return ActionAddBooleanFunctionFactory::sFactory->tagname();
    }

    void ActionAddBooleanFunction::writeToXml(QXmlStreamWriter &xmlOut) const
    {

    }

    void ActionAddBooleanFunction::readFromXml(QXmlStreamReader &xmlIn)
    {

    }

    void ActionAddBooleanFunction::addToHash(QCryptographicHash &cryptoHash) const
    {

    }

}
