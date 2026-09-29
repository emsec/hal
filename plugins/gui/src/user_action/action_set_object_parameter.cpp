#include "gui/user_action/action_set_object_parameter.h"

#include "gui/gui_globals.h"
#include "hal_core/utilities/log.h"

namespace hal
{
    ActionSetObjectParameterFactory::ActionSetObjectParameterFactory() : UserActionFactory("SetObjectParameter")
    {
    }

    ActionSetObjectParameterFactory* ActionSetObjectParameterFactory::sFactory = new ActionSetObjectParameterFactory;

    UserAction* ActionSetObjectParameterFactory::newAction() const
    {
        return new ActionSetObjectParameter;
    }

    ActionSetObjectParameter::ActionSetObjectParameter(QString source, QString name, QString value, QString type, int size)
        : mSource(source), mName(name), mValue(value), mType(type), mSize(size), mRemove(false)
    {
    }

    namespace
    {
        DataContainer* containerOf(const UserActionObject& obj)
        {
            switch (obj.type())
            {
                case UserActionObjectType::Gate:
                    return gNetlist->get_gate_by_id(obj.id());
                case UserActionObjectType::Module:
                    return gNetlist->get_module_by_id(obj.id());
                case UserActionObjectType::Net:
                    return gNetlist->get_net_by_id(obj.id());
                default:
                    return nullptr;
            }
        }

        Result<Parameter> declare(const std::string& name, const std::string& type, u16 size, Parameter::Source source)
        {
            if (!is_valid_enum<Parameter::Type>(type))
            {
                return ERR("unknown parameter type '" + type + "'");
            }
            switch (enum_from_string<Parameter::Type>(type))
            {
                case Parameter::Type::Boolean:
                    return Parameter::Boolean(name, "false", source);
                case Parameter::Type::BitVector:
                    return Parameter::BitVector(name, size, "", source);
                case Parameter::Type::LogicVector:
                    return Parameter::LogicVector(name, size, "", source);
                case Parameter::Type::Integer:
                    return Parameter::Integer(name, "0", source);
                case Parameter::Type::String:
                    return Parameter::String(name, "", source);
                case Parameter::Type::Float:
                    return Parameter::Float(name, "0", source);
                case Parameter::Type::Time:
                    return Parameter::Time(name, "0s", source);
                case Parameter::Type::Enum:
                    return ERR("an enum parameter can only be declared by a gate type");
            }
            return ERR("unknown parameter type '" + type + "'");
        }
    }    // namespace

    bool ActionSetObjectParameter::exec()
    {
        DataContainer* container = containerOf(mObject);
        if (container == nullptr || !is_valid_enum<Parameter::Source>(mSource.toStdString()))
        {
            return false;
        }
        const Parameter::Source source = enum_from_string<Parameter::Source>(mSource.toStdString());
        const std::string name         = mName.toStdString();
        const bool exists              = container->has_parameter(name, source);

        if (mRemove)
        {
            if (!exists)
            {
                return false;
            }
            const Parameter old = container->get_parameter_declaration(name, source).get();
            auto* undo          = new ActionSetObjectParameter(mSource, mName, QString::fromStdString(container->get_parameter_value(name, source).get()), QString::fromStdString(enum_to_string(old.get_type())), old.get_size());
            undo->setObject(mObject);
            mUndoAction = undo;
            container->delete_parameter(name, source);
            return UserAction::exec();
        }

        Parameter declaration;
        if (exists)
        {
            declaration = container->get_parameter_declaration(name, source).get();
            auto* undo  = new ActionSetObjectParameter(mSource, mName, QString::fromStdString(container->get_parameter_value(name, source).get()));
            undo->setObject(mObject);
            mUndoAction = undo;
        }
        else
        {
            auto res = declare(name, mType.toStdString(), static_cast<u16>(mSize), source);
            if (res.is_error())
            {
                log_warning("gui", "cannot create {} '{}': {}", mSource.toStdString(), name, res.get_error().get());
                return false;
            }
            declaration = res.get();
            auto* undo  = new ActionSetObjectParameter(mSource, mName);
            undo->setRemove();
            undo->setObject(mObject);
            mUndoAction = undo;
        }

        if (auto res = container->set_parameter(declaration, mValue.toStdString()); res.is_error())
        {
            log_warning("gui", "cannot set {} '{}' to '{}': {}", mSource.toStdString(), name, mValue.toStdString(), res.get_error().get());
            delete mUndoAction;
            mUndoAction = nullptr;
            return false;
        }
        return UserAction::exec();
    }

    QString ActionSetObjectParameter::tagname() const
    {
        return ActionSetObjectParameterFactory::sFactory->tagname();
    }

    void ActionSetObjectParameter::writeToXml(QXmlStreamWriter& xmlOut) const
    {
        xmlOut.writeTextElement("source", mSource);
        xmlOut.writeTextElement("name", mName);
        xmlOut.writeTextElement("value", mValue);
        if (!mType.isEmpty())
        {
            xmlOut.writeTextElement("type", mType);
            xmlOut.writeTextElement("size", QString::number(mSize));
        }
        if (mRemove)
        {
            xmlOut.writeTextElement("remove", "true");
        }
    }

    void ActionSetObjectParameter::readFromXml(QXmlStreamReader& xmlIn)
    {
        while (xmlIn.readNextStartElement())
        {
            if (xmlIn.name() == QString("source"))
                mSource = xmlIn.readElementText();
            else if (xmlIn.name() == QString("name"))
                mName = xmlIn.readElementText();
            else if (xmlIn.name() == QString("value"))
                mValue = xmlIn.readElementText();
            else if (xmlIn.name() == QString("type"))
                mType = xmlIn.readElementText();
            else if (xmlIn.name() == QString("size"))
                mSize = xmlIn.readElementText().toInt();
            else if (xmlIn.name() == QString("remove"))
                mRemove = xmlIn.readElementText() == "true";
            else
                xmlIn.skipCurrentElement();
        }
    }

    void ActionSetObjectParameter::addToHash(QCryptographicHash& cryptoHash) const
    {
        cryptoHash.addData("source", 6);
        cryptoHash.addData(mSource.toUtf8());
        cryptoHash.addData("name", 4);
        cryptoHash.addData(mName.toUtf8());
        cryptoHash.addData("value", 5);
        cryptoHash.addData(mValue.toUtf8());
        cryptoHash.addData("type", 4);
        cryptoHash.addData(mType.toUtf8());
        cryptoHash.addData("size", 4);
        cryptoHash.addData(QString::number(mSize).toUtf8());
        cryptoHash.addData("remove", 6);
        cryptoHash.addData(mRemove ? "1" : "0", 1);
    }

    void ActionSetObjectParameter::setRemove(bool remove)
    {
        mRemove = remove;
    }
}    // namespace hal
