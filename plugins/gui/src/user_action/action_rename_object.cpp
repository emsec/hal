#include "gui/user_action/action_rename_object.h"

#include "gui/graph_widget/contexts/graph_context.h"
#include "gui/grouping/grouping_manager_widget.h"
#include "gui/grouping/grouping_table_model.h"
#include "gui/gui_globals.h"
#include <QDebug>

namespace hal
{
    ActionRenameObjectFactory::ActionRenameObjectFactory() : UserActionFactory("RenameObject")
    {
    }

    ActionRenameObjectFactory* ActionRenameObjectFactory::sFactory = new ActionRenameObjectFactory;

    UserAction* ActionRenameObjectFactory::newAction() const
    {
        return new ActionRenameObject;
    }

    QString ActionRenameObject::tagname() const
    {
        return ActionRenameObjectFactory::sFactory->tagname();
    }

    void ActionRenameObject::addToHash(QCryptographicHash& cryptoHash) const
    {
        cryptoHash.addData(mNewName.toUtf8());
    }

    void ActionRenameObject::writeToXml(QXmlStreamWriter& xmlOut) const
    {
        writeParentObjectToXml(xmlOut);
        xmlOut.writeTextElement("name", mNewName);
        if(mObject.type() == UserActionObjectType::Pin || mObject.type() == UserActionObjectType::PinGroup)
            xmlOut.writeTextElement("identifier", mPinOrPingroupIdentifier);
    }

    void ActionRenameObject::readFromXml(QXmlStreamReader& xmlIn)
    {
        while (xmlIn.readNextStartElement())
        {
            readParentObjectFromXml(xmlIn);
            if (xmlIn.name() == "name")
                mNewName = xmlIn.readElementText();
            if (xmlIn.name() == "identifier")
                mPinOrPingroupIdentifier = xmlIn.readElementText();
        }
    }

    bool ActionRenameObject::exec()
    {
        QString oldName;
        Module* mod;
        Gate* gat;
        Net* net;
        GraphContext* ctx;
        switch (mObject.type())
        {
            case UserActionObjectType::Module:
                mod = gNetlist->get_module_by_id(mObject.id());
                if (mod)
                {
                    oldName = QString::fromStdString(mod->get_name());
                    mod->set_name(mNewName.toStdString());
                }
                else
                    return false;
                break;
            case UserActionObjectType::Gate:
                gat = gNetlist->get_gate_by_id(mObject.id());
                if (gat)
                {
                    oldName = QString::fromStdString(gat->get_name());
                    gat->set_name(mNewName.toStdString());
                }
                else
                    return false;
                break;
            case UserActionObjectType::Net:
                net = gNetlist->get_net_by_id(mObject.id());
                if (net)
                {
                    oldName = QString::fromStdString(net->get_name());
                    net->set_name(mNewName.toStdString());
                }
                else
                    return false;
                break;
            case UserActionObjectType::Grouping:
                oldName = gContentManager->getGroupingManagerWidget()->getModel()->renameGrouping(mObject.id(), mNewName);
                break;
            case UserActionObjectType::Context:
                ctx = gGraphContextManager->getContextById(mObject.id());
                if (ctx)
                {
                    oldName = ctx->name();
                    gGraphContextManager->renameGraphContextAction(ctx, mNewName);
                }
                else
                    return false;
                break;
            case UserActionObjectType::Pin:
                mod = gNetlist->get_module_by_id(mObject.id());
                if(mod && mod->get_pin(mPinOrPingroupIdentifier.toStdString()))
                {
                    oldName = mPinOrPingroupIdentifier;
                    mod->set_pin_name(mod->get_pin(mPinOrPingroupIdentifier.toStdString()), mNewName.toStdString());
                }
                else
                    return false;
                break;
            case UserActionObjectType::PinGroup:
                mod = gNetlist->get_module_by_id(mObject.id());
                if(mod && mod->get_pin_group(mPinOrPingroupIdentifier.toStdString()))
                {
                    oldName = mPinOrPingroupIdentifier;
                    mod->set_pin_group_name(mod->get_pin_group(mPinOrPingroupIdentifier.toStdString()), mNewName.toStdString());
                }
                else
                    return false;
                break;
            default:
                return false;
        }
        ActionRenameObject* undo = new ActionRenameObject(oldName);
        undo->setObject(mObject);
        if (mObject.type() == UserActionObjectType::Pin || mObject.type() == UserActionObjectType::PinGroup)
        {
            undo->mPinOrPingroupIdentifier = mNewName;
        }
        mUndoAction = undo;
        return UserAction::exec();
    }
}    // namespace hal
