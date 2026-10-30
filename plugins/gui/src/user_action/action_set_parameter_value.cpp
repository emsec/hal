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

        const std::string name = mParameterName.toStdString();

        // An empty value removes the instance value, so that the gate falls back to the default of its type.
        // This is only used to undo setting a parameter that had no instance value before.
        if (mValue.isEmpty())
        {
            if (!gate->has_parameter(name))
            {
                return false;
            }
            auto* undo = new ActionSetParameterValue(mParameterName, QString::fromStdString(gate->get_parameter_value(name).get()));
            undo->setObject(mObject);
            mUndoAction = undo;
            gate->delete_parameter(name);
        }
        else
        {
            // The gate may not hold an instance value yet, in that case the declaration is taken from its type
            // and undo removes the instance value again.
            Result<Parameter> decl = gate->has_parameter(name) ? gate->get_parameter_declaration(name) : gate->get_type()->get_parameter(name);
            if (decl.is_error())
            {
                return false;
            }

            // Capture the current value BEFORE mutating, for the undo action:
            const QString oldValue = gate->has_parameter(name) ? QString::fromStdString(gate->get_parameter_value(name).get()) : QString();

            if (gate->set_parameter(decl.get(), mValue.toStdString()).is_error())
            {
                return false;
            }

            auto* undo = new ActionSetParameterValue(mParameterName, oldValue);
            undo->setObject(mObject);
            mUndoAction = undo;
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
            if (xmlIn.name() == QString("name"))
            {
                mParameterName = xmlIn.readElementText();
            }
            else if (xmlIn.name() == QString("value"))
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