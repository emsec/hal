#include "gui/user_action/action_set_parameter_value.h"

#include "gui/gui_globals.h"
#include "gui/netlist_relay/netlist_relay.h"
#include "hal_core/netlist/gate.h"

namespace hal
{
    ActionSetParameterValueFactory::ActionSetParameterValueFactory() : UserActionFactory("SetParameterValue")
    {
    }
    ActionSetParameterValueFactory* ActionSetParameterValueFactory::sFactory = new ActionSetParameterValueFactory;

    UserAction* ActionSetParameterValueFactory::newAction() const
    {
        return new ActionSetParameterValue;
    }

    ActionSetParameterValue::ActionSetParameterValue(const QString& parameterName, const QString& value) : mParameterName(parameterName), mValue(value)
    {
    }

    bool ActionSetParameterValue::exec()
    {
        if (mObject.type() != UserActionObjectType::Gate)
        {
            return false;
        }

        // Resolve the gate.
        Gate* gate = gNetlist->get_gate_by_id(mObject.id());
        if (!gate)
        {
            return false;
        }

        // Capture the current value BEFORE mutating, for the undo action:
        Result<std::string> old = gate->get_parameter_value(mParameterName.toStdString());
        if (old.is_error())
        {
            return false;
        }

        // Build the inverse action and store it in mUndoAction:
        auto* undo = new ActionSetParameterValue(mParameterName, QString::fromStdString(old.get()));
        undo->setObject(mObject);
        mUndoAction = undo;

        // Perform the mutation
        Result<Parameter> decl = gate->get_parameter_declaration(mParameterName.toStdString());
        if (decl.is_error())
        {
            return false;
        }

        auto result = gate->set_parameter(decl.get(), mValue.toStdString());
        if (result.is_error())
        {
            return false;
        }

        Q_EMIT gNetlistRelay->gateBooleanFunctionChanged(gate);
        return UserAction::exec();
    }

    QString ActionSetParameterValue::tagname() const
    {
        return ActionSetParameterValueFactory::sFactory->tagname();
    }

    void ActionSetParameterValue::writeToXml(QXmlStreamWriter& xmlOut) const
    {
        xmlOut.writeTextElement("name", mParameterName);
        xmlOut.writeTextElement("value", mValue);
    }

    void ActionSetParameterValue::readFromXml(QXmlStreamReader& xmlIn)
    {
        while (xmlIn.readNextStartElement())
        {
            if (xmlIn.name() == "name")
            {
                mParameterName = xmlIn.readElementText();
            }
            else if (xmlIn.name() == "value")
            {
                mValue = xmlIn.readElementText();
            }
        }
    }

    void ActionSetParameterValue::addToHash(QCryptographicHash& cryptoHash) const
    {
        cryptoHash.addData("name", 4);
        cryptoHash.addData(mParameterName.toUtf8());
        cryptoHash.addData("value", 5);
        cryptoHash.addData(mValue.toUtf8());
    }
}    // namespace hal